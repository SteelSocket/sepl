#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

void single_var() {
    // basic variable assignment
    assert_sepl(
        {
            @a = 1;
            return a;
        },
        1);

    // reassignment of variable
    assert_sepl(
        {
            @a = 1;
            a = 2;
            return a;
        },
        2);

    // increment variable using itself
    assert_sepl(
        {
            @a = 1;
            a = a + 1;
            return a;
        },
        2);

    // inner shadowed variable doesn't affect outer
    assert_sepl(
        {
            @a = 1;
            {
                @a = 2;
            };
            return a;
        },
        1);

    // inner block modifies outer variable
    assert_sepl(
        {
            @a = 1;
            {
                a = 2;
            };
            return a;
        },
        2);

    // assigning empty block
    assert_sepl_none({
        @a = {};
        return a;
    });

    // variable set to NONE
    assert_sepl_none({
        @a = NONE;
        return a;
    });

    // variable set to none after initialization
    assert_sepl_none({
        @a = 1;
        a = NONE;
        return a;
    });

    // variable set to none implicitly
    assert_sepl_none({
        @a;
        return a;
    });

    // assign returned block value to variable
    assert_str("{ @a = { return 1;}; return a; }", 1);

    // inner variable assigned and returned to outer variable
    assert_str("{ @a = { @a = 1; return a;}; return a; }", 1);
}

void var_conditional() {
    // conditional assigned not executed
    assert_sepl(
        {
            @a = 1;
            if (0) {
                a = 2;
            }
            return a;
        },
        1);

    // conditional assignment executed
    assert_sepl(
        {
            @a = 1;
            if (1) {
                a = 2;
            }
            return a;
        },
        2);
}

void var_assign_block() {
    // nested block modifies inner var, returns result
    assert_str("{ @a = { @b = 10; { b = b + 1; }; return b; }; return a; }",
               11);

    // while loop updates variable within block, returns result
    assert_str(
        "{ @a = { @b = 10; while (b != 20) { b = b + 1; } return b; }; return "
        "a; }",
        20);
}

void multi_var() {
    // var depends on another var
    assert_sepl(
        {
            @a = 2;
            @b = a + 2;
            return b;
        },
        4);

    // multiple vars and one modifies using another
    assert_sepl(
        {
            @a = 2;
            @b = 5;
            b = b + a;
            return b;
        },
        7);

    // nested variable operations in block
    assert_str("{ @a = 2; @b = { @c = a + 3; return c * 2; }; return b; }", 10);
}

void var_oper() {
    // Arithmetic tests between two variables
    assert_sepl(
        {
            @a = 1;
            @b = 2;
            return a + b;
        },
        3);

    assert_sepl(
        {
            @a = 5;
            @b = 2;
            return a - b;
        },
        3);
    assert_sepl(
        {
            @a = 4;
            @b = 2;
            return a * b;
        },
        8);
    assert_sepl(
        {
            @a = 10;
            @b = 2;
            return a / b;
        },
        5);

    
    // Relational tests between two variables
    assert_sepl(
        {
            @a = 1;
            @b = 2;
            return a > b;
        },
        0);

    assert_sepl(
        {
            @a = 5;
            @b = 2;
            return a < b;
        },
        0);
    assert_sepl(
        {
            @a = 4;
            @b = 4;
            return a >= b;
        },
        1);
    assert_sepl(
        {
            @a = 10;
            @b = 2;
            return a <= b;
        },
        0);
    assert_sepl(
        {
            @a = 4;
            @b = 4;
            return a == b;
        },
        1);
    assert_sepl(
        {
            @a = 10;
            @b = 2;
            return a != b;
        },
        1);


    // Logical tests between two variables
    assert_sepl(
        {
            @a = 1;
            return !a;
        },
        0);
    assert_sepl(
        {
            @a = 0;
            return !a;
        },
        1);

    assert_sepl(
        {
            @a = 5;
            @b = 1;
            return a && b;
        },
        1);
    assert_sepl(
        {
            @a = 0;
            @b = 1;
            return a || b;
        },
        1);
}

SEPL_TEST_GROUP(single_var, var_conditional, var_assign_block, multi_var, var_oper);
