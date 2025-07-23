#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"
#include "../sepl_com.h"

#include "tests.h"

#define str_eq(s1, s2) (strcmp(s1, s2) == 0)
#define assert_seplstr(expr, s) (assert(str_eq(tst_run(#expr).as.obj, s)))

void basic_test() {
    assert_seplstr({ return "Hello"; }, "Hello");
    assert_seplstr({ return ""; }, "");
    assert_seplstr({ return "\a"; }, "\a");
    assert_seplstr({ return "\""; }, "\"");
    assert_seplstr({ return "\\"; }, "\\");
    assert_seplstr({ return "\y"; }, "y");
}

SEPL_TEST_GROUP(basic_test);
