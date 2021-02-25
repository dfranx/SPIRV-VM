#include <spvm/ext/GLSL450.h>
#include <spvm/state.h>
#include "GLSL.std.450.h"
#include <math.h>

#ifndef M_PI
#    define M_PI 3.14159265358979323846
#endif

void spvm_execute_GLSL450_Round(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = round(state->results[x].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = roundf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_RoundEven(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = round(state->results[x].members[i].value.d * 0.5) + 0.5;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = roundf(state->results[x].members[i].value.f * 0.5f) + 0.5f;
}
void spvm_execute_GLSL450_Trunc(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = trunc(state->results[x].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = truncf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_FAbs(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = fabs(state->results[x].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = fabsf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_SAbs(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = abs(state->results[x].members[i].value.s);
}
void spvm_execute_GLSL450_FSign(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			double d = state->results[x].members[i].value.d;
			state->results[id].members[i].value.d = (d < 0.0) ? -1 : (d > 0.0 ? 1 : 0);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			float f = state->results[x].members[i].value.f;
			state->results[id].members[i].value.f = (f < 0.0f) ? -1 : (f > 0.0f ? 1 : 0);
		}
}
void spvm_execute_GLSL450_SSign(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		int s = state->results[x].members[i].value.s;
		state->results[id].members[i].value.s = (s < 0) ? -1 : (s > 0 ? 1 : 0);
	}
}
void spvm_execute_GLSL450_Floor(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = floor(state->results[x].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = floorf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Ceil(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = ceil(state->results[x].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = ceilf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Fract(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[x].members[i].value.d - floor(state->results[x].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[x].members[i].value.f - floorf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Radians(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = state->results[x].members[i].value.f * M_PI / 180.0f;
}
void spvm_execute_GLSL450_Degrees(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = state->results[x].members[i].value.f * 180.0f / M_PI;
}
void spvm_execute_GLSL450_Sin(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = sinf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Cos(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = cosf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Tan(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = tanf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Asin(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = asinf(state->results[x].members[i].value.f);
	
		if (state->analyzer && fabsf(state->results[x].members[i].value.f) > 1.0f)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_acos);
	}
}
void spvm_execute_GLSL450_Acos(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = acosf(state->results[x].members[i].value.f);

		if (state->analyzer && fabsf(state->results[x].members[i].value.f) > 1.0f)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_acos);
	}
}
void spvm_execute_GLSL450_Atan(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = atanf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Sinh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = sinhf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Cosh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = coshf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Tanh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = tanhf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Asinh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = asinhf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Acosh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = acoshf(state->results[x].members[i].value.f);
	
		if (state->analyzer && state->results[x].members[i].value.f < 1.0f)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_acosh);
	}
}
void spvm_execute_GLSL450_Atanh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = atanhf(state->results[x].members[i].value.f);

		if (state->analyzer && fabsf(state->results[x].members[i].value.f) >= 1.0f)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_atanh);
	}
}
void spvm_execute_GLSL450_Atan2(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word y = SPVM_READ_WORD(state->code_current);
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = atan2f(state->results[y].members[i].value.f, state->results[x].members[i].value.f);

		if (state->analyzer && state->results[x].members[i].value.f == 0.0f && state->results[y].members[i].value.f == 0.0f)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_atan2);
	}
}
void spvm_execute_GLSL450_Pow(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = powf(state->results[x].members[i].value.f, state->results[y].members[i].value.f);
		
		if (state->analyzer && (state->results[x].members[i].value.f < 0.0f ||
			(state->results[x].members[i].value.f == 0.0f && state->results[y].members[i].value.f <= 0.0f)))
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_pow);
	}
}
void spvm_execute_GLSL450_Exp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = expf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Log(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = logf(state->results[x].members[i].value.f);

		if (state->analyzer && state->results[x].members[i].value.f <= 0.0f)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_log);
	}
}
void spvm_execute_GLSL450_Exp2(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = exp2f(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Log2(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = log2f(state->results[x].members[i].value.f);

		if (state->analyzer && state->results[x].members[i].value.f <= 0.0f)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_log2);
	}
}
void spvm_execute_GLSL450_Sqrt(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = sqrt(state->results[x].members[i].value.d);

			if (state->analyzer && state->results[x].members[i].value.d < 0.0)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_sqrt);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = sqrtf(state->results[x].members[i].value.f);

			if (state->analyzer && state->results[x].members[i].value.f < 0.0f)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_sqrt);
		}
}
void spvm_execute_GLSL450_InverseSqrt(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = 1.0 / sqrt(state->results[x].members[i].value.d);

			if (state->analyzer && state->results[x].members[i].value.d <= 0.0)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_inverse_sqrt);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = 1.0f / sqrtf(state->results[x].members[i].value.f);

			if (state->analyzer && state->results[x].members[i].value.f <= 0.0f)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_inverse_sqrt);
		}
}
#define MATRIX_GET(m, c, r) *(m + c * 4 + r)
void matrix_to_array_d(double* out, spvm_byte use_d, spvm_member_t mems, spvm_word mem_count) {
	if (use_d) {
		for (spvm_word i = 0; i < mem_count; i++)
			for (spvm_word j = 0; j < mems[i].member_count; j++)
				MATRIX_GET(out, i, j) = mems[i].members[j].value.d;
	}
	else {
		for (spvm_word i = 0; i < mem_count; i++)
			for (spvm_word j = 0; j < mems[i].member_count; j++)
				MATRIX_GET(out, i, j) = (double)mems[i].members[j].value.f;
	}
}
double matrix_determinant(double* m, spvm_word mtype)
{
	if (mtype == 2) // 2x2 matrix
		return MATRIX_GET(m,0,0) * MATRIX_GET(m, 1, 1) - MATRIX_GET(m, 1, 0) * MATRIX_GET(m, 0, 1);
	else if (mtype == 3) // 3x3 matrix
		return  MATRIX_GET(m,0,0) * (MATRIX_GET(m,1,1) * MATRIX_GET(m,2,2) - MATRIX_GET(m,2,1) * MATRIX_GET(m,1,2))
		       -MATRIX_GET(m,1,0) * (MATRIX_GET(m,0,1) * MATRIX_GET(m,2,2) - MATRIX_GET(m,2,1) * MATRIX_GET(m,0,2))
			   +MATRIX_GET(m,2,0) * (MATRIX_GET(m,0,1) * MATRIX_GET(m,1,2) - MATRIX_GET(m,1,1) * MATRIX_GET(m,0,2));
	else if (mtype == 4) {
		double m2233 = MATRIX_GET(m,2,2) * MATRIX_GET(m,3,3) - MATRIX_GET(m,3,2) * MATRIX_GET(m,2,3);
		double m2133 = MATRIX_GET(m,2,1) * MATRIX_GET(m,3,3) - MATRIX_GET(m,3,1) * MATRIX_GET(m,2,3);
		double m2132 = MATRIX_GET(m,2,1) * MATRIX_GET(m,3,2) - MATRIX_GET(m,3,1) * MATRIX_GET(m,2,2);
		double m2033 = MATRIX_GET(m,2,0) * MATRIX_GET(m,3,3) - MATRIX_GET(m,3,0) * MATRIX_GET(m,2,3);
		double m2032 = MATRIX_GET(m,2,0) * MATRIX_GET(m,3,2) - MATRIX_GET(m,3,0) * MATRIX_GET(m,2,2);
		double m2031 = MATRIX_GET(m,2,0) * MATRIX_GET(m,3,1) - MATRIX_GET(m,3,0) * MATRIX_GET(m,2,1);

		return  MATRIX_GET(m,0,0) * (MATRIX_GET(m,1,1) * m2233 - MATRIX_GET(m,1,2) * m2133 + MATRIX_GET(m,1,3) * m2132)
			   -MATRIX_GET(m,0,1) * (MATRIX_GET(m,1,0) * m2233 - MATRIX_GET(m,1,2) * m2033 + MATRIX_GET(m,1,3) * m2032)
			   +MATRIX_GET(m,0,2) * (MATRIX_GET(m,1,0) * m2133 - MATRIX_GET(m,1,1) * m2033 + MATRIX_GET(m,1,3) * m2031)
			   -MATRIX_GET(m,0,3) * (MATRIX_GET(m,1,0) * m2132 - MATRIX_GET(m,1,1) * m2032 + MATRIX_GET(m,1,2) * m2031);
	}

	return 0.0;
}
void spvm_execute_GLSL450_Determinant(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	double m[4][4];
	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);
	spvm_word mtype = state->results[x].member_count;
	
	matrix_to_array_d(&m[0][0], type_info->value_bitcount > 32, state->results[x].members, mtype);
	double res = matrix_determinant(&m[0][0], mtype);

	if (type_info->value_bitcount > 32)
		state->results[id].members[0].value.d = res;
	else
		state->results[id].members[0].value.f = res;
}
void spvm_execute_GLSL450_MatrixInverse(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	double m[4][4];
	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);
	spvm_word mtype = state->results[x].member_count;

	matrix_to_array_d(&m[0][0], type_info->value_bitcount > 32, state->results[x].members, mtype);
	double invdet = 1.0 / matrix_determinant(&m[0][0], mtype);

	double res[4][4];

	if (mtype == 2) {
		res[0][0] = +m[1][1] * invdet;
		res[0][1] = -m[0][1] * invdet;
		res[1][0] = -m[1][0] * invdet;
		res[1][1] = +m[0][0] * invdet;
	} else if (mtype == 3) {
		res[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * invdet;
		res[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * invdet;
		res[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * invdet;
		res[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * invdet;
		res[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * invdet;
		res[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * invdet;
		res[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * invdet;
		res[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * invdet;
		res[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * invdet;
	} else if (mtype == 4) {
		double c00 = m[2][2] * m[3][3] - m[3][2] * m[2][3];
		double c02 = m[1][2] * m[3][3] - m[3][2] * m[1][3];
		double c03 = m[1][2] * m[2][3] - m[2][2] * m[1][3];

		double c04 = m[2][1] * m[3][3] - m[3][1] * m[2][3];
		double c06 = m[1][1] * m[3][3] - m[3][1] * m[1][3];
		double c07 = m[1][1] * m[2][3] - m[2][1] * m[1][3];

		double c08 = m[2][1] * m[3][2] - m[3][1] * m[2][2];
		double c10 = m[1][1] * m[3][2] - m[3][1] * m[1][2];
		double c11 = m[1][1] * m[2][2] - m[2][1] * m[1][2];

		double c12 = m[2][0] * m[3][3] - m[3][0] * m[2][3];
		double c14 = m[1][0] * m[3][3] - m[3][0] * m[1][3];
		double c15 = m[1][0] * m[2][3] - m[2][0] * m[1][3];

		double c16 = m[2][0] * m[3][2] - m[3][0] * m[2][2];
		double c18 = m[1][0] * m[3][2] - m[3][0] * m[1][2];
		double c19 = m[1][0] * m[2][2] - m[2][0] * m[1][2];

		double c20 = m[2][0] * m[3][1] - m[3][0] * m[2][1];
		double c22 = m[1][0] * m[3][1] - m[3][0] * m[1][1];
		double c23 = m[1][0] * m[2][1] - m[2][0] * m[1][1];

		res[0][0] = +(m[1][1] * c00 - m[1][2] * c04 + m[1][3] * c08) * invdet;
		res[0][1] = -(m[0][1] * c00 - m[0][2] * c04 + m[0][3] * c08) * invdet;
		res[0][2] = +(m[0][1] * c02 - m[0][2] * c06 + m[0][3] * c10) * invdet;
		res[0][3] = -(m[0][1] * c03 - m[0][2] * c07 + m[0][3] * c11) * invdet;

		res[1][0] = -(m[1][0] * c00 - m[1][2] * c12 + m[1][3] * c16) * invdet;
		res[1][1] = +(m[0][0] * c00 - m[0][2] * c12 + m[0][3] * c16) * invdet;
		res[1][2] = -(m[0][0] * c02 - m[0][2] * c14 + m[0][3] * c18) * invdet;
		res[1][3] = +(m[0][0] * c03 - m[0][2] * c15 + m[0][3] * c19) * invdet;

		res[2][0] = +(m[1][0] * c04 - m[1][1] * c12 + m[1][3] * c20) * invdet;
		res[2][1] = -(m[0][0] * c04 - m[0][1] * c12 + m[0][3] * c20) * invdet;
		res[2][2] = +(m[0][0] * c06 - m[0][1] * c14 + m[0][3] * c22) * invdet;
		res[2][3] = -(m[0][0] * c07 - m[0][1] * c15 + m[0][3] * c23) * invdet;

		res[3][0] = -(m[1][0] * c08 - m[1][1] * c16 + m[1][2] * c20) * invdet;
		res[3][1] = +(m[0][0] * c08 - m[0][1] * c16 + m[0][2] * c20) * invdet;
		res[3][2] = -(m[0][0] * c10 - m[0][1] * c18 + m[0][2] * c22) * invdet;
		res[3][3] = +(m[0][0] * c11 - m[0][1] * c19 + m[0][2] * c23) * invdet;
	}

	if (type_info->value_bitcount > 32)
		for (int i = 0; i < mtype; i++)
			for (int j = 0; j < mtype; j++)
				state->results[id].members[i].members[j].value.d = res[i][j];
	else
		for (int i = 0; i < mtype; i++)
			for (int j = 0; j < mtype; j++)
				state->results[id].members[i].members[j].value.f = res[i][j];
}
void spvm_execute_GLSL450_Modf(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word out = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = modf(state->results[x].members[i].value.d, &state->results[out].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = modff(state->results[x].members[i].value.f, &state->results[out].members[i].value.f);
}
void spvm_execute_GLSL450_ModfStruct(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[0].members[i].value.d = modf(state->results[x].members[i].value.d, &state->results[id].members[1].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[0].members[i].value.f = modff(state->results[x].members[i].value.f, &state->results[id].members[1].members[i].value.f);
}
void spvm_execute_GLSL450_FMin(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = MIN(state->results[x].members[i].value.d, state->results[y].members[i].value.d);

			if (state->analyzer && (isnan(state->results[x].members[i].value.d) || isnan(state->results[y].members[i].value.d)))
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_fmin);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = MIN(state->results[x].members[i].value.f, state->results[y].members[i].value.f);

			if (state->analyzer && (isnan(state->results[x].members[i].value.f) || isnan(state->results[y].members[i].value.f)))
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_fmin);
		}
}
void spvm_execute_GLSL450_UMin(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = MIN(state->results[x].members[i].value.u64, state->results[y].members[i].value.u64);
}
void spvm_execute_GLSL450_SMin(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = MIN(state->results[x].members[i].value.s, state->results[y].members[i].value.s);
}
void spvm_execute_GLSL450_FMax(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = MAX(state->results[x].members[i].value.d, state->results[y].members[i].value.d);

			if (state->analyzer && (isnan(state->results[x].members[i].value.d) || isnan(state->results[y].members[i].value.d)))
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_fmax);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = MAX(state->results[x].members[i].value.f, state->results[y].members[i].value.f);

			if (state->analyzer && (isnan(state->results[x].members[i].value.f) || isnan(state->results[y].members[i].value.f)))
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_fmax);
		}
}
void spvm_execute_GLSL450_UMax(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = MAX(state->results[x].members[i].value.u64, state->results[y].members[i].value.u64);
}
void spvm_execute_GLSL450_SMax(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = MAX(state->results[x].members[i].value.s, state->results[y].members[i].value.s);
}
void spvm_execute_GLSL450_FClamp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word minVal = SPVM_READ_WORD(state->code_current);
	spvm_word maxVal = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = CLAMP(state->results[x].members[i].value.d, state->results[minVal].members[i].value.d, state->results[maxVal].members[i].value.d);

			if (state->analyzer && (state->results[minVal].members[i].value.d > state->results[maxVal].members[i].value.d))
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_clamp);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = CLAMP(state->results[x].members[i].value.f, state->results[minVal].members[i].value.f, state->results[maxVal].members[i].value.f);
			
			if (state->analyzer && (state->results[minVal].members[i].value.f > state->results[maxVal].members[i].value.f))
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_clamp);
		}
}

