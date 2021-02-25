#include <spvm/state.h>
#include <spvm/value.h>
#include <string.h>

spvm_state_t spvm_state_create(spvm_program_t prog)
{
	return _spvm_state_create_base(prog, 0, 0);
}
spvm_state_t _spvm_state_create_base(spvm_program_t prog, spvm_byte force_derv, spvm_byte is_derv_member)
{
	spvm_state_t state = (spvm_state_t)calloc(1, sizeof(spvm_state));

	state->owner = prog;
	state->code_current = prog->code;
	state->results = (spvm_result*)calloc(prog->bound + 1, sizeof(spvm_result));
	state->current_file = NULL;
	state->current_line = -1;
	state->current_column = -1;
	state->current_parameter = 0;
	state->return_id = -1;
	state->function_stack_count = 0;
	state->function_stack_current = 0;
	state->function_stack = NULL;
	state->context = prog->context;
	state->analyzer = NULL;
	state->_derivative_is_group_member = is_derv_member;

	for (size_t i = 0; i < prog->bound; i++)
		state->results[i].storage_class = SpvStorageClassMax;

	for (size_t i = 0; i < prog->code_length; i++) {
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		spvm_word opcode = (opcode_data & SpvOpCodeMask);
		spvm_source cur_code = state->code_current;

		if (opcode <= SPVM_OPCODE_TABLE_LENGTH && state->context->opcode_setup[opcode] != 0)
			state->context->opcode_setup[opcode](word_count, state);

		state->code_current = (cur_code + word_count);
		i += word_count;
	}

	state->derivative_used = force_derv;

	if (!is_derv_member && state->derivative_used) {
		state->derivative_group_x = _spvm_state_create_base(prog, force_derv, 1);
		state->derivative_group_y = _spvm_state_create_base(prog, force_derv, 1);
		state->derivative_group_d = _spvm_state_create_base(prog, force_derv, 1);

		// setup pointers
		// x
		state->derivative_group_x->derivative_group_x = state;
		state->derivative_group_x->derivative_group_y = state->derivative_group_d;
		state->derivative_group_x->derivative_group_d = state->derivative_group_y;

		// y
		state->derivative_group_y->derivative_group_x = state->derivative_group_d;
		state->derivative_group_y->derivative_group_y = state;
		state->derivative_group_y->derivative_group_d = state->derivative_group_x;

		// d
		state->derivative_group_d->derivative_group_x = state->derivative_group_y;
		state->derivative_group_d->derivative_group_y = state->derivative_group_x;
		state->derivative_group_d->derivative_group_d = state;
	}

	return state;
}
spvm_result_t spvm_state_get_type_info(spvm_result_t res_list, spvm_result_t res)
{
	if (res->value_type == spvm_value_type_pointer)
		return spvm_state_get_type_info(res_list, &res_list[res->pointer]);
	return res;
}
void spvm_state_set_extension(spvm_state_t state, const spvm_string name, spvm_ext_opcode_func* ext)
{
	spvm_result_t ptr = spvm_state_get_result(state, name);
	if (ptr) ptr->extension = ext;
	if (!state->_derivative_is_group_member) {
		if (state->derivative_group_x) {
			ptr = spvm_state_get_result(state->derivative_group_x, name);
			if (ptr) ptr->extension = ext;
		}
		if (state->derivative_group_y) {
			ptr = spvm_state_get_result(state->derivative_group_y, name);
			if (ptr) ptr->extension = ext;
		}
		if (state->derivative_group_d) {
			ptr = spvm_state_get_result(state->derivative_group_d, name);
			if (ptr) ptr->extension = ext;
		}
	}
}

