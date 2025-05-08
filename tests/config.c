#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_DEF_SIZE unsigned long long
#define SEPL_NULL ((void *)1)

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

void definitions_tests() {
    assert(sizeof(sepl_size) == sizeof(size_t));
    assert(SEPL_NULL == ((void *)1));
}

void struct_tests() {
    SeplModule mod;
    assert(sizeof(mod.bpos) == sizeof(size_t));
    assert(sizeof(mod.vpos) == sizeof(size_t));
    assert(sizeof(mod.bsize) == sizeof(size_t));
    assert(sizeof(mod.vsize) == sizeof(size_t));
}

SEPL_TEST_GROUP(definitions_tests, struct_tests);
