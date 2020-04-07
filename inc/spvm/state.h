#ifndef __SPIRV_VM_STATE_H__
#define __SPIRV_VM_STATE_H__

#include <spvm/program.h>
#include <spvm/result.h>

typedef struct {
	spvm_context_t context;
	spvm_program_t owner;
	spvm_source code_current; // current position in the code
	spvm_result* results;

	spvm_byte did_jump;
	spvm_byte discarded;

	spvm_result_t current_function;

	spvm_word current_parameter;

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

spvm_result_t spvm_state_get_type_info(spvm_result_t res_list, spvm_result_t res);

spvm_state_t spvm_state_create(spvm_program_t prog);
void spvm_state_call_function(spvm_state_t state);
void spvm_state_prepare(spvm_state_t state, spvm_result_t code);
void spvm_state_step_opcode(spvm_state_t state);
void spvm_state_step_into(spvm_state_t state);
void spvm_state_jump_to(spvm_state_t state, spvm_word line);
spvm_result_t spvm_state_get_result(spvm_state_t state, const spvm_string str);
spvm_result_t spvm_state_get_local_result(spvm_state_t state, spvm_result_t fn, const spvm_string str);
spvm_member_t spvm_state_get_object_member(spvm_state_t state, spvm_result_t var, const spvm_string member_name);
void spvm_state_push_function_stack(spvm_state_t state, spvm_result_t func, spvm_word func_res_id);
void spvm_state_pop_function_stack(spvm_state_t state);
void spvm_state_delete(spvm_state_t state);

#endif // __SPIRV_VM_STATE_H__