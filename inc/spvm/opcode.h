#ifndef __SPIRV_VM_OPCODE_H__
#define __SPIRV_VM_OPCODE_H__

#include <spvm/types.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

struct spvm_state;

typedef void(*spvm_opcode_func)(spvm_word word_count, struct spvm_state* state);
typedef void(*spvm_ext_opcode_func)(spvm_word type, spvm_word id, spvm_word word_count, struct spvm_state* state);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __SPIRV_VM_OPCODE_H__
