#include <spvm/context.h>
#include <spvm/opcode.h>
#include <spvm/state.h>
#include <spvm/spirv.h>
#include <spvm/image.h>
#include <string.h>
#include <math.h>

/*
1. textures
2. extensions
*/

/* opcodes */
/* 3.32.2 Debug Instructions */
void spvm_execute_OpLine(spvm_word word_count, spvm_state_t state)
{
	spvm_word file = SPVM_READ_WORD(state->code_current);
	spvm_word line = SPVM_READ_WORD(state->code_current);
	spvm_word clmn = SPVM_READ_WORD(state->code_current);

	state->current_file = state->results[file].name;
	state->current_line = line;
	state->current_column = clmn;
}
void spvm_execute_OpNoLine(spvm_word word_count, spvm_state_t state)
{
	state->current_file = NULL;
	state->current_line = -1;
	state->current_column = -1;
}

/* 3.32.4 Debug Instructions */
void spvm_execute_OpExtInst(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word set = SPVM_READ_WORD(state->code_current);
	spvm_word inst = SPVM_READ_WORD(state->code_current);

	state->results[set].extension[inst](type, id, word_count - 4, state);
}

/* 3.32.8 Memory Instructions */
void spvm_execute_OpStore(spvm_word word_count, spvm_state_t state)
{
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);
	spvm_word val_id = SPVM_READ_WORD(state->code_current);

	spvm_member_memcpy(state->results[ptr_id].members, state->results[val_id].members, state->results[ptr_id].member_count);
}
void spvm_execute_OpLoad(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);

	spvm_member_memcpy(state->results[id].members, state->results[ptr_id].members, state->results[ptr_id].member_count);
}
void spvm_execute_OpCopyMemory(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	spvm_word source = SPVM_READ_WORD(state->code_current);

	spvm_member_memcpy(state->results[target].members, state->results[source].members, state->results[target].member_count);
}
void spvm_execute_OpCopyMemorySized(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	spvm_word source = SPVM_READ_WORD(state->code_current);
	spvm_word size_id = SPVM_READ_WORD(state->code_current);

	spvm_word size = state->results[size_id].members[0].value.s;

	// TODO: spvm_member_copy_sized()
	memcpy(state->results[target].members, state->results[source].members, size);
}
void spvm_execute_OpAccessChain(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word value_id = SPVM_READ_WORD(state->code_current);

	spvm_word index_count = word_count - 4;

	state->results[id].type = spvm_result_type_access_chain;
	state->results[id].pointer = var_type;

	spvm_word index_id = SPVM_READ_WORD(state->code_current);
	spvm_word index = state->results[index_id].members[0].value.s;

	spvm_member_t result = state->results[value_id].members + index;

	while (index_count) {
		index_id = SPVM_READ_WORD(state->code_current);
		index = state->results[index_id].members[0].value.s;

		result = result->members + index;

		index_count--;
	}

	if (result->member_count != 0) {
		state->results[id].member_count = result->member_count;
		state->results[id].members = result->members;
	}
	else {
		state->results[id].member_count = 1;
		state->results[id].members = result;
	}
}
void spvm_execute_OpPtrEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	// TODO: should this actually check if they point to the same value or if the values are same?
	state->results[id].members[0].value.b = (state->results[op1].members == state->results[op2].members);
}
void spvm_execute_OpPtrNotEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	// TODO: should this actually check if they point to the same value or if the values are same?
	state->results[id].members[0].value.b = (state->results[op1].members != state->results[op2].members);
}
void spvm_execute_OpFunctionCall(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word func = SPVM_READ_WORD(state->code_current);

	spvm_word argc = word_count - 3;

	spvm_result_t next_func = &state->results[func];

	for (spvm_word i = 0; i < argc; i++) {
		spvm_word arg_id = SPVM_READ_WORD(state->code_current);
		state->results[next_func->params[i]].members = state->results[arg_id].members;
	}

	spvm_state_push_function_stack(state, next_func, id);
}
void spvm_execute_OpReturn(spvm_word word_count, spvm_state_t state)
{
	state->return_id = -1;
	spvm_state_pop_function_stack(state);
}
void spvm_execute_OpReturnValue(spvm_word word_count, spvm_state_t state)
{
	state->return_id = SPVM_READ_WORD(state->code_current);
	spvm_state_pop_function_stack(state);
}

