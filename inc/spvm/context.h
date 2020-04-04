#ifndef __SPIRV_VM_CONTEXT_H__
#define __SPIRV_VM_CONTEXT_H__

#include <spvm/opcode.h>

#define SPVM_OPCODE_TABLE_LENGTH 405

typedef struct spvm_context {
	spvm_opcode_func* opcode_execute;
	spvm_opcode_func* opcode_setup;
} spvm_context;
typedef struct spvm_context* spvm_context_t;

spvm_context_t spvm_context_initialize();
void spvm_context_deinitialize(spvm_context_t ctx);

/* PRIVATE FUNCTIONS */
void _spvm_context_create_execute_table(spvm_context_t ctx);
void _spvm_context_create_setup_table(spvm_context_t ctx);

#endif // __SPIRV_VM_CONTEXT_H__