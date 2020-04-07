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
		target[i].image_data = source[i].image_data;
		target[i].value.u64 = source[i].value.u64;
		if (target[i].member_count != 0)
			spvm_member_memcpy(target[i].members, source[i].members, target[i].member_count);
	}
}

void spvm_member_set_value_f(spvm_member_t mems, size_t mem_count, float* f)
{
	if (mems != NULL && mem_count != 0)
		for (spvm_word i = 0; i < mem_count; i++) {
			if (mems[i].member_count == 0)
				mems[i].value.f = f[i];
			else
				for (spvm_word j = 0; j < mems[i].member_count; j++)
					mems[i].members[j].value.f = f[i * mems[i].member_count + j];
		}
}
void spvm_member_set_value_i(spvm_member_t mems, size_t mem_count,  int* d)
{
	if (mems != NULL && mem_count != 0)
		for (spvm_word i = 0; i < mem_count; i++)
			mems[i].value.s = d[i];
}