#include <spvm/opcode.h>
#include <spvm/state.h>
#include <spvm/spirv.h>
#include <string.h>

/*
OpUndef, OpSizeOf, OpMemberDecorate
OpDecorateId, OpDecorateString, OpMemberDecorateString
OpExtension, OpExtInst
OpMemberName
OpExecutionMode, OpExecutionModeId
OpTypeImage, OpTypeSampler, OpTypeSampledImage, OpTypeRuntimeArray, OpTypeStruct, OpTypeOpaque, OpTypeForwardPointer; test: matrix
OpConstantSampler, OpConstantNull; OpConstant >32bits, Spec OpCodes
OpImageTexelPointer, OpInBoundsAccessChain, OpPtrAccessChain, OpArrayLength, OpGenericPtrMemSemantics, OpInBoundsPtrAccessChain, OpFunctionParameter



1. structures
*/

/* opcodes */
/* 3.32.1 Miscellaneous Instructions */
void spvm_execute_OpNop(spvm_word word_count, spvm_state_t state)
{
}

/* 3.32.2 Debug Instructions */
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
void spvm_execute_OpName(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->results[id].name = (spvm_string)malloc(word_count * sizeof(spvm_word) + 1);
	spvm_string_read(state->code_current, state->results[id].name, word_count - 1);
}
void spvm_execute_OpString(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->results[id].type = spvm_result_type_string;
	state->results[id].name = (spvm_string)malloc((word_count - 1) * sizeof(spvm_word) + 1);
	spvm_string_read(state->code_current, state->results[id].name, word_count - 1);
}
void spvm_execute_OpLine(spvm_word word_count, spvm_state_t state)
{
	spvm_word file = SPVM_READ_WORD(state->code_current);
	spvm_word line = SPVM_READ_WORD(state->code_current);
	spvm_word clmn = SPVM_READ_WORD(state->code_current);

	state->current_file = state->results[file].name;
	state->current_line = line;
	state->current_column = clmn;
}
void spvm_execute_OpNoLine(spvm_word word_count, spvm_state_t state)
{
	state->current_file = NULL;
	state->current_line = -1;
	state->current_column = -1;
}

/* 3.32.3 Annotation Instructions */
void spvm_execute_OpDecorate(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	SpvDecoration decor = SPVM_READ_WORD(state->code_current);
	spvm_word literal1 = 0, literal2 = 0;

	spvm_decoration_read(state->code_current, decor, &literal1, &literal2);
	spvm_result_add_decoration(&state->results[target], decor, literal1, literal2);
}

/* 3.32.4 Extension Instructions */
void spvm_execute_OpExtInstImport(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word name_index = state->owner->import_count;

	spvm_string imp = spvm_program_add_import(state->owner, word_count);
	spvm_string_read(state->code_current, imp, word_count);

	state->results[id].type = spvm_result_type_extension;
	state->results[id].extension_name = name_index;
}

