/*
 * zlib License
 *
 * (C) 2025 G.Nithesh (SteelSocket)
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#ifndef SEPL_COMPILER
#define SEPL_COMPILER

#include "sepl.h"

typedef struct {
    SeplLexer lex;
    SeplModule *mod;
    SeplEnv env;
    SeplError error;

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
SEPL_LIB void sepl_com_else(SeplCompiler *com, unsigned char *if_jump,
                            unsigned char **end_jump);
SEPL_LIB void sepl_com_while(SeplCompiler *com);

SEPL_LIB void sepl_com_statement(SeplCompiler *com);
SEPL_LIB void sepl_com_block(SeplCompiler *com);
SEPL_LIB void sepl_com_module(SeplCompiler *com);

SEPL_LIB SeplCompiler sepl_com_init(const char *source, SeplModule *mod,
                                    SeplEnv env);
SEPL_LIB SeplError sepl_com_finish(SeplCompiler *com);

#ifdef SEPL_IMPLEMENTATION

#define SEPL__ASSIGN_NONE 0
#define SEPL__ASSIGN_LOC 1
#define SEPL__ASSIGN_UPS 2
#define SEPL__ASSIGN_UPV 3

typedef enum {
    SEPL_VAL_UNKNOWN = SEPL_VAL_OBJ + 1,
    SEPL_VAR_NONE,
    SEPL_VAR_SCOPE,
    SEPL_VAR_NUM,
    SEPL_VAR_STR,
    SEPL_VAR_FUNC,
    SEPL_VAR_CFUNC,
    SEPL_VAR_REF,
    SEPL_VAR_OBJ,
    SEPL_VAR_UNKNOWN
} SeplComVar;

typedef void (*sepl_parse_func)(SeplCompiler *);

typedef enum {
    SEPL_PRE_NONE,
    SEPL_PRE_ASSIGN, /* assign, get */
    SEPL_PRE_EXPR,   /* expr, ( */
    SEPL_PRE_OR,     /* || */
    SEPL_PRE_AND,    /* && */
    SEPL_PRE_REL,    /* >, <, >=, <= */
    SEPL_PRE_REL_EQ, /* ==, != */
    SEPL_PRE_TERM,   /* +, -, */
    SEPL_PRE_FACTOR, /* *, / */
    SEPL_PRE_UNARY,  /* +, - */
    SEPL_PRE_CALL    /* () */
} SeplComPrec;

typedef struct {
    sepl_parse_func prefix;
    sepl_parse_func infix;
    SeplComPrec pre;
} SeplParseRule;

SeplParseRule rules[SEPL_TOK_EOF + 1] = {
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE}, /* SEPL_TOK_ERROR */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE}, /* SEPL_TOK_SEMICOLON */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE}, /* SEPL_TOK_COMMA */

    {sepl_com_number, SEPL_NULL, SEPL_PRE_ASSIGN},   /* SEPL_TOK_NUM */
    {sepl_com_variable, SEPL_NULL, SEPL_PRE_ASSIGN}, /* SEPL_TOK_VAR */
    {sepl_com_func, SEPL_NULL, SEPL_PRE_ASSIGN},     /* SEPL_TOK_FUNC */
    {sepl_com_none, SEPL_NULL, SEPL_PRE_ASSIGN},     /* SEPL_TOK_NONE */
    {sepl_com_string, SEPL_NULL, SEPL_PRE_ASSIGN},   /* SEPL_TOK_STRING */

    {SEPL_NULL, sepl_com_binary, SEPL_PRE_ASSIGN}, /* SEPL_TOK_ASSIGN */

    {sepl_com_unary, sepl_com_binary, SEPL_PRE_TERM}, /* SEPL_TOK_ADD */
    {sepl_com_unary, sepl_com_binary, SEPL_PRE_TERM}, /* SEPL_TOK_SUB */
    {SEPL_NULL, sepl_com_binary, SEPL_PRE_FACTOR},    /* SEPL_TOK_MUL */
    {SEPL_NULL, sepl_com_binary, SEPL_PRE_FACTOR},    /* SEPL_TOK_DIV */

    {sepl_com_unary, SEPL_NULL, SEPL_PRE_TERM}, /* SEPL_TOK_NOT */
    {SEPL_NULL, sepl_com_and, SEPL_PRE_AND},    /* SEPL_TOK_AND */
    {SEPL_NULL, sepl_com_or, SEPL_PRE_OR},      /* SEPL_TOK_OR */

    {SEPL_NULL, sepl_com_binary, SEPL_PRE_REL},    /* SEPL_TOK_LT */
    {SEPL_NULL, sepl_com_binary, SEPL_PRE_REL},    /* SEPL_TOK_LTE */
    {SEPL_NULL, sepl_com_binary, SEPL_PRE_REL},    /* SEPL_TOK_GT */
    {SEPL_NULL, sepl_com_binary, SEPL_PRE_REL},    /* SEPL_TOK_GTE */
    {SEPL_NULL, sepl_com_binary, SEPL_PRE_REL_EQ}, /* SEPL_TOK_EQ */
    {SEPL_NULL, sepl_com_binary, SEPL_PRE_REL_EQ}, /* SEPL_TOK_NEQ */

    {sepl_com_grouping, sepl_com_call, SEPL_PRE_CALL}, /* SEPL_TOK_LPAREN */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE},             /* SEPL_TOK_RPAREN */
    {sepl_com_block, SEPL_NULL, SEPL_PRE_NONE},        /* SEPL_TOK_RCURLY */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE},             /* SEPL_TOK_LCURLY */

    {sepl_com_identifier, SEPL_NULL, SEPL_PRE_ASSIGN}, /* SEPL_TOK_IDENTIFIER */

    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE}, /* SEPL_TOK_RETURN */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_CALL}, /* SEPL_TOK_IF */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_CALL}, /* SEPL_TOK_ELSE */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_CALL}, /* SEPL_TOK_WHILE */

    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE} /* SEPL_TOK_EOF */
};