void spvm_state_prepare(spvm_state_t state, spvm_word fnLocation)
{
	state->code_current = state->results[fnLocation].source_location;
	state->current_function = &state->results[fnLocation];

	if (state->function_stack_count == 0) {
		state->function_stack_count = 10;
		state->function_stack = (spvm_source*)malloc(state->function_stack_count * sizeof(spvm_source));
		state->function_stack_info = (spvm_result_t*)malloc(state->function_stack_count * sizeof(spvm_result_t));
		state->function_stack_returns = (spvm_word*)malloc(state->function_stack_count * sizeof(spvm_word));
		state->function_stack_cfg = (spvm_word*)malloc(state->function_stack_count * sizeof(spvm_word));
	}

	state->function_stack_current = 0;
	state->function_stack[0] = state->code_current;
	state->function_stack_info[0] = state->current_function;
	state->function_stack_cfg[0] = 0;
	state->did_jump = 0;
	state->discarded = 0;
	state->instruction_count = 0;
	
	if (!state->_derivative_is_group_member) {
		if (state->derivative_group_x) spvm_state_prepare(state->derivative_group_x, fnLocation);
		if (state->derivative_group_y) spvm_state_prepare(state->derivative_group_y, fnLocation);
		if (state->derivative_group_d) spvm_state_prepare(state->derivative_group_d, fnLocation);
	}
}
void spvm_state_set_frag_coord(spvm_state_t state, float x, float y, float z, float w)
{
	state->frag_coord[0] = x;
	state->frag_coord[1] = y;
	state->frag_coord[2] = z;
	state->frag_coord[3] = w;

	spvm_word fc_count = 0;
	spvm_member_t fc = spvm_state_get_builtin(state, SpvBuiltInFragCoord, &fc_count);

	if (fc) {
		for (spvm_word i = 0; i < fc_count; i++)
			fc[i].value.f = state->frag_coord[i];

		if (state->derivative_used && !state->_derivative_is_group_member) {
			spvm_byte is_odd_x = ((int)x) % 2 != 0;
			spvm_byte is_odd_y = ((int)y) % 2 != 0;
			float mod_x = 1, mod_y = 1;

			// setup frag_coord
			if (is_odd_x) mod_x = -1;
			if (is_odd_y) mod_y = -1;

			spvm_state_set_frag_coord(state->derivative_group_x, x+mod_x, y, z, w);
			spvm_state_set_frag_coord(state->derivative_group_y, x, y+mod_y, z, w);
			spvm_state_set_frag_coord(state->derivative_group_d, x+mod_x, y+mod_y, z, w);
		}
	}
}
void spvm_state_copy_uniforms(spvm_state_t dst, spvm_state_t src)
{
	for (spvm_word i = 0; i < dst->owner->bound; i++) {
		const spvm_string name1 = dst->results[i].name;
		if (name1 == NULL)
			continue;

		spvm_word ptr = dst->results[i].pointer;
		if (dst->results[ptr].storage_class != SpvStorageClassUniform &&
			dst->results[ptr].storage_class != SpvStorageClassUniformConstant)
			continue;
		
		for (spvm_word j = 0; j < src->owner->bound; j++) {
			const spvm_string name2 = src->results[j].name;
			ptr = src->results[j].pointer;

			if (name2 != NULL) {
				ptr = src->results[i].pointer;
				if (src->results[ptr].storage_class != SpvStorageClassUniform &&
					src->results[ptr].storage_class != SpvStorageClassUniformConstant)
					continue;

				if (strcmp(name1, name2) == 0)
					spvm_member_memcpy(dst->results[i].members, src->results[j].members, dst->results[i].member_count);
			}
		}
	}
}
void spvm_state_group_sync(spvm_state_t state)
{
	if (state->derivative_group_x) spvm_state_jump_to_instruction(state->derivative_group_x, state->instruction_count);
	if (state->derivative_group_y) spvm_state_jump_to_instruction(state->derivative_group_y, state->instruction_count);
	if (state->derivative_group_d) spvm_state_jump_to_instruction(state->derivative_group_d, state->instruction_count);
}
void spvm_state_group_step(spvm_state_t state)
{
	if (state->derivative_group_x) spvm_state_step_opcode(state->derivative_group_x);
	if (state->derivative_group_y) spvm_state_step_opcode(state->derivative_group_y);
	if (state->derivative_group_d) spvm_state_step_opcode(state->derivative_group_d);
}

