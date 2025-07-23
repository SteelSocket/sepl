cmake_minimum_required(VERSION 3.29.0)
project(sepl VERSION 0.0.1)

add_subdirectory(sepl/)

option(SEPL_EXAMPLES "BUILD EXAMPLES" ON)
if(SEPL_EXAMPLES)
    add_subdirectory(examples/)
endif()

option(SEPL_TESTS "BUILD TESTS" OFF)
if(SEPL_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests/)
endif()