/* 3.32.10 Image Instruction */
void spvm_execute_OpImageSampleImplicitLod(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);

	spvm_result_t coord = &state->results[coord_id];

	spvm_result_t image_container = &state->results[image_id];
	spvm_image_t image = image_container->members[0].image_data;

	float stu[3] = { 0.0f, 0.0f, 0.0f };
	for (spvm_word i = 0; i < coord->member_count; i++)
		stu[i] = coord->members[i].value.f;

	float* px = spvm_image_sample(image, stu[0], stu[1], stu[2]);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = *(px + i);
}

/* 3.32.11 Conversion Instructions */
void spvm_execute_OpConvertFToU(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[state->results[val].pointer]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.u64 = (unsigned long long)state->results[val].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.u64 = (unsigned long long)state->results[val].members[i].value.f;
}
void spvm_execute_OpConvertFToS(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[state->results[val].pointer]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.s = (int)state->results[val].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.s = (int)state->results[val].members[i].value.f;
}
void spvm_execute_OpConvertUToF(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = (double)state->results[val].members[i].value.u;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = (float)state->results[val].members[i].value.u;
}
void spvm_execute_OpConvertSToF(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = (double)state->results[val].members[i].value.s;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = (float)state->results[val].members[i].value.s;
}
void spvm_execute_OpFConvert(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t res_type_info = spvm_state_get_type_info(state->results, &state->results[res_type]);
	spvm_result_t val_info = spvm_state_get_type_info(state->results, &state->results[state->results[val].pointer]);

	if (res_type_info->value_bitmask > val_info->value_bitmask)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = (double)state->results[val].members[i].value.f;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = (float)state->results[val].members[i].value.d;
}
void spvm_execute_OpBitcast(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[val].members[i].value.u64;
}

/* 3.32.12 Composite Instructions */
void spvm_execute_OpVectorExtractDynamic(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vector = SPVM_READ_WORD(state->code_current);
	spvm_word index_id = SPVM_READ_WORD(state->code_current);

	spvm_word index = state->results[index_id].members[0].value.u64;

	state->results[id].members[0].value.u64 = state->results[vector].members[index].value.u64;
}
void spvm_execute_OpVectorInsertDynamic(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vector = SPVM_READ_WORD(state->code_current);
	spvm_word comp = SPVM_READ_WORD(state->code_current);
	spvm_word index_id = SPVM_READ_WORD(state->code_current);

	spvm_word index = state->results[index_id].members[0].value.u64;

	spvm_member_memcpy(state->results[id].members, state->results[vector].members, state->results[id].member_count);
	state->results[id].members[index].value.u64 = state->results[comp].members[0].value.u64;
}
void spvm_execute_OpVectorShuffle(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vector1_id = SPVM_READ_WORD(state->code_current);
	spvm_word vector2_id = SPVM_READ_WORD(state->code_current);

	spvm_result_t vector1 = &state->results[vector1_id];
	spvm_result_t vector2 = &state->results[vector2_id];

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		spvm_word index = SPVM_READ_WORD(state->code_current);

		if (index >= vector1->member_count)
			state->results[id].members[i].value.u64 = vector2->members[index - vector1->member_count].value.u64;
		else
			state->results[id].members[i].value.u64 = vector1->members[index].value.u64;
	}
}
void spvm_execute_OpCompositeConstruct(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		spvm_word index = SPVM_READ_WORD(state->code_current);
		if (state->results[id].members[i].member_count == 0)
			state->results[id].members[i].value.s = state->results[index].members[0].value.s;
		else
			for (spvm_word j = 0; j < state->results[index].member_count; j++)
				state->results[id].members[i].members[j].value.s = state->results[index].members[j].value.s;
	}
}
void spvm_execute_OpCompositeExtract(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word value_id = SPVM_READ_WORD(state->code_current);

	spvm_word index_count = word_count - 4;

	spvm_word index = SPVM_READ_WORD(state->code_current);
	spvm_member_t result = state->results[value_id].members + index;

	while (index_count) {
		index = SPVM_READ_WORD(state->code_current);

		result = result->members + index;

		index_count--;
	}

	spvm_member_memcpy(state->results[id].members, result, 1);
}
void spvm_execute_OpCopyObject(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op = SPVM_READ_WORD(state->code_current);

	spvm_member_memcpy(state->results[id].members, state->results[op].members, state->results[op].member_count);
}
void spvm_execute_OpTranspose(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word mat = SPVM_READ_WORD(state->code_current);

	spvm_word s1 = state->results[mat].member_count;
	spvm_word s2 = state->results[mat].members[0].member_count;
	for (spvm_word i = 0; i < s1; i++)
		for (spvm_word j = 0; j < s2; ++j)
			state->results[id].members[j].members[i].value.u64 = state->results[mat].members[i].members[j].value.u64;
}

