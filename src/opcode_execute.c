#include <spvm/context.h>
#include <spvm/opcode.h>
#include <spvm/state.h>
#include <spvm/spirv.h>
#include <spvm/image.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <math.h>

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

	if (state->results[set].extension && state->results[set].extension[inst])
		state->results[set].extension[inst](type, id, word_count - 4, state);
}

/* 3.32.8 Memory Instructions */
void spvm_execute_OpStore(spvm_word word_count, spvm_state_t state)
{
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);
	spvm_word val_id = SPVM_READ_WORD(state->code_current);

	spvm_member_memcpy(state->results[ptr_id].members, state->results[val_id].members, state->results[ptr_id].member_count);

	if (state->results[ptr_id].storage_class == SpvStorageClassWorkgroup && state->owner->write_workgroup_memory)
		state->owner->write_workgroup_memory(state, ptr_id, val_id);
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
	spvm_source source_pointer = state->code_current;

	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word value_id = SPVM_READ_WORD(state->code_current);

	spvm_word index_count = word_count - 4;

	state->results[id].type = spvm_result_type_access_chain;
	state->results[id].pointer = var_type;
	state->results[id].storage_class = state->results[value_id].storage_class;
	state->results[id].source_location = source_pointer;
	state->results[id].source_word_count = word_count;

	spvm_word index_id = SPVM_READ_WORD(state->code_current);
	spvm_word index = state->results[index_id].members[0].value.s;

	spvm_member_t result = state->results[value_id].members + SPVM_MIN(index, state->results[value_id].member_count - 1);

	while (index_count) {
		index_id = SPVM_READ_WORD(state->code_current);
		index = state->results[index_id].members[0].value.s;

		result = result->members + SPVM_MIN(index, result->member_count - 1);

		index_count--;
	}

	if (result->member_count != 0) {
		state->results[id].member_count = result->member_count;
		state->results[id].members = result->members;
	} else {
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
typedef struct {
	spvm_word bits;

	float bias;
	float lod;
	float dx[3];
	float dy[3];
	char uses_offsets;
	int offsets[4][3];
	int sample;
	float min_lod;
} spvm_image_operands;
void spvm_image_operands_parse(spvm_image_operands* info, spvm_state_t state)
{
	spvm_word bits = info->bits = SPVM_READ_WORD(state->code_current);
	if (bits & SpvImageOperandsBiasMask)
		info->bias = state->results[SPVM_READ_WORD(state->code_current)].members[0].value.f;
	if (bits & SpvImageOperandsLodMask)
		info->lod = state->results[SPVM_READ_WORD(state->code_current)].members[0].value.f;
	if (bits & SpvImageOperandsGradMask) {
		spvm_result_t dx = &state->results[SPVM_READ_WORD(state->code_current)];
		spvm_result_t dy = &state->results[SPVM_READ_WORD(state->code_current)];
		for (spvm_word i = 0; i < dx->member_count; i++)
			info->dx[i] = dx->members[i].value.f;
		for (spvm_word i = 0; i < dy->member_count; i++)
			info->dy[i] = dy->members[i].value.f;
	}
	if (bits & SpvImageOperandsConstOffsetMask) {
		spvm_result_t offs = &state->results[SPVM_READ_WORD(state->code_current)];
		for (spvm_word i = 0; i < offs->member_count; i++)
			info->offsets[0][i] = offs->members[i].value.s;
	}
	if (bits & SpvImageOperandsOffsetMask) {
		spvm_result_t offs = &state->results[SPVM_READ_WORD(state->code_current)];
		for (spvm_word i = 0; i < offs->member_count; i++)
			info->offsets[0][i] = offs->members[i].value.s;
	}
	if (bits & SpvImageOperandsConstOffsetsMask) {
		spvm_result_t offs = &state->results[SPVM_READ_WORD(state->code_current)];
		for (spvm_word i = 0; i < offs->member_count; i++)
			for (spvm_word j = 0; j < offs->member_count; j++)
				info->offsets[i][j] = offs->members[i].members[j].value.s;
		info->uses_offsets = 1;
	}
	if (bits & SpvImageOperandsSampleMask)
		info->sample = state->results[SPVM_READ_WORD(state->code_current)].members[0].value.s;
	if (bits & SpvImageOperandsMinLodMask)
		info->min_lod = state->results[SPVM_READ_WORD(state->code_current)].members[0].value.f;
}

// Returns the layer to be sampled
float process_cubemap_normal(float* nrml)
{
	/* https://github.com/zauonlok/renderer/blob/a6b50f06e85659edbc8ee18c674e6f781aacfd84/renderer/core/texture.c#L210 */
	/* https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/chap16.html#_cube_map_face_selection */
	float absX = (float)fabs(nrml[0]);
	float absY = (float)fabs(nrml[1]);
	float absZ = (float)fabs(nrml[2]);
	float ma, layer;

	if (absX > absY && absX > absZ) {
		ma = absX;
		if (nrml[0] > 0) {
			nrml[0] = -nrml[2];
			nrml[1] = -nrml[1];
			layer = 0.0f;
		} else {
			nrml[0] = +nrml[2];
			nrml[1] = -nrml[1];
			layer = 1.0f;
		}
	} else if (absY > absZ) {
		ma = absY;
		if (nrml[1] > 0) {
			nrml[0] = +nrml[0];
			nrml[1] = +nrml[2];
			layer = 2.0f;
		} else {
			nrml[0] = +nrml[0];
			nrml[1] = -nrml[2];
			layer = 3.0f;
		}
	} else {
		ma = absZ;
		if (nrml[2] > 0) {
			nrml[0] = +nrml[0];
			nrml[1] = -nrml[1];
			layer = 4.0f;
		} else {
			nrml[0] = -nrml[0];
			nrml[1] = -nrml[1];
			layer = 5.0f;
		}
	}

	nrml[0] = (nrml[0] / ma + 1) / 2;
	nrml[1] = (nrml[1] / ma + 1) / 2;
	return layer;
}

float spvm_execute_compare(float val, float ref, spvm_sampler_compare_op op) {
	switch(op) {
		case spvm_sampler_compare_op_never: return 0.f;
		case spvm_sampler_compare_op_always: return 1.f;
		case spvm_sampler_compare_op_equal:
			return (ref == val) ? 1.f : 0.f;
		case spvm_sampler_compare_op_not_equal:
			return (ref != val) ? 1.f : 0.f;
		case spvm_sampler_compare_op_greater:
			return (ref > val) ? 1.f : 0.f;
		case spvm_sampler_compare_op_less:
			return (ref < val) ? 1.f : 0.f;
		case spvm_sampler_compare_op_greater_or_equal:
			return (ref >= val) ? 1.f : 0.f;
		case spvm_sampler_compare_op_less_or_equal:
			return (ref <= val) ? 1.f : 0.f;
		default:
			assert(!"Invalid sampler compare op");
			return 0.f;
	}
}

static spvm_vec4f spvm_op_image_sample(spvm_state_t state,
		spvm_word sampled_image_id, spvm_word coord_id,
		spvm_image_operands* operands, const float* dref, bool proj)
{
	spvm_result_t coord = &state->results[coord_id];

	spvm_result_t container = &state->results[sampled_image_id];
	assert(container->member_count == 2u);
	spvm_image* image = container->members[0].value.image;
	assert(image);
	spvm_sampler* sampler = container->members[1].value.sampler;
	assert(sampler);

	spvm_result_t sampled_image_type = spvm_state_get_type_info(state->results, &state->results[container->pointer]);
	assert(sampled_image_type);
	assert(sampled_image_type->value_type == spvm_value_type_sampled_image);

	spvm_result_t image_type = spvm_state_get_type_info(state->results, &state->results[sampled_image_type->pointer]);
	assert(image_type);
	assert(image_type->value_type == spvm_value_type_image);
	assert(image_type->image_info);

	float stu[4] = { 0.0f };
	for (spvm_word i = 0; i < coord->member_count; i++) {
		stu[i] = coord->members[i].value.f;
	}

	spvm_vec4f res = {0};

	unsigned dim;
	if (image_type->image_info->dim == SpvDim1D) {
		dim = 1u;
	} else if (image_type->image_info->dim == SpvDim2D ||
			image_type->image_info->dim == SpvDimSubpassData) {
		dim = 2u;
	} else if (image_type->image_info->dim == SpvDim3D) {
		dim = 3u;
	} else if (image_type->image_info->dim == SpvDimCube) {
		dim = 3u;
	} else {
		assert(!"Unhandled SpvDim");
		dim = 3u;
	}

	assert(!image_type->image_info->arrayed || !proj); // per spec

	float layer = 0.f;
	if (image_type->image_info->arrayed) {
		layer = stu[dim];

		if (image_type->image_info->dim == SpvDimCube) {
			layer *= 6;
		}
	}

	if (proj) {
		for (unsigned i = 0u; i < dim; ++i) {
			stu[i] /= stu[dim];
		}
	}

	if (!operands->uses_offsets) {
		stu[0] += operands->offsets[0][0] / (float)image->width;
		stu[1] += operands->offsets[0][1] / (float)image->height;
		stu[2] += operands->offsets[0][2] / (float)image->depth;
	}

	if (image_type->image_info->dim == SpvDimCube) {
		layer += process_cubemap_normal(&stu[0]);
	}

	float level = 0.f;
	if (operands->bits & SpvImageOperandsLodMask) {
		level = operands->lod;
	} else if (operands->bits & SpvImageOperandsGradMask) {
		// TODO: would need to transform grad for cubemap
		level = spvm_sampled_image_query_lod_from_grad(state, image, image_type->image_info,
			sampler, operands->dx, operands->dy, operands->bias, operands->min_lod).dl;
	} else {
		level = spvm_sampled_image_query_lod(state, image, image_type->image_info,
			sampler, coord_id, operands->bias, operands->min_lod).dl;
	}

	spvm_vec4f ret = spvm_sampled_image_sample(state, image, sampler,
		stu[0], stu[1], stu[2], layer, level);

	if (dref) {
		ret.data[0] = spvm_execute_compare(ret.data[0], *dref, sampler->desc.compare_op);
	}

	return ret;
}

static spvm_vec4f spvm_op_image_gather(spvm_state_t state,
		spvm_word sampled_image_id, spvm_word coord_id, int comp,
		spvm_image_operands* operands, const float* dref)
{
	spvm_result_t coord = &state->results[coord_id];

	spvm_result_t container = &state->results[sampled_image_id];
	assert(container->member_count == 2u);
	spvm_image* image = container->members[0].value.image;
	assert(image);
	spvm_sampler* sampler = container->members[1].value.sampler;
	assert(sampler);

	spvm_result_t sampled_image_type = spvm_state_get_type_info(state->results, &state->results[container->pointer]);
	assert(sampled_image_type);
	assert(sampled_image_type->value_type == spvm_value_type_sampled_image);

	spvm_result_t image_type = spvm_state_get_type_info(state->results, &state->results[sampled_image_type->pointer]);
	assert(image_type);
	assert(image_type->value_type == spvm_value_type_image);
	assert(image_type->image_info);

	float stu[4] = { 0.0f };
	for (spvm_word i = 0; i < coord->member_count; i++) {
		stu[i] = coord->members[i].value.f;
	}

	spvm_vec4f res = {0};

	unsigned dim;
	if (image_type->image_info->dim == SpvDim1D) {
		dim = 1u;
	} else if (image_type->image_info->dim == SpvDim2D ||
			image_type->image_info->dim == SpvDimSubpassData) {
		dim = 2u;
	} else if (image_type->image_info->dim == SpvDim3D) {
		dim = 3u;
	} else if (image_type->image_info->dim == SpvDimCube) {
		dim = 3u;
	} else {
		assert(!"Unhandled SpvDim");
		dim = 3u;
	}

	float layer = 0.f;
	if (image_type->image_info->arrayed) {
		layer = stu[dim];

		if (image_type->image_info->dim == SpvDimCube) {
			layer *= 6;
		}
	}

	int offs[4][2] = {
		{ 0, 1 },
		{ 1, 1 },
		{ 1, 0 },
		{ 0, 0 }
	};

	if (!operands->uses_offsets) {
		stu[0] += operands->offsets[0][0] / (float)image->width;
		stu[1] += operands->offsets[0][1] / (float)image->height;
		stu[2] += operands->offsets[0][2] / (float)image->depth;
	} else {
		for (int i = 0; i < 4; i++) {
			offs[i][0] = operands->offsets[i][0];
			offs[i][1] = operands->offsets[i][1];
		}
	}

	if (image_type->image_info->dim == SpvDimCube) {
		layer += process_cubemap_normal(&stu[0]);
	}

	// Implicit sampling not allowed for gather. The base level
	// is used by default.
	float level = 0.f;
	if (operands->bits & SpvImageOperandsLodMask) {
		level = operands->lod;
	}

	spvm_vec4f ret;
	for (int i = 0; i < 4; i++) {
		spvm_vec4f sample = spvm_fetch_texel(state, image, &sampler->desc,
			stu[0] + (offs[i][0] / (float)image->width),
			stu[1] + (offs[i][1] / (float)image->height),
			stu[2], layer, level);
		ret.data[i] = sample.data[comp];

		// comparison is per gather operation
		if (dref) {
			ret.data[i] = spvm_execute_compare(ret.data[i], *dref, sampler->desc.compare_op);
		}
	}

	return ret;
}

// same implementation for ImplicitLod and ExplicitLod
void spvm_execute_OpImageSample_base(spvm_word word_count, spvm_state_t state, bool proj)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word sampled_image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);

	spvm_image_operands operands = {0};
	if (word_count > 4) {
		spvm_image_operands_parse(&operands, state);
	}

	spvm_vec4f res = spvm_op_image_sample(state, sampled_image_id,
		coord_id, &operands, NULL, proj);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = res.data[i];
}

