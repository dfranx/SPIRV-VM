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

	spvm_word mem_count = type_info->member_count;
	if (mem_count == 0)
		mem_count = 1;

	spvm_member_allocate_value(val, mem_count);
	val->type = type;

	if (type_info->value_type == spvm_value_type_struct) {
		for (spvm_word i = 0; i < val->member_count; i++)
			spvm_member_allocate_typed_value(&val->members[i], results, type_info->param_type[i]);
	}
	else if (type_info->value_type == spvm_value_type_matrix) {
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

	spvm_word mem_count = type_info->member_count;
	if (mem_count == 0)
		mem_count = 1;

	spvm_result_allocate_value(val, mem_count);
	val->pointer = type;

	if (type_info->value_type == spvm_value_type_struct) {
		for (spvm_word i = 0; i < val->member_count; i++)
			spvm_member_allocate_typed_value(&val->members[i], results, type_info->param_type[i]);
	}
	else if (type_info->value_type == spvm_value_type_matrix) {
		for (spvm_word i = 0; i < val->member_count; i++)
			spvm_member_allocate_typed_value(&val->members[i], results, type_info->pointer);
	}
}
