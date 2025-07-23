cmake_minimum_required(VERSION 3.29.0)

add_library(sepl
    lex.c
    val.c
    mod.c
)
target_include_directories(sepl PUBLIC .)

option(BUILD_SINGLE_HEADER "BUILD SINGLE HEADER FILE" ON)
if (BUILD_SINGLE_HEADER)
    add_executable(combiner ../scripts/combiner.c)
    target_compile_definitions(combiner PRIVATE ROOT_DIR="./")
    add_dependencies(sepl combiner)

    add_custom_command(
        TARGET sepl
        PRE_BUILD
        COMMAND combiner
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running combiner script to generate single header file: sepl.h"
    )
endif()