void spvm_execute_GLSL450_UClamp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word minVal = SPVM_READ_WORD(state->code_current);
	spvm_word maxVal = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.u64 = CLAMP(state->results[x].members[i].value.u64, state->results[minVal].members[i].value.u64, state->results[maxVal].members[i].value.u64);
		
		if (state->analyzer && (state->results[minVal].members[i].value.u64 > state->results[maxVal].members[i].value.u64))
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_clamp);
	}
}
void spvm_execute_GLSL450_SClamp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word minVal = SPVM_READ_WORD(state->code_current);
	spvm_word maxVal = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.s = CLAMP(state->results[x].members[i].value.s, state->results[minVal].members[i].value.s, state->results[maxVal].members[i].value.s);

		if (state->analyzer && (state->results[minVal].members[i].value.s > state->results[maxVal].members[i].value.s))
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_clamp);
	}
}
void spvm_execute_GLSL450_FMix(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);
	spvm_word a = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			double aVal = state->results[a].members[i].value.d;
			state->results[id].members[i].value.d = state->results[x].members[i].value.d * (1 - aVal) + state->results[y].members[i].value.d * aVal;
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			float aVal = state->results[a].members[i].value.f;
			state->results[id].members[i].value.f = state->results[x].members[i].value.f * (1 - aVal) + state->results[y].members[i].value.f * aVal;
		}
}
void spvm_execute_GLSL450_Step(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word edge = SPVM_READ_WORD(state->code_current);
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[x].members[i].value.d >= state->results[edge].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[x].members[i].value.f >= state->results[edge].members[i].value.f;
}
void spvm_execute_GLSL450_SmoothStep(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word edge0 = SPVM_READ_WORD(state->code_current);
	spvm_word edge1 = SPVM_READ_WORD(state->code_current);
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			double xVal = state->results[x].members[i].value.d;
			double edge0Val = state->results[edge0].members[i].value.d;
			double edge1Val = state->results[edge1].members[i].value.d;

			if (edge0Val == edge1Val) {
				state->results[id].members[i].value.d = 0.0;
			} else {
				xVal = CLAMP((xVal - edge0Val) / (edge1Val - edge0Val), 0.0, 1.0);
				state->results[id].members[i].value.d = xVal * xVal * (3.0 - 2.0 * xVal);
			}

			
			if (state->analyzer && edge0Val > edge1Val)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_smoothstep);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			float xVal = state->results[x].members[i].value.f;
			float edge0Val = state->results[edge0].members[i].value.f;
			float edge1Val = state->results[edge1].members[i].value.f;

			if (edge0Val == edge1Val) {
				state->results[id].members[i].value.f = 0.0f;
			} else {
				xVal = CLAMP((xVal - edge0Val) / (edge1Val - edge0Val), 0.0f, 1.0f);
				state->results[id].members[i].value.f = xVal * xVal * (3.0f - 2.0f * xVal);
			}

			if (state->analyzer && edge0Val > edge1Val)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_smoothstep);
		}
}
void spvm_execute_GLSL450_Fma(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word a = SPVM_READ_WORD(state->code_current);
	spvm_word b = SPVM_READ_WORD(state->code_current);
	spvm_word c = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[a].members[i].value.d * state->results[b].members[i].value.d + state->results[c].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[a].members[i].value.f * state->results[b].members[i].value.f + state->results[c].members[i].value.f;
}
void spvm_execute_GLSL450_Frexp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word out = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = frexp(state->results[x].members[i].value.d, &state->results[out].members[i].value.s);
			
			if (state->analyzer && (isnan(state->results[x].members[i].value.d) || isinf(state->results[x].members[i].value.d)))
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_frexp);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = frexpf(state->results[x].members[i].value.f, &state->results[out].members[i].value.s);

			if (state->analyzer && (isnan(state->results[x].members[i].value.f) || isinf(state->results[x].members[i].value.f)))
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_frexp);
		}
}
void spvm_execute_GLSL450_FrexpStruct(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[0].members[i].value.d = frexp(state->results[x].members[i].value.d, &state->results[id].members[1].members[i].value.s);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[0].members[i].value.f = frexpf(state->results[x].members[i].value.f, &state->results[id].members[1].members[i].value.s);
}
void spvm_execute_GLSL450_Ldexp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word exp = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = ldexp(state->results[x].members[i].value.d, state->results[exp].members[i].value.s);
		
			
			if (state->analyzer && state->results[exp].members[i].value.d > 1024)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_ldexp);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = ldexpf(state->results[x].members[i].value.f, state->results[exp].members[i].value.s);

			if (state->analyzer && state->results[exp].members[i].value.f > 128)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_ldexp);
		}
}
void spvm_execute_GLSL450_PackSnorm4x8(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = 0;
	for (spvm_word i = 0; i < 4; i++) {
		char s = roundf(CLAMP(state->results[x].members[i].value.f, -1.0f, +1.0f) * 127.0f);
		res |= s << (8 * i);
	}
	state->results[id].members[0].value.u = res;
}
void spvm_execute_GLSL450_PackUnorm4x8(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = 0;
	for (spvm_word i = 0; i < 4; i++) {
		unsigned char s = roundf(CLAMP(state->results[x].members[i].value.f, 0.0f, +1.0f) * 255.0f);
		res |= s << (8 * i);
	}
	state->results[id].members[0].value.u = res;
}
void spvm_execute_GLSL450_PackSnorm2x16(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = 0;
	for (spvm_word i = 0; i < 2; i++) {
		short s = roundf(CLAMP(state->results[x].members[i].value.f, -1.0f, +1.0f) * 32767.0f);
		res |= s << (16 * i);
	}
	state->results[id].members[0].value.u = res;
}
void spvm_execute_GLSL450_PackUnorm2x16(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = 0;
	for (spvm_word i = 0; i < 2; i++) {
		unsigned short s = roundf(CLAMP(state->results[x].members[i].value.f, 0.0f, +1.0f) * 65535.0f);
		res |= s << (16 * i);
	}
	state->results[id].members[0].value.u = res;
}
void spvm_execute_GLSL450_PackDouble2x32(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	union {
		unsigned int data[2];
		double res;
	} u;

	u.data[0] = state->results[x].members[0].value.u;
	u.data[1] = state->results[x].members[1].value.u;

	state->results[id].members[0].value.d = u.res;
}
typedef union {
	unsigned int i;
	float f;
} floatConverter;
float toFloat32(short value)
{
	/* https://github.com/g-truc/glm/blob/ddebaba03308475b1e33670267eadd596d039949/glm/detail/type_half.inl */
	int s = (value >> 15) & 0x00000001;
	int e = (value >> 10) & 0x0000001f;
	int m = value & 0x000003ff;

	if (e == 0) {
		if (m == 0) {
			floatConverter result;
			result.i = s << 31;
			return result.f;
		}
		else {
			while (!(m & 0x00000400)) {
				m <<= 1;
				e -= 1;
			}

			e += 1;
			m &= ~0x00000400;
		}
	}
	else if (e == 31) {
		if (m == 0) {
			floatConverter result;
			result.i = (s << 31) | 0x7f800000;
			return result.f;
		}
		else {
			floatConverter result;
			result.i = (s << 31) | 0x7f800000 | (m << 13);
			return result.f;
		}
	}

	e = e + (127 - 15);
	m = m << 13;

	floatConverter Result;
	Result.i = (s << 31) | (e << 23) | m;
	return Result.f;
}
short toFloat16(float value)
{
	/* https://github.com/g-truc/glm/blob/ddebaba03308475b1e33670267eadd596d039949/glm/detail/type_half.inl */
	floatConverter Entry;
	Entry.f = value;
	int i = Entry.i;

	int s = (i >> 16) & 0x00008000;
	int e = ((i >> 23) & 0x000000ff) - (127 - 15);
	int m = i & 0x007fffff;

	if (e <= 0) {
		if (e < -10)
			return s;

		m = (m | 0x00800000) >> (1 - e);

		if (m & 0x00001000)
			m += 0x00002000;

		return s | (m >> 13);
	}
	else if (e == 0xff - (127 - 15)) {
		if (m == 0)
			return s | 0x7c00;
		else {
			m >>= 13;
			return s | 0x7c00 | m | (m == 0);
		}
	}
	else {
		if (m & 0x00001000) {
			m += 0x00002000;
			if (m & 0x00800000) {
				m = 0;
				e += 1;
			}
		}

		if (e > 30)
			return s | 0x7c00;

		return s | (e << 10) | (m >> 13);
	}

	return 0;
}
void spvm_execute_GLSL450_PackHalf2x16(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = toFloat16(state->results[x].members[0].value.f);
	res |= ((unsigned int)toFloat16(state->results[x].members[1].value.f)) << 16;

	state->results[id].members[0].value.u = res;
}
void spvm_execute_GLSL450_UnpackHalf2x16(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	state->results[id].members[0].value.f = toFloat32(state->results[x].members[0].value.u);
	state->results[id].members[1].value.f = toFloat32(state->results[x].members[0].value.u >> 16);
}
void spvm_execute_GLSL450_UnpackDouble2x32(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	union {
		unsigned int res[2];
		double data;
	} u;

	u.data = state->results[x].members[0].value.d;

	state->results[id].members[0].value.u = u.res[0];
	state->results[id].members[1].value.u = u.res[1];
}
void spvm_execute_GLSL450_UnpackUnorm2x16(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = state->results[x].members[0].value.u;
	for (spvm_word i = 0; i < 2; i++) {
		unsigned short s = res >> (16 * i);
		state->results[id].members[i].value.f = CLAMP(s / 65535.0f, 0.0f, +1.0f);
	}
}
void spvm_execute_GLSL450_UnpackSnorm2x16(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = state->results[x].members[0].value.u;
	for (spvm_word i = 0; i < 2; i++) {
		short s = res >> (16 * i);
		state->results[id].members[i].value.f = CLAMP(s / 32767.0f, -1.0f, +1.0f);
	}
}
void spvm_execute_GLSL450_UnpackUnorm4x8(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = state->results[x].members[0].value.u;
	for (spvm_word i = 0; i < 4; i++) {
		unsigned char s = res >> (8 * i);
		state->results[id].members[i].value.f = CLAMP(s / 255.0f, 0.0f, +1.0f);
	}
}
void spvm_execute_GLSL450_UnpackSnorm4x8(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	unsigned int res = state->results[x].members[0].value.u;
	for (spvm_word i = 0; i < 4; i++) {
		char s = res >> (8 * i);
		state->results[id].members[i].value.f = CLAMP(s / 127.0f, -1.0f, +1.0f);
	}
}
void spvm_execute_GLSL450_Length(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32) {
		double sum = 0.0;
		for (spvm_word i = 0; i < state->results[x].member_count; i++)
			sum += state->results[x].members[i].value.d * state->results[x].members[i].value.d;
		state->results[id].members[0].value.d = sqrt(sum);
	} else {
		float sum = 0.0f;
		for (spvm_word i = 0; i < state->results[x].member_count; i++)
			sum += state->results[x].members[i].value.f * state->results[x].members[i].value.f;
		state->results[id].members[0].value.f = sqrtf(sum);
	}
}
void spvm_execute_GLSL450_Distance(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word p0 = SPVM_READ_WORD(state->code_current);
	spvm_word p1 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32) {
		double sum = 0.0;
		for (spvm_word i = 0; i < state->results[p0].member_count; i++)
			sum += (state->results[p0].members[i].value.d - state->results[p1].members[i].value.d) * (state->results[p0].members[i].value.d - state->results[p1].members[i].value.d);
		state->results[id].members[0].value.d = sqrt(sum);
	} else {
		float sum = 0.0f;
		for (spvm_word i = 0; i < state->results[p0].member_count; i++)
			sum += (state->results[p0].members[i].value.f - state->results[p1].members[i].value.f) * (state->results[p0].members[i].value.f - state->results[p1].members[i].value.f);
		state->results[id].members[0].value.f = sqrtf(sum);
	}
}
void spvm_execute_GLSL450_Cross(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32) {
		spvm_member_t xMems = state->results[x].members;
		spvm_member_t yMems = state->results[y].members;
		state->results[id].members[0].value.d = xMems[1].value.d * yMems[2].value.d - yMems[1].value.d * xMems[2].value.d;
		state->results[id].members[1].value.d = xMems[2].value.d * yMems[0].value.d - yMems[2].value.d * xMems[0].value.d;
		state->results[id].members[2].value.d = xMems[0].value.d * yMems[1].value.d - yMems[0].value.d * xMems[1].value.d;
	}
	else {
		spvm_member_t xMems = state->results[x].members;
		spvm_member_t yMems = state->results[y].members;
		state->results[id].members[0].value.f = xMems[1].value.f * yMems[2].value.f - yMems[1].value.f * xMems[2].value.f;
		state->results[id].members[1].value.f = xMems[2].value.f * yMems[0].value.f - yMems[2].value.f * xMems[0].value.f;
		state->results[id].members[2].value.f = xMems[0].value.f * yMems[1].value.f - yMems[0].value.f * xMems[1].value.f;
	}
}
void spvm_execute_GLSL450_Normalize(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32) {
		double sum = 0.0;
		for (spvm_word i = 0; i < state->results[x].member_count; i++)
			sum += state->results[x].members[i].value.d * state->results[x].members[i].value.d;
		sum = sqrt(sum);
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[x].members[i].value.d / sum;
	}
	else {
		float sum = 0.0;
		for (spvm_word i = 0; i < state->results[x].member_count; i++)
			sum += state->results[x].members[i].value.f * state->results[x].members[i].value.f;
		sum = sqrtf(sum);
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[x].members[i].value.f / sum;
	}
}
void spvm_execute_GLSL450_FaceForward(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word N = SPVM_READ_WORD(state->code_current);
	spvm_word I = SPVM_READ_WORD(state->code_current);
	spvm_word Nref = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32) {
		double dot = 0.0;
		for (spvm_word i = 0; i < state->results[Nref].member_count; i++)
			dot += state->results[Nref].members[i].value.d * state->results[I].members[i].value.d;
		dot = (dot < 0.0) ? 1.0 : -1.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = dot * state->results[N].members[i].value.d;
	}
	else {
		float dot = 0.0;
		for (spvm_word i = 0; i < state->results[Nref].member_count; i++)
			dot += state->results[Nref].members[i].value.f * state->results[I].members[i].value.f;
		dot = (dot < 0.0f) ? 1.0f : -1.0f;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = dot * state->results[N].members[i].value.f;
	}
}
void spvm_execute_GLSL450_Reflect(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word I = SPVM_READ_WORD(state->code_current);
	spvm_word N = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32) {
		double dotNI = 0.0;
		for (spvm_word i = 0; i < state->results[N].member_count; i++)
			dotNI += state->results[N].members[i].value.d * state->results[I].members[i].value.d;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[I].members[i].value.d - 2 * dotNI * state->results[N].members[i].value.d;
	}
	else {
		float dotNI = 0.0;
		for (spvm_word i = 0; i < state->results[N].member_count; i++)
			dotNI += state->results[N].members[i].value.f * state->results[I].members[i].value.f;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[I].members[i].value.f - 2 * dotNI * state->results[N].members[i].value.f;
	}
}
void spvm_execute_GLSL450_Refract(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word I = SPVM_READ_WORD(state->code_current);
	spvm_word N = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32) {
		double eta = state->results[SPVM_READ_WORD(state->code_current)].members[0].value.d;

		double dotNI = 0.0;
		for (spvm_word i = 0; i < state->results[N].member_count; i++)
			dotNI += state->results[N].members[i].value.d * state->results[I].members[i].value.d;

		double k = 1.0 - eta * eta * (1.0 - dotNI * dotNI);

		if (k < 0.0)
			for (spvm_word i = 0; i < state->results[id].member_count; i++)
				state->results[id].members[i].value.d = 0.0;
		else
			for (spvm_word i = 0; i < state->results[id].member_count; i++)
				state->results[id].members[i].value.d = eta * state->results[I].members[i].value.d - (eta * dotNI + sqrt(k)) * state->results[N].members[i].value.d;
	}
	else {
		float eta = state->results[SPVM_READ_WORD(state->code_current)].members[0].value.f;

		float dotNI = 0.0f;
		for (spvm_word i = 0; i < state->results[N].member_count; i++)
			dotNI += state->results[N].members[i].value.f * state->results[I].members[i].value.f;

		float k = 1.0f - eta * eta * (1.0f - dotNI * dotNI);

		if (k < 0.0)
			for (spvm_word i = 0; i < state->results[id].member_count; i++)
				state->results[id].members[i].value.f = 0.0f;
		else
			for (spvm_word i = 0; i < state->results[id].member_count; i++)
				state->results[id].members[i].value.f = eta * state->results[I].members[i].value.f - (eta * dotNI + sqrtf(k)) * state->results[N].members[i].value.f;
	}
}
void spvm_execute_GLSL450_FindILsb(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
/*
TODO:
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		_BitScanForward(&state->results[id].members[0].value.u, state->results[x].members[0].value.u);
*/
}
void spvm_execute_GLSL450_FindSMsb(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
/*
TODO:
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		_BitScanReverse(&state->results[id].members[0].value.s, state->results[x].members[0].value.s);
*/
}
void spvm_execute_GLSL450_FindUMsb(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
/*
TODO:
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		_BitScanReverse(&state->results[id].members[0].value.u, state->results[x].members[0].value.u);
*/
}
void spvm_execute_GLSL450_NMin(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = NMIN(state->results[x].members[i].value.d, state->results[y].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = NMIN(state->results[x].members[i].value.f, state->results[y].members[i].value.f);
}
void spvm_execute_GLSL450_NMax(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = NMAX(state->results[x].members[i].value.d, state->results[y].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = NMAX(state->results[x].members[i].value.f, state->results[y].members[i].value.f);
}
void spvm_execute_GLSL450_NClamp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word minVal = SPVM_READ_WORD(state->code_current);
	spvm_word maxVal = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = NCLAMP(state->results[x].members[i].value.d, state->results[minVal].members[i].value.d, state->results[maxVal].members[i].value.d);

			if (state->analyzer && state->results[minVal].members[i].value.d > state->results[maxVal].members[i].value.d)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_clamp);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = NCLAMP(state->results[x].members[i].value.f, state->results[minVal].members[i].value.f, state->results[maxVal].members[i].value.f);

			if (state->analyzer && state->results[minVal].members[i].value.f > state->results[maxVal].members[i].value.f)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_clamp);
		}
}

