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

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
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
		state->results[id].members[i].value.d = (s < 0) ? -1 : (s > 0 ? 1 : 0);
	}
}
void spvm_execute_GLSL450_Floor(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
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
		state->results[id].members[i].value.f = sin(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Cos(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = cos(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Tan(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = tan(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Asin(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = asin(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Acos(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = acos(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Atan(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = atan(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Sinh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = sinh(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Cosh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = cosh(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Tanh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = tanh(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Asinh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = asinh(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Acosh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = acosh(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Atanh(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = atanh(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Atan2(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word y = SPVM_READ_WORD(state->code_current);
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = atan2(state->results[y].members[i].value.f, state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Pow(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = pow(state->results[x].members[i].value.f, state->results[y].members[i].value.f);
}
void spvm_execute_GLSL450_Exp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = exp(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Log(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = log(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Exp2(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = exp2(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Log2(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = log2(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Sqrt(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = sqrt(state->results[x].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = sqrtf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_InverseSqrt(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = 1.0 / sqrt(state->results[x].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = 1.0f / sqrtf(state->results[x].members[i].value.f);
}
void spvm_execute_GLSL450_Modf(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word out = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = MIN(state->results[x].members[i].value.d, state->results[y].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = MIN(state->results[x].members[i].value.f, state->results[y].members[i].value.f);
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

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = MAX(state->results[x].members[i].value.d, state->results[y].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = MAX(state->results[x].members[i].value.f, state->results[y].members[i].value.f);
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

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = CLAMP(state->results[x].members[i].value.d, state->results[minVal].members[i].value.d, state->results[maxVal].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = CLAMP(state->results[x].members[i].value.f, state->results[minVal].members[i].value.f, state->results[maxVal].members[i].value.f);
}
void spvm_execute_GLSL450_UClamp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word minVal = SPVM_READ_WORD(state->code_current);
	spvm_word maxVal = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = CLAMP(state->results[x].members[i].value.u64, state->results[minVal].members[i].value.u64, state->results[maxVal].members[i].value.u64);
}
void spvm_execute_GLSL450_SClamp(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word minVal = SPVM_READ_WORD(state->code_current);
	spvm_word maxVal = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = CLAMP(state->results[x].members[i].value.s, state->results[minVal].members[i].value.s, state->results[maxVal].members[i].value.s);
}
void spvm_execute_GLSL450_FMix(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);
	spvm_word a = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			double xVal = state->results[x].members[i].value.d;
			double edge0Val = state->results[edge0].members[i].value.d;
			double edge1Val = state->results[edge1].members[i].value.d;

			xVal = CLAMP((xVal - edge0Val) / (edge1Val - edge0Val), 0.0, 1.0);

			state->results[id].members[i].value.d = xVal * xVal * (3.0 - 2.0 * xVal);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			float xVal = state->results[x].members[i].value.f;
			float edge0Val = state->results[edge0].members[i].value.f;
			float edge1Val = state->results[edge1].members[i].value.f;
			
			xVal = CLAMP((xVal - edge0Val) / (edge1Val - edge0Val), 0.0f, 1.0f);

			state->results[id].members[i].value.f = xVal * xVal * (3.0f - 2.0f * xVal);
		}
}
void spvm_execute_GLSL450_Fma(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word a = SPVM_READ_WORD(state->code_current);
	spvm_word b = SPVM_READ_WORD(state->code_current);
	spvm_word c = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = frexp(state->results[x].members[i].value.d, &state->results[out].members[i].value.s);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = frexpf(state->results[x].members[i].value.f, &state->results[out].members[i].value.s);
}
void spvm_execute_GLSL450_FrexpStruct(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
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

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = ldexp(state->results[x].members[i].value.d, &state->results[exp].members[i].value.s);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = ldexpf(state->results[x].members[i].value.f, &state->results[exp].members[i].value.s);
}
void spvm_execute_GLSL450_Length(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32) {
		double sum = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
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

	if (type_info->value_bitmask > 32) {
		double sum = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			sum += (state->results[p0].members[i].value.d - state->results[p1].members[i].value.d) * (state->results[p0].members[i].value.d - state->results[p1].members[i].value.d);
		state->results[id].members[0].value.d = sqrt(sum);
	} else {
		float sum = 0.0f;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			sum += (state->results[p0].members[i].value.f - state->results[p1].members[i].value.f) * (state->results[p0].members[i].value.f - state->results[p1].members[i].value.f);
		state->results[id].members[0].value.f = sqrtf(sum);
	}
}
void spvm_execute_GLSL450_Cross(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word x = SPVM_READ_WORD(state->code_current);
	spvm_word y = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32) {
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

	if (type_info->value_bitmask > 32) {
		double sum = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			sum += state->results[x].members[i].value.d * state->results[x].members[i].value.d;
		sum = sqrt(sum);
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[x].members[i].value.d / sum;
	}
	else {
		float sum = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
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

	if (type_info->value_bitmask > 32) {
		double dot = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			dot += state->results[Nref].members[i].value.d * state->results[I].members[i].value.d;
		dot = (dot < 0.0) ? 1.0 : -1.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = dot * state->results[N].members[i].value.d;
	}
	else {
		float dot = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			dot += state->results[Nref].members[i].value.f * state->results[I].members[i].value.f;
		dot = (dot < 0.0f) ? 1.0f : -1.0f;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = dot * state->results[N].members[i].value.f;
	}
}
void spvm_execute_GLSL450_Reflect(spvm_word type, spvm_word id, spvm_word word_count, spvm_state_t state)
{
	spvm_word N = SPVM_READ_WORD(state->code_current);
	spvm_word I = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32) {
		double dotNI = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			dotNI += state->results[N].members[i].value.d * state->results[I].members[i].value.d;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[I].members[i].value.d - 2 * dotNI * state->results[N].members[i].value.d;
	}
	else {
		float dotNI = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
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

	if (type_info->value_bitmask > 32) {
		double eta = state->results[SPVM_READ_WORD(state->code_current)].members[0].value.d;

		double dotNI = 0.0;
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
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
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
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

spvm_ext_opcode_func* spvm_build_glsl450_ext()
{
	spvm_ext_opcode_func* ret = (spvm_ext_opcode_func*)malloc(GLSLstd450Count * sizeof(spvm_ext_opcode_func));
	
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
	ret[GLSLstd450Determinant] = 0;
	ret[GLSLstd450MatrixInverse] = 0;
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
	ret[GLSLstd450Length] = spvm_execute_GLSL450_Length;
	ret[GLSLstd450Distance] = spvm_execute_GLSL450_Distance;
	ret[GLSLstd450Cross] = spvm_execute_GLSL450_Cross;
	ret[GLSLstd450Normalize] = spvm_execute_GLSL450_Normalize;
	ret[GLSLstd450FaceForward] = spvm_execute_GLSL450_FaceForward;
	ret[GLSLstd450Reflect] = spvm_execute_GLSL450_Reflect;
	ret[GLSLstd450Refract] = spvm_execute_GLSL450_Refract;
	
	return ret;
}