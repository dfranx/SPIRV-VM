#include <spvm/opcode.h>
#include <spvm/state.h>
#include <spvm/spirv.h>
#include <string.h>
#include <math.h>

/*
1. split opcodes into spvm_execute_OpXYZ and spvm_setup_OpXYZ & make the opcode tables global
2. executing shaders multiple times & memory leaks
3. X bit sized values
4. textures
5. extensions
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
void spvm_execute_OpMemberName(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word memb = SPVM_READ_WORD(state->code_current);

	state->results[id].member_name_count = max(memb + 1, state->results[id].member_name_count);
	state->results[id].member_name = (spvm_string*)realloc(state->results[id].member_name, sizeof(spvm_string) * state->results[id].member_name_count);

	spvm_word slen = word_count - 2;
	state->results[id].member_name[memb] = (spvm_string)malloc(sizeof(spvm_word) * slen);
	spvm_string_read(state->code_current, state->results[id].member_name[memb], slen);
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
void spvm_execute_OpMemberDecorate(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	SpvDecoration memb = SPVM_READ_WORD(state->code_current);
	SpvDecoration decor = SPVM_READ_WORD(state->code_current);
	spvm_word literal1 = 0, literal2 = 0;

	spvm_decoration_read(state->code_current, decor, &literal1, &literal2);
	spvm_result_add_member_decoration(&state->results[target], decor, literal1, literal2, memb);
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
	state->results[store_id].member_count = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypeMatrix(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_matrix;

	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[store_id].member_count = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypeArray(spvm_word word_count, spvm_state_t state)
{
	spvm_word store_id = SPVM_READ_WORD(state->code_current);
	state->results[store_id].type = spvm_result_type_type;
	state->results[store_id].value_type = spvm_value_type_array;

	state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);
	state->results[store_id].member_count = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpTypeStruct(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word mcnt = word_count - 1;
	state->results[id].type = spvm_result_type_type;
	state->results[id].value_type = spvm_value_type_struct;
	state->results[id].member_count = mcnt;

	state->results[id].param_type = (spvm_word*)malloc(sizeof(spvm_word) * mcnt);
	for (spvm_word i = 0; i < mcnt; i++)
		state->results[id].param_type[i] = SPVM_READ_WORD(state->code_current);
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
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = SPVM_READ_WORD(state->code_current);
}
void spvm_execute_OpConstantTrue(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = 1;
}
void spvm_execute_OpConstantFalse(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = 0;
}
void spvm_execute_OpConstantComposite(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		spvm_word index = SPVM_READ_WORD(state->code_current);
		state->results[id].members[i].value.s = state->results[index].members[0].value.s;
	}
}
void spvm_execute_OpConstantNull(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);
}

/* 3.32.8 Memory Instructions */
void spvm_execute_OpVariable(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_variable;
	state->results[id].storage_class = SPVM_READ_WORD(state->code_current);

	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);
}
void spvm_execute_OpStore(spvm_word word_count, spvm_state_t state)
{
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);
	spvm_word val_id = SPVM_READ_WORD(state->code_current);

	spvm_member_memcpy(state->results[ptr_id].members, state->results[val_id].members, state->results[ptr_id].member_count);
}
void spvm_execute_OpLoad(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word ptr_id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_member_memcpy(state->results[id].members, state->results[ptr_id].members, state->results[ptr_id].member_count);
}
void spvm_execute_OpCopyMemory(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	spvm_word source = SPVM_READ_WORD(state->code_current);

	spvm_member_memcpy(state->results[target].members, state->results[source].members, state->results[target].member_count);
}
void spvm_execute_OpCopyMemorySized(spvm_word word_count, spvm_state_t state)
{
	spvm_word target = SPVM_READ_WORD(state->code_current);
	spvm_word source = SPVM_READ_WORD(state->code_current);
	spvm_word size_id = SPVM_READ_WORD(state->code_current);

	spvm_word size = state->results[size_id].members[0].value.s;

	// TODO: spvm_member_copy_sized()
	memcpy(state->results[target].members, state->results[source].members, size);
}
void spvm_execute_OpAccessChain(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word value_id = SPVM_READ_WORD(state->code_current);

	spvm_word index_count = word_count - 4;

	state->results[id].type = spvm_result_type_access_chain;
	state->results[id].pointer = var_type;

	spvm_word index_id = SPVM_READ_WORD(state->code_current);
	spvm_word index = state->results[index_id].members[0].value.s;

	spvm_member_t result = state->results[value_id].members + index;

	while (index_count) {
		index_id = SPVM_READ_WORD(state->code_current);
		index = state->results[index_id].members[0].value.s;

		result = result->members + index;

		index_count--;
	}

	if (result->member_count != 0) {
		state->results[id].member_count = result->member_count;
		state->results[id].members = result->members;
	} else {
		state->results[id].member_count = 1;
		state->results[id].members = result;
	}
}
void spvm_execute_OpPtrEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	// TODO: should this actually check if they point to the same value or if the values are same?

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);
	state->results[id].members[0].value.b = (state->results[op1].members == state->results[op2].members);
}
void spvm_execute_OpPtrNotEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	// TODO: should this actually check if they point to the same value or if the values are same?

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);
	state->results[id].members[0].value.b = (state->results[op1].members != state->results[op2].members);
}
void spvm_execute_OpPtrDiff(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	// TODO: should this actually check if they point to the same value or if the values are same?

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);
	state->results[id].members[0].value.s = (op1 - op2);
}
void spvm_execute_OpFunction(spvm_word word_count, spvm_state_t state)
{
	spvm_word ret_type = SPVM_READ_WORD(state->code_current);
	spvm_word store_id = SPVM_READ_WORD(state->code_current);

	state->results[store_id].type = spvm_result_type_function;
	state->results[store_id].return_type = ret_type;

	SPVM_READ_WORD(state->code_current); // skip function control

	spvm_word info = state->results[store_id].pointer = SPVM_READ_WORD(state->code_current);

	state->results[store_id].source_location = state->code_current;
	state->results[info].source_location = state->code_current;

	state->function_reached_label = 0;
	state->current_function = &state->results[store_id];
}
void spvm_execute_OpFunctionParameter(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_variable;
	state->results[id].storage_class = SPVM_READ_WORD(state->code_current);
	state->results[id].parameter_owner = state->current_function;

	// TODO: parameteres are pointers == memory leak here
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);
}
void spvm_execute_OpFunctionEnd(spvm_word word_count, spvm_state_t state)
{
	state->function_reached_label = 0;
}
void spvm_execute_OpFunctionCall(spvm_word word_count, spvm_state_t state)
{
	spvm_word ret_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word func = SPVM_READ_WORD(state->code_current);

	spvm_word argc = word_count - 3;

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, ret_type);

	spvm_result_t next_func = &state->results[func];

	for (spvm_word i = 0; i < argc; i++) {
		spvm_word arg_id = SPVM_READ_WORD(state->code_current);
		spvm_result_t arg = NULL;

		spvm_word cur_arg = 0;
		for (spvm_word j = 0; j < state->owner->bound; j++) {
			if (state->results[j].parameter_owner == next_func) {
				if (i == cur_arg) {
					state->results[j].members = state->results[arg_id].members;
					break;
				}
				cur_arg++;
			}
		}
	}

	spvm_state_push_function_stack(state, next_func, id);
}
void spvm_execute_OpReturn(spvm_word word_count, spvm_state_t state)
{
	state->return_id = -1;
	spvm_state_pop_function_stack(state);
}
void spvm_execute_OpReturnValue(spvm_word word_count, spvm_state_t state)
{
	state->return_id = SPVM_READ_WORD(state->code_current);
	spvm_state_pop_function_stack(state);
}

