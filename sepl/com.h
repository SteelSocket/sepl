#ifndef SEPL_COMPILER
#define SEPL_COMPILER

#include "env.h"
#include "err.h"
#include "mod.h"

typedef struct {
    SeplLexer lex;
    SeplModule *mod;
    SeplEnv env;
    SeplError error;

    /* The number of values in the current scope */
    sepl_size scope_size;
    /* The number of values in the previous block */
    sepl_size block_size;
    char block_ret;
    char inner_ret;
    char func_block;
    /* 0 - no assign, 1 - local, 2 - upvalue (scope), 3 - upvalue (function) */
    char assign_type;
} SeplCompiler;

SEPL_LIB void sepl_com_number(SeplCompiler *com);
SEPL_LIB void sepl_com_string(SeplCompiler *com);
SEPL_LIB void sepl_com_none(SeplCompiler *com);

SEPL_LIB void sepl_com_unary(SeplCompiler *com);
SEPL_LIB void sepl_com_binary(SeplCompiler *com);
SEPL_LIB void sepl_com_and(SeplCompiler *com);
SEPL_LIB void sepl_com_or(SeplCompiler *com);

SEPL_LIB void sepl_com_assign(SeplCompiler *com, sepl_size index,
                           sepl_size upvalue);
SEPL_LIB void sepl_com_identifier(SeplCompiler *com);
SEPL_LIB void sepl_com_variable(SeplCompiler *com);
SEPL_LIB void sepl_com_func(SeplCompiler *com);
SEPL_LIB void sepl_com_call(SeplCompiler *com);

SEPL_LIB void sepl_com_grouping(SeplCompiler *com);
SEPL_LIB void sepl_com_expr(SeplCompiler *com);
SEPL_LIB void sepl_com_return(SeplCompiler *com);

SEPL_LIB void sepl_com_if(SeplCompiler *com);
SEPL_LIB void sepl_com_while(SeplCompiler *com);

SEPL_LIB void sepl_com_block(SeplCompiler *com);
SEPL_LIB void sepl_com_module(SeplCompiler *com);

SEPL_LIB SeplCompiler sepl_com_init(const char *source, SeplModule *mod,
                                   SeplEnv env);
SEPL_LIB SeplError sepl_com_finish(SeplCompiler *com);

#endif
