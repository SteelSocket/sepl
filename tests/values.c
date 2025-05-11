#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

#define str_eq(s1, s2) (strcmp(s1, s2) == 0)

void conversions() {
    assert(sepl_val_number(3.14).as.num == 3.14);
    assert(sepl_val_object(NULL).as.obj == NULL);
    char *str = "Hello";
    assert(sepl_val_object(str).as.obj == str);
    assert(str_eq(sepl_val_str("Hello").as.obj, "Hello"));
}

void type_check_null() {
    assert(sepl_val_isnone(SEPL_NONE));
    assert(!sepl_val_isnum(SEPL_NONE));
    assert(!sepl_val_isscp(SEPL_NONE));
    assert(!sepl_val_isobj(SEPL_NONE));
}

void type_check_num() {
    assert(!sepl_val_isnone(sepl_val_number(3.14)));
    assert(sepl_val_isnum(sepl_val_number(3.14)));
    assert(!sepl_val_isscp(sepl_val_number(3.14)));
    assert(!sepl_val_isobj(sepl_val_number(3.14)));
}

void type_check_obj() {
    assert(!sepl_val_isnone(sepl_val_object("Hello")));
    assert(!sepl_val_isnum(sepl_val_object("Hello")));
    assert(!sepl_val_isscp(sepl_val_object("Hello")));
    assert(sepl_val_isobj(sepl_val_object("Hello")));
}

SEPL_TEST_GROUP(conversions, type_check_null, type_check_num, type_check_obj)
