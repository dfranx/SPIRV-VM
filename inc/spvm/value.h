#ifndef __SPIRV_VM_VALUE_H__
#define __SPIRV_VM_VALUE_H__

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
typedef union
{
	float f;
	double d;
	int i;
	unsigned int ui;
	unsigned long long ui64;
	char b;
} spvm_value;

#endif // __SPIRV_VM_VALUE_H__