/* 3.32.13 Arithmetic Instructions */
void spvm_execute_OpSNegate(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = -state->results[op].members[i].value.s;
}
void spvm_execute_OpFNegate(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = -state->results[op].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = -state->results[op].members[i].value.f;
}
void spvm_execute_OpIAdd(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s + state->results[op2].members[i].value.s;
}
void spvm_execute_OpFAdd(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	// TODO: is there a better way to do this?
	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[op1].members[i].value.d + state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[op1].members[i].value.f + state->results[op2].members[i].value.f;
}
void spvm_execute_OpISub(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s - state->results[op2].members[i].value.s;
}
void spvm_execute_OpFSub(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	// TODO: is there a better way to do this?
	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[op1].members[i].value.d - state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[op1].members[i].value.f - state->results[op2].members[i].value.f;
}
void spvm_execute_OpIMul(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s * state->results[op2].members[i].value.s;
}
void spvm_execute_OpFMul(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	// TODO: is there a better way to do this?
	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[op1].members[i].value.d * state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[op1].members[i].value.f * state->results[op2].members[i].value.f;
}
void spvm_execute_OpUDiv(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 / state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSDiv(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s / state->results[op2].members[i].value.s;
}
void spvm_execute_OpFDiv(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	// TODO: is there a better way to do this?
	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[op1].members[i].value.d / state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[op1].members[i].value.f / state->results[op2].members[i].value.f;
}
void spvm_execute_OpUMod(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 % state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSMod(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s % state->results[op2].members[i].value.s;
}
void spvm_execute_OpFMod(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	// TODO: is there a better way to do this?
	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = fmod(state->results[op1].members[i].value.d, state->results[op2].members[i].value.d);
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = fmodf(state->results[op1].members[i].value.f, state->results[op2].members[i].value.f);
}
void spvm_execute_OpVectorTimesScalar(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);
	spvm_word scalar = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = state->results[vec].members[i].value.d * state->results[scalar].members[0].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = state->results[vec].members[i].value.f * state->results[scalar].members[0].value.f;
}
void spvm_execute_OpMatrixTimesScalar(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word mat = SPVM_READ_WORD(state->code_current);
	spvm_word scalar = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	spvm_word s1 = state->results[mat].member_count;
	spvm_word s2 = state->results[mat].members[0].member_count;
	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < s1; i++)
			for (spvm_word j = 0; j < s2; j++)
				state->results[id].members[i].members[j].value.d = state->results[mat].members[i].members[j].value.d * state->results[scalar].members[0].value.d;
	else
		for (spvm_word i = 0; i < s1; i++)
			for (spvm_word j = 0; j < s2; j++)
				state->results[id].members[i].members[j].value.f = state->results[mat].members[i].members[j].value.f * state->results[scalar].members[0].value.f;
}
void spvm_execute_OpVectorTimesMatrix(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);
	spvm_word mat = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	spvm_word resSize = state->results[id].member_count;
	spvm_word mySize = state->results[vec].member_count;
	if (type_info->value_bitmask > 32) {
		for (spvm_word i = 0; i < resSize; i++) {
			double res = 0.0;

			for (spvm_word j = 0; j < mySize; j++)
				res += state->results[mat].members[i].members[j].value.d * state->results[vec].members[j].value.d;

			state->results[id].members[i].value.d = res;
		}
	} else {
		for (spvm_word i = 0; i < resSize; i++) {
			float res = 0.0f;

			for (spvm_word j = 0; j < mySize; j++)
				res += state->results[mat].members[i].members[j].value.f * state->results[vec].members[j].value.f;

			state->results[id].members[i].value.f = res;
		}
	}
}
void spvm_execute_OpMatrixTimesVector(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word mat = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	spvm_word resSize = state->results[id].member_count;
	spvm_word mySize = state->results[vec].member_count;
	if (type_info->value_bitmask > 32) {
		for (spvm_word i = 0; i < resSize; i++) {
			double res = 0.0;

			for (spvm_word j = 0; j < mySize; j++)
				res += state->results[mat].members[j].members[i].value.d * state->results[vec].members[j].value.d;

			state->results[id].members[i].value.d = res;
		}
	} else {
		for (spvm_word i = 0; i < resSize; i++) {
			float res = 0.0f;

			for (spvm_word j = 0; j < mySize; j++)
				res += state->results[mat].members[j].members[i].value.f * state->results[vec].members[j].value.f;

			state->results[id].members[i].value.f = res;
		}
	}
}
void spvm_execute_OpMatrixTimesMatrix(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word left = SPVM_READ_WORD(state->code_current);
	spvm_word right = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	spvm_word s1 = state->results[id].member_count;
	spvm_word s2 = state->results[id].members[0].member_count;
	if (type_info->value_bitmask > 32) {
		for (spvm_word i = 0; i < s1; i++)
			for (spvm_word j = 0; j < s2; j++)
				state->results[id].members[i].members[j].value.d = state->results[left].members[i].members[j].value.d * state->results[right].members[i].members[j].value.d;
	} else {
		for (spvm_word i = 0; i < s1; i++)
			for (spvm_word j = 0; j < s2; j++)
				state->results[id].members[i].members[j].value.f = state->results[left].members[i].members[j].value.f * state->results[right].members[i].members[j].value.f;
	}
}
void spvm_execute_OpOuterProduct(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec1 = SPVM_READ_WORD(state->code_current);
	spvm_word vec2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	spvm_word s1 = state->results[id].member_count;
	spvm_word s2 = state->results[id].members[0].member_count;
	if (type_info->value_bitmask > 32) {
		for (spvm_word i = 0; i < s1; i++)
			for (spvm_word j = 0; j < s2; j++)
				state->results[id].members[i].members[j].value.d = state->results[vec1].members[i].value.d * state->results[vec2].members[j].value.d;
	} else {
		for (spvm_word i = 0; i < s1; i++)
			for (spvm_word j = 0; j < s2; j++)
				state->results[id].members[i].members[j].value.f = state->results[vec1].members[i].value.f * state->results[vec2].members[j].value.f;
	}
}
void spvm_execute_OpDot(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec1 = SPVM_READ_WORD(state->code_current);
	spvm_word vec2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32) {
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d += state->results[vec1].members[i].value.d * state->results[vec2].members[i].value.d;
	} else {
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f += state->results[vec1].members[i].value.f * state->results[vec2].members[i].value.f;
	}
}

