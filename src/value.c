#include <spvm/value.h>

void spvm_member_free(spvm_member_t source, spvm_word value_count)
{
	for (spvm_word i = 0; i < value_count; i++)
		if (source[i].member_count)
			spvm_member_free(source[i].members, source[i].member_count);

	free(source);
}
void spvm_member_memcpy(spvm_member_t target, spvm_member_t source, spvm_word value_count)
{
	for (spvm_word i = 0; i < value_count; i++) {
		target[i].value.u64 = source[i].value.u64;
		if (target[i].member_count != 0)
			spvm_member_memcpy(target[i].members, source[i].members, target[i].member_count);
	}
}
