#include <spvm/program.h>


spvm_program_t spvm_program_create(spvm_context_t ctx, spvm_source spv, size_t spv_length)
{
	spvm_word magic = SPVM_READ_WORD(spv);
	if (magic != SpvMagicNumber)
		return NULL;

	spvm_program_t prog = (spvm_program_t)calloc(1, sizeof(spvm_program));

	prog->context = ctx;

	spvm_word version = SPVM_READ_WORD(spv);
	prog->major_version = (version & 0x00FF0000) >> 16;
	prog->minor_version = (version & 0x0000FF00) >> 8;

	spvm_word generator = SPVM_READ_WORD(spv);
	prog->generator_id = (generator & 0xFFFF0000) >> 16;
	prog->generator_version = (generator & 0x0000FFFF);

	prog->bound = SPVM_READ_WORD(spv);

	SPVM_SKIP_WORD(spv); // skip [4] -> 0

	prog->code_length = spv_length - 5;
	prog->code = spv;

	prog->local_size_x = 1;
	prog->local_size_y = 1;
	prog->local_size_z = 1;

	prog->geometry_invocations = 1;
	prog->geometry_output_count = 1;
	prog->geometry_input = SpvExecutionModeInputPoints;
	prog->geometry_output = SpvExecutionModeOutputPoints;

	return prog;
}
spvm_string spvm_program_add_extension(spvm_program_t prog, spvm_word length)
{
	prog->extension_count++;
	prog->extensions = (spvm_string*)realloc(prog->extensions, prog->extension_count * sizeof(spvm_string));
	return (prog->extensions[prog->extension_count - 1] = (spvm_string)malloc(length * sizeof(spvm_word) + 1));
}
spvm_entry_point* spvm_program_create_entry_point(spvm_program_t prog)
{
	prog->entry_point_count++;
	prog->entry_points = (spvm_entry_point*)realloc(prog->entry_points, prog->entry_point_count * sizeof(spvm_entry_point));
	return &prog->entry_points[prog->entry_point_count - 1];
}
void spvm_program_add_capability(spvm_program_t prog, SpvCapability cap)
{
	prog->capability_count++;
	prog->capabilities = (SpvCapability*)realloc(prog->capabilities, prog->capability_count * sizeof(SpvCapability));
	prog->capabilities[prog->capability_count - 1] = cap;
}
void spvm_program_delete(spvm_program_t prog)
{
	if (prog == NULL) return;

	// entry points
	for (spvm_word i = 0; i < prog->entry_point_count; i++) {
		if (prog->entry_points[i].globals_count > 0)
			free(prog->entry_points[i].globals);
		free(prog->entry_points[i].name);
	}
	if (prog->entry_point_count)
		free(prog->entry_points);

	// extensions
	for (spvm_word i = 0; i < prog->extension_count; i++)
		free(prog->extensions[i]);
	if (prog->extension_count)
		free(prog->extensions);

	// capabilities
	if (prog->capability_count)
		free(prog->capabilities);

	free(prog);
}