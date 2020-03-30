#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spvm/state.h>


spvm_source load_source(const char* fname, size_t* src_size) {
	FILE* f = fopen(fname, "rb");
	if (f == 0) {
		printf("Failed to load file %s\n", fname);
		return 0;
	}

	fseek(f, 0, SEEK_END);
	long file_size = ftell(f);
	fseek(f, 0, SEEK_SET);

	size_t el_count = (file_size / sizeof(spvm_word));
	spvm_source ret = (spvm_source)malloc(el_count * sizeof(spvm_word));
	fread(ret, el_count, sizeof(spvm_word), f);
	fclose(f);

	*src_size = file_size;

	return ret;
}
int main()
{
	size_t spv_length = 0;
	spvm_source spv = load_source("shader.spv", &spv_length);
	spvm_program_t prog = spvm_program_create(spv, spv_length);
	spvm_state_t state = spvm_state_create(prog);

	float uValue[2] = { 0.2f, 0.3f };
	spvm_state_set_value_f(state, "uValue", uValue);

	spvm_source fnMain = spvm_state_get_result(state, "main");
	spvm_state_call_function(fnMain, state);

	spvm_result_t outColor = spvm_state_get_result(state, "outColor");
	printf("outColor = ");
	for (int i = 0; i < outColor->value_count; i++)
		printf("%.2f ", outColor->value[i].f);
	printf("\n");

	spvm_program_delete(prog);
	free(spv);

	return 0;
}