void spvm_execute_OpImageSample(spvm_word word_count, spvm_state_t state)
{
	spvm_execute_OpImageSample_base(word_count, state, false);
}

void spvm_execute_OpImageSampleProj(spvm_word word_count, spvm_state_t state)
{
	spvm_execute_OpImageSample_base(word_count, state, true);
}

void spvm_execute_OpImageSampleDref_base(spvm_word word_count, spvm_state_t state,
		bool proj)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word sampled_image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);
	spvm_word dref_id = SPVM_READ_WORD(state->code_current);

	spvm_image_operands operands = {0};
	if (word_count > 4) {
		spvm_image_operands_parse(&operands, state);
	}

	spvm_result_t dref = &state->results[dref_id];
	float dref_val = dref->members[0].value.f;

	spvm_vec4f res = spvm_op_image_sample(state, sampled_image_id,
		coord_id, &operands, &dref_val, proj);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.f = res.data[i];
	}
}

void spvm_execute_OpImageSampleDref(spvm_word word_count, spvm_state_t state)
{
	spvm_execute_OpImageSampleDref_base(word_count, state, false);
}

void spvm_execute_OpImageSampleProjDref(spvm_word word_count, spvm_state_t state)
{
	spvm_execute_OpImageSampleDref_base(word_count, state, true);
}