#define seplc__getrule(tok) (&rules[tok.type])
#define seplc__currtok(com) ((com)->lex.current)
#define seplc__nexttok(com) (sepl_lex_next(&(com)->lex))
#define seplc__peektok(com) (sepl_lex_peek((com)->lex))

#define seplc__isnum(v) (v == SEPL_VAL_NUM || v == SEPL_VAL_UNKNOWN)

#define seplc__writebyte(com, code) \
    (sepl_mod_bc((com)->mod, code, &(com)->error))
#define seplc__writenum(com, value) \
    (sepl_mod_bcnum((com)->mod, value, &(com)->error))
#define seplc__writesize(com, value) \
    (sepl_mod_bcsize((com)->mod, value, &(com)->error))
#define seplc__writepop(com) \
    (seplc__writebyte((com), SEPL_BC_POP), (com)->mod->vpos--)

#define seplc__writeconst(com, type, value)                        \
    (seplc__writebyte((com), type), seplc__writenum((com), value), \
     seplc__markval(com, SEPL_VAL_NUM))
#define seplc__writesized(com, type, value) \
    (seplc__writebyte((com), type), seplc__writesize((com), value))

#define seplc__writeplaceholder(com) \
    (seplc__writesize((com), 0),     \
     (com)->mod->bytes + (com)->mod->bpos - sizeof(sepl_size))
#define seplc__setpholder(com, ph) (*(sepl_size *)ph = (com)->mod->bpos)

#define seplc__check(com)                     \
    do {                                      \
        if ((com)->error.code != SEPL_ERR_OK) \
            return;                           \
    } while (0)

#define seplc__check_tok(com, tok)                                     \
    do {                                                               \
        if (seplc__currtok((com)).type != tok) {                       \
            sepl_err_uptok(&(com)->error, seplc__currtok((com)), tok); \
            return;                                                    \
        }                                                              \
    } while (0);

SEPL_API void seplc__parse(SeplCompiler *com, SeplComPrec pre) {
    SeplToken tok = seplc__currtok(com);

    SeplParseRule *rule = seplc__getrule(tok);
    if (rule->prefix == SEPL_NULL) {
        if (tok.type == SEPL_TOK_ERROR) {
            sepl_err_new(&com->error, SEPL_ERR_SYN);
            com->error.info.tokd.up = tok;
            com->error.info.tokd.exp = SEPL_TOK_ERROR;
            return;
        }
        sepl_err_uptok(&com->error, tok, SEPL_TOK_ERROR);
        return;
    }

    rule->prefix(com);
    seplc__check(com);

    while (1) {
        SeplParseRule *next_rule;
        tok = seplc__peektok(com);
        next_rule = &rules[tok.type];
        if (pre > next_rule->pre) {
            break;
        }

        if (next_rule->infix == SEPL_NULL) {
            sepl_err_uptok(&com->error, tok, SEPL_TOK_ERROR);
            return;
        }

        seplc__nexttok(com);
        next_rule->infix(com);
        seplc__check(com);
    }
}

