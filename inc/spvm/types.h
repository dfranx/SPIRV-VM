#ifndef __SPIRV_VM_TYPES_H__
#define __SPIRV_VM_TYPES_H__

#include <stdlib.h>

typedef unsigned char spvm_byte;
typedef int spvm_word;
typedef spvm_word* spvm_source;
typedef char* spvm_string;
struct spvm_state;

#define SPVM_READ_WORD(spv) *((spv)++)
#define SPVM_SKIP_WORD(spv) ((spv)++)

void spvm_string_read(spvm_source spv, spvm_string str, spvm_word length);
spvm_string spvm_string_read_all(spvm_source spv, spvm_word* length);

#endif // __SPIRV_VM_TYPES_H__