void spvm_execute_OpSampledImage(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);
	spvm_word sampler_id = SPVM_READ_WORD(state->code_current);

	assert(state->results[id].member_count == 2u);
	state->results[id].members[0].value.image = state->results[image_id].members[0].value.image;
	state->results[id].members[1].value.sampler = state->results[sampler_id].members[0].value.sampler;
}

void spvm_execute_OpImageFetch(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);

	spvm_image_operands operands = {0};
	if (word_count > 4) {
		spvm_image_operands_parse(&operands, state);
	}

	spvm_result_t coord = &state->results[coord_id];

	spvm_result_t image_container = &state->results[image_id];
	spvm_image* image = image_container->members[0].value.image;

	int stu[4] = { 0 };
	for (spvm_word i = 0; i < coord->member_count; i++)
		stu[i] = coord->members[i].value.s + operands.offsets[0][i];

	int level = 0u;
	if (operands.bits & SpvImageOperandsLodMask)
		level = operands.lod;

	spvm_vec4f px = spvm_image_read(state, image, stu[0], stu[1], stu[2], stu[3], level);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = px.data[i];
}
void spvm_execute_OpImageGather(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word sampled_image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);
	spvm_word comp_id = SPVM_READ_WORD(state->code_current);

	spvm_image_operands operands = {0};
	if (word_count > 4) {
		spvm_image_operands_parse(&operands, state);
	}

	int comp = state->results[comp_id].members[0].value.s;
	if (state->analyzer && (comp < 0 || comp > 3)) {
		state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_image_gather_invalid_comp);
	}

	spvm_vec4f res = spvm_op_image_gather(state, sampled_image_id,
		coord_id, comp, &operands, NULL);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = res.data[i];
}
void spvm_execute_OpImageDrefGather(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word sampled_image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);
	spvm_word dref_id = SPVM_READ_WORD(state->code_current);

	spvm_image_operands operands = {0};
	if (word_count > 4) {
		spvm_image_operands_parse(&operands, state);
	}

	spvm_result_t dref = &state->results[dref_id];
	float dref_val = dref->members[0].value.f;

	const int comp = 0; // fixed for depth compare samplers
	spvm_vec4f res = spvm_op_image_gather(state, sampled_image_id,
		coord_id, comp, &operands, &dref_val);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = res.data[i];
}
void spvm_execute_OpImageQuerySizeLod(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);
	spvm_word lod_id = SPVM_READ_WORD(state->code_current);

	int lod = state->results[lod_id].members[0].value.s;
	spvm_image* img = state->results[image_id].members[0].value.image;

	// TODO: we just assume the lod sizes here, might be slightly different
	// (there are vulkan extensions allowing to round up lod sizes).
	// Should probably just be exposed as callback via state as well.
	// TODO: layer, check image type
	if (state->results[id].member_count > 0)
		state->results[id].members[0].value.s = SPVM_MAX(img->width >> lod, 1);
	if (state->results[id].member_count > 1)
		state->results[id].members[1].value.s = SPVM_MAX(img->height >> lod, 1);
	if (state->results[id].member_count > 2)
		state->results[id].members[2].value.s = SPVM_MAX(img->depth >> lod, 1);
}
void spvm_execute_OpImageQuerySize(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);

	spvm_image* img = state->results[image_id].members[0].value.image;

	// TODO: layer, check image type
	if (state->results[id].member_count > 0)
		state->results[id].members[0].value.s = img->width;
	if (state->results[id].member_count > 1)
		state->results[id].members[1].value.s = img->height;
	if (state->results[id].member_count > 2)
		state->results[id].members[2].value.s = img->depth;
}
void spvm_execute_OpImageQueryLod(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word sampled_image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);

	spvm_result_t container = &state->results[sampled_image_id];
	assert(container->member_count == 2u);
	spvm_image* image = container->members[0].value.image;
	assert(image);
	spvm_sampler* sampler = container->members[1].value.sampler;
	assert(sampler);

	spvm_result_t sampled_image_type = spvm_state_get_type_info(state->results, &state->results[container->pointer]);
	assert(sampled_image_type);
	spvm_result_t image_type = spvm_state_get_type_info(state->results, &state->results[sampled_image_type->pointer]);

	assert(image_type);
	assert(image_type->image_info);
	assert(image_type->value_type == spvm_value_type_sampled_image);

	struct spvm_sampled_image_lod_query lodq = spvm_sampled_image_query_lod(
		state, image, image_type->image_info, sampler, coord_id, 0.f, 0.f);

	// writeback
	assert(state->results[id].member_count == 2);

	// Per vulkan spec, those are the returned values
	state->results[id].members[0].value.f = lodq.lambda_prime;
	state->results[id].members[1].value.f = lodq.dl;
}
void spvm_execute_OpImageQueryLevels(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);

	assert(state->results[image_id].members);
	state->results[id].members[0].value.u = state->results[image_id].members[0].value.image->levels;
}
void spvm_execute_OpImageQuerySamples(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);

	assert(state->results[image_id].members);
	(void) image_id;

	// TODO: spvm doesn't support multisampling atm
	state->results[id].members[0].value.u = 1u;
}
void spvm_execute_OpImageRead(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);

	spvm_image* img = state->results[image_id].members[0].value.image;
	assert(img);

	spvm_result_t coord = &state->results[coord_id];
	spvm_result_t output = &state->results[id];

	int x = 0, y = 0, z = 0;

	x = coord->members[0].value.s;
	if (coord->member_count > 1)
		y = coord->members[1].value.s;
	if (coord->member_count > 2)
		z = coord->members[2].value.s;

	int level = 0;
	int layer = 0;

	// report undefined behavior
	if (state->analyzer) {
		if (x < 0 || y < 0 || z < 0 || x >= img->width || y >= img->height || z >= img->depth)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_image_read_out_of_bounds);
	}

	spvm_vec4f data = spvm_image_read(state, img, x, y, z, layer, level);

	for (int i = 0; i < output->member_count; i++)
		output->members[i].value.f = data.data[i];
}
void spvm_execute_OpImageWrite(spvm_word word_count, spvm_state_t state)
{
	spvm_word image_id = SPVM_READ_WORD(state->code_current);
	spvm_word coord_id = SPVM_READ_WORD(state->code_current);
	spvm_word texel_id = SPVM_READ_WORD(state->code_current);

	spvm_image* img = state->results[image_id].members[0].value.image;
	assert(img);

	spvm_result_t coord = &state->results[coord_id];
	spvm_result_t texel = &state->results[texel_id];

	int x = 0, y = 0, z = 0;

	x = coord->members[0].value.s;
	if (coord->member_count > 1)
		y = coord->members[1].value.s;
	if (coord->member_count > 2)
		z = coord->members[2].value.s;

	spvm_vec4f rgba;
	for (int i = 0; i < texel->member_count; i++)
		rgba.data[i] = texel->members[i].value.f;

	int level = 0;
	int layer = 0;

	// report undefined behavior
	if (state->analyzer) {
		if (x < 0 || y < 0 || z < 0 || x >= img->width || y >= img->height || z >= img->depth)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_image_read_out_of_bounds);
	}

	spvm_image_write(state, img, x, y, z, level, layer, &rgba);
}
void spvm_execute_OpImage(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word image_id = SPVM_READ_WORD(state->code_current);

	spvm_image* image = state->results[image_id].members[0].value.image;
	state->results[id].members[0].value.image = image;
}