/* 3.32.5 Mode-Setting Instructions */
void spvm_execute_OpMemoryModel(spvm_word word_count, spvm_state_t state)
{
	state->owner->addressing = SPVM_READ_WORD(state->code_current);
	state->owner->memory_model = SPVM_READ_WORD(state->code_current);
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
void spvm_execute_OpCapability(spvm_word word_count, spvm_state_t state)
{
	SpvCapability cap = SPVM_READ_WORD(state->code_current);
	spvm_program_add_capability(state->owner, cap);
}

/* 3.32.6 Type-Declaration Instructions */
void spvm_execute_OpTypeVoid(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_void;
	state->results[store_id].value_bitmask = 0x00;
}
void spvm_execute_OpTypeBool(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_bool;
	state->results[store_id].value_bitmask = ~0x00;
}
void spvm_execute_OpTypeInt(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	spvm_word n = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_int;
	state->results[store_id].value_bitmask = n == 32 ? 0xFFFFFFFF : ~(~0u << n);
	state->results[store_id].value_sign = SPVM_READ_WORD(state->code_current);
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
	state->results[store_id].value_count = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypeMatrix(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_matrix;

	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[store_id].value_count = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypeArray(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_array;

	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[store_id].value_count = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypePointer(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_pointer;
	state->results[store_id].storage_class = SPVM_READ_WORD(state->code_current);
	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypeFunction(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word return_id = SPVM_READ_WORD(state->code_current);
	spvm_word param_count = word_count - 2;

	state->results[id].type = spvm_result_type_function_type;
	state->results[id].pointer = return_id;
	state->results[id].param_count = param_count;
	state->results[id].param_type = (spvm_word*)malloc(param_count * sizeof(spvm_word));

	for (spvm_word i = 0; i < param_count; i++)
		state->results[id].param_type[i] = SPVM_READ_WORD(state->code_current);
}

/* 3.32.7 Constant-Creation Instructions */
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
void spvm_execute_OpConstantTrue(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	state->results[id].pointer = var_type;

	spvm_word val_count = spvm_value_get_count(state->results, &state->results[var_type]);

	state->results[id].value_count = val_count;
	state->results[id].value = (spvm_value*)calloc(val_count, sizeof(spvm_value));

	for (spvm_word i = 0; i < val_count; i++)
		state->results[id].value[i].b = 1;
}
void spvm_execute_OpConstantFalse(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	state->results[id].pointer = var_type;

	spvm_word val_count = spvm_value_get_count(state->results, &state->results[var_type]);

	state->results[id].value_count = val_count;
	state->results[id].value = (spvm_value*)calloc(val_count, sizeof(spvm_value));

	for (spvm_word i = 0; i < val_count; i++)
		state->results[id].value[i].b = 0;
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

/* 3.32.8 Memory Instructions */
void spvm_execute_OpVariable(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	spvm_word val_count = spvm_value_get_count(state->results, &state->results[var_type]);

	state->results[store_id].type = spvm_result_type_variable;
	state->results[store_id].pointer = var_type;
	state->results[store_id].storage_class = SPVM_READ_WORD(state->code_current);
	state->results[store_id].value_count = val_count;
	state->results[store_id].value = (spvm_value*)calloc(val_count, sizeof(spvm_value));
}
void spvm_execute_OpStore(spvm_word word_count, spvm_state_t state)
{
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);
	spvm_word val_id = SPVM_READ_WORD(state->code_current);

	memcpy(state->results[ptr_id].value, state->results[val_id].value, sizeof(spvm_value) * state->results[ptr_id].value_count);
}
void spvm_execute_OpLoad(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	state->results[id].value_count = 1;
	state->results[id].value = (spvm_value*)calloc(1, sizeof(spvm_value));
	state->results[id].pointer = res_type;
	memcpy(state->results[id].value, state->results[ptr_id].value, sizeof(spvm_value));
}
void spvm_execute_OpCopyMemory(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	spvm_word source = SPVM_READ_WORD(state->code_current);

	// TODO: structures
	state->results[target].value_count = state->results[source].value_count;
	memcpy(state->results[target].value, state->results[source].value, state->results[target].value_count * sizeof(spvm_value));
}
void spvm_execute_OpCopyMemorySized(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	spvm_word source = SPVM_READ_WORD(state->code_current);
	spvm_word size_id = SPVM_READ_WORD(state->code_current);

	spvm_word size = state->results[size_id].value[0].i;

	// TODO: structures
	state->results[target].value_count = state->results[source].value_count;
	memcpy(state->results[target].value, state->results[source].value, size);
}
void spvm_execute_OpAccessChain(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	spvm_word value_id = SPVM_READ_WORD(state->code_current);
	spvm_word index_id = SPVM_READ_WORD(state->code_current);

	// TODO: index list

	spvm_word index = state->results[index_id].value[0].i;

	state->results[store_id].type = spvm_result_type_access_chain;
	state->results[store_id].pointer = var_type;
	state->results[store_id].value_count = 1;
	state->results[store_id].value = &state->results[value_id].value[index];
}
void spvm_execute_OpPtrEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	// TODO: should this actually check if they point to the same value or if the values are same?

	state->results[id].type = spvm_result_type_constant;
	state->results[id].value_count = 1;
	state->results[id].value_type = spvm_value_type_bool;
	state->results[id].value = (spvm_value*)calloc(1, sizeof(spvm_value));
	state->results[id].pointer = res_type;
	state->results[id].value[0].i = (state->results[op1].value == state->results[op2].value);
}
void spvm_execute_OpPtrNotEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	// TODO: should this actually check if they point to the same value or if the values are same?

	state->results[id].type = spvm_result_type_constant;
	state->results[id].value_count = 1;
	state->results[id].value_type = spvm_value_type_bool;
	state->results[id].value = (spvm_value*)calloc(1, sizeof(spvm_value));
	state->results[id].pointer = res_type;
	state->results[id].value[0].i = (state->results[op1].value != state->results[op2].value);
}
void spvm_execute_OpPtrDiff(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	// TODO: should this actually check if they point to the same value or if the values are same?

	state->results[id].type = spvm_result_type_constant;
	state->results[id].value_type = spvm_value_type_int;
	state->results[id].value_count = 1;
	state->results[id].value = (spvm_value*)calloc(1, sizeof(spvm_value));
	state->results[id].pointer = res_type;
	state->results[id].value[0].i = (op1 - op2);
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
	state->function_reached_label = 0;
	state->current_function = &state->results[store_id];
}
void spvm_execute_OpFunctionParameter(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word val_count = spvm_value_get_count(state->results, &state->results[var_type]);

	state->results[id].type = spvm_result_type_variable;
	state->results[id].pointer = var_type;
	state->results[id].storage_class = SPVM_READ_WORD(state->code_current);
	state->results[id].value_count = val_count;
	state->results[id].value = (spvm_value*)calloc(val_count, sizeof(spvm_value));
	state->results[id].parameter_owner = state->current_function;
}
void spvm_execute_OpFunctionEnd(spvm_word word_count, spvm_state_t state)
{
	state->function_parsing = 0;
	state->function_reached_label = 0;

	spvm_state_pop_function_stack(state);
}
void spvm_execute_OpFunctionCall(spvm_word word_count, spvm_state_t state)
{
	spvm_word ret_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word func = SPVM_READ_WORD(state->code_current);

	spvm_word argc = word_count - 3;

	spvm_word val_count = spvm_value_get_count(state->results, &state->results[ret_type]);

	state->results[id].type = spvm_result_type_constant;
	state->results[id].pointer = ret_type;
	state->results[id].value_count = val_count;
	state->results[id].value = (spvm_value*)calloc(val_count, sizeof(spvm_value));

	spvm_result_t next_func = &state->results[func];

	for (spvm_word i = 0; i < argc; i++) {
		spvm_word arg_id = SPVM_READ_WORD(state->code_current);
		spvm_result_t arg = NULL;

		spvm_word cur_arg = 0;
		for (spvm_word j = 0; j < state->owner->bound; j++) {
			if (state->results[j].parameter_owner == next_func) {
				if (i == cur_arg) {
					state->results[j].value = state->results[arg_id].value;
					break;
				}
				cur_arg++;
			}
		}
	}

	spvm_state_push_function_stack(state, next_func, id);
}

void spvm_execute_OpCompositeConstruct(spvm_word word_count, spvm_state_t state)
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

void spvm_execute_OpFAdd(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	state->results[id].value_type = spvm_value_type_float;
	state->results[id].value_count = 1;
	state->results[id].value = (spvm_value*)calloc(1, sizeof(spvm_value));
	state->results[id].pointer = res_type;
	state->results[id].value[0].f = state->results[op1].value[0].f + state->results[op2].value[0].f;
}
void spvm_execute_OpReturn(spvm_word word_count, spvm_state_t state)
{
	state->return_id = -1;
}
void spvm_execute_OpReturnValue(spvm_word word_count, spvm_state_t state)
{
	state->return_id = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpLabel(spvm_word word_count, spvm_state_t state)
{
	SPVM_READ_WORD(state->code_current);

	state->function_reached_label = 1;
	state->current_function->function_start = state->code_current;
}




void spvm_program_create_opcode_table(spvm_program_t prog)
{
	prog->opcode_table = (spvm_opcode_func*)calloc(404, sizeof(spvm_opcode_func));

	prog->opcode_table[SpvOpNop] = spvm_execute_OpNop;

	prog->opcode_table[SpvOpSource] = spvm_execute_OpSource;
	prog->opcode_table[SpvOpSourceExtension] = spvm_execute_OpSourceExtension;
	prog->opcode_table[SpvOpName] = spvm_execute_OpName;
	prog->opcode_table[SpvOpString] = spvm_execute_OpString;
	prog->opcode_table[SpvOpLine] = spvm_execute_OpLine;
	prog->opcode_table[SpvOpNoLine] = spvm_execute_OpNoLine;

	prog->opcode_table[SpvOpDecorate] = spvm_execute_OpDecorate;

	prog->opcode_table[SpvOpExtInstImport] = spvm_execute_OpExtInstImport;

	prog->opcode_table[SpvOpMemoryModel] = spvm_execute_OpMemoryModel;
	prog->opcode_table[SpvOpEntryPoint] = spvm_execute_OpEntryPoint;
	prog->opcode_table[SpvOpCapability] = spvm_execute_OpCapability;

	prog->opcode_table[SpvOpTypeVoid] = spvm_execute_OpTypeVoid;
	prog->opcode_table[SpvOpTypeBool] = spvm_execute_OpTypeBool;
	prog->opcode_table[SpvOpTypeInt] = spvm_execute_OpTypeInt;
	prog->opcode_table[SpvOpTypeFloat] = spvm_execute_OpTypeFloat;
	prog->opcode_table[SpvOpTypeVector] = spvm_execute_OpTypeVector;
	prog->opcode_table[SpvOpTypeMatrix] = spvm_execute_OpTypeMatrix;
	prog->opcode_table[SpvOpTypeArray] = spvm_execute_OpTypeArray;
	prog->opcode_table[SpvOpTypePointer] = spvm_execute_OpTypePointer;
	prog->opcode_table[SpvOpTypeFunction] = spvm_execute_OpTypeFunction;

	prog->opcode_table[SpvOpConstantTrue] = spvm_execute_OpConstantTrue;
	prog->opcode_table[SpvOpConstantFalse] = spvm_execute_OpConstantFalse;
	prog->opcode_table[SpvOpConstant] = spvm_execute_OpConstant;
	prog->opcode_table[SpvOpConstantComposite] = spvm_execute_OpConstantComposite;

	prog->opcode_table[SpvOpVariable] = spvm_execute_OpVariable;
	prog->opcode_table[SpvOpStore] = spvm_execute_OpStore;
	prog->opcode_table[SpvOpLoad] = spvm_execute_OpLoad;
	prog->opcode_table[SpvOpCopyMemory] = spvm_execute_OpCopyMemory;
	prog->opcode_table[SpvOpCopyMemorySized] = spvm_execute_OpCopyMemorySized;
	prog->opcode_table[SpvOpAccessChain] = spvm_execute_OpAccessChain;
	prog->opcode_table[SpvOpPtrEqual] = spvm_execute_OpPtrEqual;
	prog->opcode_table[SpvOpPtrNotEqual] = spvm_execute_OpPtrNotEqual;
	prog->opcode_table[SpvOpPtrDiff] = spvm_execute_OpPtrNotEqual;
	prog->opcode_table[SpvOpFunction] = spvm_execute_OpFunction;
	prog->opcode_table[SpvOpFunctionParameter] = spvm_execute_OpFunctionParameter;
	prog->opcode_table[SpvOpFunctionEnd] = spvm_execute_OpFunctionEnd;
	prog->opcode_table[SpvOpFunctionCall] = spvm_execute_OpFunctionCall;
	prog->opcode_table[SpvOpReturn] = spvm_execute_OpReturn;
	prog->opcode_table[SpvOpReturnValue] = spvm_execute_OpReturnValue;

	prog->opcode_table[SpvOpFAdd] = spvm_execute_OpFAdd;
	prog->opcode_table[SpvOpLabel] = spvm_execute_OpLabel;
	prog->opcode_table[SpvOpCompositeConstruct] = spvm_execute_OpCompositeConstruct;

}