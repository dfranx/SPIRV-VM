#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spirv.h"

#define SPVM_READ_WORD(spv) *((spv)++)

/* types */
typedef unsigned char spvm_byte;
typedef int spvm_word;
typedef spvm_word* spvm_source;
typedef char* spvm_string;
struct spvm_state;
void spvm_string_read(spvm_source spv, spvm_string str, spvm_word length)
{
	while (length) {
		spvm_word word = SPVM_READ_WORD(spv);

		str[0] = (word & 0x000000FF);
		str[1] = (word & 0x0000FF00) >> 8;
		str[2] = (word & 0x00FF0000) >> 16;
		str[3] = (word & 0xFF000000) >> 24;
		str += 4;

		length--;
	}
}
spvm_string spvm_string_read_all(spvm_source spv, spvm_word* length)
{
	spvm_word word_count = 8;
	spvm_string ret = (spvm_string)malloc(word_count * sizeof(spvm_word));
	spvm_byte run = 1;

	spvm_string start = ret;

	while (run) {
		spvm_word word = SPVM_READ_WORD(spv);

		ret[0] = (word & 0x000000FF);
		ret[1] = (word & 0x0000FF00) >> 8;
		ret[2] = (word & 0x00FF0000) >> 16;
		ret[3] = (word & 0xFF000000) >> 24;
		(*length)++;

		if (ret[3] == 0)
			run = 0;
		else if (*length >= word_count) {
			word_count += 8;
			ret = start = (spvm_string)realloc(start, word_count * sizeof(spvm_word));
			ret += (*length) * 4;
		} else ret += 4;
	}

	return start;
}

/* value type */
enum spvm_value_type
{
	spvm_value_type_void,
	spvm_value_type_int,
	spvm_value_type_float,
	spvm_value_type_vector,
	spvm_value_type_pointer
};
typedef union 
{
	float f;
	double d;
	int i;
	unsigned int ui;
	unsigned long long ui64;
	char b;
} spvm_value;

/* opcode */
typedef void(*spvm_opcode_func)(spvm_word word_count, struct spvm_state* state);

/* result */
enum spvm_result_type {
	spvm_result_type_none,
	spvm_result_type_extension,
	spvm_result_type_function_type,
	spvm_result_type_type,
	spvm_result_type_variable,
	spvm_result_type_constant,
	spvm_result_type_function
};
typedef struct {
	spvm_word argc;
} spvm_function_info;
typedef struct {
	SpvDecoration type;
	spvm_word literal1, literal2;
} spvm_decoration;
typedef struct {
	char type;

	spvm_string name;
	spvm_word pointer; // pointer to spvm_result
	SpvStorageClass storage_class;

	spvm_word value_count;
	spvm_value* value;

	spvm_word decoration_count;
	spvm_decoration* decorations;

	/* spvm_result_type_function */
	spvm_word return_type;

	/* spvm_result_type_extension */
	spvm_word extension_name; // index in the spvm_program::imports

	/* spvm_result_type_function_type */
	spvm_word param_count;
	spvm_word* param_type;
	spvm_source function_start;

	/* op type */
	char value_type;
	unsigned int value_bitmask;
	unsigned int vector_comp_count;
} spvm_result;
typedef spvm_result* spvm_result_t;