/* 3.32.11 Conversion Instructions */
void spvm_execute_OpConvertFToU(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[state->results[val].pointer]);

	if (type_info->value_bitcount > 32)
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

	if (type_info->value_bitcount > 32)
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

	if (type_info->value_bitcount > 32)
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

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.d = (double)state->results[val].members[i].value.s;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.f = (float)state->results[val].members[i].value.s;
}
void spvm_execute_OpUConvert(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t res_type_info = spvm_state_get_type_info(state->results, &state->results[res_type]);

	unsigned long long mask = 0xFF;
	if (res_type_info->value_bitcount == 16)
		mask = 0xFFFF;
	else if (res_type_info->value_bitcount == 32)
		mask = 0xFFFFFFFF;

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[val].members[i].value.u64 & mask;
}
void spvm_execute_OpSConvert(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t res_type_info = spvm_state_get_type_info(state->results, &state->results[res_type]);

	int mask = 0xFF;
	if (res_type_info->value_bitcount == 16)
		mask = 0xFFFF;
	else if (res_type_info->value_bitcount == 32)
		mask = 0xFFFFFFFF;

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[val].members[i].value.s & mask;
}
void spvm_execute_OpFConvert(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	spvm_result_t res_type_info = spvm_state_get_type_info(state->results, &state->results[res_type]);
	spvm_result_t val_info = spvm_state_get_type_info(state->results, &state->results[state->results[val].pointer]);

	if (res_type_info->value_bitcount > val_info->value_bitcount)
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

	if (index < 0 || index >= state->results[vector].member_count)
		state->results[id].members[0].value.u64 = state->results[vector].members[index].value.u64;
	else if (state->analyzer)
		state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_vector_extract_dynamic);
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

	if (index < 0 || index >= state->results[id].member_count)
		state->results[id].members[index].value.u64 = state->results[comp].members[0].value.u64;
	else if (state->analyzer)
		state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_vector_insert_dynamic);
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
			spvm_member_memcpy(&state->results[id].members[i].members[0], &state->results[index].members[0], state->results[index].member_count);
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

	spvm_word memcount = 1;
	if (result->member_count) {
		memcount = result->member_count;
		result = result->members;
	}

	spvm_member_memcpy(state->results[id].members, result, memcount);
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

	if (type_info->value_bitcount > 32)
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
	if (type_info->value_bitcount > 32)
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
	if (type_info->value_bitcount > 32)
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
	if (type_info->value_bitcount > 32)
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

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 / state->results[op2].members[i].value.u64;

		if (state->analyzer && state->results[op2].members[i].value.u64 == 0)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_div_by_zero);
	}
}
void spvm_execute_OpSDiv(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s / state->results[op2].members[i].value.s;

		if (state->analyzer && state->results[op2].members[i].value.s == 0) // TODO: also op2 == -1, op1 == minimum representable value?
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_div_by_zero);
	}
}
void spvm_execute_OpFDiv(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	// TODO: is there a better way to do this?
	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.d = state->results[op1].members[i].value.d / state->results[op2].members[i].value.d;

			if (state->analyzer && state->results[op2].members[i].value.d == 0.0)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_div_by_zero);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			state->results[id].members[i].value.f = state->results[op1].members[i].value.f / state->results[op2].members[i].value.f;

			if (state->analyzer && state->results[op2].members[i].value.f == 0.0f)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_div_by_zero);
		}
}
void spvm_execute_OpUMod(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 % state->results[op2].members[i].value.u64;

		if (state->analyzer && state->results[op2].members[i].value.u64 == 0)
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_mod_by_zero);
	}
}
void spvm_execute_OpSMod(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s % state->results[op2].members[i].value.s;

		if (state->analyzer && state->results[op2].members[i].value.s == 0) // TODO: mod by -1, while op1 is min representable value for operands type
			state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_mod_by_zero);
	}
}
void spvm_execute_OpFMod(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	// TODO: is there a better way to do this?
	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			double op2_sign = SPVM_SIGN(state->results[op2].members[i].value.d);
			double op1_sign = SPVM_SIGN(state->results[op1].members[i].value.d);
			double sign = 1.0;
			if (op2_sign != op1_sign)
				sign = -1.0;

			state->results[id].members[i].value.d = sign * fmod(state->results[op1].members[i].value.d, state->results[op2].members[i].value.d);

			if (state->analyzer && state->results[op2].members[i].value.d == 0.0)
					state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_mod_by_zero);
		}
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++) {
			float op2_sign = SPVM_SIGN(state->results[op2].members[i].value.f);
			float op1_sign = SPVM_SIGN(state->results[op1].members[i].value.f);
			float sign = 1.0;
			if (op2_sign != op1_sign)
				sign = -1.0;

			state->results[id].members[i].value.f = sign * fmodf(state->results[op1].members[i].value.f, state->results[op2].members[i].value.f);

			if (state->analyzer && state->results[op2].members[i].value.f == 0.0f)
				state->analyzer->on_undefined_behavior(state, spvm_undefined_behavior_mod_by_zero);
		}
}
void spvm_execute_OpVectorTimesScalar(spvm_word word_count, spvm_state_t state)
{
	spvm_word type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);
	spvm_word scalar = SPVM_READ_WORD(state->code_current);

	spvm_result_t type_info = spvm_state_get_type_info(state->results, &state->results[type]);

	if (type_info->value_bitcount > 32)
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
	if (type_info->value_bitcount > 32)
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
	if (type_info->value_bitcount > 32) {
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
	if (type_info->value_bitcount > 32) {
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

	spvm_word s1 = state->results[left].member_count;
	spvm_word s2 = state->results[left].members[0].member_count;
	spvm_word s3 = state->results[right].member_count;

	if (type_info->value_bitcount > 32) {
		for (spvm_word i = 0; i < s3; i++) {
			for (spvm_word j = 0; j < s2; j++) {
				state->results[id].members[i].members[j].value.u64 = 0;
				for (spvm_word k = 0; k < s1; k++)
					state->results[id].members[i].members[j].value.d += state->results[left].members[k].members[j].value.d * state->results[right].members[i].members[k].value.d;
			}
		}
	}
	else {
		for (spvm_word i = 0; i < s3; i++) {
			for (spvm_word j = 0; j < s2; j++) {
				state->results[id].members[i].members[j].value.u64 = 0;
				for (spvm_word k = 0; k < s1; k++)
					state->results[id].members[i].members[j].value.f += state->results[left].members[k].members[j].value.f * state->results[right].members[i].members[k].value.f;
			}
		}
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
	if (type_info->value_bitcount > 32) {
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

	if (type_info->value_bitcount > 32) {
		double dot = 0.0;
		for (spvm_word i = 0; i < state->results[vec1].member_count; i++)
			dot += state->results[vec1].members[i].value.d * state->results[vec2].members[i].value.d;
		state->results[id].members[0].value.d = dot;
	} else {
		float dot = 0.0f;
		for (spvm_word i = 0; i < state->results[vec1].member_count; i++)
			dot += state->results[vec1].members[i].value.f * state->results[vec2].members[i].value.f;
		state->results[id].members[0].value.f = dot;
	}
}

/* 3.32.14 Bit Instructions */
void spvm_execute_OpShiftRightLogical(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word base = SPVM_READ_WORD(state->code_current);
	spvm_word shift = SPVM_READ_WORD(state->code_current);

	// TODO: call state->analyzer->on_defined_behavior when shift is >>> base

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[base].members[i].value.u64 >> state->results[shift].members[i].value.u64;
}
void spvm_execute_OpShiftRightArithmetic(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word base = SPVM_READ_WORD(state->code_current);
	spvm_word shift = SPVM_READ_WORD(state->code_current);

	// TODO: call state->analyzer->on_defined_behavior when shift is >>> base

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[base].members[i].value.s >> state->results[shift].members[i].value.s;
}
void spvm_execute_OpShiftLeftLogical(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word base = SPVM_READ_WORD(state->code_current);
	spvm_word shift = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[base].members[i].value.u64 << state->results[shift].members[i].value.u64;
}
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
void spvm_execute_OpSelect(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word cond = SPVM_READ_WORD(state->code_current);
	spvm_word obj1 = SPVM_READ_WORD(state->code_current);
	spvm_word obj2 = SPVM_READ_WORD(state->code_current);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		spvm_byte condValue = state->results[cond].members[0].value.b;
		spvm_word obj = condValue ? obj1 : obj2;
		if (state->results[id].members[i].member_count == 0)
			state->results[id].members[i].value.u64 = state->results[obj].members[i].value.u64;
		else
			spvm_member_memcpy(&state->results[id].members[i].members[0], &state->results[obj].members[i].members[0], state->results[obj].members[i].member_count);
	}
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

	if (type_info->value_bitcount > 32)
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

	if (type_info->value_bitcount > 32)
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

	if (type_info->value_bitcount > 32)
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

	if (type_info->value_bitcount > 32)
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

	if (type_info->value_bitcount > 32)
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

	if (type_info->value_bitcount > 32)
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.d >= state->results[op2].members[i].value.d;
	else
		for (spvm_word i = 0; i < state->results[id].member_count; i++)
			state->results[id].members[i].value.b = state->results[op1].members[i].value.f >= state->results[op2].members[i].value.f;
}

/* 3.32.16 Derivative Instructions */
void spvm_execute_OpDPdx(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word res_id = SPVM_READ_WORD(state->code_current);
	spvm_word P_id = SPVM_READ_WORD(state->code_current);

	int index = 0;

	spvm_state_ddx(state, P_id);

	for (spvm_word i = 0; i < state->results[res_id].member_count; i++) {
		if (state->results[res_id].members[i].member_count == 0) {
			state->results[res_id].members[i].value.f = state->derivative_buffer_x[index];
			index++;
		}
		else {
			for (spvm_word j = 0; j < state->results[res_id].members[i].member_count; j++)
				state->results[res_id].members[i].members[j].value.f = state->derivative_buffer_x[index];
			index += state->results[res_id].members[i].member_count;
		}
	}

	if (!state->_derivative_is_group_member)
		spvm_state_group_step(state);
}
void spvm_execute_OpDPdy(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word res_id = SPVM_READ_WORD(state->code_current);
	spvm_word P_id = SPVM_READ_WORD(state->code_current);

	int index = 0;

	spvm_state_ddy(state, P_id);

	for (spvm_word i = 0; i < state->results[res_id].member_count; i++) {
		if (state->results[res_id].members[i].member_count == 0) {
			state->results[res_id].members[i].value.f = state->derivative_buffer_y[index];
			index++;
		}
		else {
			for (spvm_word j = 0; j < state->results[res_id].members[i].member_count; j++)
				state->results[res_id].members[i].members[j].value.f = state->derivative_buffer_y[index];
			index += state->results[res_id].members[i].member_count;
		}
	}

	if (!state->_derivative_is_group_member)
		spvm_state_group_step(state);
}
void spvm_execute_OpFwidth(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word res_id = SPVM_READ_WORD(state->code_current);
	spvm_word P_id = SPVM_READ_WORD(state->code_current);

	int index = 0;

	spvm_state_ddx(state, P_id);
	spvm_state_ddy(state, P_id);

	for (spvm_word i = 0; i < state->results[res_id].member_count; i++) {
		if (state->results[res_id].members[i].member_count == 0) {
			state->results[res_id].members[i].value.f = fabsf(state->derivative_buffer_x[index]) + fabsf(state->derivative_buffer_y[index]);
			index++;
		}
		else {
			for (spvm_word j = 0; j < state->results[res_id].members[i].member_count; j++)
				state->results[res_id].members[i].members[j].value.f = fabsf(state->derivative_buffer_x[index]) + fabsf(state->derivative_buffer_y[index]);
			index += state->results[res_id].members[i].member_count;
		}
	}

	if (!state->_derivative_is_group_member)
		spvm_state_group_step(state);
}

/* 3.32.17 Control-Flow Instructions */
void spvm_execute_OpPhi(spvm_word word_count, spvm_state_t state)
{
	SPVM_SKIP_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	word_count = (word_count - 2) / 2;

	int found = 0;
	for (spvm_word i = 0; i < word_count; i++) {
		spvm_word variable = SPVM_READ_WORD(state->code_current);
		spvm_word parent = SPVM_READ_WORD(state->code_current);

		if (parent == state->function_stack_cfg_parent[state->function_stack_current]) {
			spvm_member_memcpy(state->results[id].members, state->results[variable].members, state->results[id].member_count);
			found = 1;
			break;
		}
	}
	assert(found);
}
void spvm_execute_OpLabel(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);

	// Make sure we do not overwrite parent block id if we encounter an explicit label after a jump
	if (state->function_stack_cfg[state->function_stack_current] != id) {
		state->function_stack_cfg_parent[state->function_stack_current] = state->function_stack_cfg[state->function_stack_current];
		state->function_stack_cfg[state->function_stack_current] = id;
	}
}
void spvm_execute_OpBranch(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->code_current = state->results[id].source_location;
	state->function_stack_cfg_parent[state->function_stack_current] = state->function_stack_cfg[state->function_stack_current];
	state->function_stack_cfg[state->function_stack_current] = id;

	state->did_jump = 1;
}
void spvm_execute_OpBranchConditional(spvm_word word_count, spvm_state_t state)
{
	spvm_word cond = SPVM_READ_WORD(state->code_current);
	spvm_word true_branch = SPVM_READ_WORD(state->code_current);
	spvm_word false_branch = SPVM_READ_WORD(state->code_current);

	state->function_stack_cfg_parent[state->function_stack_current] = state->function_stack_cfg[state->function_stack_current];
	if (state->results[cond].members[0].value.b) {
		state->code_current = state->results[true_branch].source_location;
		state->function_stack_cfg[state->function_stack_current] = true_branch;
	} else {
		state->code_current = state->results[false_branch].source_location;
		state->function_stack_cfg[state->function_stack_current] = false_branch;
	}

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
			state->function_stack_cfg_parent[state->function_stack_current] = state->function_stack_cfg[state->function_stack_current];
			state->function_stack_cfg[state->function_stack_current] = lbl;
			found = 1;
			break;
		}
	}

	if (!found) {
		state->code_current = state->results[def_lbl].source_location;
		state->function_stack_cfg_parent[state->function_stack_current] = state->function_stack_cfg[state->function_stack_current];
		state->function_stack_cfg[state->function_stack_current] = def_lbl;
	}

	state->did_jump = 1;
}
void spvm_execute_OpKill(spvm_word word_count, spvm_state_t state)
{
	state->code_current = NULL;
	state->did_jump = 1;
	state->discarded = 1;
}

