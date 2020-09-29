#ifndef __SPIRV_VM_RESULT_H__
#define __SPIRV_VM_RESULT_H__

#include <spvm/types.h>
#include <spvm/value.h>
#include <spvm/spirv.h>
#include <spvm/opcode.h>

enum spvm_result_type {
	spvm_result_type_none,
	spvm_result_type_string,
	spvm_result_type_extension,
	spvm_result_type_function_type,
	spvm_result_type_type,
	spvm_result_type_variable,
	spvm_result_type_constant,
	spvm_result_type_function,
	spvm_result_type_access_chain,
	spvm_result_type_function_parameter,
	spvm_result_type_label
};
typedef struct {
	SpvDim dim;
	spvm_byte depth;
	spvm_byte arrayed;
	spvm_byte ms;
	spvm_byte sampled;
	SpvImageFormat format;
	SpvAccessQualifier access;
} spvm_image_info;
typedef struct {
	SpvDecoration type;
	spvm_word literal1, literal2;
	spvm_word index; // member
} spvm_decoration;
typedef struct spvm_result {
	char type;

	spvm_string name;
	spvm_word pointer; // pointer to spvm_result
	SpvStorageClass storage_class;
	struct spvm_result* owner;

	spvm_word member_name_count;
	spvm_string* member_name;

	spvm_word member_count;
	spvm_member* members;

	spvm_word decoration_count;
	spvm_decoration* decorations;

	/* spvm_result_type_function */
	spvm_word return_type;

	/* spvm_result_type_extension */
	spvm_word extension_name; // index in the spvm_program::imports
	spvm_ext_opcode_func* extension;

	/* spvm_result_type_function_type / spvm_result_type_type (structure) / spvm_result_type_function */
	spvm_word* params;

	/* function, label & access chain */
	spvm_source source_location;

	/* word count when a opcode uses source_location (OpAccessChain) */
	spvm_word source_word_count;

	/* op type */
	char value_type;
	spvm_word value_bitcount;
	char value_sign;
	spvm_image_info* image_info;
} spvm_result;
typedef spvm_result* spvm_result_t;


void spvm_decoration_read(spvm_source src, SpvDecoration decor, spvm_word* literal1, spvm_word* literal2);
void spvm_result_add_decoration(spvm_result_t result, SpvDecoration decor, spvm_word literal1, spvm_word literal2);
void spvm_result_add_member_decoration(spvm_result_t result, SpvDecoration decor, spvm_word literal1, spvm_word literal2, spvm_word index);
void spvm_result_allocate_value(spvm_result_t val, spvm_word count);
void spvm_result_allocate_typed_value(spvm_result_t val, spvm_result* results, spvm_word type);
void spvm_result_delete(spvm_result_t val);

void spvm_member_allocate_value(spvm_member_t val, spvm_word count);
void spvm_member_allocate_typed_value(spvm_member_t val, spvm_result* results, spvm_word type);
spvm_word spvm_result_calculate_size(spvm_result_t results, spvm_word type);
void spvm_member_recursive_fill(spvm_result_t results, float* data, spvm_member_t values, spvm_word value_count, spvm_word element_type, spvm_word* offset); // offset is number of 4byte elements

#endif // __SPIRV_VM_RESULT_H__