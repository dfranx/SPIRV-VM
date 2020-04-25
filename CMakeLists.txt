cmake_minimum_required(VERSION 3.1)
project(SPIRVVM)

# include_directories(inc)
set(SOURCES 
src/program.c
src/types.c
src/image.c
src/result.c
src/state.c
src/value.c
src/context.c
src/opcode_execute.c
src/opcode_setup.c
src/ext/GLSL450.c
)

add_library(SPIRVVM STATIC ${SOURCES})

if (UNIX)
	target_link_libraries(SPIRVVM m)
endif()

target_include_directories(SPIRVVM PRIVATE inc)
