#ifndef SEPL_ENVIRONMENT
#define SEPL_ENVIRONMENT

#include "val.h"

typedef struct {
    const char *key;
    SeplValue value;
} SeplValuePair;

typedef struct {
    sepl_free_func free;

    SeplValuePair *predef;
    sepl_size predef_len;
} SeplEnv;

#endif
