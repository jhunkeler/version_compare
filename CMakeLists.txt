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

include(CTest)

add_library(vcmp STATIC version_compare.c version_compare.h)
target_compile_definitions(vcmp PUBLIC ENABLE_TESTING=1)

add_executable(test_version_compare tests.c version_compare.h)
target_link_libraries(test_version_compare vcmp)

add_test(test test_version_compare)
add_executable(version_compare main.c version_compare.h)
target_link_libraries(version_compare vcmp)


