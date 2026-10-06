cmake_minimum_required(VERSION 3.25)

project(chip8 VERSION 0.1.0 LANGUAGES C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(chip8 src/main.c src/vm.c)
target_include_directories(chip8 PRIVATE src)
