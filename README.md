# SPIRV-VM
**SPIRV-VM** is a virtual machine for executing SPIR-V shaders. It is written in C, has no dependencies & is licensed under MIT license.
Both **HLSL & GLSL shaders** can be compiled to SPIR-V using tools such as [glslangValidator](https://github.com/KhronosGroup/glslang) and [shaderc](https://github.com/google/shaderc) which means
that you can use this library to debug shaders.

## Example
First, create a SPIRV-VM context:
```c
spvm_context_t ctx = spvm_context_initialize();
```

Load your SPIR-V binary file:
```c
size_t spv_length = 0;
spvm_source spv = load_source("shader.spv", &spv_length);
```

Now you can create a SPIR-V program and a state. The program holds general information about 
the SPIR-V file (like generator version, used capabilities, etc...) while the `spvm_state` holds information
about state of program while executing it. The idea behind states is to make it possible, for example,
to create four states for one program and run the same shader simultaneously in 4 different threads (this is not tested though):
```c
spvm_program_t prog = spvm_program_create(ctx, spv, spv_length);
spvm_state_t state = spvm_state_create(prog);
```

You have to add extensions manually (if your program uses it):
```c
spvm_ext_opcode_func* glsl_ext_data = spvm_build_glsl450_ext();
spvm_result_t glsl_std_450 = spvm_state_get_result(state, "GLSL.std.450");
if (glsl_std_450)
	glsl_std_450->extension = glsl_ext_data;
```


Before debugging your shader, you have to initialize global and uniform variables.

To set a single uniform:
```c
float someUniformData[2] = { 0.5f, 0.6f };
spvm_result_t someUniform = spvm_state_get_result(state, "someUniform");
spvm_member_set_value_f(someUniform->members, someUniform->member_count, someUniformData); // vec2
```

But if you are using newer GLSL version, you probably have to deal with interface blocks:
```c
// first get the block
spvm_result_t uBlock = spvm_state_get_result(state, "uBlock");

// then set its members
float timeData = 0.5f;
spvm_member_t uBlock_time = spvm_state_get_object_member(state, uBlock, "time"); // uBlock.time
spvm_member_set_value_f(uBlock_time->members, uBlock_time->member_count, &timeData);
```

To bind textures:
```c
spvm_image noise2D_data;
spvm_image_create(&noise2D_data, image_data, image_width, image_height, 1);
spvm_result_t noise2D = spvm_state_get_result(state, "noise2D");
noise2D->members[0].image_data = &noise2D_data;
```

You can run your shader in two different ways. You can step line by line through the code.
This can be done with the `spvm_state_step_into` function (which executes only one line).
Or you can execute whole shader with a single function call:
```c
spvm_word fnMain = spvm_state_get_result_location(state, "main");
spvm_state_prepare(state, fnMain);
spvm_state_call_function(state);
```

You can then retrieve results:
```c
spvm_result_t outColor = spvm_state_get_result(state, "outColor");
for (int i = 0; i < outColor->member_count; i++)
	printf("%.2f ", outColor->members[i].value.f);
printf("\n");
```

Use `spvm_state_get_local_result()` function if you are stepping through code line by line and want to get local variable's value.
Example:
```c
spvm_result_t a = spvm_state_get_local_result(state, fnMain, "a");
printf("a = %.2f\n", a->members[0].value.f);
```

Don't forget to free the memory:
```c
spvm_state_delete(state);
spvm_program_delete(prog);
free(glsl_ext_data);
free(spv);

spvm_context_deinitialize(ctx);
```

## How to link
If you are using CMake add these lines to your `CMakeLists.txt` file:
```
add_subdirectory(./path/to/your/SPIRV-VM)
target_include_directories(example PRIVATE ./path/to/your/SPIRV-VM/inc)
```

If you just want to build this project, run these two commands:
```
cmake .
make
```

## TODO
- better image support (mipmaps, image arrays, samplers)

Contact me on this e-mail address: **dfranx at shadered dot org**

## LICENSE
SPIRV-VM is licensed under MIT license. See [LICENSE](./LICENSE) for more details.