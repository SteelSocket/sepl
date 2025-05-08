#ifndef SEPL_TESTS
#define SEPL_TESTS

#include "../sepl.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

static inline SeplValue tst_run(const char *src) {
    SeplEnv env = {0};
    unsigned char bytes[1024];
    SeplValue values[100];

    SeplModule mod = sepl_mod_new(bytes, 1024, values, 1024);
    SeplError err = sepl_com_block(src, &mod, env);

    if (err.code != SEPL_ERR_OK) {
        fprintf(stderr, "\nFailed to compile:\n%s\n", src);
        assert(err.code == SEPL_ERR_OK);
    }

    SeplValue val = sepl_mod_exec(&mod, &err, env);

    if (err.code != SEPL_ERR_OK) {
        fprintf(stderr, "\nFailed to run:\n%s\nError code: %d\n", src,
                err.code);
        assert(err.code == SEPL_ERR_OK);
    }
    return val;
}

#define assert_str(expr, expected) assert(tst_run(expr).as.num == expected)
#define assert_sepl(expr, expected) assert_str(#expr, expected)
#define assert_sepl_none(expr) assert(tst_run(#expr).type == SEPL_VAL_NONE)

#define SEPL_TEST_GROUP(...)                                                  \
    int main() {                                                              \
        void (*test_funcs[])() = {__VA_ARGS__};                               \
        unsigned long long size = sizeof(test_funcs) / sizeof(test_funcs[0]); \
        int i;                                                                \
        for (i = 0; i < size; i++) {                                          \
            test_funcs[i]();                                                  \
        }                                                                     \
        return 0;                                                             \
    }

#endif
