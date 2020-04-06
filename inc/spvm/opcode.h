#ifndef __SPIRV_VM_OPCODE_H__
#define __SPIRV_VM_OPCODE_H__

#include <spvm/types.h>

struct spvm_state;

typedef void(*spvm_opcode_func)(spvm_word word_count, struct spvm_state* state);
typedef void(*spvm_ext_opcode_func)(spvm_word type, spvm_word id, spvm_word word_count, struct spvm_state* state);

#endif // __SPIRV_VM_OPCODE_H__