spvm_ext_opcode_func* spvm_build_glsl450_ext()
{
	spvm_ext_opcode_func* ret = (spvm_ext_opcode_func*)calloc(GLSLstd450Count, sizeof(spvm_ext_opcode_func));
	
	ret[GLSLstd450Round] = spvm_execute_GLSL450_Round;
	ret[GLSLstd450RoundEven] = spvm_execute_GLSL450_RoundEven;
	ret[GLSLstd450Trunc] = spvm_execute_GLSL450_Trunc;
	ret[GLSLstd450FAbs] = spvm_execute_GLSL450_FAbs;
	ret[GLSLstd450SAbs] = spvm_execute_GLSL450_SAbs;
	ret[GLSLstd450FSign] = spvm_execute_GLSL450_FSign;
	ret[GLSLstd450SSign] = spvm_execute_GLSL450_SSign;
	ret[GLSLstd450Floor] = spvm_execute_GLSL450_Floor;
	ret[GLSLstd450Ceil] = spvm_execute_GLSL450_Ceil;
	ret[GLSLstd450Fract] = spvm_execute_GLSL450_Fract;
	ret[GLSLstd450Radians] = spvm_execute_GLSL450_Radians;
	ret[GLSLstd450Degrees] = spvm_execute_GLSL450_Degrees;
	ret[GLSLstd450Sin] = spvm_execute_GLSL450_Sin;
	ret[GLSLstd450Cos] = spvm_execute_GLSL450_Cos;
	ret[GLSLstd450Tan] = spvm_execute_GLSL450_Tan;
	ret[GLSLstd450Asin] = spvm_execute_GLSL450_Asin;
	ret[GLSLstd450Acos] = spvm_execute_GLSL450_Acos;
	ret[GLSLstd450Atan] = spvm_execute_GLSL450_Atan;
	ret[GLSLstd450Sinh] = spvm_execute_GLSL450_Sinh;
	ret[GLSLstd450Cosh] = spvm_execute_GLSL450_Cosh;
	ret[GLSLstd450Tanh] = spvm_execute_GLSL450_Tanh;
	ret[GLSLstd450Asinh] = spvm_execute_GLSL450_Asinh;
	ret[GLSLstd450Acosh] = spvm_execute_GLSL450_Acosh;
	ret[GLSLstd450Atanh] = spvm_execute_GLSL450_Atanh;
	ret[GLSLstd450Atan2] = spvm_execute_GLSL450_Atan2;
	ret[GLSLstd450Pow] = spvm_execute_GLSL450_Pow;
	ret[GLSLstd450Exp] = spvm_execute_GLSL450_Exp;
	ret[GLSLstd450Log] = spvm_execute_GLSL450_Log;
	ret[GLSLstd450Exp2] = spvm_execute_GLSL450_Exp2;
	ret[GLSLstd450Log2] = spvm_execute_GLSL450_Log2;
	ret[GLSLstd450Sqrt] = spvm_execute_GLSL450_Sqrt;
	ret[GLSLstd450InverseSqrt] = spvm_execute_GLSL450_InverseSqrt;
	ret[GLSLstd450Determinant] = spvm_execute_GLSL450_Determinant;
	ret[GLSLstd450MatrixInverse] = spvm_execute_GLSL450_MatrixInverse;
	ret[GLSLstd450Modf] = spvm_execute_GLSL450_Modf;
	ret[GLSLstd450ModfStruct] = spvm_execute_GLSL450_ModfStruct;
	ret[GLSLstd450FMin] = spvm_execute_GLSL450_FMin;
	ret[GLSLstd450UMin] = spvm_execute_GLSL450_UMin;
	ret[GLSLstd450SMin] = spvm_execute_GLSL450_SMin;
	ret[GLSLstd450FMax] = spvm_execute_GLSL450_FMax;
	ret[GLSLstd450UMax] = spvm_execute_GLSL450_UMax;
	ret[GLSLstd450SMax] = spvm_execute_GLSL450_SMax;
	ret[GLSLstd450FClamp] = spvm_execute_GLSL450_FClamp;
	ret[GLSLstd450UClamp] = spvm_execute_GLSL450_UClamp;
	ret[GLSLstd450SClamp] = spvm_execute_GLSL450_SClamp;
	ret[GLSLstd450FMix] = spvm_execute_GLSL450_FMix;
	ret[GLSLstd450Step] = spvm_execute_GLSL450_Step;
	ret[GLSLstd450SmoothStep] = spvm_execute_GLSL450_SmoothStep;
	ret[GLSLstd450Fma] = spvm_execute_GLSL450_Fma;
	ret[GLSLstd450Frexp] = spvm_execute_GLSL450_Frexp;
	ret[GLSLstd450FrexpStruct] = spvm_execute_GLSL450_FrexpStruct;
	ret[GLSLstd450Ldexp] = spvm_execute_GLSL450_Ldexp;
	ret[GLSLstd450PackSnorm4x8] = spvm_execute_GLSL450_PackSnorm4x8;
	ret[GLSLstd450PackUnorm4x8] = spvm_execute_GLSL450_PackUnorm4x8;
	ret[GLSLstd450PackSnorm2x16] = spvm_execute_GLSL450_PackSnorm2x16;
	ret[GLSLstd450PackUnorm2x16] = spvm_execute_GLSL450_PackUnorm2x16;
	ret[GLSLstd450PackDouble2x32] = spvm_execute_GLSL450_PackDouble2x32;
	ret[GLSLstd450PackHalf2x16] = spvm_execute_GLSL450_PackHalf2x16;
	ret[GLSLstd450UnpackHalf2x16] = spvm_execute_GLSL450_UnpackHalf2x16;
	ret[GLSLstd450UnpackDouble2x32] = spvm_execute_GLSL450_UnpackDouble2x32;
	ret[GLSLstd450UnpackSnorm2x16] = spvm_execute_GLSL450_UnpackSnorm2x16;
	ret[GLSLstd450UnpackUnorm2x16] = spvm_execute_GLSL450_UnpackUnorm2x16;
	ret[GLSLstd450UnpackSnorm4x8] = spvm_execute_GLSL450_UnpackSnorm4x8;
	ret[GLSLstd450UnpackUnorm4x8] = spvm_execute_GLSL450_UnpackUnorm4x8;
	ret[GLSLstd450Length] = spvm_execute_GLSL450_Length;
	ret[GLSLstd450Distance] = spvm_execute_GLSL450_Distance;
	ret[GLSLstd450Cross] = spvm_execute_GLSL450_Cross;
	ret[GLSLstd450Normalize] = spvm_execute_GLSL450_Normalize;
	ret[GLSLstd450FaceForward] = spvm_execute_GLSL450_FaceForward;
	ret[GLSLstd450Reflect] = spvm_execute_GLSL450_Reflect;
	ret[GLSLstd450Refract] = spvm_execute_GLSL450_Refract;
	ret[GLSLstd450FindILsb] = spvm_execute_GLSL450_FindILsb;
	ret[GLSLstd450FindSMsb] = spvm_execute_GLSL450_FindSMsb;
	ret[GLSLstd450FindUMsb] = spvm_execute_GLSL450_FindUMsb;
	ret[GLSLstd450InterpolateAtCentroid] = 0;
	ret[GLSLstd450InterpolateAtSample] = 0;
	ret[GLSLstd450InterpolateAtOffset] = 0;
	ret[GLSLstd450NMin] = spvm_execute_GLSL450_NMin;
	ret[GLSLstd450NMax] = spvm_execute_GLSL450_NMax;
	ret[GLSLstd450NClamp] = spvm_execute_GLSL450_NClamp;
	
	return ret;
}
