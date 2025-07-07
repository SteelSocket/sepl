#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

SeplEnv env = {0};
unsigned char bytes[1024];
SeplValue values[100];

static inline SeplModule new_mod() {
    SeplModule mod = sepl_mod_new(bytes, 1024, values, 100);
    return mod;
}

static SeplValue exec_mod(const char *source, SeplModule *mod) {
    SeplCompiler com = sepl_com_init(source, mod, env);
    sepl_com_module(&com);
    SeplError err = sepl_com_finish(&com);
    assert(err.code == SEPL_ERR_OK);
    sepl_mod_init(mod, &err, env);
    SeplValue v = sepl_mod_exec(mod, &err, env);
    assert(err.code == SEPL_ERR_OK);
    return v;
}

static SeplValue exec_main(const char *source, SeplArgs args) {
    SeplModule mod = new_mod();
    SeplError err = {0};
    const char *exports[] = {"main"};
    mod.exports = exports;
    mod.esize = 1;

    exec_mod(source, &mod);
    SeplValue main = sepl_mod_getexport(&mod, env, "main");
    assert(main.type == SEPL_VAL_FUNC);

    sepl_mod_initfunc(&mod, &err, main, args);
    return sepl_mod_exec(&mod, &err, env);
}

#define assert_vpos(size, expr)                            \
    do {                                                   \
        SeplModule mod = new_mod();                        \
        assert((exec_mod(#expr, &mod), mod.vpos == size)); \
    } while (0)
#define assert_main(expr, args, value)                  \
    do {                                                \
        assert(exec_main(#expr, args).as.num == value); \
    } while (0)

void vpos_test() {
    assert_vpos(1, @a = 10;);
    assert_vpos(2, @a = 10; @b = 320;);
    assert_vpos(2, @a = 10; @b = 320; a = 20;);
}

void export_tests() {
    SeplModule mod = new_mod();
    const char *exports[] = {"a", "b", "c", "d", "main"};
    mod.exports = exports;
    mod.esize = 5;
    exec_mod("a = 10; b = a; d = { return 20; }; main = $(){ return 200; }; ",
             &mod);

    assert(mod.vpos == 5);
    assert(sepl_mod_getexport(&mod, env, "a").as.num == 10.0);
    assert(sepl_mod_getexport(&mod, env, "b").type == SEPL_VAL_NUM);
    assert(sepl_mod_getexport(&mod, env, "c").type == SEPL_VAL_NONE);
    assert(sepl_mod_getexport(&mod, env, "d").as.num == 20.0);
    assert(sepl_mod_getexport(&mod, env, "main").type == SEPL_VAL_FUNC);
}

void main_test() {
    SeplArgs args = {0};
    assert_main(main = $() { return 10; };, args, 10.0);

    args.values = (SeplValue[]){sepl_val_number(3.14)};
    args.size = 1;
    assert_main(main = $(a) { return a; };, args, 3.14);

    args.values = (SeplValue[]){sepl_val_number(1)};
    args.size = 1;
    assert_main(main = $(a, b) { return a && b; };, args, 0);

    args.values = (SeplValue[]){sepl_val_number(1), sepl_val_number(1)};
    args.size = 2;
    assert_main(main = $(a, b) { return a && b; };, args, 1);

    args.values = (SeplValue[]){sepl_val_number(0), sepl_val_number(1),
                                sepl_val_number(1)};
    args.size = 3;
    assert_main(main = $(a, b) { return a && b; };, args, 0);
}

SEPL_TEST_GROUP(vpos_test, export_tests, main_test);
