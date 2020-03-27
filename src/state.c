#include <spvm/state.h>
#include <spvm/value.h>


spvm_state_t spvm_state_create(spvm_program_t prog)
{
	spvm_state_t state = (spvm_state_t)malloc(sizeof(spvm_state));

	state->owner = prog;
	state->code_current = prog->code;
	state->results = (spvm_result*)calloc(prog->bound + 1, sizeof(spvm_result));
	state->function_called = 0;
	state->function_parsing = 0;

	for (size_t i = 0; i < prog->code_length; i++) {
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		spvm_word opcode = (opcode_data & SpvOpCodeMask);
		spvm_source cur_code = state->code_current;

		if (opcode <= 256 && prog->opcode_table[opcode] != 0 &&
			(!state->function_parsing || (state->function_parsing && opcode == SpvOpFunctionEnd)))
		{
			prog->opcode_table[opcode](word_count, state);
		}

		state->code_current = (cur_code + word_count);
		i += word_count;
	}

	return state;
}
spvm_word spvm_value_get_count(spvm_result_t res_list, spvm_result_t res)
{
	if (res->value_type == spvm_value_type_pointer)
		return spvm_value_get_count(res_list, &res_list[res->pointer]);
	else if (res->value_type == spvm_value_type_vector)
		return res->vector_comp_count;

	return 1;
}
spvm_source spvm_state_get_function(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->entry_point_count; i++)
		if (strcmp(state->owner->entry_points[i].name, str) == 0)
			return state->results[state->owner->entry_points[i].id].function_start;

	return NULL;
}
void spvm_state_call_function(spvm_source code, spvm_state_t state)
{
	state->code_current = code;
	state->function_called = 1;

	spvm_program_t prog = state->owner;

	while (state->function_called)
	{
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		spvm_word opcode = (opcode_data & SpvOpCodeMask);
		spvm_source cur_code = state->code_current;

		if (opcode <= 256 && prog->opcode_table[opcode] != 0)
			prog->opcode_table[opcode](word_count, state);

		state->code_current = (cur_code + word_count);
	}
}
spvm_result_t spvm_state_get_result(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, str) == 0)
			return &state->results[i];

	return NULL;
}

void spvm_state_set_value(spvm_state_t state, const spvm_string name, float m11, float m12)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].name != NULL && strcmp(state->results[i].name, name) == 0) {
			spvm_value* val = state->results[i].value;
			val[0].f = m11;
			val[1].f = m12;
		}
}