/* 3.32.10 Image Instruction */
// :(

/* 3.32.11 Conversion Instructions */
void spvm_execute_OpConvertFToU(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = (unsigned long long)state->results[val].members[i].value.f;
}
void spvm_execute_OpConvertFToS(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = (int)state->results[val].members[i].value.s;
}
void spvm_execute_OpConvertUToF(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = (float)state->results[val].members[i].value.u;
}
void spvm_execute_OpBitcast(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word val = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[val].members[i].value.u64;
}

/* 3.32.12 Composite Instructions */
void spvm_execute_OpVectorExtractDynamic(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vector = SPVM_READ_WORD(state->code_current);
	spvm_word index_id = SPVM_READ_WORD(state->code_current);

	spvm_word index = state->results[index_id].members[0].value.u64;

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);
	state->results[id].members[0].value.u64 = state->results[vector].members[index].value.u64;
}
void spvm_execute_OpVectorInsertDynamic(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vector = SPVM_READ_WORD(state->code_current);
	spvm_word comp = SPVM_READ_WORD(state->code_current);
	spvm_word index_id = SPVM_READ_WORD(state->code_current);

	spvm_word index = state->results[index_id].members[0].value.u64;

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);
	spvm_member_memcpy(state->results[id].members, state->results[vector].members, state->results[id].member_count);
	state->results[id].members[index].value.u64 = state->results[comp].members[0].value.u64;
}
void spvm_execute_OpVectorShuffle(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vector1_id = SPVM_READ_WORD(state->code_current);
	spvm_word vector2_id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_result_t vector1 = &state->results[vector1_id];
	spvm_result_t vector2 = &state->results[vector2_id];

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		spvm_word index = SPVM_READ_WORD(state->code_current);

		if (index >= vector1->member_count)
			state->results[id].members[i].value.u64 = vector2->members[index-vector1->member_count].value.u64;
		else
			state->results[id].members[i].value.u64 = vector1->members[index].value.u64;
	}
}
void spvm_execute_OpCompositeConstruct(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++) {
		spvm_word index = SPVM_READ_WORD(state->code_current);
		state->results[id].members[i].value.s = state->results[index].members[0].value.s;
	}
}
void spvm_execute_OpCompositeExtract(spvm_word word_count, spvm_state_t state)
{
	spvm_word var_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word value_id = SPVM_READ_WORD(state->code_current);

	spvm_word index_count = word_count - 4;

	state->results[id].type = spvm_result_type_access_chain;
	state->results[id].pointer = var_type;

	spvm_word index = SPVM_READ_WORD(state->code_current);

	spvm_member_t result = state->results[value_id].members + index;

	while (index_count) {
		index = SPVM_READ_WORD(state->code_current);

		result = result->members + index;

		index_count--;
	}

	spvm_result_allocate_typed_value(&state->results[id], state->results, var_type);
	spvm_member_memcpy(state->results[id].members, result, 1);
}
void spvm_execute_OpCopyObject(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_member_memcpy(state->results[id].members, state->results[op].members, state->results[op].member_count);
}
void spvm_execute_OpTranspose(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word mat = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_word s1 = state->results[mat].member_count;
	spvm_word s2 = state->results[mat].members[0].member_count;
	for (spvm_word i = 0; i < s1; i++)
		for (spvm_word j = 0; j < s2; ++j)
			state->results[id].members[j].members[i].value.u64 = state->results[mat].members[i].members[j].value.u64;
}