void spvm_decoration_read(spvm_source src, SpvDecoration decor, spvm_word* literal1, spvm_word* literal2)
{
	switch (decor) {
	case SpvDecorationSpecId:
	case SpvDecorationArrayStride:
	case SpvDecorationMatrixStride:
	case SpvDecorationBuiltIn:
	//case SpvDecorationUniformId:
	case SpvDecorationStream:
	case SpvDecorationLocation:
	case SpvDecorationComponent:
	case SpvDecorationIndex:
	case SpvDecorationBinding:
	case SpvDecorationDescriptorSet:
	case SpvDecorationOffset:
	case SpvDecorationXfbBuffer:
	case SpvDecorationXfbStride:
	case SpvDecorationFuncParamAttr:
	case SpvDecorationFPRoundingMode:
	case SpvDecorationFPFastMathMode:
	case SpvDecorationInputAttachmentIndex:
	case SpvDecorationAlignment:
	// case SpvDecorationMaxByteOffset:
	// case SpvDecorationAlignmentId:
	// case SpvDecorationMaxByteOffsetId:
	case SpvDecorationSecondaryViewportRelativeNV:
	// case SpvDecorationCounterBuffer:
	case SpvDecorationHlslCounterBufferGOOGLE:
	// case SpvDecorationUserSemantic:
	case SpvDecorationHlslSemanticGOOGLE:
	// case SpvDecorationUserTypeGOOGLE:
		*literal1 = SPVM_READ_WORD(src);
	break;

	case SpvDecorationLinkageAttributes:
		*literal1 = SPVM_READ_WORD(src);
		*literal2 = SPVM_READ_WORD(src);
	break;

	default: break;
	}
}
void spvm_result_add_decoration(spvm_result_t result, SpvDecoration decor, spvm_word literal1, spvm_word literal2)
{
	result->decoration_count++;
	result->decorations = (spvm_decoration*)realloc(result->decorations, result->decoration_count * sizeof(spvm_decoration));

	spvm_decoration* d = &result->decorations[result->decoration_count - 1];
	d->type = decor;
	d->literal1 = literal1;
	d->literal2 = literal2;
}

/* entry point */
typedef struct
{
	SpvExecutionModel exec_model;
	spvm_word id;
	spvm_string name;
	spvm_word* globals; // interface <id>
} spvm_entry_point;

/* program */
typedef struct {
	spvm_byte major_version;
	spvm_byte minor_version;

	spvm_word generator_id;
	spvm_word generator_version;

	spvm_word bound;

	size_t code_length;
	spvm_source code;

	spvm_opcode_func* opcode_table;

	SpvSourceLanguage language;
	spvm_word language_version;

	spvm_word extension_count;
	spvm_string* extensions;

	spvm_word import_count;
	spvm_string* imports;

	spvm_word capability_count;
	SpvCapability* capabilities;

	SpvAddressingModel addressing;
	SpvMemoryModel memory_model;

	spvm_word entry_point_count;
	spvm_entry_point* entry_points;
} spvm_program;
typedef spvm_program* spvm_program_t;

/* state */
typedef struct {
	spvm_program_t owner;
	spvm_source code_current; // current position in the code
	spvm_result* results;
	char function_parsing; // reached OpFunction
	char function_called;
} spvm_state;
typedef spvm_state* spvm_state_t;

/* functions */
spvm_state_t spvm_state_create(spvm_program_t prog);
void spvm_program_create_opcode_table(spvm_program_t prog);
spvm_program_t spvm_program_create(spvm_source spv, size_t spv_length);
spvm_string spvm_program_add_extension(spvm_program_t prog, spvm_word length);
spvm_string spvm_program_add_import(spvm_program_t prog, spvm_word length);
spvm_entry_point* spvm_program_create_entry_point(spvm_program_t prog);
void spvm_program_add_capability(spvm_program_t prog, SpvCapability cap);
void spvm_program_delete(spvm_program_t prog);
spvm_word spvm_value_get_count(spvm_result_t res_list, spvm_result_t res);
spvm_source spvm_function_get(spvm_state_t state, const spvm_string str);
void spvm_function_call(spvm_source code, spvm_state_t state);
spvm_result_t spvm_state_get_value(spvm_state_t state, const spvm_string str);


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

	spvm_source main_code = spvm_function_get(state, "main");

	spvm_function_call(main_code, state);

	spvm_result_t gl_FragCoord = spvm_state_get_value(state, "gl_FragColor");

	for (int i = 0; i < gl_FragCoord->value_count; i++) {
		printf("%.2f ", gl_FragCoord->value[i].f);
	}
	printf("\n");

	printf("v%d.%d -> generated by %d v%d\n", prog->major_version, prog->minor_version, prog->generator_id, prog->generator_version);
	printf("limit: %d\n", prog->bound);

	spvm_program_delete(prog);
	free(spv);

	return 0;
}



