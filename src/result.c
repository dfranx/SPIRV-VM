#include <spvm/result.h>
#include <spvm/spirv.h>

void spvm_decoration_read(spvm_source src, SpvDecoration decor, spvm_word* literal1, spvm_word* literal2)
{
	switch (decor) {
	case SpvDecorationSpecId:
	case SpvDecorationArrayStride:
	case SpvDecorationMatrixStride:
	case SpvDecorationBuiltIn:
		//case SpvDecorationUniformId:
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
		// case SpvDecorationMaxByteOffset:
		// case SpvDecorationAlignmentId:
		// case SpvDecorationMaxByteOffsetId:
	case SpvDecorationSecondaryViewportRelativeNV:
		// case SpvDecorationCounterBuffer:
	case SpvDecorationHlslCounterBufferGOOGLE:
		// case SpvDecorationUserSemantic:
	case SpvDecorationHlslSemanticGOOGLE:
		// case SpvDecorationUserTypeGOOGLE:
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
