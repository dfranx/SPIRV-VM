#ifndef __SPIRV_VM_STATE_H__
#define __SPIRV_VM_STATE_H__

#include <spvm/program.h>
#include <spvm/result.h>

typedef struct {
	spvm_program_t owner;
	spvm_source code_current; // current position in the code
	spvm_result* results;
	char function_reached_label;
	char parsing;

	spvm_byte did_jump;

	spvm_result_t current_function;

	spvm_word function_stack_current;
	spvm_word function_stack_count;
	spvm_source* function_stack;
	spvm_result_t* function_stack_info;
	spvm_word return_id;
	spvm_word* function_stack_returns;

	// debug information
	spvm_string current_file;
	spvm_word current_line;
	spvm_word current_column;
} spvm_state;
typedef spvm_state* spvm_state_t;

spvm_word spvm_state_get_value_count(spvm_result_t res_list, spvm_result_t res);

spvm_state_t spvm_state_create(spvm_program_t prog);
void spvm_state_call_function(spvm_result_t code, spvm_state_t state);
spvm_result_t spvm_state_get_result(spvm_state_t state, const spvm_string str);
void spvm_state_set_value_f(spvm_state_t state, const spvm_string name, float* f);
void spvm_state_set_value_i(spvm_state_t state, const spvm_string name, int* f);
void spvm_state_push_function_stack(spvm_state_t state, spvm_result_t func, spvm_word func_res_id);
void spvm_state_pop_function_stack(spvm_state_t state);

#endif // __SPIRV_VM_STATE_H__