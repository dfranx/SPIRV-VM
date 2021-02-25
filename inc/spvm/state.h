#ifndef __SPIRV_VM_STATE_H__
#define __SPIRV_VM_STATE_H__

#include <spvm/program.h>
#include <spvm/result.h>
#include <spvm/analyzer.h>

typedef struct spvm_state {
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
	spvm_word* function_stack_cfg;

	void(*emit_vertex)(struct spvm_state*, spvm_word);
	void(*end_primitive)(struct spvm_state*, spvm_word);

	void(*control_barrier)(struct spvm_state*, spvm_word, spvm_word, spvm_word);

	float frag_coord[4];

	// derivative group
	spvm_byte _derivative_is_group_member;
	spvm_byte derivative_used;
	float derivative_buffer_x[16];
	float derivative_buffer_y[16];
	struct spvm_state* derivative_group_x; // right
	struct spvm_state* derivative_group_y; // bottom
	struct spvm_state* derivative_group_d; // bottom right / diagonal

	// debug information
	spvm_string current_file;
	spvm_word current_line;
	spvm_word current_column;
	spvm_word instruction_count;

	// pointer to analyzer
	spvm_analyzer_t analyzer;

	void* user_data;
} spvm_state;
typedef spvm_state* spvm_state_t;

spvm_result_t spvm_state_get_type_info(spvm_result_t res_list, spvm_result_t res);

spvm_state_t spvm_state_create(spvm_program_t prog);
spvm_state_t _spvm_state_create_base(spvm_program_t prog, spvm_byte force_derv, spvm_byte is_derv_member);
void spvm_state_set_extension(spvm_state_t state, const spvm_string name, spvm_ext_opcode_func* ext);
void spvm_state_call_function(spvm_state_t state);
void spvm_state_prepare(spvm_state_t state, spvm_word fnLocation);
void spvm_state_copy_uniforms(spvm_state_t dst, spvm_state_t src);
void spvm_state_set_frag_coord(spvm_state_t state, float x, float y, float z, float w);
void spvm_state_ddx(spvm_state_t state, spvm_word id);
void spvm_state_ddy(spvm_state_t state, spvm_word id);
void spvm_state_group_sync(spvm_state_t state);
void spvm_state_group_step(spvm_state_t state);
void spvm_state_step_opcode(spvm_state_t state);
void spvm_state_step_into(spvm_state_t state);
void spvm_state_jump_to(spvm_state_t state, spvm_word line);
void spvm_state_jump_to_instruction(spvm_state_t state, spvm_word instruction_count);
spvm_word spvm_state_get_result_location(spvm_state_t state, const spvm_string str);
spvm_member_t spvm_state_get_builtin(spvm_state_t state, SpvBuiltIn decor, spvm_word* mem_count);
spvm_result_t spvm_state_get_result(spvm_state_t state, const spvm_string str);
spvm_result_t spvm_state_get_result_with_value(spvm_state_t state, const spvm_string str);
spvm_result_t spvm_state_get_local_result(spvm_state_t state, spvm_result_t fn, const spvm_string str);
spvm_member_t spvm_state_get_object_member(spvm_state_t state, spvm_result_t var, const spvm_string member_name);
void spvm_state_push_function_stack(spvm_state_t state, spvm_result_t func, spvm_word func_res_id);
void spvm_state_pop_function_stack(spvm_state_t state);
void spvm_state_delete(spvm_state_t state);

#endif // __SPIRV_VM_STATE_H__