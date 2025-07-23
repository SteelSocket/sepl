#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"
#include "../sepl_com.h"

#include "tests.h"

void if_tests() {
    assert_sepl(
        {
            if (1 + 1 == 2) {
                return 20;
            }
            return 10;
        },
        20);  // if condition (true)

    assert_sepl(
        {
            if (1 + 1 == 1) {
                return 20;
            }
            return 10;
        },
        10);  // if condition (false), fall through

    assert_sepl(
        {
            if (1) {
                ;
            }
            return 10;
        },
        10);  // if condition (true), no body, pass through

    assert_sepl_none({
        if (1) {
            return;
        }
        return 10;
    });  // return without value
}

void else_tests() {
    assert_sepl(
        {
            if (1 + 1 == 2) {
                return 20;
            } else {
                return 10;
            }
        },
        20);  // if-else, if branch taken

    assert_sepl(
        {
            if (1 + 1 == 1) {
                return 20;
            } else {
                return 10;
            }
        },
        10);  // if-else, else branch taken

    assert_sepl(
        {
            if (1 + 1 == 1) {
                return 20;
            } else {
                ;
            }
            return 30;
        },
        30);  // else branch with empty body
}

void if_else_tests() {
    assert_sepl(
        {
            if (0) {
                return 1;
            } else if (1) {
                return 2;
            } else if (1) {
                return 3;
            } else {
                return 4;
            }
        },
        2);  // else-if chain, second branch taken

    assert_sepl(
        {
            if (0) {
                return 1;
            } else if (0) {
                return 2;
            } else if (1) {
                ;
            } else {
                return 4;
            }
            return 5;
        },
        5);  // else-if with empty body, final return
}

void while_tests() {
    assert_sepl(
        {
            while (1 + 1 == 2) {
                return 20;
            }
            return 10;
        },
        20);  // while condition true, early return

    assert_sepl(
        {
            while (1 + 1 == 1) {
                return 20;
            }
            return 10;
        },
        10);  // while condition false, skip loop
}

SEPL_TEST_GROUP(if_tests, else_tests, if_else_tests, while_tests)
