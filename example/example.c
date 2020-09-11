#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <spvm/context.h>
#include <spvm/state.h>
#include <spvm/ext/GLSL450.h>

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
	// context holds all opcode functions
	spvm_context_t ctx = spvm_context_initialize();

	// load source code
	size_t spv_length = 0;
	spvm_source spv = load_source("shader.spv", &spv_length);

	// create a program and a state
	spvm_program_t prog = spvm_program_create(ctx, spv, spv_length);
	spvm_state_t state = spvm_state_create(prog);

	// load extension
	spvm_ext_opcode_func* glsl_ext_data = spvm_build_glsl450_ext();
	spvm_result_t glsl_std_450 = spvm_state_get_result(state, "GLSL.std.450");
	if (glsl_std_450)
		glsl_std_450->extension = glsl_ext_data;

	// get uBlock
	spvm_result_t uBlock = spvm_state_get_result(state, "uBlock");

	// set uBlock.time
	float timeData = 0.5f;
	spvm_member_t uBlock_time = spvm_state_get_object_member(state, uBlock, "time");
	spvm_member_set_value_f(uBlock_time->members, uBlock_time->member_count, &timeData);

	// set uBlock.info
	float infoData[2] = { 0.2f, 0.8f };
	spvm_member_t uBlock_info = spvm_state_get_object_member(state, uBlock, "info");
	spvm_member_set_value_f(uBlock_info->members, uBlock_info->member_count, infoData);

	// call main
	spvm_word fnMain = spvm_state_get_result_location(state, "main");
	spvm_state_prepare(state, fnMain);
	spvm_state_call_function(state);

	// get outColor
	spvm_result_t outColor = spvm_state_get_result(state, "outColor");
	for (int i = 0; i < outColor->member_count; i++)
		printf("%.2f ", outColor->members[i].value.f);
	printf("\n");

	// check if this pixel was discarded
	printf("discarded: %d\n", state->discarded);

	// free memory
	spvm_state_delete(state);
	spvm_program_delete(prog);
	free(glsl_ext_data);
	free(spv);

	spvm_context_deinitialize(ctx);

	return 0;
}