SEPL_API void seplc__markvar(SeplCompiler *com, const char *start) {
    SeplValue v = sepl_val_object((void *)start);
    v.type = SEPL_VAR_NONE;
    sepl_mod_val(com->mod, v, &com->error);
}

SEPL_API void seplc__updtvar(SeplCompiler *com, sepl_size index, int type) {
    if ((int)type < (int)SEPL_VAR_NONE) {
        type += SEPL_VAR_NONE;
    }
    com->mod->values[index].type = type;
}

SEPL_API void seplc__markval(SeplCompiler *com, sepl_size type) {
    SeplValue v = {0};
    v.type = type;
    sepl_mod_val(com->mod, v, &com->error);
}

SEPL_API int seplc__peekval(SeplCompiler *com, sepl_size index) {
    int t = com->mod->values[index].type;
    if (t == SEPL_VAR_OBJ)
        return SEPL_VAL_REF;
    else if (t >= SEPL_VAR_NONE)
        return t - SEPL_VAR_NONE;
    return t;
}

SEPL_API int seplc__popval(SeplCompiler *com) {
    if (com->mod->vpos == 0) {
        sepl_err_new(&com->error, SEPL_ERR_VUNDERFLOW);
        return SEPL_VAL_UNKNOWN;
    }
    return seplc__peekval(com, --com->mod->vpos);
}

SEPL_API char seplc__varcmp(const char *v, const char *i) {
    char match = *v == *i;
    if (!match)
        return match;

    for (;; v++, i++) {
        if (!sepl_is_identifier(*v)) {
            if (sepl_is_identifier(*i))
                match = 0;
            break;
        } else {
            if (*v == *i)
                continue;
            match = 0;
            break;
        }
    }
    return match;
}

SEPL_API sepl_size seplc__findvar(SeplCompiler *com, SeplToken iden,
                                  sepl_size *upv) {
    sepl_size pos;
    *upv = SEPL__ASSIGN_LOC;

    for (pos = com->mod->vpos; pos-- != 0;) {
        char *v_start, *i_start;
        SeplValue v = com->mod->values[pos];

        if (sepl_val_isscp(v)) {
            *upv = SEPL__ASSIGN_UPS;
            continue;
        } else if (sepl_val_isfun(v)) {
            *upv = SEPL__ASSIGN_UPV;
            continue;
        } else if (v.type < SEPL_VAR_NONE) {
            continue;
        }

        v_start = (char *)v.as.obj;
        i_start = (char *)iden.start;

        if (seplc__varcmp(v_start, i_start)) {
            return pos;
        }
    }

    sepl_err_iden(&com->error, SEPL_ERR_IDEN_NDEF, iden);
    return 0;
}

SEPL_API char seplc__existsvar(SeplCompiler *com, SeplToken iden) {
    sepl_size pos;

    for (pos = com->mod->vpos; pos-- != 0;) {
        char *v_start, *i_start;
        SeplValue v = com->mod->values[pos];

        if (sepl_val_isscp(v)) {
            break;
        } else if (sepl_val_isfun(v)) {
            break;
        } else if (v.type < SEPL_VAR_NONE) {
            continue;
        }

        v_start = (char *)v.as.obj;
        i_start = (char *)iden.start;

        if (seplc__varcmp(v_start, i_start)) {
            return 1;
        }
    }
    return 0;
}

/* -------------------------------------------------
 *
 *               Compiler Functions
 *
 * ------------------------------------------------- */

SEPL_LIB void sepl_com_number(SeplCompiler *com) {
    double num = sepl_lex_num(seplc__currtok(com));
    seplc__writeconst(com, SEPL_BC_CONST, num);
}

SEPL_LIB void sepl_com_string(SeplCompiler *com) {
    SeplToken cur = seplc__currtok(com);
    sepl_size i, slen = cur.end - cur.start - 2;
    unsigned char *wlen;

    seplc__writebyte(com, SEPL_BC_STR);
    wlen = seplc__writeplaceholder(com);
    for (i = 0; i < slen; i++) {
        char c = cur.start[i + 1];
        if (c == '\\')
            c = sepl_to_special(cur.start[++i + 1]);
        seplc__writebyte(com, (SeplBC)c);
    }
    seplc__writebyte(com, (SeplBC)'\0');
    *(sepl_size *)wlen =
        (com->mod->bytes + com->mod->bpos) - wlen - sizeof(sepl_size) - 1;

    seplc__markval(com, SEPL_VAL_STR);
}

