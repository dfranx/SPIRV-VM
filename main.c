#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spvm/context.h>
#include <spvm/state.h>
#include <spvm/ext/GLSL450.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

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
#define IMG_WIDTH 512
#define IMG_HEIGHT 288
int main()
{
	spvm_context_t ctx = spvm_context_initialize();

	size_t spv_length = 0;
	spvm_source spv = load_source("shader.spv", &spv_length);
	spvm_program_t prog = spvm_program_create(ctx, spv, spv_length);
	spvm_state_t state = spvm_state_create(prog);

	spvm_ext_opcode_func glsl_ext_table = spvm_build_glsl450_ext();
	spvm_result_t glsl_ext = spvm_state_get_result(state, "GLSL.std.450");
	glsl_ext->extension = glsl_ext_table;

	float iResolution[2] = { IMG_WIDTH, IMG_HEIGHT };
	spvm_state_set_value_f(state, "iResolution", iResolution);

	float iTime = 1.0f;
	spvm_state_set_value_f(state, "iTime", &iTime);

	char* outImg = (char*)malloc(IMG_WIDTH * IMG_HEIGHT * 4 * sizeof(char));

	spvm_source fnMain = spvm_state_get_result(state, "main");
	for (int x = 0; x < IMG_WIDTH; x++)
		for (int y = 0; y < IMG_HEIGHT; y++) {
			int actualY = IMG_HEIGHT - y - 1;

			float gl_FragCoord[4] = { x, actualY, 0.0f, 1.0f };
			spvm_state_set_value_f(state, "gl_FragCoord", gl_FragCoord);
			spvm_state_call_function(fnMain, state);

			spvm_result_t outColor = spvm_state_get_result(state, "outColor");
			for (int i = 0; i < outColor->member_count; i++)
				outImg[(y * IMG_WIDTH + x) * 4 + i] = fabsf(outColor->members[i].value.f) * 255;
		}

	stbi_write_png("out.png", IMG_WIDTH, IMG_HEIGHT, 4, outImg, IMG_WIDTH * 4);

	printf("discarded: %d\n", state->discarded);

	spvm_state_delete(state);
	spvm_program_delete(prog);
	free(spv);
	free(outImg);

	spvm_context_deinitialize(ctx);

	return 0;
}