/* 3.32.14 Bit Instructions */
void spvm_execute_OpBitwiseOr(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 | state->results[op2].members[i].value.u64;
}
void spvm_execute_OpBitwiseAnd(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 & state->results[op2].members[i].value.u64;
}
void spvm_execute_OpBitwiseXor(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 ^ state->results[op2].members[i].value.u64;
}
void spvm_execute_OpNot(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = ~state->results[op1].members[i].value.u64;
}

/* 3.32.15 Relational and Logical Instructions */
void spvm_execute_OpAny(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);

	spvm_byte result = 0;
	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		if (state->results[vec].members[i].value.b)
			result = 1;
	state->results[id].members[0].value.b = result;
}
void spvm_execute_OpAll(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);

	spvm_byte result = 1;
	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		if (!state->results[vec].members[i].value.b)
			result = 0;
	state->results[id].members[0].value.b = result;
}
void spvm_execute_OpIsNan(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = isnan(state->results[x].members[i].value.f);
}
void spvm_execute_OpIsInf(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word x = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = isinf(state->results[x].members[i].value.f);
}
void spvm_execute_OpLogicalEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.b == state->results[op2].members[i].value.b;
}
void spvm_execute_OpLogicalNotEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.b != state->results[op2].members[i].value.b;
}
void spvm_execute_OpLogicalAnd(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.b && state->results[op2].members[i].value.b;
}
void spvm_execute_OpLogicalOr(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.b || state->results[op2].members[i].value.b;
}
void spvm_execute_OpLogicalNot(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = !state->results[op1].members[i].value.b;
}
void spvm_execute_OpIEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s == state->results[op2].members[i].value.s;
}
void spvm_execute_OpINotEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s != state->results[op2].members[i].value.s;
}
void spvm_execute_OpUGreaterThan(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.u64 > state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSGreaterThan(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s > state->results[op2].members[i].value.s;
}
void spvm_execute_OpUGreaterThanEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.u64 >= state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSGreaterThanEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s >= state->results[op2].members[i].value.s;
}
void spvm_execute_OpULessThan(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.u64 < state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSLessThan(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s < state->results[op2].members[i].value.s;
}
void spvm_execute_OpULessThanEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.u64 <= state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSLessThanEqual(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s <= state->results[op2].members[i].value.s;
}
void spvm_execute_OpFOrdEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.d == state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.f == state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdNotEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.d != state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.f != state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdLessThan(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.d < state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.f < state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdGreaterThan(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.d > state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.f > state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdLessThanEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.d <= state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.f <= state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdGreaterThanEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitmask > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.d >= state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.f >= state->results[op2].members[i].value.f;
}

/* 3.32.17 Control-Flow Instructions */
void spvm_execute_OpBranch(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->code_current = state->results[id].source_location;

	state->did_jump = 1;
}
void spvm_execute_OpBranchConditional(spvm_word word_count, spvm_state_t state)
{
	spvm_word cond = SPVM_READ_WORD(state->code_current);
	spvm_word true_branch = SPVM_READ_WORD(state->code_current);
	spvm_word false_branch = SPVM_READ_WORD(state->code_current);

	if (state->results[cond].members[0].value.b)
		state->code_current = state->results[true_branch].source_location;
	else
		state->code_current = state->results[false_branch].source_location;

	state->did_jump = 1;
}
void spvm_execute_OpSwitch(spvm_word word_count, spvm_state_t state)
{
	spvm_word sel = SPVM_READ_WORD(state->code_current);
	spvm_word def_lbl = SPVM_READ_WORD(state->code_current);

	spvm_word case_count = (word_count - 2) / 2;

	spvm_word val = state->results[sel].members[0].value.s;

	spvm_byte found = 0;
	for (spvm_word i = 0; i < case_count; i++) {
		spvm_word lit = SPVM_READ_WORD(state->code_current);
		spvm_word lbl = SPVM_READ_WORD(state->code_current);

		if (val == lit) {
			state->code_current = state->results[lbl].source_location;
			found = 1;
			break;
		}
	}

	if (!found)
		state->code_current = state->results[def_lbl].source_location;

	state->did_jump = 1;
}
void spvm_execute_OpKill(spvm_word word_count, spvm_state_t state)
{
	state->code_current = NULL;
	state->did_jump = 1;
	state->discarded = 1;
}

void _spvm_context_create_execute_table(spvm_context_t ctx)
{
	ctx->opcode_execute = (spvm_opcode_func*)calloc(SPVM_OPCODE_TABLE_LENGTH, sizeof(spvm_opcode_func));

	ctx->opcode_execute[SpvOpLine] = spvm_execute_OpLine;
	ctx->opcode_execute[SpvOpNoLine] = spvm_execute_OpNoLine;

	ctx->opcode_execute[SpvOpStore] = spvm_execute_OpStore;
	ctx->opcode_execute[SpvOpLoad] = spvm_execute_OpLoad;
	ctx->opcode_execute[SpvOpCopyMemory] = spvm_execute_OpCopyMemory;
	ctx->opcode_execute[SpvOpCopyMemorySized] = spvm_execute_OpCopyMemorySized;
	ctx->opcode_execute[SpvOpAccessChain] = spvm_execute_OpAccessChain;
	ctx->opcode_execute[SpvOpPtrEqual] = spvm_execute_OpPtrEqual;
	ctx->opcode_execute[SpvOpPtrNotEqual] = spvm_execute_OpPtrNotEqual;
	ctx->opcode_execute[SpvOpPtrDiff] = spvm_execute_OpPtrNotEqual;
	ctx->opcode_execute[SpvOpFunctionCall] = spvm_execute_OpFunctionCall;
	ctx->opcode_execute[SpvOpReturn] = spvm_execute_OpReturn;
	ctx->opcode_execute[SpvOpReturnValue] = spvm_execute_OpReturnValue;

	ctx->opcode_execute[SpvOpExtInst] = spvm_execute_OpExtInst;
	
	ctx->opcode_execute[SpvOpConvertFToU] = spvm_execute_OpConvertFToU;
	ctx->opcode_execute[SpvOpConvertFToS] = spvm_execute_OpConvertFToS;
	ctx->opcode_execute[SpvOpConvertUToF] = spvm_execute_OpConvertUToF;
	ctx->opcode_execute[SpvOpConvertSToF] = spvm_execute_OpConvertSToF;
	ctx->opcode_execute[SpvOpFConvert] = spvm_execute_OpFConvert;
	ctx->opcode_execute[SpvOpBitcast] = spvm_execute_OpBitcast;

	ctx->opcode_execute[SpvOpVectorExtractDynamic] = spvm_execute_OpVectorExtractDynamic;
	ctx->opcode_execute[SpvOpVectorInsertDynamic] = spvm_execute_OpVectorInsertDynamic;
	ctx->opcode_execute[SpvOpVectorShuffle] = spvm_execute_OpVectorShuffle;
	ctx->opcode_execute[SpvOpCompositeConstruct] = spvm_execute_OpCompositeConstruct;
	ctx->opcode_execute[SpvOpCompositeExtract] = spvm_execute_OpCompositeExtract;
	ctx->opcode_execute[SpvOpCopyObject] = spvm_execute_OpCopyObject;
	ctx->opcode_execute[SpvOpTranspose] = spvm_execute_OpTranspose;

	ctx->opcode_execute[SpvOpSNegate] = spvm_execute_OpSNegate;
	ctx->opcode_execute[SpvOpFNegate] = spvm_execute_OpFNegate;
	ctx->opcode_execute[SpvOpIAdd] = spvm_execute_OpIAdd;
	ctx->opcode_execute[SpvOpFAdd] = spvm_execute_OpFAdd;
	ctx->opcode_execute[SpvOpISub] = spvm_execute_OpISub;
	ctx->opcode_execute[SpvOpFSub] = spvm_execute_OpFSub;
	ctx->opcode_execute[SpvOpIMul] = spvm_execute_OpIMul;
	ctx->opcode_execute[SpvOpFMul] = spvm_execute_OpFMul;
	ctx->opcode_execute[SpvOpUDiv] = spvm_execute_OpUDiv;
	ctx->opcode_execute[SpvOpSDiv] = spvm_execute_OpSDiv;
	ctx->opcode_execute[SpvOpFDiv] = spvm_execute_OpFDiv;
	ctx->opcode_execute[SpvOpUMod] = spvm_execute_OpUMod;
	ctx->opcode_execute[SpvOpSMod] = spvm_execute_OpSMod;
	ctx->opcode_execute[SpvOpFMod] = spvm_execute_OpFMod;
	ctx->opcode_execute[SpvOpVectorTimesScalar] = spvm_execute_OpVectorTimesScalar;
	ctx->opcode_execute[SpvOpMatrixTimesScalar] = spvm_execute_OpMatrixTimesScalar;
	ctx->opcode_execute[SpvOpVectorTimesMatrix] = spvm_execute_OpVectorTimesMatrix;
	ctx->opcode_execute[SpvOpMatrixTimesVector] = spvm_execute_OpMatrixTimesVector;
	ctx->opcode_execute[SpvOpMatrixTimesMatrix] = spvm_execute_OpMatrixTimesMatrix;
	ctx->opcode_execute[SpvOpOuterProduct] = spvm_execute_OpOuterProduct;
	ctx->opcode_execute[SpvOpDot] = spvm_execute_OpDot;

	ctx->opcode_execute[SpvOpBitwiseOr] = spvm_execute_OpBitwiseOr;
	ctx->opcode_execute[SpvOpBitwiseAnd] = spvm_execute_OpBitwiseAnd;
	ctx->opcode_execute[SpvOpBitwiseXor] = spvm_execute_OpBitwiseXor;
	ctx->opcode_execute[SpvOpNot] = spvm_execute_OpNot;

	ctx->opcode_execute[SpvOpAny] = spvm_execute_OpAny;
	ctx->opcode_execute[SpvOpAll] = spvm_execute_OpAll;
	ctx->opcode_execute[SpvOpIsNan] = spvm_execute_OpIsNan;
	ctx->opcode_execute[SpvOpIsInf] = spvm_execute_OpIsInf;
	ctx->opcode_execute[SpvOpLogicalEqual] = spvm_execute_OpLogicalEqual;
	ctx->opcode_execute[SpvOpLogicalNotEqual] = spvm_execute_OpLogicalNotEqual;
	ctx->opcode_execute[SpvOpLogicalAnd] = spvm_execute_OpLogicalAnd;
	ctx->opcode_execute[SpvOpLogicalOr] = spvm_execute_OpLogicalOr;
	ctx->opcode_execute[SpvOpLogicalNot] = spvm_execute_OpLogicalNot;
	ctx->opcode_execute[SpvOpIEqual] = spvm_execute_OpIEqual;
	ctx->opcode_execute[SpvOpINotEqual] = spvm_execute_OpINotEqual;
	ctx->opcode_execute[SpvOpUGreaterThan] = spvm_execute_OpUGreaterThan;
	ctx->opcode_execute[SpvOpSGreaterThan] = spvm_execute_OpSGreaterThan;
	ctx->opcode_execute[SpvOpUGreaterThanEqual] = spvm_execute_OpUGreaterThanEqual;
	ctx->opcode_execute[SpvOpSGreaterThanEqual] = spvm_execute_OpSGreaterThanEqual;
	ctx->opcode_execute[SpvOpULessThan] = spvm_execute_OpULessThan;
	ctx->opcode_execute[SpvOpSLessThan] = spvm_execute_OpSLessThan;
	ctx->opcode_execute[SpvOpULessThanEqual] = spvm_execute_OpULessThanEqual;
	ctx->opcode_execute[SpvOpSLessThanEqual] = spvm_execute_OpSLessThanEqual;
	ctx->opcode_execute[SpvOpFOrdEqual] = spvm_execute_OpFOrdEqual;
	ctx->opcode_execute[SpvOpFOrdNotEqual] = spvm_execute_OpFOrdNotEqual;
	ctx->opcode_execute[SpvOpFOrdLessThan] = spvm_execute_OpFOrdLessThan;
	ctx->opcode_execute[SpvOpFOrdGreaterThan] = spvm_execute_OpFOrdGreaterThan;
	ctx->opcode_execute[SpvOpFOrdLessThanEqual] = spvm_execute_OpFOrdLessThanEqual;
	ctx->opcode_execute[SpvOpFOrdGreaterThanEqual] = spvm_execute_OpFOrdGreaterThanEqual;

	ctx->opcode_execute[SpvOpImageSampleImplicitLod] = spvm_execute_OpImageSampleImplicitLod;

	ctx->opcode_execute[SpvOpBranch] = spvm_execute_OpBranch;
	ctx->opcode_execute[SpvOpBranchConditional] = spvm_execute_OpBranchConditional;
	ctx->opcode_execute[SpvOpSwitch] = spvm_execute_OpSwitch;
	ctx->opcode_execute[SpvOpKill] = spvm_execute_OpKill;
}