SEPL_LIB void sepl_com_none(SeplCompiler *com) {
    seplc__writebyte(com, SEPL_BC_NONE);
    seplc__markval(com, SEPL_VAL_NONE);
}

SEPL_LIB void sepl_com_unary(SeplCompiler *com) {
    SeplToken op = seplc__currtok(com);

    seplc__nexttok(com);
    seplc__parse(com, SEPL_PRE_UNARY);
    seplc__check(com);

    /* Check left operand */
    int vtyp = seplc__peekval(com, com->mod->vpos - 1);
    if (vtyp != SEPL_VAL_NUM && vtyp != SEPL_VAL_UNKNOWN) {
        sepl_err_new(&com->error, SEPL_ERR_OPER);
        return;
    }

    switch (op.type) {
        case SEPL_TOK_SUB:
            seplc__writebyte(com, SEPL_BC_NEG);
            break;
        case SEPL_TOK_NOT:
            seplc__writebyte(com, SEPL_BC_NOT);
            break;
        default:
            break;
    }
}

SEPL_LIB void sepl_com_binary(SeplCompiler *com) {
    SeplToken op = seplc__currtok(com);
    SeplParseRule *rule = seplc__getrule(op);

    seplc__nexttok(com);
    seplc__parse(com, (SeplComPrec)(rule->pre + 1));
    seplc__check(com);

    /* Check right operand */
    int vtyp = seplc__popval(com);
    seplc__check(com);
    if (vtyp != SEPL_VAL_NUM && vtyp != SEPL_VAL_UNKNOWN) {
        sepl_err_new(&com->error, SEPL_ERR_OPER);
        return;
    }

    /* Check left operand */
    vtyp = seplc__peekval(com, com->mod->vpos - 1);
    if (vtyp != SEPL_VAL_NUM && vtyp != SEPL_VAL_UNKNOWN) {
        sepl_err_new(&com->error, SEPL_ERR_OPER);
        return;
    }

    switch (op.type) {
        case SEPL_TOK_ADD:
            seplc__writebyte(com, SEPL_BC_ADD);
            break;
        case SEPL_TOK_SUB:
            seplc__writebyte(com, SEPL_BC_SUB);
            break;
        case SEPL_TOK_MUL:
            seplc__writebyte(com, SEPL_BC_MUL);
            break;
        case SEPL_TOK_DIV:
            seplc__writebyte(com, SEPL_BC_DIV);
            break;

        case SEPL_TOK_LT:
            seplc__writebyte(com, SEPL_BC_LT);
            break;
        case SEPL_TOK_LTE:
            seplc__writebyte(com, SEPL_BC_LTE);
            break;
        case SEPL_TOK_GT:
            seplc__writebyte(com, SEPL_BC_GT);
            break;
        case SEPL_TOK_GTE:
            seplc__writebyte(com, SEPL_BC_GTE);
            break;
        case SEPL_TOK_EQ:
            seplc__writebyte(com, SEPL_BC_EQ);
            break;
        case SEPL_TOK_NEQ:
            seplc__writebyte(com, SEPL_BC_NEQ);
            break;

        default:
            break;
    }
}

SEPL_LIB void sepl_com_and(SeplCompiler *com) {
    unsigned char *jump;
    int vtyp;

    vtyp = seplc__popval(com);
    seplc__check(com);
    if (!seplc__isnum(vtyp)) {
        sepl_err_new(&com->error, SEPL_ERR_OPER);
        return;
    }
    seplc__writebyte(com, SEPL_BC_JUMPIF);
    jump = seplc__writeplaceholder(com);

    seplc__nexttok(com);
    seplc__parse(com, (SeplComPrec)(SEPL_PRE_AND + 1));
    seplc__check(com);

    if (seplc__peektok(com).type != SEPL_TOK_AND) {
        unsigned char *jfalse, *jtrue;

        vtyp = seplc__popval(com);
        seplc__check(com);
        if (!seplc__isnum(vtyp)) {
            sepl_err_new(&com->error, SEPL_ERR_OPER);
            return;
        }
        seplc__writebyte(com, SEPL_BC_JUMPIF);
        jfalse = seplc__writeplaceholder(com);

        seplc__writeconst(com, SEPL_BC_CONST, 1.0);
        seplc__writebyte(com, SEPL_BC_JUMP);
        jtrue = seplc__writeplaceholder(com);

        seplc__setpholder(com, jump);
        seplc__setpholder(com, jfalse);
        seplc__writeconst(com, SEPL_BC_CONST, 0.0);
        seplc__setpholder(com, jtrue);

        com->mod->vpos--;
    } else {
        seplc__nexttok(com);
        sepl_com_and(com);
        seplc__check(com);
        *(sepl_size *)jump = com->mod->bpos - 1 - sizeof(sepl_size);
    }
}