/* 3.32.18 Atomic Instructions */
void spvm_execute_OpAtomicLoad(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicLoad, word_count, state);
}
void spvm_execute_OpAtomicStore(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicStore, word_count, state);
}
void spvm_execute_OpAtomicExchange(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicExchange, word_count, state);
}
void spvm_execute_OpAtomicCompareExchange(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicCompareExchange, word_count, state);
}
void spvm_execute_OpAtomicIIncrement(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicIIncrement, word_count, state);
}
void spvm_execute_OpAtomicIDecrement(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicIDecrement, word_count, state);
}
void spvm_execute_OpAtomicIAdd(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicIAdd, word_count, state);
}
void spvm_execute_OpAtomicISub(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicISub, word_count, state);
}
void spvm_execute_OpAtomicSMin(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicSMin, word_count, state);
}
void spvm_execute_OpAtomicUMin(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicUMin, word_count, state);
}
void spvm_execute_OpAtomicSMax(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicSMax, word_count, state);
}
void spvm_execute_OpAtomicUMax(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicUMax, word_count, state);
}
void spvm_execute_OpAtomicAnd(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicAnd, word_count, state);
}
void spvm_execute_OpAtomicOr(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicOr, word_count, state);
}
void spvm_execute_OpAtomicXor(spvm_word word_count, spvm_state_t state)
{
	if (state->owner->atomic_operation)
		state->owner->atomic_operation(SpvOpAtomicXor, word_count, state);
}

