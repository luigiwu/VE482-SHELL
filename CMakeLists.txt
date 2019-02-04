cmake_minimum_required(VERSION 2.7)
set(CMAKE_C_STANDARD 11)
set(SOURCE_FILES)

set(CMAKE_C_FLAGS "-std=c11 -O2 -Wall -Wextra -Werror -pedantic -Wno-unused-result")
add_executable(mumsh main.c parse.h builtin.h glo_value.h exec.h)

set(CMAKE_C_FLAGS " -g -fsanitize=address -fno-omit-frame-pointer -fsanitize=undefined -fsanitize=integer")
add_executable(mumsh_memory_check main.c parse.h builtin.h glo_value.h exec.h)