SEPL_LIB void sepl_com_or(SeplCompiler *com) {
    unsigned char *end, *next;
    int vtyp;

    vtyp = seplc__popval(com);
    seplc__check(com);
    if (!seplc__isnum(vtyp)) {
        sepl_err_new(&com->error, SEPL_ERR_OPER);
        return;
    }
    seplc__writebyte(com, SEPL_BC_JUMPIF);
    next = seplc__writeplaceholder(com);

    seplc__writebyte(com, SEPL_BC_JUMP);
    end = seplc__writeplaceholder(com);
    seplc__setpholder(com, next);

    seplc__nexttok(com);
    seplc__parse(com, (SeplComPrec)(SEPL_PRE_OR + 1));
    seplc__check(com);

    if (seplc__peektok(com).type != SEPL_TOK_OR) {
        unsigned char *jfalse, *jtrue;

        vtyp = seplc__popval(com);
        seplc__check(com);
        if (!seplc__isnum(vtyp)) {
            sepl_err_new(&com->error, SEPL_ERR_OPER);
            return;
        }
        seplc__writebyte(com, SEPL_BC_JUMPIF);
        jfalse = seplc__writeplaceholder(com);

        seplc__setpholder(com, end);
        seplc__writeconst(com, SEPL_BC_CONST, 1.0);
        seplc__writebyte(com, SEPL_BC_JUMP);
        jtrue = seplc__writeplaceholder(com);

        seplc__setpholder(com, jfalse);
        seplc__writeconst(com, SEPL_BC_CONST, 0.0);
        seplc__setpholder(com, jtrue);

        com->mod->vpos--;
    } else {
        seplc__nexttok(com);
        sepl_com_or(com);
        seplc__check(com);
        *(sepl_size *)end = com->mod->bpos - (1 * 3 + sizeof(sepl_size) * 3);
    }
}

SEPL_LIB void sepl_com_assign(SeplCompiler *com, sepl_size index,
                              sepl_size upvalue) {
    sepl_size ovp = com->mod->vpos;

    seplc__nexttok(com);

    com->assign_type = upvalue;
    sepl_com_expr(com);
    seplc__check(com);
    com->assign_type = 0;

    if (upvalue <= SEPL__ASSIGN_UPS)
        seplc__writesized(com, SEPL_BC_SET, (com->mod->vpos - index));
    else
        seplc__writesized(com, SEPL_BC_SET_UP, index);

    if (com->mod->vpos != ovp) {
        int vtyp = seplc__popval(com);
        seplc__check(com);
        if (vtyp == SEPL_VAL_REF) {
            sepl_err_new(&com->error, SEPL_ERR_REFMOVE);
            return;
        }
        seplc__updtvar(com, index, vtyp);
    }
    com->mod->vpos = ovp;
}

SEPL_LIB void sepl_com_identifier(SeplCompiler *com) {
    SeplToken iden;
    sepl_size index, upvalue;
    SeplToken next;

    seplc__check_tok(com, SEPL_TOK_IDENTIFIER);
    iden = seplc__currtok(com);
    index = seplc__findvar(com, iden, &upvalue);

    seplc__check(com);
    next = seplc__peektok(com);
    if (next.type == SEPL_TOK_ASSIGN) {
        if (index < com->env.predef_len) {
            sepl_err_new(&com->error, SEPL_ERR_PREDEF);
            return;
        }

        seplc__nexttok(com);
        sepl_com_assign(com, index, upvalue);
    } else {
        if (upvalue <= SEPL__ASSIGN_UPS) {
            seplc__writesized(com, SEPL_BC_GET, (com->mod->vpos - index));
        } else {
            seplc__writesized(com, SEPL_BC_GET_UP, index);
        }

        seplc__markval(com, seplc__peekval(com, index));
    }
}