/* 3.32.19 Primitive Instructions */
void spvm_execute_OpEmitVertex(spvm_word word_count, spvm_state_t state)
{
	if (state->emit_vertex)
		state->emit_vertex(state, 0);
}
void spvm_execute_OpEndPrimitive(spvm_word word_count, spvm_state_t state)
{
	if (state->end_primitive)
		state->end_primitive(state, 0);
}
void spvm_execute_OpEmitStreamVertex(spvm_word word_count, spvm_state_t state)
{
	if (state->emit_vertex) {
		spvm_word stream_id = SPVM_READ_WORD(state->code_current);
		state->emit_vertex(state, state->results[stream_id].members[0].value.u);
	}
}
void spvm_execute_OpEndStreamPrimitive(spvm_word word_count, spvm_state_t state)
{
	if (state->end_primitive) {
		spvm_word stream_id = SPVM_READ_WORD(state->code_current);
		state->end_primitive(state, state->results[stream_id].members[0].value.u);
	}
}

/* 3.32.20 Barrier Instructions */
void spvm_execute_OpControlBarrier(spvm_word word_count, spvm_state_t state)
{
	if (state->control_barrier) {
		spvm_word execution = SPVM_READ_WORD(state->code_current);
		spvm_word memory = SPVM_READ_WORD(state->code_current);
		spvm_word semantics = SPVM_READ_WORD(state->code_current);
		state->control_barrier(state, execution, memory, semantics);
	}
}


