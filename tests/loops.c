#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

void while_loop() {
    assert_sepl(
        {
            @a = 0;
            while (a != 20) {
                a = a + 1;
            }
            return a;
        },
        20);  // basic while loop

    assert_sepl(
        {
            @a = 0;
            while (a != 20) {
                a = a + 1;
                return 10;
            }
            return a;
        },
        10);  // return inside loop

    assert_sepl(
        {
            @a = 0;
            while (a != 20) {
                @_ = 0;
                if (a == 7) {
                    return a;
                }
                a = a + 1;
            }
            return a;
        },
        7);  // return propagated from if statement
}

SEPL_TEST_GROUP(while_loop);
