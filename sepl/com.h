#ifndef SEPL_COMPILER
#define SEPL_COMPILER

#include "env.h"
#include "err.h"
#include "mod.h"

SEPL_LIB SeplError sepl_com_module(const char *source, SeplModule *mod,
                                   SeplEnv env);
SEPL_LIB SeplError sepl_com_block(const char *source, SeplModule *mod,
                                  SeplEnv env);
SEPL_LIB SeplError sepl_com_expr(const char *source, SeplModule *mod,
                                 SeplEnv env);

#endif
