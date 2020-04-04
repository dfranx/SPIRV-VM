cmake_minimum_required(VERSION 3.1)
project(SPIRVVM)

# include_directories(inc)
set(SOURCES 
main.c
src/opcode.c
src/program.c
src/types.c
src/result.c
src/state.c
src/value.c
)

add_executable(SPIRVVM ${SOURCES})

target_include_directories(SPIRVVM PRIVATE inc)
