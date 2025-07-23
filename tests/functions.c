#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"
#include "../sepl_com.h"

#include "tests.h"

void basic_test() {
    assert_sepl(
        {
            @a = $() {
                return 10;
            };
            return a();
        },
        10);  // function call with no params

    assert_sepl(
        {
            @add = $(a, b) {
                return a + b;
            };
            return add(100, 200);
        },
        300);  // function with params
}

void params_test() {
    assert_sepl_none({
        @a = $(b) {
            return b;
        };
        return a();
    });  // missing argument will be taken as NONE

    assert_sepl(
        {
            @a = $(b) {
                return b;
            };
            return a(100, 200);
        },
        100);  // extra argument ignored, first one used

    assert_sepl(
        {
            @a = $(b) {
                @b = 200;
                return b;
            };
            return a(100);
        },
        200);  // parameter shadowed by inner variable
}

void recursive_test() {
    assert_sepl(
        {
            @factorial = $(n) {
                if (n == 0) {
                    return 1;
                }
                return n * factorial(n - 1);
            };
            return factorial(5);
        },
        1 * 2 * 3 * 4 * 5);  // recursive function (factorial)

    assert_sepl(
        {
            @a = $(a) {
                return a;
            };
            return a(10);
        },
        10);  // function shadowed by param
}

void upvalue_tests() {
    assert_sepl(
        {
            @a = 10;
            @b = $() {
                return a;
            };
            return b();
        },
        10);  // function captures outer variable

    assert_sepl(
        {
            @a = 10;
            @b = $(a) {
                return a;
            };
            return b(20);
        },
        20);  // parameter shadows outer variable

    assert_sepl(
        {
            @a = 10;
            @b = $() {
                return a;
            };
            a = 20;
            return b();
        },
        20);  // function uses updated value of captured variable
}

SEPL_TEST_GROUP(basic_test, params_test, recursive_test, upvalue_tests)
