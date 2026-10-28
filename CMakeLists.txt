cmake_minimum_required(VERSION 3.16)
project(chip8 C)

find_package(SDL3 REQUIRED)

add_executable(chip8 chip8.c)
target_link_libraries(chip8 PRIVATE SDL3::SDL3)