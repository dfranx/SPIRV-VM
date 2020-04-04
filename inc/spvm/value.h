#ifndef __SPIRV_VM_VALUE_H__
#define __SPIRV_VM_VALUE_H__

#include <spvm/types.h>

enum spvm_value_type
{
	spvm_value_type_void,
	spvm_value_type_bool,
	spvm_value_type_int,
	spvm_value_type_float,
	spvm_value_type_vector,
	spvm_value_type_matrix,
	spvm_value_type_array,
	spvm_value_type_struct,
	spvm_value_type_pointer
};

typedef struct spvm_member {
	spvm_word type;

	union spvm_value
	{
		float f;
		double d;
		int s;
		unsigned int u;
		unsigned long long u64;
		char b;
	} value;
	
	spvm_word member_count;
	struct spvm_member* members;
} spvm_member;
typedef spvm_member* spvm_member_t;

#endif // __SPIRV_VM_VALUE_H__