#include <spvm/context.h>


spvm_context_t spvm_context_initialize()
{
	spvm_context_t ret = (spvm_context_t)malloc(sizeof(spvm_context));
	_spvm_context_create_execute_table(ret);
	_spvm_context_create_setup_table(ret);
	return ret;
}
void spvm_context_deinitialize(spvm_context_t ctx)
{
	free(ctx->opcode_execute);
	free(ctx->opcode_setup);
	free(ctx);
}