void spvm_state_step_opcode(spvm_state_t state)
{
	// read data
	spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
	spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
	SpvOp opcode = (opcode_data & SpvOpCodeMask);
	spvm_source cur_code = state->code_current;

	if (opcode <= SPVM_OPCODE_TABLE_LENGTH && state->context->opcode_execute[opcode] != 0) {
		state->context->opcode_execute[opcode](word_count, state);
		if (opcode != SpvOpLine && opcode != SpvOpNoLine)
			state->instruction_count++;
	}

	if (!state->did_jump)
		state->code_current = (cur_code + word_count);
	else state->did_jump = 0;
}
void spvm_state_step_into(spvm_state_t state)
{
	spvm_word ln = state->current_line;
	while (ln == state->current_line && state->code_current)
		spvm_state_step_opcode(state);
}
void spvm_state_jump_to(spvm_state_t state, spvm_word line)
{
	while (line != state->current_line && state->code_current)
		spvm_state_step_opcode(state);
}
void spvm_state_jump_to_instruction(spvm_state_t state, spvm_word inst)
{
	while (state->instruction_count < inst && state->code_current)
		spvm_state_step_opcode(state);
}
void spvm_state_call_function(spvm_state_t state)
{
	spvm_source cur_code = state->code_current;

	while (state->code_current)
	{ 
		// read data
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		SpvOp opcode = (opcode_data & SpvOpCodeMask);
		cur_code = state->code_current;

		if (opcode <= SPVM_OPCODE_TABLE_LENGTH && state->context->opcode_execute[opcode] != 0) {
			state->context->opcode_execute[opcode](word_count, state);
			if (opcode != SpvOpLine && opcode != SpvOpNoLine)
				state->instruction_count++;
		}

		if (!state->did_jump)
			state->code_current = (cur_code + word_count);
		else state->did_jump = 0;
	}
}
void spvm_state_ddx(spvm_state_t state, spvm_word id)
{
	if (!state->derivative_group_x) return;

	if (!state->_derivative_is_group_member)
		spvm_state_group_sync(state); // first sync the group

	spvm_byte is_even = ((int)state->frag_coord[0]) % 2 == 0;
	int index = 0;

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		if (state->results[id].members[i].member_count == 0) {
			if (is_even)
				state->derivative_buffer_x[index] = state->derivative_group_x->results[id].members[i].value.f - state->results[id].members[i].value.f;
			else
				state->derivative_buffer_x[index] = state->results[id].members[i].value.f - state->derivative_group_x->results[id].members[i].value.f;
			index++;
		}
		else {
			if (is_even) {
				for (spvm_word j = 0; j < state->results[id].members[i].member_count; j++)
					state->derivative_buffer_x[index] = state->derivative_group_x->results[id].members[i].members[j].value.f - state->results[id].members[i].members[j].value.f;
			} else {
				for (spvm_word j = 0; j < state->results[id].members[i].member_count; j++)
					state->derivative_buffer_x[index] = state->results[id].members[i].members[j].value.f - state->derivative_group_x->results[id].members[i].members[j].value.f;
			}
			index += state->results[id].members[i].member_count;
		}
	}
}
void spvm_state_ddy(spvm_state_t state, spvm_word id)
{
	if (!state->derivative_group_y) return;

	if (!state->_derivative_is_group_member)
		spvm_state_group_sync(state); // first sync the group

	spvm_byte is_even = ((int)state->frag_coord[1]) % 2 == 0;
	int index = 0;

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		if (state->results[id].members[i].member_count == 0) {
			if (is_even)
				state->derivative_buffer_y[index] = state->derivative_group_y->results[id].members[i].value.f - state->results[id].members[i].value.f;
			else
				state->derivative_buffer_y[index] = state->results[id].members[i].value.f - state->derivative_group_y->results[id].members[i].value.f;
			index++;
		}
		else {
			if (is_even) {
				for (spvm_word j = 0; j < state->results[id].members[i].member_count; j++)
					state->derivative_buffer_y[index] = state->derivative_group_y->results[id].members[i].members[j].value.f - state->results[id].members[i].members[j].value.f;
			}
			else {
				for (spvm_word j = 0; j < state->results[id].members[i].member_count; j++)
					state->derivative_buffer_y[index] = state->results[id].members[i].members[j].value.f - state->derivative_group_y->results[id].members[i].members[j].value.f;
			}
			index += state->results[id].members[i].member_count;
		}
	}
}

