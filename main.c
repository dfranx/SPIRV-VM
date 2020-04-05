#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spvm/context.h>
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

	*src_size = el_count;

	return ret;
}
int main()
{
	spvm_context_t ctx = spvm_context_initialize();

	size_t spv_length = 0;
	spvm_source spv = load_source("shader.spv", &spv_length);
	spvm_program_t prog = spvm_program_create(ctx, spv, spv_length);
	spvm_state_t state = spvm_state_create(prog);

	float uValue[4] = { 0.2f, 0.3f, 0.4f, 0.5f };
	spvm_state_set_value_f(state, "uValue", uValue);

	int sel = 1;
	spvm_state_set_value_i(state, "sel", &sel);

	float noise2d_data[] = {
		0.4f, 0.4f, 0.4f, 0.4f,
		0.5f, 0.5f, 0.5f, 0.5f,
		0.6f, 0.6f, 0.6f, 0.6f,
		0.7f, 0.7f, 0.7f, 0.7f,
	};

	spvm_image noise2d;
	spvm_image_create(&noise2d, noise2d_data, 2, 2, 1);

	spvm_result_t noise2d_container = spvm_state_get_result(state, "Noise2d");
	noise2d_container->members[0].image_data = &noise2d;

	spvm_source fnMain = spvm_state_get_result(state, "main");
	spvm_state_call_function(fnMain, state);

	printf("discarded: %d\n", state->discarded);

	spvm_result_t outColor = spvm_state_get_result(state, "outColor");
	printf("outColor = ");
	for (int i = 0; i < outColor->member_count; i++)
		printf("%.2f ", outColor->members[i].value.f);
	printf("\n");

	spvm_state_delete(state);
	spvm_program_delete(prog);
	free(spv);

	spvm_context_deinitialize(ctx);

	return 0;
}