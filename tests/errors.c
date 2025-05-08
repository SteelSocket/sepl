#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

SeplError compile_s(const char *src, unsigned char bytes[], size_t n1,
                    SeplValue values[], size_t n2) {
    SeplEnv env = {0};
    SeplModule mod = sepl_mod_new(bytes, n1, values, n2);
    SeplError err = sepl_com_block(src, &mod, env);
    return err;
}

SeplError run_s(const char *src, unsigned char bytes[], size_t n1,
                SeplValue values[], size_t n2) {
    SeplEnv env = {0};
    SeplModule mod = sepl_mod_new(bytes, n1, values, n2);
    SeplError err = sepl_com_block(src, &mod, env);
    if (err.code != SEPL_ERR_OK) {
        fprintf(stderr, "\nFailed to compile:\n%s\n", src);
        assert(err.code == SEPL_ERR_OK);
    }

    SeplValue val = sepl_mod_exec(&mod, &err, env);
    return err;
}

void memory_errors() {
#define compile(src, bn, vn) (compile_s(src, bytes, bn, values, vn).code)
    unsigned char bytes[1024];
    SeplValue values[100];
    // Not enough bytecode buffer
    assert(compile("{ return 10 + 20 + 30; }", 1, 100) == SEPL_ERR_BOVERFLOW);
    // Not enough value buffer
    assert(compile("{ return 10 + 20 + 30; }", 1024, 1) == SEPL_ERR_VOVERFLOW);
    // Not enough value buffer
    assert(compile("{ @a = 20; @b = 20; }", 1024, 1) == SEPL_ERR_VOVERFLOW);
#undef compile
}

void compile_time_errors() {
    unsigned char bytes[1024];
    SeplValue values[100];
#define compile(src) (compile_s(src, bytes, 1024, values, 100).code)
#define error_info(src) (compile_s(src, bytes, 1024, values, 100).info)

    // Missing Semicolon
    assert(compile("{ 139 }") == SEPL_ERR_UPTOK);
    assert(error_info("{ 139 }").tokd.exp == SEPL_TOK_SEMICOLON);

    // Invalid Number
    assert(compile("{ 139a; }") == SEPL_ERR_SYN);

    // Undefined Identifier
    assert(compile("{ value; }") == SEPL_ERR_IDEN_NDEF);
    // Undefined Function
    assert(compile("{ value(); }") == SEPL_ERR_IDEN_NDEF);
    // Undefined Variable Assign
    assert(compile("{ a = 20; }") == SEPL_ERR_IDEN_NDEF);
    // Undefined Variable Init
    assert(compile("{ @a = b; }") == SEPL_ERR_IDEN_NDEF);

    // Invalid Variable Assign
    assert(compile("{ 1 = 20; }") == SEPL_ERR_UPTOK);
    assert(error_info("{ 1 = 20; }").tokd.up.type == SEPL_TOK_ASSIGN);
    // Invalid Variable Init
    assert(compile("{ @20 = 4; }") == SEPL_ERR_UPTOK);
    assert(error_info("{ @20 = 4; }").tokd.up.type == SEPL_TOK_NUM);

    // Variable Redefinition
    assert(compile("{ @a = 20; @a = 10; }") == SEPL_ERR_IDEN_RDEF);

    // No if conditional expression
    assert(compile("{if () {}}") == SEPL_ERR_EEXPR);
    // No while conditional expression
    assert(compile("{while () {}}") == SEPL_ERR_EEXPR);

    // No if statement
    assert(compile("{else {}}") == SEPL_ERR_UPTOK);
    // No if body
    assert(compile("{if (1)}") == SEPL_ERR_UPTOK);
    // No else body
    assert(compile("{if (1) {} else}") == SEPL_ERR_UPTOK);

    // Closure not supported
    assert(compile("{ @a = $(){ @b = $(){}; }; }") == SEPL_ERR_CLOSURE);
    // Function cannot be set to upvalue
    assert(compile("{ @a; { a = $(){}; }; }") == SEPL_ERR_FUNC_UPV);
}

void run_time_errors() {
    unsigned char bytes[1024];
    SeplValue values[100];
#define run(src) (run_s(src, bytes, 1024, values, 100).code)

    // Attempting to call const
    assert(run("{ 120(); }") == SEPL_ERR_FUNC_CALL);
    // Attempting to call non function variable
    assert(run("{ @a = 20; a(); }") == SEPL_ERR_FUNC_CALL);
    // Attempting return function. Not supported
    assert(run("{ @a = $(){}; return a; }") == SEPL_ERR_FUNC_RET);
    // Overwriting function name in param and calling the param
    assert(run("{ @a = $(a){ a(2); }; a(1); }") == SEPL_ERR_FUNC_CALL);
}

SEPL_TEST_GROUP(memory_errors, compile_time_errors, run_time_errors)
