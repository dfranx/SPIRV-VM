#ifndef __SPIRV_VM_PROGRAM_H__
#define __SPIRV_VM_PROGRAM_H__

#include <spvm/types.h>
#include <spvm/spirv.h>
#include <spvm/context.h>

typedef struct
{
	SpvExecutionModel exec_model;
	spvm_word id;
	spvm_string name;
	spvm_word globals_count;
	spvm_word* globals; // interface <id>
} spvm_entry_point;

typedef struct {
	spvm_context_t context;

	spvm_byte major_version;
	spvm_byte minor_version;

	spvm_word generator_id;
	spvm_word generator_version;

	spvm_word bound;

	size_t code_length;
	spvm_source code;

	SpvSourceLanguage language;
	spvm_word language_version;

	spvm_word extension_count;
	spvm_string* extensions;

	spvm_word import_count;
	spvm_string* imports;

	spvm_word capability_count;
	SpvCapability* capabilities;

	SpvAddressingModel addressing;
	SpvMemoryModel memory_model;

	spvm_word entry_point_count;
	spvm_entry_point* entry_points;

	spvm_word local_size_x;
	spvm_word local_size_y;
	spvm_word local_size_z;

	spvm_word geometry_invocations;
	spvm_word geometry_output_count;
	spvm_word geometry_input;
	spvm_word geometry_output;

	void(*allocate_workgroup_memory)(struct spvm_state*, spvm_word, spvm_word);
	void(*write_workgroup_memory)(struct spvm_state*, spvm_word, spvm_word);
	void(*atomic_operation)(spvm_word, spvm_word, struct spvm_state*);

	void* user_data;
} spvm_program;
typedef spvm_program* spvm_program_t;

spvm_program_t spvm_program_create(spvm_context_t ctx, spvm_source spv, size_t spv_length);
spvm_string spvm_program_add_extension(spvm_program_t prog, spvm_word length);
spvm_entry_point* spvm_program_create_entry_point(spvm_program_t prog);
void spvm_program_add_capability(spvm_program_t prog, SpvCapability cap);
void spvm_program_delete(spvm_program_t prog);

#endif // __SPIRV_VM_PROGRAM_H__