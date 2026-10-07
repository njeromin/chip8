cmake_minimum_required(VERSION 3.25)

project(chip8 VERSION 0.1.0 LANGUAGES C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(chip8 src/main.c src/vm/vm.c src/vm/opcode.c src/io/display.c src/platform/thread.c)
target_include_directories(chip8 PRIVATE src)