/* opcodes */
void spvm_execute_OpSource(spvm_word word_count, spvm_state_t state)
{
	state->owner->language = SPVM_READ_WORD(state->code_current);
	state->owner->language_version = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpSourceExtension(spvm_word word_count, spvm_state_t state)
{
	spvm_string ext = spvm_program_add_extension(state->owner, word_count);
	spvm_string_read(state->code_current, ext, word_count);
}
void spvm_execute_OpMemoryModel(spvm_word word_count, spvm_state_t state)
{
	state->owner->addressing = SPVM_READ_WORD(state->code_current);
	state->owner->memory_model = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpExtInstImport(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word name_index = state->owner->import_count;

	spvm_string imp = spvm_program_add_import(state->owner, word_count);
	spvm_string_read(state->code_current, imp, word_count);

	state->results[id].type = spvm_result_type_extension;
	state->results[id].extension_name = name_index;
}
void spvm_execute_OpCapability(spvm_word word_count, spvm_state_t state)
{
	SpvCapability cap = SPVM_READ_WORD(state->code_current);
	spvm_program_add_capability(state->owner, cap);
}
void spvm_execute_OpEntryPoint(spvm_word word_count, spvm_state_t state)
{
	spvm_entry_point* entry = spvm_program_create_entry_point(state->owner);
	entry->exec_model = SPVM_READ_WORD(state->code_current);
	entry->id = SPVM_READ_WORD(state->code_current);

	spvm_word name_length = 0;
	entry->name = spvm_string_read_all(state->code_current, &name_length);
	state->code_current += name_length;
	
	spvm_word interface_count = word_count - name_length - 2;
	if (interface_count) {
		entry->globals = (spvm_word*)calloc(interface_count, sizeof(spvm_word));
		spvm_word interface_index = 0;
		while (interface_count) {
			entry->globals[interface_index] = SPVM_READ_WORD(state->code_current);
			interface_index++;
			interface_count--;
		}
	}
}
void spvm_execute_OpName(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->results[id].name = (spvm_string)malloc(word_count * sizeof(spvm_word) + 1);
	spvm_string_read(state->code_current, state->results[id].name, word_count - 1);
}
void spvm_execute_OpDecorate(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	SpvDecoration decor = SPVM_READ_WORD(state->code_current);
	spvm_word literal1 = 0, literal2 = 0;

	spvm_decoration_read(state->code_current, decor, &literal1, &literal2);
	spvm_result_add_decoration(&state->results[target], decor, literal1, literal2);
}
void spvm_execute_OpTypeVoid(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_void;
	state->results[store_id].value_bitmask = 0x00;
}
void spvm_execute_OpTypeFunction(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	spvm_word return_id = SPVM_READ_WORD(state->code_current);
	spvm_word param_count = word_count - 2;

	state->results[store_id].type = spvm_result_type_function_type;
	state->results[store_id].pointer = return_id;
	state->results[store_id].param_count = param_count;

	for (spvm_word i = 0; i < param_count; i++)
		state->results[store_id].param_type[i] = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypeFloat(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	spvm_word n = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_float;
	state->results[store_id].value_bitmask = n == 32 ? 0xFFFFFFFF : ~(~0u << n);
}
void spvm_execute_OpTypeVector(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_vector;

	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[store_id].vector_comp_count = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypePointer(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_pointer;
	state->results[store_id].storage_class = SPVM_READ_WORD(state->code_current);
	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpVariable(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_variable;
	state->results[store_id].pointer = var_type;
	state->results[store_id].storage_class = SPVM_READ_WORD(state->code_current);

	spvm_word val_count = spvm_value_get_count(state->results, &state->results[var_type]);

	state->results[store_id].value_count = val_count;
	state->results[store_id].value = (spvm_value*)calloc(val_count, sizeof(spvm_value));
}
void spvm_execute_OpConstant(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_constant;
	state->results[store_id].pointer = var_type;

	spvm_word val_count = spvm_value_get_count(state->results, &state->results[var_type]);

	state->results[store_id].value_count = val_count;
	state->results[store_id].value = (spvm_value*)calloc(val_count, sizeof(spvm_value));

	for (spvm_word i = 0; i < val_count; i++)
		state->results[store_id].value[i].i = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpConstantComposite(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_constant;
	state->results[store_id].pointer = var_type;

	spvm_word val_count = spvm_value_get_count(state->results, &state->results[var_type]);

	state->results[store_id].value_count = val_count;
	state->results[store_id].value = (spvm_value*)calloc(val_count, sizeof(spvm_value));

	for (spvm_word i = 0; i < val_count; i++) {
		spvm_word index = SPVM_READ_WORD(state->code_current);
		state->results[store_id].value[i].i = state->results[index].value[0].i;
	}
}
void spvm_execute_OpFunction(spvm_word word_count, spvm_state_t state)
{
	spvm_word ret_type = SPVM_READ_WORD(state->code_current);
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_function;
	state->results[store_id].return_type = ret_type;

	SPVM_READ_WORD(state->code_current); // skip function control

	spvm_word info = state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);


	state->results[store_id].function_start = state->code_current;
	state->results[info].function_start = state->code_current;
	state->function_parsing = 1;
}
void spvm_execute_OpFunctionEnd(spvm_word word_count, spvm_state_t state)
{
	state->function_called = 0;
	state->function_parsing = 0;
}

void spvm_execute_OpStore(spvm_word word_count, spvm_state_t state)
{
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);
	spvm_word val_id = SPVM_READ_WORD(state->code_current);

	memcpy(state->results[ptr_id].value, state->results[val_id].value, sizeof(spvm_value) * state->results[ptr_id].value_count);
}
void spvm_execute_OpReturn(spvm_word word_count, spvm_state_t state)
{
	state->function_called = 0;
}



/* functions */
spvm_state_t spvm_state_create(spvm_program_t prog)
{
	spvm_state_t state = (spvm_state_t)malloc(sizeof(spvm_state));

	state->owner = prog;
	state->code_current = prog->code;
	state->results = (spvm_result*)calloc(prog->bound + 1, sizeof(spvm_result));
	state->function_called = 0;
	state->function_parsing = 0;

	for (size_t i = 0; i < prog->code_length; i++) {
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		spvm_word opcode = (opcode_data & SpvOpCodeMask);
		spvm_source cur_code = state->code_current;

		if (opcode <= 256 && prog->opcode_table[opcode] != 0 &&
			(!state->function_parsing || (state->function_parsing && opcode == SpvOpFunctionEnd)))
		{
			prog->opcode_table[opcode](word_count, state);
		}

		state->code_current = (cur_code + word_count);
		i += word_count;
	}

	return state;
}
spvm_program_t spvm_program_create(spvm_source spv, size_t spv_length)
{
	spvm_word magic = SPVM_READ_WORD(spv);
	if (magic != SpvMagicNumber)
		return NULL;

	spvm_program_t prog = (spvm_program_t)calloc(1, sizeof(spvm_program));
	
	spvm_word version = SPVM_READ_WORD(spv);
	prog->major_version = (version & 0x00FF0000) >> 16;
	prog->minor_version = (version & 0x0000FF00) >> 8;

	spvm_word generator = SPVM_READ_WORD(spv);
	prog->generator_id = (generator & 0xFFFF0000) >> 16;
	prog->generator_version = (generator & 0x0000FFFF);

	prog->bound = SPVM_READ_WORD(spv);

	spv++; // skip [4] -> 0

	prog->code_length = spv_length-5;
	prog->code = spv;

	// opcodes
	spvm_program_create_opcode_table(prog);

	return prog;
}
void spvm_program_create_opcode_table(spvm_program_t prog)
{
	prog->opcode_table = (spvm_opcode_func*)calloc(256, sizeof(spvm_opcode_func));
	prog->opcode_table[SpvOpSource] = spvm_execute_OpSource;
	prog->opcode_table[SpvOpSourceExtension] = spvm_execute_OpSourceExtension;
	prog->opcode_table[SpvOpMemoryModel] = spvm_execute_OpMemoryModel;
	prog->opcode_table[SpvOpExtInstImport] = spvm_execute_OpExtInstImport;
	prog->opcode_table[SpvOpCapability] = spvm_execute_OpCapability;
	prog->opcode_table[SpvOpEntryPoint] = spvm_execute_OpEntryPoint;
	prog->opcode_table[SpvOpName] = spvm_execute_OpName;
	prog->opcode_table[SpvOpDecorate] = spvm_execute_OpDecorate;
	prog->opcode_table[SpvOpTypeVoid] = spvm_execute_OpTypeVoid;
	prog->opcode_table[SpvOpTypeFunction] = spvm_execute_OpTypeFunction;
	prog->opcode_table[SpvOpTypeFloat] = spvm_execute_OpTypeFloat;
	prog->opcode_table[SpvOpTypeVector] = spvm_execute_OpTypeVector;
	prog->opcode_table[SpvOpTypePointer] = spvm_execute_OpTypePointer;
	prog->opcode_table[SpvOpVariable] = spvm_execute_OpVariable;
	prog->opcode_table[SpvOpConstant] = spvm_execute_OpConstant;
	prog->opcode_table[SpvOpConstantComposite] = spvm_execute_OpConstantComposite;
	prog->opcode_table[SpvOpFunction] = spvm_execute_OpFunction;
	prog->opcode_table[SpvOpFunctionEnd] = spvm_execute_OpFunctionEnd;
	prog->opcode_table[SpvOpStore] = spvm_execute_OpStore;
	prog->opcode_table[SpvOpReturn] = spvm_execute_OpReturn;
}
spvm_string spvm_program_add_extension(spvm_program_t prog, spvm_word length)
{
	prog->extension_count++;
	prog->extensions = (spvm_string*)realloc(prog->extensions, prog->extension_count * sizeof(spvm_string));
	return (prog->extensions[prog->extension_count - 1] = (spvm_string)malloc(length * sizeof(spvm_word) + 1));
}
spvm_string spvm_program_add_import(spvm_program_t prog, spvm_word length)
{
	prog->import_count++;
	prog->imports = (spvm_string*)realloc(prog->imports, prog->import_count * sizeof(spvm_string));
	return (prog->imports[prog->import_count - 1] = (spvm_string)malloc(length * sizeof(spvm_word) + 1));
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

	free(prog);
}
spvm_word spvm_value_get_count(spvm_result_t res_list, spvm_result_t res)
{
	if (res->value_type == spvm_value_type_pointer)
		return spvm_value_get_count(res_list, &res_list[res->pointer]);
	else if (res->value_type == spvm_value_type_vector)
		return res->vector_comp_count;

	return 1;
}
spvm_source spvm_function_get(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->entry_point_count; i++)
		if (strcmp(state->owner->entry_points[i].name, str) == 0)
			return state->results[state->owner->entry_points[i].id].function_start;

	return NULL;
}
void spvm_function_call(spvm_source code, spvm_state_t state)
{
	state->code_current = code;
	state->function_called = 1;

	spvm_program_t prog = state->owner;

	while (state->function_called)
	{
		spvm_word opcode_data = SPVM_READ_WORD(state->code_current);
		spvm_word word_count = ((opcode_data & (~SpvOpCodeMask)) >> SpvWordCountShift) - 1;
		spvm_word opcode = (opcode_data & SpvOpCodeMask);
		spvm_source cur_code = state->code_current;

		if (opcode <= 256 && prog->opcode_table[opcode] != 0)
			prog->opcode_table[opcode](word_count, state);

		state->code_current = (cur_code + word_count);
	}
}
spvm_result_t spvm_state_get_value(spvm_state_t state, const spvm_string str)
{
	for (spvm_word i = 0; i < state->owner->bound; i++)
		if (state->results[i].type == spvm_result_type_variable &&
			strcmp(state->results[i].name, str) == 0)
			return &state->results[i];

	return NULL;
}