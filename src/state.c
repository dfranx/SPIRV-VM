#include <spvm/state.h>
#include <spvm/value.h>


spvm_state_t spvm_state_create(spvm_program_t prog)
{
	spvm_state_t state = (spvm_state_t)calloc(1, sizeof(spvm_state));

	state->owner = prog;
	state->code_current = prog->code;
	state->results = (spvm_result*)calloc(prog->bound + 1, sizeof(spvm_result));
	state->function_reached_label = 0;
	state->current_file = NULL;
	state->current_line = -1;
	state->current_column = -1;
	state->return_id = -1;
	state->function_stack_count = 0;
	state->function_stack_current = 0;
	state->function_stack = NULL;

	state->parsing = 1;

	for (size_t i = 0; i < prog->code_length; i++) {
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		spvm_word opcode = (opcode_data & SpvOpCodeMask);
		spvm_source cur_code = state->code_current;

		if (opcode <= 256 && prog->opcode_table[opcode] != 0 &&
			(!state->function_reached_label || (state->function_reached_label && opcode == SpvOpFunctionEnd) || opcode == SpvOpLabel))
		{
			prog->opcode_table[opcode](word_count, state);
		}

		state->code_current = (cur_code + word_count);
		i += word_count;
	}

	state->parsing = 0;

	return state;
}
spvm_word spvm_state_get_value_count(spvm_result_t res_list, spvm_result_t res)
{
	if (res->value_type == spvm_value_type_pointer)
		return spvm_state_get_value_count(res_list, &res_list[res->pointer]);
	else if (res->value_type == spvm_value_type_vector)
		return res->value_count;

	return 1;
}
void spvm_state_call_function(spvm_result_t code, spvm_state_t state)
{
	state->code_current = code->source_location;
	state->current_function = code;
	state->function_stack_count = 1;
	state->function_stack_current = 0;
	state->function_stack = (spvm_source*)realloc(state->function_stack, state->function_stack_count * sizeof(spvm_source));
	state->function_stack_info = (spvm_result_t*)realloc(state->function_stack_info, state->function_stack_count * sizeof(spvm_result_t));
	state->function_stack[0] = code->source_location;
	state->function_stack_info[0] = code;
	state->did_jump = 0;
	state->discarded = 0;

	spvm_program_t prog = state->owner;

	while (state->code_current)
	{
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		SpvOp opcode = (opcode_data & SpvOpCodeMask);
		spvm_source cur_code = state->code_current;

		if (opcode <= 410 && prog->opcode_table[opcode] != 0)
			prog->opcode_table[opcode](word_count, state);

		if (!state->did_jump)
			state->code_current = (cur_code + word_count);
		else state->did_jump = 0;
	}
}
spvm_result_t spvm_state_get_result(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, str) == 0)
			return &state->results[i];

	return NULL;
}

void spvm_state_set_value_f(spvm_state_t state, const spvm_string name, float* f)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, name) == 0) {
			spvm_value* val = state->results[i].value;
			for (spvm_word j = 0; j < state->results[i].value_count; j++)
				val[j].f = f[j];
		}
}
void spvm_state_set_value_i(spvm_state_t state, const spvm_string name, int* d)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, name) == 0) {
			spvm_value* val = state->results[i].value;
			for (spvm_word j = 0; j < state->results[i].value_count; j++)
				val[j].i = d[j];
		}
}


void spvm_state_push_function_stack(spvm_state_t state, spvm_result_t func, spvm_word func_res_id)
{
	state->function_stack[state->function_stack_current] = state->code_current;

	state->function_stack_current = state->function_stack_count;
	state->function_stack_count++;
	state->function_stack = (spvm_source*)realloc(state->function_stack, state->function_stack_count * sizeof(spvm_source));
	state->function_stack_info = (spvm_result_t*)realloc(state->function_stack_info, state->function_stack_count * sizeof(spvm_result_t));
	state->function_stack_returns = (spvm_word*)realloc(state->function_stack_returns, state->function_stack_count * sizeof(spvm_word));
	
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
		memcpy(state->results[store_id].value, state->results[state->return_id].value, state->results[store_id].value_count * sizeof(spvm_word));
	}

	state->function_stack_count--;
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