/* 3.32.13 Arithmetic Instructions */
void spvm_execute_OpSNegate(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = -state->results[op].members[i].value.s;
}
void spvm_execute_OpFNegate(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = -state->results[op].members[i].value.s;
}
void spvm_execute_OpIAdd(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s + state->results[op2].members[i].value.s;
}
void spvm_execute_OpFAdd(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = state->results[op1].members[i].value.f + state->results[op2].members[i].value.f;
}
void spvm_execute_OpISub(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s - state->results[op2].members[i].value.s;
}
void spvm_execute_OpFSub(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = state->results[op1].members[i].value.f - state->results[op2].members[i].value.f;
}
void spvm_execute_OpIMul(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s * state->results[op2].members[i].value.s;
}
void spvm_execute_OpFMul(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = state->results[op1].members[i].value.f * state->results[op2].members[i].value.f;
}
void spvm_execute_OpUDiv(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 / state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSDiv(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s / state->results[op2].members[i].value.s;
}
void spvm_execute_OpFDiv(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = state->results[op1].members[i].value.f / state->results[op2].members[i].value.f;
}
void spvm_execute_OpUMod(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 % state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSMod(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.s = state->results[op1].members[i].value.s % state->results[op2].members[i].value.s;
}
void spvm_execute_OpFMod(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = fmodf(state->results[op1].members[i].value.f, state->results[op2].members[i].value.f);
}
void spvm_execute_OpVectorTimesScalar(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);
	spvm_word scalar = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f = state->results[vec].members[i].value.f * state->results[scalar].members[0].value.f;
}
void spvm_execute_OpMatrixTimesScalar(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word mat = SPVM_READ_WORD(state->code_current);
	spvm_word scalar = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_word s1 = state->results[mat].member_count;
	spvm_word s2 = state->results[mat].members[0].member_count;
	for (spvm_word i = 0; i < s1; i++)
		for (spvm_word j = 0; j < s2; j++)
			state->results[id].members[i].members[j].value.f = state->results[mat].members[i].members[j].value.f * state->results[scalar].members[0].value.f;
}
void spvm_execute_OpVectorTimesMatrix(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);
	spvm_word mat = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_word resSize = state->results[id].member_count;
	spvm_word mySize = state->results[vec].member_count;
	for (spvm_word i = 0; i < resSize; i++) {
		float res = 0.0f;
		
		for (spvm_word j = 0; j < mySize; j++)
			res += state->results[mat].members[i].members[j].value.f * state->results[vec].members[j].value.f;

		state->results[id].members[i].value.f = res;
	}
}
void spvm_execute_OpMatrixTimesVector(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word mat = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_word resSize = state->results[id].member_count;
	spvm_word mySize = state->results[vec].member_count;
	for (spvm_word i = 0; i < resSize; i++) {
		float res = 0.0f;

		for (spvm_word j = 0; j < mySize; j++)
			res += state->results[mat].members[j].members[i].value.f * state->results[vec].members[j].value.f;

		state->results[id].members[i].value.f = res;
	}
}
void spvm_execute_OpMatrixTimesMatrix(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word left = SPVM_READ_WORD(state->code_current);
	spvm_word right = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_word s1 = state->results[id].member_count;
	spvm_word s2 = state->results[id].members[0].member_count;
	for (spvm_word i = 0; i < s1; i++)
		for (spvm_word j = 0; j < s2; j++)
			state->results[id].members[i].members[j].value.f = state->results[left].members[i].members[j].value.f * state->results[right].members[i].members[j].value.f;
}
void spvm_execute_OpOuterProduct(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec1 = SPVM_READ_WORD(state->code_current);
	spvm_word vec2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_word s1 = state->results[id].member_count;
	spvm_word s2 = state->results[id].members[0].member_count;
	for (spvm_word i = 0; i < s1; i++)
		for (spvm_word j = 0; j < s2; j++)
			state->results[id].members[i].members[j].value.f = state->results[vec1].members[i].value.f * state->results[vec2].members[j].value.f;
}
void spvm_execute_OpDot(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec1 = SPVM_READ_WORD(state->code_current);
	spvm_word vec2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.f += state->results[vec1].members[i].value.f * state->results[vec2].members[i].value.f;
}

