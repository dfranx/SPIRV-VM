#ifndef __SPIRV_VM_OPCODE_H__
#define __SPIRV_VM_OPCODE_H__

#include <spvm/types.h>

struct spvm_state;
struct spvm_program;

typedef void(*spvm_opcode_func)(spvm_word word_count, struct spvm_state* state);

void spvm_program_create_opcode_table(struct spvm_program* prog);

#endif // __SPIRV_VM_OPCODE_H__