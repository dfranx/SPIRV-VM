#ifndef __SPIRV_VM_ANALYZER_H__
#define __SPIRV_VM_ANALYZER_H__

#include <spvm/types.h>

enum spvm_undefined_behavior {
	spvm_undefined_behavior_none,							// not an undefined behavior
	spvm_undefined_behavior_div_by_zero,					// a/b when b == 0
	spvm_undefined_behavior_mod_by_zero,					// a%b when b == 0
	spvm_undefined_behavior_image_read_out_of_bounds,		// OpImageRead -> coords are out of bounds
	spvm_undefined_behavior_image_write_out_of_bounds,		// OpImageWrite-> coords are out of bounds
	spvm_undefined_behavior_vector_extract_dynamic,			// OpVectorExtractDynamic -> index out of bounds
	spvm_undefined_behavior_vector_insert_dynamic,			// OpVectorInsertDynamic -> index out of bounds
	spvm_undefined_behavior_asin,							// asin(x) when |x| > 1
	spvm_undefined_behavior_acos,							// acos(x) when |x| > 1
	spvm_undefined_behavior_acosh,							// acosh(x) when x < 1
	spvm_undefined_behavior_atanh,							// atanh(x) when |x| >= 1
	spvm_undefined_behavior_atan2,							// atan2(y, x) when x = y = 0
	spvm_undefined_behavior_pow,							// pow(x, y) when x < 0 or when x = 0 and y <= 0
	spvm_undefined_behavior_log,							// log(x) when x <= 0
	spvm_undefined_behavior_log2,							// log2(x) when x <= 0
	spvm_undefined_behavior_sqrt,							// sqrt(x) when x < 0
	spvm_undefined_behavior_inverse_sqrt,					// invsqrt(x) when x <= 0
	spvm_undefined_behavior_fmin,							// min(x, y) when x or y is NaN
	spvm_undefined_behavior_fmax,							// max(x, y) when x or y is NaN
	spvm_undefined_behavior_clamp,							// clamp(x, minVal, maxVal) when minVal > maxVal
	spvm_undefined_behavior_smoothstep,						// smoothstep(edge0, edge1, x) when edge0 >= edge1
	spvm_undefined_behavior_frexp,							// frexp(x, out exp) when x is NaN or inf
	spvm_undefined_behavior_ldexp,							// ldexp(x, exp) when exp > 128 (float) or exp > 1024 (double)
	spvm_undefined_behavior_count
};

typedef struct spvm_analyzer {
	void (*on_undefined_behavior)(struct spvm_state*, spvm_word ub);
} spvm_analyzer;
typedef struct spvm_analyzer* spvm_analyzer_t;


#endif // __SPIRV_VM_ANALYZER_H__