/* 3.32.14 Bit Instructions */
void spvm_execute_OpBitwiseOr(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 | state->results[op2].members[i].value.u64;
}
void spvm_execute_OpBitwiseAnd(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 & state->results[op2].members[i].value.u64;
}
void spvm_execute_OpBitwiseXor(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = state->results[op1].members[i].value.u64 ^ state->results[op2].members[i].value.u64;
}
void spvm_execute_OpNot(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.u64 = ~state->results[op1].members[i].value.u64;
}

/* 3.32.15 Relational and Logical Instructions */
void spvm_execute_OpAny(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_byte result = 0;
	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		if (state->results[vec].members[i].value.b)
			result = 1;
	state->results[id].members[0].value.b = result;
}
void spvm_execute_OpAll(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word vec = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	spvm_byte result = 1;
	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		if (!state->results[vec].members[i].value.b)
			result = 0;
	state->results[id].members[0].value.b = result;
}
void spvm_execute_OpIsNan(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word x = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = isnan(state->results[x].members[i].value.f);
}
void spvm_execute_OpIsInf(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word x = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = isinf(state->results[x].members[i].value.f);
}
void spvm_execute_OpLogicalEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.b == state->results[op2].members[i].value.b;
}
void spvm_execute_OpLogicalNotEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.b != state->results[op2].members[i].value.b;
}
void spvm_execute_OpLogicalAnd(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.b && state->results[op2].members[i].value.b;
}
void spvm_execute_OpLogicalOr(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.b || state->results[op2].members[i].value.b;
}
void spvm_execute_OpLogicalNot(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = !state->results[op1].members[i].value.b;
}
void spvm_execute_OpIEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s == state->results[op2].members[i].value.s;
}
void spvm_execute_OpINotEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s != state->results[op2].members[i].value.s;
}
void spvm_execute_OpUGreaterThan(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.u64 > state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSGreaterThan(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s > state->results[op2].members[i].value.s;
}
void spvm_execute_OpUGreaterThanEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.u64 >= state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSGreaterThanEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s >= state->results[op2].members[i].value.s;
}
void spvm_execute_OpULessThan(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.u64 < state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSLessThan(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s < state->results[op2].members[i].value.s;
}
void spvm_execute_OpULessThanEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.u64 <= state->results[op2].members[i].value.u64;
}
void spvm_execute_OpSLessThanEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.s <= state->results[op2].members[i].value.s;
}
void spvm_execute_OpFOrdEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.f == state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdNotEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.f != state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdLessThan(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.f < state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdGreaterThan(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.f > state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdLessThanEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.f <= state->results[op2].members[i].value.f;
}
void spvm_execute_OpFOrdGreaterThanEqual(spvm_word word_count, spvm_state_t state)
{
	spvm_word res_type = SPVM_READ_WORD(state->code_current);
	spvm_word id = SPVM_READ_WORD(state->code_current);
	spvm_word op1 = SPVM_READ_WORD(state->code_current);
	spvm_word op2 = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_constant;
	spvm_result_allocate_typed_value(&state->results[id], state->results, res_type);

	for (spvm_word i = 0; i < state->results[id].member_count; i++)
		state->results[id].members[i].value.b = state->results[op1].members[i].value.f >= state->results[op2].members[i].value.f;
}

/* 3.32.17 Control-Flow Instructions */
void spvm_execute_OpLabel(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);

	state->results[id].type = spvm_result_type_label;
	state->results[id].source_location = state->code_current;

	if (state->parsing && !state->function_reached_label)
		state->current_function->source_location = state->code_current;

	state->function_reached_label = 1;
}
void spvm_execute_OpBranch(spvm_word word_count, spvm_state_t state)
{
	spvm_word id = SPVM_READ_WORD(state->code_current);
	state->code_current = state->results[id].source_location;

	state->did_jump = 1;
}
void spvm_execute_OpBranchConditional(spvm_word word_count, spvm_state_t state)
{
	spvm_word cond = SPVM_READ_WORD(state->code_current);
	spvm_word true_branch = SPVM_READ_WORD(state->code_current);
	spvm_word false_branch = SPVM_READ_WORD(state->code_current);

	if (state->results[cond].members[0].value.b)
		state->code_current = state->results[true_branch].source_location;
	else
		state->code_current = state->results[false_branch].source_location;

	state->did_jump = 1;
}
void spvm_execute_OpSwitch(spvm_word word_count, spvm_state_t state)
{
	spvm_word sel = SPVM_READ_WORD(state->code_current);
	spvm_word def_lbl = SPVM_READ_WORD(state->code_current);

	spvm_word case_count = (word_count - 2) / 2;

	spvm_word val = state->results[sel].members[0].value.s;

	spvm_byte found = 0;
	for (spvm_word i = 0; i < case_count; i++) {
		spvm_word lit = SPVM_READ_WORD(state->code_current);
		spvm_word lbl = SPVM_READ_WORD(state->code_current);

		if (val == lit) {
			state->code_current = state->results[lbl].source_location;
			found = 1;
			break;
		}
	}

	if (!found)
		state->code_current = state->results[def_lbl].source_location;

	state->did_jump = 1;
}
void spvm_execute_OpKill(spvm_word word_count, spvm_state_t state)
{
	state->code_current = NULL;
	state->did_jump = 1;
	state->discarded = 1;
}

