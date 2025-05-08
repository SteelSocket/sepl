#ifndef SEPL_MODULE
#define SEPL_MODULE

#include "def.h"
#include "env.h"
#include "err.h"
#include "val.h"

typedef enum {
    SEPL_BC_RETURN,
    SEPL_BC_JUMPIF,
    SEPL_BC_JUMP,

    SEPL_BC_CALL,
    SEPL_BC_POP,

    SEPL_BC_NONE,
    SEPL_BC_CONST,
    SEPL_BC_SCOPE,
    SEPL_BC_FUNC,

    SEPL_BC_GET,
    SEPL_BC_SET,
    SEPL_BC_GET_UP,
    SEPL_BC_SET_UP,

    SEPL_BC_NEG,

    SEPL_BC_ADD,
    SEPL_BC_SUB,
    SEPL_BC_MUL,
    SEPL_BC_DIV,

    SEPL_BC_NOT,
    SEPL_BC_AND,
    SEPL_BC_OR,

    SEPL_BC_LT,
    SEPL_BC_LTE,
    SEPL_BC_GT,
    SEPL_BC_GTE,
    SEPL_BC_EQ,
    SEPL_BC_NEQ
} SeplByteCode;

typedef struct {
    unsigned char *bytes;
    sepl_size bpos;
    sepl_size bsize;

    SeplValue *values;
    sepl_size vpos;
    sepl_size vsize;

    const char **exports;
    sepl_size esize;

    sepl_size pc;
} SeplModule;

SEPL_LIB SeplModule sepl_mod_new(unsigned char bytes[], sepl_size bsize,
                                 SeplValue values[], sepl_size vsize);
SEPL_LIB sepl_size sepl_mod_bc(SeplModule *mod, SeplByteCode bc, SeplError *e);
SEPL_LIB sepl_size sepl_mod_bcnum(SeplModule *mod, double n, SeplError *e);
SEPL_LIB sepl_size sepl_mod_bcsize(SeplModule *mod, sepl_size s, SeplError *e);
SEPL_LIB sepl_size sepl_mod_val(SeplModule *mod, SeplValue v, SeplError *e);

SEPL_LIB void sepl_mod_init(SeplModule *mod, SeplError *e, SeplEnv env);
SEPL_LIB SeplValue sepl_mod_step(SeplModule *mod, SeplError *e, SeplEnv env);
SEPL_LIB SeplValue sepl_mod_exec(SeplModule *mod, SeplError *e, SeplEnv env);
SEPL_LIB void sepl_mod_initfunc(SeplModule *mod, SeplError *e, SeplValue func,
                                SeplArgs args);

SEPL_LIB SeplValue sepl_mod_getexport(SeplModule *mod, SeplEnv env,
                                      const char *key);

#endif
