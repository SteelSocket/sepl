#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

void single_var() {
    assert_sepl(
        {
            @a = 1;
            return a;
        },
        1);  // basic variable assignment

    assert_sepl(
        {
            @a = 1;
            a = 2;
            return a;
        },
        2);  // reassignment of variable

    assert_sepl(
        {
            @a = 1;
            a = a + 1;
            return a;
        },
        2);  // increment variable using itself

    assert_sepl(
        {
            @a = 1;
            {
                @a = 2;
            };
            return a;
        },
        1);  // inner shadowed variable doesn't affect outer

    assert_sepl(
        {
            @a = 1;
            {
                a = 2;
            };
            return a;
        },
        2);  // inner block modifies outer variable

    assert_sepl_none({
        @a = {};
        return a;
    });  // assigning empty block

    assert_sepl_none({
        @a = NONE;
        return a;
    });  // variable set to NONE

    assert_sepl_none({
        @a = 1;
        a = NONE;
        return a;
    });  // variable set to none after initialization

    assert_sepl({
        @a = { return 1;};
        return a;
}, 1); // assign returned block value to variable

    assert_sepl({
    @a = { @a = 1;
    return a;};
        return a;
    }, 1); // inner variable assigned and returned to outer variable
    }

    void var_conditional() {
        assert_sepl(
            {
                @a = 1;
                if (0) {
                    a = 2;
                }
                return a;
            },
            1);  // conditional assigned not executed

        assert_sepl(
            {
                @a = 1;
                if (1) {
                    a = 2;
                }
                return a;
            },
            2);  // conditional assignment executed
    }

    void var_assign_block() {
    assert_sepl({
        @a = { @b = 10;
        {
            b = b + 1;
        };
        return b;
        };
        return a;
    }, 11); // nested block modifies inner var, returns result

    assert_sepl({
    @a = { @b = 10;
    while (b != 20) {
        b = b + 1;
    }
    return b;
        };
        return a;
    }, 20); // while loop updates variable within block, returns result
    }

    void multi_var() {
        assert_sepl(
            {
                @a = 2;
                @b = a + 2;
                return b;
            },
            4);  // var depends on another var

        assert_sepl(
            {
                @a = 2;
                @b = 5;
                b = b + a;
                return b;
            },
            7);  // multiple vars and one modifies using another

    assert_sepl({
        @a = 2;
        @b = { @c = a + 3;
        return c * 2;
        };
        return b;
    }, 10); // nested variable operations in block
    }

    SEPL_TEST_GROUP(single_var, var_conditional, var_assign_block, multi_var);
