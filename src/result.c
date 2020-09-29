#include <spvm/result.h>
#include <spvm/state.h>
#include <spvm/spirv.h>

void spvm_decoration_read(spvm_source src, SpvDecoration decor, spvm_word* literal1, spvm_word* literal2)
{
	switch (decor) {
	case SpvDecorationSpecId:
	case SpvDecorationArrayStride:
	case SpvDecorationMatrixStride:
	case SpvDecorationBuiltIn:
	case SpvDecorationUniformId:
	case SpvDecorationStream:
	case SpvDecorationLocation:
	case SpvDecorationComponent:
	case SpvDecorationIndex:
	case SpvDecorationBinding:
	case SpvDecorationDescriptorSet:
	case SpvDecorationOffset:
	case SpvDecorationXfbBuffer:
	case SpvDecorationXfbStride:
	case SpvDecorationFuncParamAttr:
	case SpvDecorationFPRoundingMode:
	case SpvDecorationFPFastMathMode:
	case SpvDecorationInputAttachmentIndex:
	case SpvDecorationAlignment:
	case SpvDecorationMaxByteOffset:
	case SpvDecorationAlignmentId:
	case SpvDecorationMaxByteOffsetId:
	case SpvDecorationSecondaryViewportRelativeNV:
	case SpvDecorationCounterBuffer:
	case SpvDecorationUserSemantic:
	case SpvDecorationUserTypeGOOGLE:
		*literal1 = SPVM_READ_WORD(src);
		break;

	case SpvDecorationLinkageAttributes:
		*literal1 = SPVM_READ_WORD(src);
		*literal2 = SPVM_READ_WORD(src);
		break;

	default: break;
	}
}
void spvm_result_add_member_decoration(spvm_result_t result, SpvDecoration decor, spvm_word literal1, spvm_word literal2, spvm_word index)
{
	result->decoration_count++;
	result->decorations = (spvm_decoration*)realloc(result->decorations, result->decoration_count * sizeof(spvm_decoration));

	spvm_decoration* d = &result->decorations[result->decoration_count - 1];
	d->index = index;
	d->type = decor;
	d->literal1 = literal1;
	d->literal2 = literal2;
}
void spvm_result_add_decoration(spvm_result_t result, SpvDecoration decor, spvm_word literal1, spvm_word literal2)
{
	spvm_result_add_member_decoration(result, decor, literal1, literal2, 0);
}


void spvm_member_allocate_value(spvm_member_t val, spvm_word count)
{
	val->member_count = count;
	val->members = (spvm_member_t)calloc(count, sizeof(spvm_member));
}
void spvm_member_allocate_typed_value(spvm_member_t val, spvm_result* results, spvm_word type)
{
	spvm_result_t type_info = spvm_state_get_type_info(results, &results[type]);

	spvm_member_allocate_value(val, type_info->member_count);
	val->type = type;

	if (type_info->value_type == spvm_value_type_struct) {
		for (spvm_word i = 0; i < val->member_count; i++) {
			val->members[i].type = type_info->params[i];
			spvm_member_allocate_typed_value(&val->members[i], results, type_info->params[i]);
		}
	}
	else if (type_info->value_type == spvm_value_type_matrix) {
		for (spvm_word i = 0; i < val->member_count; i++)
			spvm_member_allocate_typed_value(&val->members[i], results, type_info->pointer);
	}
	else if (type_info->value_type == spvm_value_type_array) {
		if (results[type_info->pointer].member_count > 0)
			for (spvm_word i = 0; i < val->member_count; i++)
				spvm_member_allocate_typed_value(&val->members[i], results, type_info->pointer);
	}
}
void spvm_result_allocate_value(spvm_result_t val, spvm_word count)
{
	val->member_count = count;
	val->members = (spvm_member_t)calloc(count, sizeof(spvm_member));
}
void spvm_result_allocate_typed_value(spvm_result_t val, spvm_result* results, spvm_word type)
{
	spvm_result_t type_info = spvm_state_get_type_info(results, &results[type]);
	
	spvm_result_allocate_value(val, type_info->member_count);
	val->pointer = type;

	if (type_info->value_type == spvm_value_type_struct) {
		for (spvm_word i = 0; i < val->member_count; i++)
			spvm_member_allocate_typed_value(&val->members[i], results, type_info->params[i]);
	}
	else if (type_info->value_type == spvm_value_type_matrix) {
		for (spvm_word i = 0; i < val->member_count; i++)
			spvm_member_allocate_typed_value(&val->members[i], results, type_info->pointer);
	}
	else if (type_info->value_type == spvm_value_type_array) {
		if (results[type_info->pointer].member_count > 0)
			for (spvm_word i = 0; i < val->member_count; i++)
				spvm_member_allocate_typed_value(&val->members[i], results, type_info->pointer);
	}
}
void spvm_result_delete(spvm_result_t res)
{
	// name
	if (res->name != NULL)
		free(res->name);

	// member names
	for (spvm_word j = 0; j < res->member_name_count; j++)
		free(res->member_name[j]);
	if (res->member_name_count)
		free(res->member_name);

	// member/parameter types
	if (res->value_type == spvm_value_type_struct || res->type == spvm_result_type_function_type)
		free(res->params);

	// decorations
	if (res->decoration_count)
		free(res->decorations);

	// image info
	if (res->image_info)
		free(res->image_info);

	// constants
	if (res->type == spvm_result_type_constant || res->type == spvm_result_type_variable)
		spvm_member_free(res->members, res->member_count);
}

spvm_word spvm_result_calculate_size(spvm_result_t results, spvm_word type)
{
	spvm_word ret = 0;
	spvm_result_t slot = &results[type];

	if (slot->value_type == spvm_value_type_vector || slot->value_type == spvm_value_type_matrix || slot->value_type == spvm_value_type_array || slot->value_type == spvm_value_type_runtime_array)
		ret += slot->member_count * spvm_result_calculate_size(results, slot->pointer);
	else if (slot->value_type == spvm_value_type_float || slot->value_type == spvm_value_type_int || slot->value_type == spvm_value_type_bool)
		ret++;
	else if (slot->value_type == spvm_value_type_struct) {
		for (spvm_word i = 0; i < slot->member_count; i++)
			ret += spvm_result_calculate_size(results, slot->params[i]);
	}
	// TODO: slot->params[0];

	return ret;
}
void spvm_member_recursive_fill(spvm_result_t results, float* data, spvm_member_t values, spvm_word value_count, spvm_word element_type, spvm_word* offset)
{
	spvm_result_t type_info = &results[element_type];
	for (spvm_word i = 0; i < value_count; i++) {
		if (values[i].member_count != 0)
			spvm_member_recursive_fill(results, data, values[i].members, values[i].member_count, type_info->pointer, offset);
		else {
			values[i].value.f = *(data + *offset);
			(*offset)++;
		}
	}
}