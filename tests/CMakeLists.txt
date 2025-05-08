set(TEST_SOURCES config.c values.c op.c errors.c lexer.c conditional.c loops.c var.c functions.c module.c)

foreach(TEST_FILE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_FILE})
    target_compile_options(${TEST_NAME} PRIVATE -gdwarf-2)
    add_test(NAME "Test_${TEST_NAME}" COMMAND ${TEST_NAME})
endforeach()