SEPL_LIB void sepl_com_variable(SeplCompiler *com) {
    SeplToken iden = seplc__nexttok(com);
    sepl_size ovp = com->mod->vpos;
    seplc__check_tok(com, SEPL_TOK_IDENTIFIER);

    /* If the identifier already exists within the current scope */
    if (seplc__existsvar(com, iden)) {
        sepl_err_iden(&com->error, SEPL_ERR_IDEN_RDEF, iden);
        return;
    }

    seplc__markvar(com, iden.start);
    seplc__check(com);

    seplc__writebyte(com, SEPL_BC_NONE);
    sepl_com_identifier(com);

    /* Empty declearation */
    if (com->mod->vpos - ovp == 2) {
        /* Remove Get instruction added by else case */
        com->mod->bpos -= 1 + sizeof(sepl_size);
        com->mod->vpos--;
    }
}

SEPL_LIB void sepl_com_params(SeplCompiler *com, unsigned char *pcount) {
    sepl_size offset = 0;
    seplc__check_tok(com, SEPL_TOK_LPAREN);

    while (seplc__nexttok(com).type != SEPL_TOK_RPAREN) {
        SeplToken tok = seplc__currtok(com);
        seplc__check_tok(com, SEPL_TOK_IDENTIFIER);
        seplc__markvar(com, tok.start);
        seplc__updtvar(com, com->mod->vpos - 1, SEPL_VAL_UNKNOWN);

        offset++;

        seplc__nexttok(com);
        if (seplc__currtok(com).type == SEPL_TOK_RPAREN)
            break;
        seplc__check_tok(com, SEPL_TOK_COMMA);
    }

    *(sepl_size *)pcount = offset;
    seplc__check_tok(com, SEPL_TOK_RPAREN);
}

SEPL_LIB void sepl_com_func(SeplCompiler *com) {
    unsigned char *skip, *params;
    sepl_size ovp = com->mod->vpos;

    if (com->func_block) {
        sepl_err_new(&com->error, SEPL_ERR_CLOSURE);
        return;
    }
    if (com->assign_type >= SEPL__ASSIGN_UPS) {
        sepl_err_new(&com->error, SEPL_ERR_FUNC_UPV);
        return;
    }

    seplc__writebyte(com, SEPL_BC_FUNC);
    skip = seplc__writeplaceholder(com);
    params = seplc__writeplaceholder(com);
    seplc__markval(com, SEPL_VAL_FUNC);

    seplc__nexttok(com);
    sepl_com_params(com, params);
    seplc__nexttok(com);

    com->func_block = 1;
    sepl_com_block(com);
    com->func_block = 0;

    seplc__writebyte(com, SEPL_BC_RETURN);
    seplc__setpholder(com, skip);

    com->mod->vpos = ovp + 1;
}

SEPL_LIB void sepl_com_call(SeplCompiler *com) {
    sepl_size ovp = com->mod->vpos;
    int vtyp = seplc__peekval(com, ovp - 1);

    while (seplc__nexttok(com).type != SEPL_TOK_RPAREN) {
        sepl_com_expr(com);
        seplc__nexttok(com);
        if (seplc__currtok(com).type == SEPL_TOK_RPAREN)
            break;
        seplc__check_tok(com, SEPL_TOK_COMMA);
    }

    seplc__writesized(com, SEPL_BC_CALL, com->mod->vpos - ovp);
    seplc__check_tok(com, SEPL_TOK_RPAREN);

    com->mod->vpos = ovp - 1;
    seplc__markval(com, SEPL_VAL_UNKNOWN);
}

SEPL_LIB void sepl_com_grouping(SeplCompiler *com) {
    seplc__check_tok(com, SEPL_TOK_LPAREN);
    seplc__nexttok(com);
    if (seplc__currtok(com).type == SEPL_TOK_RPAREN) {
        return;
    }

    sepl_com_expr(com);
    seplc__check(com);
    seplc__nexttok(com);
    seplc__check_tok(com, SEPL_TOK_RPAREN);
}

SEPL_LIB void sepl_com_expr(SeplCompiler *com) {
    seplc__parse(com, SEPL_PRE_EXPR);
}