spvm_member_t spvm_state_get_builtin(spvm_state_t state, SpvBuiltIn builtin, spvm_word* mem_count)
{
	for (spvm_word i = 0; i < state->owner->bound; i++) {
		spvm_result_t slot = &state->results[i];
		spvm_result_t type = NULL;

		if (slot->pointer)
			type = spvm_state_get_type_info(state->results, &state->results[slot->pointer]);

		if (type == NULL || slot->member_count == 0 || slot->members == NULL)
			continue;

		int found = 0;
		int index = -1;
		for (spvm_word j = 0; j < slot->decoration_count; j++)
			if (slot->decorations[j].type == SpvDecorationBuiltIn) {
				if (slot->decorations[j].literal1 == builtin) {
					found = 1;
					break;
				}
			}

		for (spvm_word j = 0; j < type->decoration_count; j++)
			if (type->decorations[j].type == SpvDecorationBuiltIn) {
				if (type->decorations[j].literal1 == builtin) {
					found = 1;
					index = type->decorations[j].index;
					break;
				}
			}

		if (found) {
			if (index == -1) {
				*mem_count = slot->member_count;
				return slot->members;
			} else {
				*mem_count = slot->members[index].member_count;
				return slot->members[index].members;
			}
		}
	}

	return NULL;
}
spvm_result_t spvm_state_get_local_result(spvm_state_t state, spvm_result_t fn, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, str) == 0 &&
			state->results[i].owner == fn)
				return &state->results[i];

	return NULL;
}
spvm_word spvm_state_get_result_location(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, str) == 0 && state->results[i].owner == NULL)
			return i;

	return 0;
}
spvm_result_t spvm_state_get_result(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, str) == 0 && state->results[i].owner == NULL)
			return &state->results[i];

	return NULL;
}
spvm_result_t spvm_state_get_result_with_value(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, str) == 0 && state->results[i].owner == NULL && state->results[i].members != NULL)
			return &state->results[i];

	return NULL;
}
spvm_member_t spvm_state_get_object_member(spvm_state_t state, spvm_result_t var, const spvm_string member_name)
{
	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[var->pointer]);
	spvm_word index = -1;

	for (spvm_word i = 0; i < type_info->member_name_count; i++)
		if (strcmp(type_info->member_name[i], member_name) == 0) {
			index = i;
			break;
		}

	if (index == -1)
		return NULL;
	return &var->members[index];
}

void spvm_state_push_function_stack(spvm_state_t state, spvm_result_t func, spvm_word func_res_id)
{
	state->function_stack[state->function_stack_current] = state->code_current;

	state->function_stack_current++;

	if (state->function_stack_current >= state->function_stack_count) {
		state->function_stack_count += 10;
		state->function_stack = (spvm_source*)realloc(state->function_stack, state->function_stack_count * sizeof(spvm_source));
		state->function_stack_info = (spvm_result_t*)realloc(state->function_stack_info, state->function_stack_count * sizeof(spvm_result_t));
		state->function_stack_returns = (spvm_word*)realloc(state->function_stack_returns, state->function_stack_count * sizeof(spvm_word));
		state->function_stack_cfg = (spvm_word*)realloc(state->function_stack_cfg, state->function_stack_count * sizeof(spvm_word));
	}

	state->function_stack[state->function_stack_current] = func->source_location;
	state->function_stack_info[state->function_stack_current] = func;
	state->function_stack_returns[state->function_stack_current] = func_res_id;
	state->function_stack_cfg[state->function_stack_current] = 0;

	state->code_current = func->source_location;
	state->current_function = func;

	state->did_jump = 1;
}
void spvm_state_pop_function_stack(spvm_state_t state)
{
	if (state->return_id >= 0) {
		spvm_word store_id = state->function_stack_returns[state->function_stack_current];
		spvm_member_memcpy(state->results[store_id].members, state->results[state->return_id].members, state->results[store_id].member_count);
	}

	state->function_stack_current--;

	if (state->function_stack_current < 0) {
		state->code_current = NULL;
		state->current_function = NULL;
	} else {
		state->code_current = state->function_stack[state->function_stack_current];
		state->current_function = state->function_stack_info[state->function_stack_current];
	}

	state->did_jump = 1;
}
void spvm_state_delete(spvm_state_t state)
{
	for (spvm_word i = 0; i < state->owner->bound; i++) {
		spvm_result_t res = &state->results[i];
		spvm_result_delete(res);
	}

	if (state->function_stack_count) {
		free(state->function_stack);
		free(state->function_stack_info);
		free(state->function_stack_returns);
		free(state->function_stack_cfg);
	}

	free(state->results);
	free(state);
}