void _spvm_context_create_execute_table(spvm_context_t ctx)
{
	ctx->opcode_execute = (spvm_opcode_func*)calloc(SPVM_OPCODE_TABLE_LENGTH, sizeof(spvm_opcode_func));

	ctx->opcode_execute[SpvOpLabel] = spvm_execute_OpLabel;
	ctx->opcode_execute[SpvOpPhi] = spvm_execute_OpPhi;

	ctx->opcode_execute[SpvOpLine] = spvm_execute_OpLine;
	ctx->opcode_execute[SpvOpNoLine] = spvm_execute_OpNoLine;

	ctx->opcode_execute[SpvOpStore] = spvm_execute_OpStore;
	ctx->opcode_execute[SpvOpLoad] = spvm_execute_OpLoad;
	ctx->opcode_execute[SpvOpCopyMemory] = spvm_execute_OpCopyMemory;
	ctx->opcode_execute[SpvOpCopyMemorySized] = spvm_execute_OpCopyMemorySized;
	ctx->opcode_execute[SpvOpAccessChain] = spvm_execute_OpAccessChain;
	ctx->opcode_execute[SpvOpPtrEqual] = spvm_execute_OpPtrEqual;
	ctx->opcode_execute[SpvOpPtrNotEqual] = spvm_execute_OpPtrNotEqual;
	ctx->opcode_execute[SpvOpPtrDiff] = NULL;
	ctx->opcode_execute[SpvOpFunctionCall] = spvm_execute_OpFunctionCall;
	ctx->opcode_execute[SpvOpReturn] = spvm_execute_OpReturn;
	ctx->opcode_execute[SpvOpReturnValue] = spvm_execute_OpReturnValue;

	ctx->opcode_execute[SpvOpExtInst] = spvm_execute_OpExtInst;

	ctx->opcode_execute[SpvOpConvertFToU] = spvm_execute_OpConvertFToU;
	ctx->opcode_execute[SpvOpConvertFToS] = spvm_execute_OpConvertFToS;
	ctx->opcode_execute[SpvOpConvertUToF] = spvm_execute_OpConvertUToF;
	ctx->opcode_execute[SpvOpConvertSToF] = spvm_execute_OpConvertSToF;
	ctx->opcode_execute[SpvOpUConvert] = spvm_execute_OpUConvert;
	ctx->opcode_execute[SpvOpSConvert] = spvm_execute_OpSConvert;
	ctx->opcode_execute[SpvOpFConvert] = spvm_execute_OpFConvert;
	ctx->opcode_execute[SpvOpQuantizeToF16] = NULL;
	ctx->opcode_execute[SpvOpBitcast] = spvm_execute_OpBitcast;

	ctx->opcode_execute[SpvOpVectorExtractDynamic] = spvm_execute_OpVectorExtractDynamic;
	ctx->opcode_execute[SpvOpVectorInsertDynamic] = spvm_execute_OpVectorInsertDynamic;
	ctx->opcode_execute[SpvOpVectorShuffle] = spvm_execute_OpVectorShuffle;
	ctx->opcode_execute[SpvOpCompositeConstruct] = spvm_execute_OpCompositeConstruct;
	ctx->opcode_execute[SpvOpCompositeExtract] = spvm_execute_OpCompositeExtract;
	ctx->opcode_execute[SpvOpCompositeInsert] = NULL;
	ctx->opcode_execute[SpvOpCopyObject] = spvm_execute_OpCopyObject;
	ctx->opcode_execute[SpvOpTranspose] = spvm_execute_OpTranspose;
	ctx->opcode_execute[SpvOpCopyLogical] = NULL;

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
	ctx->opcode_execute[SpvOpSRem] = NULL;
	ctx->opcode_execute[SpvOpSMod] = spvm_execute_OpSMod;
	ctx->opcode_execute[SpvOpFRem] = NULL;
	ctx->opcode_execute[SpvOpFMod] = spvm_execute_OpFMod;
	ctx->opcode_execute[SpvOpVectorTimesScalar] = spvm_execute_OpVectorTimesScalar;
	ctx->opcode_execute[SpvOpMatrixTimesScalar] = spvm_execute_OpMatrixTimesScalar;
	ctx->opcode_execute[SpvOpVectorTimesMatrix] = spvm_execute_OpVectorTimesMatrix;
	ctx->opcode_execute[SpvOpMatrixTimesVector] = spvm_execute_OpMatrixTimesVector;
	ctx->opcode_execute[SpvOpMatrixTimesMatrix] = spvm_execute_OpMatrixTimesMatrix;
	ctx->opcode_execute[SpvOpOuterProduct] = spvm_execute_OpOuterProduct;
	ctx->opcode_execute[SpvOpDot] = spvm_execute_OpDot;
	ctx->opcode_execute[SpvOpIAddCarry] = NULL;
	ctx->opcode_execute[SpvOpISubBorrow] = NULL;
	ctx->opcode_execute[SpvOpUMulExtended] = NULL;
	ctx->opcode_execute[SpvOpSMulExtended] = NULL;

	ctx->opcode_execute[SpvOpShiftRightLogical] = spvm_execute_OpShiftRightLogical;
	ctx->opcode_execute[SpvOpShiftRightArithmetic] = spvm_execute_OpShiftRightArithmetic;
	ctx->opcode_execute[SpvOpShiftLeftLogical] = spvm_execute_OpShiftLeftLogical;
	ctx->opcode_execute[SpvOpBitwiseOr] = spvm_execute_OpBitwiseOr;
	ctx->opcode_execute[SpvOpBitwiseAnd] = spvm_execute_OpBitwiseAnd;
	ctx->opcode_execute[SpvOpBitwiseXor] = spvm_execute_OpBitwiseXor;
	ctx->opcode_execute[SpvOpNot] = spvm_execute_OpNot;
	ctx->opcode_execute[SpvOpBitFieldInsert] = NULL;
	ctx->opcode_execute[SpvOpBitFieldSExtract] = NULL;
	ctx->opcode_execute[SpvOpBitFieldUExtract] = NULL;
	ctx->opcode_execute[SpvOpBitReverse] = NULL;
	ctx->opcode_execute[SpvOpBitCount] = NULL;

	ctx->opcode_execute[SpvOpAny] = spvm_execute_OpAny;
	ctx->opcode_execute[SpvOpAll] = spvm_execute_OpAll;
	ctx->opcode_execute[SpvOpIsNan] = spvm_execute_OpIsNan;
	ctx->opcode_execute[SpvOpIsInf] = spvm_execute_OpIsInf;
	ctx->opcode_execute[SpvOpLogicalEqual] = spvm_execute_OpLogicalEqual;
	ctx->opcode_execute[SpvOpLogicalNotEqual] = spvm_execute_OpLogicalNotEqual;
	ctx->opcode_execute[SpvOpLogicalAnd] = spvm_execute_OpLogicalAnd;
	ctx->opcode_execute[SpvOpLogicalOr] = spvm_execute_OpLogicalOr;
	ctx->opcode_execute[SpvOpLogicalNot] = spvm_execute_OpLogicalNot;
	ctx->opcode_execute[SpvOpSelect] = spvm_execute_OpSelect;
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
	ctx->opcode_execute[SpvOpFUnordEqual] = NULL;
	ctx->opcode_execute[SpvOpFUnordNotEqual] = NULL;
	ctx->opcode_execute[SpvOpFUnordLessThan] = NULL;
	ctx->opcode_execute[SpvOpFUnordGreaterThan] = NULL;
	ctx->opcode_execute[SpvOpFUnordLessThanEqual] = NULL;
	ctx->opcode_execute[SpvOpFUnordGreaterThanEqual] = NULL;

	ctx->opcode_execute[SpvOpSampledImage] = spvm_execute_OpSampledImage;
	ctx->opcode_execute[SpvOpImageSampleImplicitLod] = spvm_execute_OpImageSample;
	ctx->opcode_execute[SpvOpImageSampleExplicitLod] = spvm_execute_OpImageSample;
	ctx->opcode_execute[SpvOpImageFetch] = spvm_execute_OpImageFetch;
	ctx->opcode_execute[SpvOpImageGather] = spvm_execute_OpImageGather;
	ctx->opcode_execute[SpvOpImageQuerySizeLod] = spvm_execute_OpImageQuerySizeLod;
	ctx->opcode_execute[SpvOpImageQuerySize] = spvm_execute_OpImageQuerySize;
	ctx->opcode_execute[SpvOpImageSampleDrefImplicitLod] = spvm_execute_OpImageSampleDref;
	ctx->opcode_execute[SpvOpImageSampleDrefExplicitLod] = spvm_execute_OpImageSampleDref;
	ctx->opcode_execute[SpvOpImageSampleProjImplicitLod] = spvm_execute_OpImageSampleProj;
	ctx->opcode_execute[SpvOpImageSampleProjExplicitLod] = spvm_execute_OpImageSampleProj;
	ctx->opcode_execute[SpvOpImageSampleProjDrefImplicitLod] = spvm_execute_OpImageSampleProjDref;
	ctx->opcode_execute[SpvOpImageSampleProjDrefExplicitLod] = spvm_execute_OpImageSampleProjDref;
	ctx->opcode_execute[SpvOpImageDrefGather] = spvm_execute_OpImageDrefGather;
	ctx->opcode_execute[SpvOpImageRead] = spvm_execute_OpImageRead;
	ctx->opcode_execute[SpvOpImageWrite] = spvm_execute_OpImageWrite;
	ctx->opcode_execute[SpvOpImage] = spvm_execute_OpImage;
	ctx->opcode_execute[SpvOpImageQueryFormat] = NULL;
	ctx->opcode_execute[SpvOpImageQueryOrder] = NULL;
	ctx->opcode_execute[SpvOpImageQueryLod] = spvm_execute_OpImageQueryLod;
	ctx->opcode_execute[SpvOpImageQueryLevels] = spvm_execute_OpImageQueryLevels;
	ctx->opcode_execute[SpvOpImageQuerySamples] = spvm_execute_OpImageQuerySamples;

	ctx->opcode_execute[SpvOpImageSparseSampleImplicitLod] = NULL;
	ctx->opcode_execute[SpvOpImageSparseSampleExplicitLod] = NULL;
	ctx->opcode_execute[SpvOpImageSparseSampleDrefImplicitLod] = NULL;
	ctx->opcode_execute[SpvOpImageSparseSampleDrefExplicitLod] = NULL;
	ctx->opcode_execute[SpvOpImageSparseSampleProjImplicitLod] = NULL;
	ctx->opcode_execute[SpvOpImageSparseSampleProjExplicitLod] = NULL;
	ctx->opcode_execute[SpvOpImageSparseSampleProjDrefImplicitLod] = NULL;
	ctx->opcode_execute[SpvOpImageSparseSampleProjDrefExplicitLod] = NULL;
	ctx->opcode_execute[SpvOpImageSparseFetch] = NULL;
	ctx->opcode_execute[SpvOpImageSparseGather] = NULL;
	ctx->opcode_execute[SpvOpImageSparseDrefGather] = NULL;
	ctx->opcode_execute[SpvOpImageSparseTexelsResident] = NULL;
	ctx->opcode_execute[SpvOpImageSparseRead] = NULL;

	ctx->opcode_execute[SpvOpDPdx] = spvm_execute_OpDPdx;
	ctx->opcode_execute[SpvOpDPdxFine] = spvm_execute_OpDPdx;
	ctx->opcode_execute[SpvOpDPdxCoarse] = spvm_execute_OpDPdx;
	ctx->opcode_execute[SpvOpDPdy] = spvm_execute_OpDPdy;
	ctx->opcode_execute[SpvOpDPdyFine] = spvm_execute_OpDPdy;
	ctx->opcode_execute[SpvOpDPdyCoarse] = spvm_execute_OpDPdy;
	ctx->opcode_execute[SpvOpFwidth] = spvm_execute_OpFwidth;
	ctx->opcode_execute[SpvOpFwidthFine] = spvm_execute_OpFwidth;
	ctx->opcode_execute[SpvOpFwidthCoarse] = spvm_execute_OpFwidth;

	ctx->opcode_execute[SpvOpBranch] = spvm_execute_OpBranch;
	ctx->opcode_execute[SpvOpBranchConditional] = spvm_execute_OpBranchConditional;
	ctx->opcode_execute[SpvOpSwitch] = spvm_execute_OpSwitch;
	ctx->opcode_execute[SpvOpKill] = spvm_execute_OpKill;

	ctx->opcode_execute[SpvOpEmitVertex] = spvm_execute_OpEmitVertex;
	ctx->opcode_execute[SpvOpEndPrimitive] = spvm_execute_OpEndPrimitive;
	ctx->opcode_execute[SpvOpEmitStreamVertex] = spvm_execute_OpEmitStreamVertex;
	ctx->opcode_execute[SpvOpEndStreamPrimitive] = spvm_execute_OpEndStreamPrimitive;

	ctx->opcode_execute[SpvOpAtomicLoad] = spvm_execute_OpAtomicLoad;
	ctx->opcode_execute[SpvOpAtomicStore] = spvm_execute_OpAtomicStore;
	ctx->opcode_execute[SpvOpAtomicExchange] = spvm_execute_OpAtomicExchange;
	ctx->opcode_execute[SpvOpAtomicCompareExchange] = spvm_execute_OpAtomicCompareExchange;
	ctx->opcode_execute[SpvOpAtomicIIncrement] = spvm_execute_OpAtomicIIncrement;
	ctx->opcode_execute[SpvOpAtomicIDecrement] = spvm_execute_OpAtomicIDecrement;
	ctx->opcode_execute[SpvOpAtomicIAdd] = spvm_execute_OpAtomicIAdd;
	ctx->opcode_execute[SpvOpAtomicISub] = spvm_execute_OpAtomicISub;
	ctx->opcode_execute[SpvOpAtomicSMin] = spvm_execute_OpAtomicSMin;
	ctx->opcode_execute[SpvOpAtomicUMin] = spvm_execute_OpAtomicUMin;
	ctx->opcode_execute[SpvOpAtomicSMax] = spvm_execute_OpAtomicSMax;
	ctx->opcode_execute[SpvOpAtomicUMax] = spvm_execute_OpAtomicUMax;
	ctx->opcode_execute[SpvOpAtomicAnd] = spvm_execute_OpAtomicAnd;
	ctx->opcode_execute[SpvOpAtomicOr] = spvm_execute_OpAtomicOr;
	ctx->opcode_execute[SpvOpAtomicXor] = spvm_execute_OpAtomicXor;

	ctx->opcode_execute[SpvOpControlBarrier] = spvm_execute_OpControlBarrier;
}