SEPL_LIB void sepl_com_return(SeplCompiler *com) {
    if (seplc__peektok(com).type != SEPL_TOK_SEMICOLON) {
        sepl_size ovp = com->mod->vpos;
        seplc__nexttok(com);
        sepl_com_expr(com);
        if (ovp == com->mod->vpos) {
            seplc__writebyte(com, SEPL_BC_NONE);
            seplc__markval(com, SEPL_VAL_NONE);
        }
    } else {
        seplc__writebyte(com, SEPL_BC_NONE);
        seplc__markval(com, SEPL_VAL_NONE);
    }
    seplc__writebyte(com, SEPL_BC_RETURN);
}

SEPL_LIB void sepl_com_if(SeplCompiler *com) {
    sepl_size ovp = com->mod->vpos;
    unsigned char *if_jump = SEPL_NULL;
    unsigned char *end_jump = SEPL_NULL;
    int vtyp;

    seplc__nexttok(com);
    sepl_com_grouping(com);
    seplc__nexttok(com);
    seplc__check(com);

    if (com->mod->vpos == ovp) {
        sepl_err_new(&com->error, SEPL_ERR_EEXPR);
        return;
    }

    vtyp = seplc__popval(com);
    seplc__check(com);
    if (!seplc__isnum(vtyp)) {
        sepl_err_new(&com->error, SEPL_ERR_OPER);
        return;
    }
    seplc__writebyte(com, SEPL_BC_JUMPIF);
    if_jump = seplc__writeplaceholder(com);

    sepl_com_block(com);
    seplc__check(com);

    if (com->block_ret) {
        /* Propagate return value */
        seplc__writebyte(com, SEPL_BC_RETURN);
    }

    sepl_com_else(com, if_jump, &end_jump);
}

SEPL_LIB void sepl_com_else(SeplCompiler *com, unsigned char *if_jump,
                            unsigned char **end_jump) {
    unsigned char *pop_skip;
    seplc__writepop(com);

    if (seplc__peektok(com).type != SEPL_TOK_ELSE) {
        seplc__writebyte(com, SEPL_BC_JUMP);
        pop_skip = seplc__writeplaceholder(com);
        seplc__setpholder(com, if_jump);
        seplc__setpholder(com, pop_skip);
        return;
    }

    seplc__writebyte(com, SEPL_BC_JUMP);
    *end_jump = seplc__writeplaceholder(com);

    seplc__nexttok(com);
    seplc__nexttok(com);

    seplc__setpholder(com, if_jump);

    if (seplc__currtok(com).type == SEPL_TOK_IF) {
        sepl_com_if(com);
    } else {
        sepl_com_block(com);
        if (com->block_ret) {
            /* Propagate return value */
            seplc__writebyte(com, SEPL_BC_RETURN);
        } else {
            seplc__writepop(com);
        }
    }

    seplc__check(com);
    seplc__setpholder(com, *end_jump);
}

SEPL_LIB void sepl_com_while(SeplCompiler *com) {
    sepl_size ovp = com->mod->vpos;
    unsigned char *cond_jump = SEPL_NULL;
    sepl_size loop_start = com->mod->bpos, scope_jump;
    int vtyp;

    seplc__nexttok(com);
    sepl_com_grouping(com);
    seplc__nexttok(com);
    seplc__check(com);

    if (com->mod->vpos == ovp) {
        sepl_err_new(&com->error, SEPL_ERR_EEXPR);
        return;
    }

    vtyp = seplc__popval(com);
    seplc__check(com);
    if (!seplc__isnum(vtyp)) {
        sepl_err_new(&com->error, SEPL_ERR_OPER);
        return;
    }
    seplc__writebyte(com, SEPL_BC_JUMPIF);
    cond_jump = seplc__writeplaceholder(com);
    scope_jump = com->mod->bpos;

    sepl_com_block(com);
    seplc__check(com);

    if (com->block_ret) {
        seplc__writebyte(com, SEPL_BC_RETURN);
    } else if (!com->block_ret && com->inner_ret) {
        /* Remove implicit RETURN NONE */
        com->mod->bpos -= 1 + 1;

        /* Simulate RETURN for while loop end */
        while (com->block_size != ovp) {
            seplc__writebyte(com, SEPL_BC_POP);
            com->block_size--;
        }
        com->mod->vpos--;
        seplc__writesized(com, SEPL_BC_JUMP, loop_start);

        /* Return the value from other control structures to outer scope */
        *(sepl_size *)(com->mod->bytes + scope_jump + 1) = com->mod->bpos;
        seplc__writebyte(com, SEPL_BC_RETURN);
    } else {
        seplc__writepop(com);
        seplc__writesized(com, SEPL_BC_JUMP, loop_start);
    }

    seplc__setpholder(com, cond_jump);
}

