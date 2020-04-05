cmake_minimum_required(VERSION 3.1)
project(SPIRVVM)

# include_directories(inc)
set(SOURCES 
main.c
src/program.c
src/types.c
src/result.c
src/state.c
src/value.c
src/context.c
src/opcode_execute.c
src/opcode_setup.c
)

add_executable(SPIRVVM ${SOURCES})
target_link_libraries(SPIRVVM m)

target_include_directories(SPIRVVM PRIVATE inc)
