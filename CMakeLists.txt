cmake_minimum_required(VERSION 3.0)
project(version_compare C)

set(CMAKE_C_STANDARD 99)

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_executable(version_compare main.c)