SEPL_LIB void sepl_com_statement(SeplCompiler *com) {
    SeplToken curr = seplc__currtok(com);

    /* Empty Statement */
    if (curr.type == SEPL_TOK_SEMICOLON) {
        return;
    }

    if (curr.type == SEPL_TOK_RETURN) {
        sepl_com_return(com);
    } else if (curr.type == SEPL_TOK_IF) {
        sepl_com_if(com);
        return;
    } else if (curr.type == SEPL_TOK_WHILE) {
        sepl_com_while(com);
        return;
    } else if (curr.type == SEPL_TOK_VAR) {
        sepl_com_variable(com);
    } else if (curr.type != SEPL_TOK_EOF) {
        sepl_size ovp = com->mod->vpos;
        sepl_com_expr(com);
        if (com->mod->vpos - ovp)
            seplc__writepop(com);
    }

    seplc__check(com);
    seplc__nexttok(com);
    seplc__check_tok(com, SEPL_TOK_SEMICOLON);
}

SEPL_LIB void sepl_com_block(SeplCompiler *com) {
    char ret = 0;
    SeplToken tok;
    sepl_size ovp = com->mod->vpos;
    unsigned char *scope_end = SEPL_NULL;

    seplc__check_tok(com, SEPL_TOK_LCURLY);
    tok = seplc__nexttok(com);

    seplc__writebyte(com, SEPL_BC_SCOPE);
    scope_end = seplc__writeplaceholder(com);
    seplc__markval(com, SEPL_VAL_SCOPE);
    com->inner_ret = 0;

    while (tok.type != SEPL_TOK_RCURLY) {
        sepl_com_statement(com);
        seplc__check(com);
        com->inner_ret = com->inner_ret || com->block_ret;

        if (tok.type == SEPL_TOK_RETURN) {
            /* Ignore all statements after return */
            do {
                seplc__nexttok(com);
            } while (seplc__currtok(com).type != SEPL_TOK_RCURLY &&
                     seplc__currtok(com).type != SEPL_TOK_EOF);
            ret = 1;
            break;
        }
        tok = seplc__nexttok(com);
    }
    seplc__check_tok(com, SEPL_TOK_RCURLY);

    if (!ret) {
        seplc__writebyte(com, SEPL_BC_NONE);
        seplc__writebyte(com, SEPL_BC_RETURN);
        seplc__markval(com, SEPL_VAL_NONE);
    }

    com->block_size = com->mod->vpos - 1;
    com->block_ret = ret;

    com->mod->values[ovp + 1] = com->mod->values[com->mod->vpos - 1];
    com->mod->vpos = ovp + 1;

    seplc__setpholder(com, scope_end);
}

SEPL_LIB void sepl_com_module(SeplCompiler *com) {
    while (seplc__currtok(com).type != SEPL_TOK_EOF) {
        if (seplc__currtok(com).type == SEPL_TOK_IDENTIFIER &&
            seplc__peektok(com).type == SEPL_TOK_ASSIGN) {
            sepl_com_identifier(com);
        } else {
            sepl_com_variable(com);
        }
        seplc__check(com);
        seplc__nexttok(com);
        seplc__check_tok(com, SEPL_TOK_SEMICOLON);
        seplc__nexttok(com);
    }
}

SEPL_LIB SeplCompiler sepl_com_init(const char *source, SeplModule *mod,
                                    SeplEnv env) {
    SeplCompiler com = {0};
    sepl_size i;

    com.lex = sepl_lex_init(source);
    com.mod = mod;
    com.env = env;
    for (i = 0; i < env.predef_len; i++) {
        seplc__markvar(&com, env.predef[i].key);
        seplc__updtvar(&com, i, env.predef[i].value.type);
    }
    for (i = 0; i < mod->esize; i++) {
        seplc__markvar(&com, mod->exports[i]);
    }

    seplc__nexttok(&com);
    return com;
}

SEPL_LIB SeplError sepl_com_finish(SeplCompiler *com) {
    if (com->error.code != SEPL_ERR_OK) {
        com->error.line = com->lex.line;
    }
    com->mod->vpos = 0;
    return com->error;
}

#endif
#endif
