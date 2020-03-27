#ifndef __SPIRV_VM_STATE_H__
#define __SPIRV_VM_STATE_H__

#include <spvm/program.h>
#include <spvm/result.h>

typedef struct {
	spvm_program_t owner;
	spvm_source code_current; // current position in the code
	spvm_result* results;
	char function_parsing; // reached OpFunction
	char function_called;
} spvm_state;
typedef spvm_state* spvm_state_t;

spvm_state_t spvm_state_create(spvm_program_t prog);
spvm_word spvm_value_get_count(spvm_result_t res_list, spvm_result_t res);
spvm_source spvm_state_get_function(spvm_state_t state, const spvm_string str);
void spvm_state_call_function(spvm_source code, spvm_state_t state);
spvm_result_t spvm_state_get_result(spvm_state_t state, const spvm_string str);
void spvm_state_set_value(spvm_state_t state, const spvm_string name, float m11, float m12);

#endif // __SPIRV_VM_STATE_H__