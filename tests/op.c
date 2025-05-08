#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

/* Check to see if sepl and c expressions have the same output */
#define assert_sepl_to_c(expr)                                        \
    do {                                                              \
        double sepl_result = tst_run("{return " #expr ";}").as.num;   \
        double c_result = (expr);                                     \
        if (sepl_result != c_result) {                                \
            fprintf(stderr, "\n%lf == %lf\n", sepl_result, c_result); \
            assert(sepl_result == c_result);                          \
        }                                                             \
    } while (0)

void check_arithmetic() {
    assert_sepl_to_c(10 + 20);

    assert_sepl_to_c(10 - 20);
    assert_sepl_to_c(20 - 10);
    assert_sepl_to_c(20 - 20);

    assert_sepl_to_c(10 * 2);
    assert_sepl_to_c(10 * -2);

    assert_sepl_to_c(10.0 / 2.0);
    assert_sepl_to_c(10.0 / 3);
    assert_sepl_to_c(3.0 / 10.0);
    assert_sepl_to_c(10.0 / -2.0);
}

void check_logical() {
    assert_sepl_to_c(!1);
    assert_sepl_to_c(!0);

    assert_sepl_to_c(1 && 0);
    assert_sepl_to_c(1 && 1);
    assert_sepl_to_c(20 && 40);

    assert_sepl_to_c(1 || 1);
    assert_sepl_to_c(0 || 1);
    assert_sepl_to_c(20 || 40);
}

void check_relational() {
    assert_sepl_to_c(10 > 20);
    assert_sepl_to_c(20 > 10);
    assert_sepl_to_c(20 > 20);

    assert_sepl_to_c(10 < 20);
    assert_sepl_to_c(20 < 10);
    assert_sepl_to_c(20 < 20);

    assert_sepl_to_c(10 >= 20);
    assert_sepl_to_c(20 >= 10);
    assert_sepl_to_c(20 >= 20);

    assert_sepl_to_c(10 <= 20);
    assert_sepl_to_c(20 <= 10);
    assert_sepl_to_c(20 <= 20);

    assert_sepl_to_c(20 == 20);
    assert_sepl_to_c(20 == 10);
    assert_sepl_to_c(20 != 20);
    assert_sepl_to_c(20 != 10);
}

void check_expr() {
    assert_sepl_to_c(10 > 20 && 329 * 499 == 32 * 94 || 2);
    assert_sepl_to_c(((5 + 3) * 2 > 10) && ((4.0 / 3.0) + 1 == 2) ||
                     !(8 - 6 < 2 * 1));
    assert_sepl_to_c(-2 > 3 && 94 < 20 || 2 / 0.0 < 0.0 && 23 * 32 - 32 == 23);
}

SEPL_TEST_GROUP(check_arithmetic, check_logical, check_relational, check_expr)
