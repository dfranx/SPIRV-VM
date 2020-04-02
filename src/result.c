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
void spvm_result_add_decoration(spvm_result_t result, SpvDecoration decor, spvm_word literal1, spvm_word literal2)
{
	result->decoration_count++;
	result->decorations = (spvm_decoration*)realloc(result->decorations, result->decoration_count * sizeof(spvm_decoration));

	spvm_decoration* d = &result->decorations[result->decoration_count - 1];
	d->type = decor;
	d->literal1 = literal1;
	d->literal2 = literal2;
}
void spvm_result_allocate_value(spvm_result_t val, spvm_word count)
{
	val->value_count = count;
	val->value = (spvm_value*)calloc(count, sizeof(spvm_value));
}
void spvm_result_allocate_typed_value(spvm_result_t val, spvm_result* results, spvm_word type)
{
	spvm_word count = spvm_state_get_value_count(results, &results[type]);
	spvm_result_allocate_value(val, count);
	val->pointer = type;
}