void spvm_program_create_opcode_table(spvm_program_t prog)
{
	prog->opcode_table = (spvm_opcode_func*)calloc(404, sizeof(spvm_opcode_func));

	prog->opcode_table[SpvOpNop] = spvm_execute_OpNop;

	prog->opcode_table[SpvOpSource] = spvm_execute_OpSource;
	prog->opcode_table[SpvOpSourceExtension] = spvm_execute_OpSourceExtension;
	prog->opcode_table[SpvOpName] = spvm_execute_OpName;
	prog->opcode_table[SpvOpMemberName] = spvm_execute_OpMemberName;
	prog->opcode_table[SpvOpString] = spvm_execute_OpString;
	prog->opcode_table[SpvOpLine] = spvm_execute_OpLine;
	prog->opcode_table[SpvOpNoLine] = spvm_execute_OpNoLine;

	prog->opcode_table[SpvOpDecorate] = spvm_execute_OpDecorate;
	prog->opcode_table[SpvOpMemberDecorate] = spvm_execute_OpMemberDecorate;
	
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
	prog->opcode_table[SpvOpTypeStruct] = spvm_execute_OpTypeStruct;
	prog->opcode_table[SpvOpTypePointer] = spvm_execute_OpTypePointer;
	prog->opcode_table[SpvOpTypeFunction] = spvm_execute_OpTypeFunction;

	prog->opcode_table[SpvOpConstantTrue] = spvm_execute_OpConstantTrue;
	prog->opcode_table[SpvOpConstantFalse] = spvm_execute_OpConstantFalse;
	prog->opcode_table[SpvOpConstant] = spvm_execute_OpConstant;
	prog->opcode_table[SpvOpConstantComposite] = spvm_execute_OpConstantComposite;
	prog->opcode_table[SpvOpConstantNull] = spvm_execute_OpConstantNull;
	
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

	prog->opcode_table[SpvOpConvertFToU] = spvm_execute_OpConvertFToU;
	prog->opcode_table[SpvOpConvertFToS] = spvm_execute_OpConvertFToS;
	prog->opcode_table[SpvOpConvertUToF] = spvm_execute_OpConvertUToF;
	prog->opcode_table[SpvOpBitcast] = spvm_execute_OpBitcast;

	prog->opcode_table[SpvOpVectorExtractDynamic] = spvm_execute_OpVectorExtractDynamic;
	prog->opcode_table[SpvOpVectorInsertDynamic] = spvm_execute_OpVectorInsertDynamic;
	prog->opcode_table[SpvOpVectorShuffle] = spvm_execute_OpVectorShuffle;
	prog->opcode_table[SpvOpCompositeConstruct] = spvm_execute_OpCompositeConstruct;
	prog->opcode_table[SpvOpCompositeExtract] = spvm_execute_OpCompositeExtract;
	prog->opcode_table[SpvOpCopyObject] = spvm_execute_OpCopyObject;
	prog->opcode_table[SpvOpTranspose] = spvm_execute_OpTranspose;
	
	prog->opcode_table[SpvOpSNegate] = spvm_execute_OpSNegate;
	prog->opcode_table[SpvOpFNegate] = spvm_execute_OpFNegate;
	prog->opcode_table[SpvOpIAdd] = spvm_execute_OpIAdd;
	prog->opcode_table[SpvOpFAdd] = spvm_execute_OpFAdd;
	prog->opcode_table[SpvOpISub] = spvm_execute_OpISub;
	prog->opcode_table[SpvOpFSub] = spvm_execute_OpFSub;
	prog->opcode_table[SpvOpIMul] = spvm_execute_OpIMul;
	prog->opcode_table[SpvOpFMul] = spvm_execute_OpFMul;
	prog->opcode_table[SpvOpUDiv] = spvm_execute_OpUDiv;
	prog->opcode_table[SpvOpSDiv] = spvm_execute_OpSDiv;
	prog->opcode_table[SpvOpFDiv] = spvm_execute_OpFDiv;
	prog->opcode_table[SpvOpUMod] = spvm_execute_OpUMod;
	prog->opcode_table[SpvOpSMod] = spvm_execute_OpSMod;
	prog->opcode_table[SpvOpFMod] = spvm_execute_OpFMod;
	prog->opcode_table[SpvOpVectorTimesScalar] = spvm_execute_OpVectorTimesScalar;
	prog->opcode_table[SpvOpMatrixTimesScalar] = spvm_execute_OpMatrixTimesScalar;
	prog->opcode_table[SpvOpVectorTimesMatrix] = spvm_execute_OpVectorTimesMatrix;
	prog->opcode_table[SpvOpMatrixTimesVector] = spvm_execute_OpMatrixTimesVector;
	prog->opcode_table[SpvOpMatrixTimesMatrix] = spvm_execute_OpMatrixTimesMatrix;
	prog->opcode_table[SpvOpOuterProduct] = spvm_execute_OpOuterProduct;
	prog->opcode_table[SpvOpDot] = spvm_execute_OpDot;

	prog->opcode_table[SpvOpBitwiseOr] = spvm_execute_OpBitwiseOr;
	prog->opcode_table[SpvOpBitwiseAnd] = spvm_execute_OpBitwiseAnd;
	prog->opcode_table[SpvOpBitwiseXor] = spvm_execute_OpBitwiseXor;
	prog->opcode_table[SpvOpNot] = spvm_execute_OpNot;

	prog->opcode_table[SpvOpAny] = spvm_execute_OpAny;
	prog->opcode_table[SpvOpAll] = spvm_execute_OpAll;
	prog->opcode_table[SpvOpIsNan] = spvm_execute_OpIsNan;
	prog->opcode_table[SpvOpIsInf] = spvm_execute_OpIsInf;
	prog->opcode_table[SpvOpLogicalEqual] = spvm_execute_OpLogicalEqual;
	prog->opcode_table[SpvOpLogicalNotEqual] = spvm_execute_OpLogicalNotEqual;
	prog->opcode_table[SpvOpLogicalAnd] = spvm_execute_OpLogicalAnd;
	prog->opcode_table[SpvOpLogicalOr] = spvm_execute_OpLogicalOr;
	prog->opcode_table[SpvOpLogicalNot] = spvm_execute_OpLogicalNot;
	prog->opcode_table[SpvOpIEqual] = spvm_execute_OpIEqual;
	prog->opcode_table[SpvOpINotEqual] = spvm_execute_OpINotEqual;
	prog->opcode_table[SpvOpUGreaterThan] = spvm_execute_OpUGreaterThan;
	prog->opcode_table[SpvOpSGreaterThan] = spvm_execute_OpSGreaterThan;
	prog->opcode_table[SpvOpUGreaterThanEqual] = spvm_execute_OpUGreaterThanEqual;
	prog->opcode_table[SpvOpSGreaterThanEqual] = spvm_execute_OpSGreaterThanEqual;
	prog->opcode_table[SpvOpULessThan] = spvm_execute_OpULessThan;
	prog->opcode_table[SpvOpSLessThan] = spvm_execute_OpSLessThan;
	prog->opcode_table[SpvOpULessThanEqual] = spvm_execute_OpULessThanEqual;
	prog->opcode_table[SpvOpSLessThanEqual] = spvm_execute_OpSLessThanEqual;
	prog->opcode_table[SpvOpFOrdEqual] = spvm_execute_OpFOrdEqual;
	prog->opcode_table[SpvOpFOrdNotEqual] = spvm_execute_OpFOrdNotEqual;
	prog->opcode_table[SpvOpFOrdLessThan] = spvm_execute_OpFOrdLessThan;
	prog->opcode_table[SpvOpFOrdGreaterThan] = spvm_execute_OpFOrdGreaterThan;
	prog->opcode_table[SpvOpFOrdLessThanEqual] = spvm_execute_OpFOrdLessThanEqual;
	prog->opcode_table[SpvOpFOrdGreaterThanEqual] = spvm_execute_OpFOrdGreaterThanEqual;

	prog->opcode_table[SpvOpLabel] = spvm_execute_OpLabel;
	prog->opcode_table[SpvOpBranch] = spvm_execute_OpBranch;
	prog->opcode_table[SpvOpBranchConditional] = spvm_execute_OpBranchConditional;
	prog->opcode_table[SpvOpSwitch] = spvm_execute_OpSwitch;
	prog->opcode_table[SpvOpKill] = spvm_execute_OpKill;
}