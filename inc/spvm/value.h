#ifndef __SPIRV_VM_VALUE_H__
#define __SPIRV_VM_VALUE_H__

#include <spvm/types.h>
#include <spvm/image.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#define SPVM_MAX(x, y) (((x) > (y)) ? (x) : (y))
#define SPVM_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define SPVM_CLAMP(x, minVal, maxVal) SPVM_MIN(SPVM_MAX((x), (minVal)), (maxVal))
#define SPVM_NMAX(x, y) (isnan(x) ? (y) : (isnan(y) ? (x) : SPVM_MAX(x,y)))
#define SPVM_NMIN(x, y) (isnan(x) ? (y) : (isnan(y) ? (x) : SPVM_MIN(x,y)))
#define SPVM_NCLAMP(x, minVal, maxVal) SPVM_NMIN(SPVM_NMAX((x), (minVal)), (maxVal))
#define SPVM_SIGN(x) ((x) > 0) - ((x) < 0)

enum spvm_value_type
{
	spvm_value_type_void,
	spvm_value_type_bool,
	spvm_value_type_int,
	spvm_value_type_float,
	spvm_value_type_vector,
	spvm_value_type_matrix,
	spvm_value_type_array,
	spvm_value_type_runtime_array,
	spvm_value_type_struct,
	spvm_value_type_image,
	spvm_value_type_sampler,
	spvm_value_type_sampled_image,
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
		spvm_image* image;
		spvm_sampler* sampler;
	} value;

	spvm_word member_count;
	struct spvm_member* members;
} spvm_member;
typedef spvm_member* spvm_member_t;

void spvm_member_free(spvm_member_t source, spvm_word value_count);
void spvm_member_memcpy(spvm_member_t target, spvm_member_t source, spvm_word value_count);

void spvm_member_set_value_f(spvm_member_t mems, size_t mem_count, float* f);
void spvm_member_set_value_i(spvm_member_t mems, size_t mem_count,  int* d);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SPIRV_VM_VALUE_H__
