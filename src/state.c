#include <spvm/state.h>
#include <spvm/value.h>


spvm_state_t spvm_state_create(spvm_program_t prog)
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

	return state;
}
spvm_result_t spvm_state_get_type_info(spvm_result_t res_list, spvm_result_t res)
{
	if (res->value_type == spvm_value_type_pointer)
		return spvm_state_get_type_info(res_list, &res_list[res->pointer]);
	return res;
}

void spvm_state_prepare(spvm_state_t state, spvm_result_t code)
{
	state->code_current = code->source_location;
	state->current_function = code;

	if (state->function_stack_count == 0) {
		state->function_stack_count = 10;
		state->function_stack = (spvm_source*)malloc(state->function_stack_count * sizeof(spvm_source));
		state->function_stack_info = (spvm_result_t*)malloc(state->function_stack_count * sizeof(spvm_result_t));
		state->function_stack_returns = (spvm_word*)malloc(state->function_stack_count * sizeof(spvm_word));
	}

	state->function_stack_current = 0;
	state->function_stack[0] = code->source_location;
	state->function_stack_info[0] = code;
	state->did_jump = 0;
	state->discarded = 0;
}

void spvm_state_step_opcode(spvm_state_t state)
{
	spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
	spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
	SpvOp opcode = (opcode_data & SpvOpCodeMask);
	spvm_source cur_code = state->code_current;

	if (opcode <= SPVM_OPCODE_TABLE_LENGTH && state->context->opcode_execute[opcode] != 0)
		state->context->opcode_execute[opcode](word_count, state);

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
	while (line != state->current_line && state->code_current != 0)
		spvm_state_step_opcode(state);
}
void spvm_state_call_function(spvm_result_t code, spvm_state_t state)
{
	spvm_state_prepare(state, code);

	spvm_source cur_code = state->code_current;

	while (state->code_current)
	{
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		SpvOp opcode = (opcode_data & SpvOpCodeMask);
		cur_code = state->code_current;

		if (opcode <= SPVM_OPCODE_TABLE_LENGTH && state->context->opcode_execute[opcode] != 0)
			state->context->opcode_execute[opcode](word_count, state);

		if (!state->did_jump)
			state->code_current = (cur_code + word_count);
		else state->did_jump = 0;
	}
}

spvm_result_t spvm_state_get_local_result(spvm_state_t state, spvm_result_t fn, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, str) == 0 &&
			state->results[i].owner == fn)
				return &state->results[i];

	return NULL;
}
spvm_result_t spvm_state_get_result(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, str) == 0)
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
	}

	state->function_stack[state->function_stack_current] = func->source_location;
	state->function_stack_info[state->function_stack_current] = func;
	state->function_stack_returns[state->function_stack_current] = func_res_id;

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

		// name
		if (res->name != NULL)
			free(res->name);

		// member names
		for (spvm_word j = 0; j < res->member_name_count; j++)
			free(res->member_name[j]);
		if (res->member_name_count)
			free(res->member_name);

		// member/parameter types
		if (res->value_type == spvm_value_type_struct || res->type == spvm_result_type_function_type)
			free(res->params);

		// decorations
		if (res->decoration_count)
			free(res->decorations);

		// constants
		if (res->type == spvm_result_type_constant || res->type == spvm_result_type_variable)
			spvm_member_free(res->members, res->member_count);
	}

	if (state->function_stack_count) {
		free(state->function_stack);
		free(state->function_stack_info);
		free(state->function_stack_returns);
	}

	free(state->results);
	free(state);
}