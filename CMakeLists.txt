cmake_minimum_required(VERSION 3.29.0)
project(sepl VERSION 0.0.1)

add_subdirectory(sepl/)

option(BUILD_EXAMPLES "BUILD EXAMPLES" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples/)
endif()

option(BUILD_TESTS "BUILD TESTS" OFF)
if(BUILD_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests/)
endif()

