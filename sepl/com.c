#include "com.h"

#include "def.h"
#include "env.h"
#include "err.h"
#include "lex.h"
#include "mod.h"
#include "val.h"

#define SEPL__ASSIGN_NONE 0
#define SEPL__ASSIGN_LOC 1
#define SEPL__ASSIGN_UPS 2
#define SEPL__ASSIGN_UPV 3


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
} Precedence;

typedef struct {
    sepl_parse_func prefix;
    sepl_parse_func infix;
    Precedence pre;
} SeplParseRule;

SEPL_API void sepl__number(SeplCompiler *com);
SEPL_API void sepl__string(SeplCompiler *com);
SEPL_API void sepl__none(SeplCompiler *com);

SEPL_API void sepl__unary(SeplCompiler *com);
SEPL_API void sepl__binary(SeplCompiler *com);
SEPL_API void sepl__and(SeplCompiler *com);
SEPL_API void sepl__or(SeplCompiler *com);

SEPL_API void sepl__identifier(SeplCompiler *com);
SEPL_API void sepl__variable(SeplCompiler *com);
SEPL_API void sepl__func(SeplCompiler *com);
SEPL_API void sepl__call(SeplCompiler *com);

SEPL_API void sepl__grouping(SeplCompiler *com);
SEPL_API void sepl__expr(SeplCompiler *com);
SEPL_API void sepl__return(SeplCompiler *com);

SEPL_API void sepl__if(SeplCompiler *com);
SEPL_API void sepl__while(SeplCompiler *com);

SEPL_API void sepl__block(SeplCompiler *com);

SEPL_API SeplParseRule rules[SEPL_TOK_EOF + 1] = {
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE}, /* SEPL_TOK_ERROR */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE}, /* SEPL_TOK_SEMICOLON */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE}, /* SEPL_TOK_COMMA */

    {sepl__number, SEPL_NULL, SEPL_PRE_ASSIGN},   /* SEPL_TOK_NUM */
    {sepl__variable, SEPL_NULL, SEPL_PRE_ASSIGN}, /* SEPL_TOK_VAR */
    {sepl__func, SEPL_NULL, SEPL_PRE_ASSIGN},     /* SEPL_TOK_FUNC */
    {sepl__none, SEPL_NULL, SEPL_PRE_ASSIGN},     /* SEPL_TOK_NONE */
    {sepl__string, SEPL_NULL, SEPL_PRE_ASSIGN},   /* SEPL_TOK_STRING */

    {SEPL_NULL, sepl__binary, SEPL_PRE_ASSIGN}, /* SEPL_TOK_ASSIGN */

    {sepl__unary, sepl__binary, SEPL_PRE_TERM}, /* SEPL_TOK_ADD */
    {sepl__unary, sepl__binary, SEPL_PRE_TERM}, /* SEPL_TOK_SUB */
    {SEPL_NULL, sepl__binary, SEPL_PRE_FACTOR}, /* SEPL_TOK_MUL */
    {SEPL_NULL, sepl__binary, SEPL_PRE_FACTOR}, /* SEPL_TOK_DIV */

    {sepl__unary, SEPL_NULL, SEPL_PRE_TERM}, /* SEPL_TOK_NOT */
    {SEPL_NULL, sepl__and, SEPL_PRE_AND},    /* SEPL_TOK_AND */
    {SEPL_NULL, sepl__or, SEPL_PRE_OR},      /* SEPL_TOK_OR */

    {SEPL_NULL, sepl__binary, SEPL_PRE_REL},    /* SEPL_TOK_LT */
    {SEPL_NULL, sepl__binary, SEPL_PRE_REL},    /* SEPL_TOK_LTE */
    {SEPL_NULL, sepl__binary, SEPL_PRE_REL},    /* SEPL_TOK_GT */
    {SEPL_NULL, sepl__binary, SEPL_PRE_REL},    /* SEPL_TOK_GTE */
    {SEPL_NULL, sepl__binary, SEPL_PRE_REL_EQ}, /* SEPL_TOK_EQ */
    {SEPL_NULL, sepl__binary, SEPL_PRE_REL_EQ}, /* SEPL_TOK_NEQ */

    {sepl__grouping, sepl__call, SEPL_PRE_CALL}, /* SEPL_TOK_LPAREN */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE},       /* SEPL_TOK_RPAREN */
    {sepl__block, SEPL_NULL, SEPL_PRE_NONE},     /* SEPL_TOK_RCURLY */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE},       /* SEPL_TOK_LCURLY */

    {sepl__identifier, SEPL_NULL, SEPL_PRE_ASSIGN}, /* SEPL_TOK_IDENTIFIER */

    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE}, /* SEPL_TOK_RETURN */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_CALL}, /* SEPL_TOK_IF */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_CALL}, /* SEPL_TOK_ELSE */
    {SEPL_NULL, SEPL_NULL, SEPL_PRE_CALL}, /* SEPL_TOK_WHILE */

    {SEPL_NULL, SEPL_NULL, SEPL_PRE_NONE} /* SEPL_TOK_EOF */
};

#define sepl__getrule(tok) (&rules[tok.type])
#define sepl__currtok(com) ((com)->lex.current)
#define sepl__nexttok(com) (sepl_lex_next(&(com)->lex))
#define sepl__peektok(com) (sepl_lex_peek((com)->lex))

#define sepl__writebyte(com, code) \
    (sepl_mod_bc((com)->mod, code, &(com)->error))
#define sepl__writenum(com, value) \
    (sepl_mod_bcnum((com)->mod, value, &(com)->error))
#define sepl__writesize(com, value) \
    (sepl_mod_bcsize((com)->mod, value, &(com)->error))
#define sepl__writepop(com) \
    (sepl__writebyte((com), SEPL_BC_POP), (com)->scope_size--)

#define sepl__writeconst(com, type, value)                       \
    (sepl__writebyte((com), type), sepl__writenum((com), value), \
     (com)->scope_size++)
#define sepl__writesized(com, type, value) \
    (sepl__writebyte((com), type), sepl__writesize((com), value))

#define sepl__writeplaceholder(com) \
    (sepl__writesize((com), 0),     \
     (com)->mod->bytes + (com)->mod->bpos - sizeof(sepl_size))
#define sepl__setpholder(com, ph) (*(sepl_size *)ph = (com)->mod->bpos)

#define sepl__check(com)                      \
    do {                                      \
        if ((com)->error.code != SEPL_ERR_OK) \
            return;                           \
    } while (0)

#define sepl__check_tok(com, tok)                                     \
    do {                                                              \
        if (sepl__currtok((com)).type != tok) {                       \
            sepl_err_uptok(&(com)->error, sepl__currtok((com)), tok); \
            return;                                                   \
        }                                                             \
    } while (0);

#define sepl__check_vlimit(com)                              \
    do {                                                     \
        if ((com)->scope_size >= (com)->mod->vsize) {        \
            sepl_err_new(&(com)->error, SEPL_ERR_VOVERFLOW); \
            return;                                          \
        }                                                    \
    } while (0);

SEPL_API void sepl__parse(SeplCompiler *com, Precedence pre) {
    SeplToken tok = sepl__currtok(com);

    SeplParseRule *rule = sepl__getrule(tok);
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
    sepl__check(com);

    while (1) {
        SeplParseRule *next_rule;
        tok = sepl__peektok(com);
        next_rule = &rules[tok.type];
        if (pre > next_rule->pre) {
            break;
        }

        if (next_rule->infix == SEPL_NULL) {
            sepl_err_uptok(&com->error, tok, SEPL_TOK_ERROR);
            return;
        }

        sepl__nexttok(com);
        next_rule->infix(com);
        sepl__check(com);
    }
}

SEPL_API void sepl__markvar(SeplCompiler *com, const char *start) {
    SeplValue v = sepl_val_object((void *)start);
    sepl_mod_val(com->mod, v, &com->error);
    com->scope_size++;
    sepl__check_vlimit(com);
}

SEPL_API void sepl__markscope(SeplCompiler *com) {
    sepl_mod_val(com->mod, sepl_val_scope(0), &com->error);
    com->scope_size++;
    sepl__check_vlimit(com);
}

SEPL_API void sepl__markfunc(SeplCompiler *com) {
    sepl_mod_val(com->mod, sepl_val_func(0), &com->error);
    com->scope_size++;
    sepl__check_vlimit(com);
}

SEPL_API char sepl__varcmp(const char *v, const char *i) {
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

SEPL_API sepl_size sepl__findvar(SeplCompiler *com, SeplToken iden,
                                 sepl_size *upv) {
    sepl_size pos;
    *upv = SEPL__ASSIGN_LOC;

    for (pos = com->mod->vpos; pos-- != 0;) {
        char *v_start, *i_start;

        if (sepl_val_isscp(com->mod->values[pos])) {
            *upv = SEPL__ASSIGN_UPS;
            continue;
        } else if (sepl_val_isfun(com->mod->values[pos])) {
            *upv = SEPL__ASSIGN_UPV;
            continue;
        }

        v_start = (char *)com->mod->values[pos].as.obj;
        i_start = (char *)iden.start;

        if (sepl__varcmp(v_start, i_start)) {
            return pos;
        }
    }

    sepl_err_iden(&com->error, SEPL_ERR_IDEN_NDEF, iden);
    return 0;
}

SEPL_API char sepl__existsvar(SeplCompiler *com, SeplToken iden) {
    sepl_size pos;

    for (pos = com->mod->vpos; pos-- != 0;) {
        char *v_start, *i_start;

        if (sepl_val_isscp(com->mod->values[pos])) {
            break;
        } else if (sepl_val_isfun(com->mod->values[pos])) {
            break;
        }

        v_start = (char *)com->mod->values[pos].as.obj;
        i_start = (char *)iden.start;

        if (sepl__varcmp(v_start, i_start)) {
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

SEPL_API void sepl__number(SeplCompiler *com) {
    double num = sepl_lex_num(sepl__currtok(com));
    sepl__writeconst(com, SEPL_BC_CONST, num);
    sepl__check_vlimit(com);
}

SEPL_API void sepl__string(SeplCompiler *com) {
    SeplToken cur = sepl__currtok(com);
    sepl_size i, slen = cur.end - cur.start - 2;
    unsigned char *wlen;

    sepl__writebyte(com, SEPL_BC_STR);
    wlen = sepl__writeplaceholder(com);
    for (i = 0; i < slen; i++) {
        char c = cur.start[i + 1];
        if (c == '\\')
            c = sepl_to_special(cur.start[++i + 1]);
        sepl__writebyte(com, (SeplBC)c);
    }
    sepl__writebyte(com, (SeplBC)'\0');
    *(sepl_size *)wlen = (com->mod->bytes + com->mod->bpos) - wlen - 4 - 1;

    com->scope_size++;
    sepl__check_vlimit(com);
}

SEPL_API void sepl__none(SeplCompiler *com) {
    sepl__writebyte(com, SEPL_BC_NONE);
    com->scope_size++;
    sepl__check_vlimit(com);
}

SEPL_API void sepl__unary(SeplCompiler *com) {
    SeplToken op = sepl__currtok(com);

    sepl__nexttok(com);
    sepl__parse(com, SEPL_PRE_UNARY);
    sepl__check(com);

    switch (op.type) {
        case SEPL_TOK_SUB:
            sepl__writebyte(com, SEPL_BC_NEG);
            break;
        case SEPL_TOK_NOT:
            sepl__writebyte(com, SEPL_BC_NOT);
            break;
        default:
            break;
    }
}

SEPL_API void sepl__binary(SeplCompiler *com) {
    SeplToken op = sepl__currtok(com);
    SeplParseRule *rule = sepl__getrule(op);

    sepl__nexttok(com);
    sepl__parse(com, (Precedence)(rule->pre + 1));
    sepl__check(com);
    com->scope_size--;

    switch (op.type) {
        case SEPL_TOK_ADD:
            sepl__writebyte(com, SEPL_BC_ADD);
            break;
        case SEPL_TOK_SUB:
            sepl__writebyte(com, SEPL_BC_SUB);
            break;
        case SEPL_TOK_MUL:
            sepl__writebyte(com, SEPL_BC_MUL);
            break;
        case SEPL_TOK_DIV:
            sepl__writebyte(com, SEPL_BC_DIV);
            break;

        case SEPL_TOK_LT:
            sepl__writebyte(com, SEPL_BC_LT);
            break;
        case SEPL_TOK_LTE:
            sepl__writebyte(com, SEPL_BC_LTE);
            break;
        case SEPL_TOK_GT:
            sepl__writebyte(com, SEPL_BC_GT);
            break;
        case SEPL_TOK_GTE:
            sepl__writebyte(com, SEPL_BC_GTE);
            break;
        case SEPL_TOK_EQ:
            sepl__writebyte(com, SEPL_BC_EQ);
            break;
        case SEPL_TOK_NEQ:
            sepl__writebyte(com, SEPL_BC_NEQ);
            break;

        default:
            break;
    }
}

SEPL_API void sepl__and(SeplCompiler *com) {
    unsigned char *jump;

    sepl__writebyte(com, SEPL_BC_JUMPIF);
    jump = sepl__writeplaceholder(com);
    com->scope_size--;

    sepl__nexttok(com);
    sepl__parse(com, (Precedence)(SEPL_PRE_AND + 1));

    if (sepl__peektok(com).type != SEPL_TOK_AND) {
        unsigned char *jfalse, *jtrue;
        sepl__writebyte(com, SEPL_BC_JUMPIF);
        jfalse = sepl__writeplaceholder(com);
        com->scope_size--;

        sepl__writeconst(com, SEPL_BC_CONST, 1.0);
        sepl__writebyte(com, SEPL_BC_JUMP);
        jtrue = sepl__writeplaceholder(com);

        sepl__setpholder(com, jump);
        sepl__setpholder(com, jfalse);
        sepl__writeconst(com, SEPL_BC_CONST, 0.0);
        sepl__setpholder(com, jtrue);
        com->scope_size--;

    } else {
        sepl__nexttok(com);
        sepl__and(com);
        sepl__check(com);
        *(sepl_size *)jump = com->mod->bpos - 1 - sizeof(sepl_size);
    }
}

SEPL_API void sepl__or(SeplCompiler *com) {
    unsigned char *end, *next;

    sepl__writebyte(com, SEPL_BC_JUMPIF);
    next = sepl__writeplaceholder(com);

    sepl__writebyte(com, SEPL_BC_JUMP);
    end = sepl__writeplaceholder(com);
    sepl__setpholder(com, next);

    sepl__nexttok(com);
    sepl__parse(com, (Precedence)(SEPL_PRE_OR + 1));

    if (sepl__peektok(com).type != SEPL_TOK_OR) {
        unsigned char *jfalse, *jtrue;
        sepl__writebyte(com, SEPL_BC_JUMPIF);
        jfalse = sepl__writeplaceholder(com);
        com->scope_size--;

        sepl__setpholder(com, end);
        sepl__writeconst(com, SEPL_BC_CONST, 1.0);
        sepl__writebyte(com, SEPL_BC_JUMP);
        jtrue = sepl__writeplaceholder(com);

        sepl__setpholder(com, jfalse);
        sepl__writeconst(com, SEPL_BC_CONST, 0.0);
        sepl__setpholder(com, jtrue);
        com->scope_size--;
    } else {
        sepl__nexttok(com);
        sepl__or(com);
        sepl__check(com);
        *(sepl_size *)end = com->mod->bpos - (1 * 3 + sizeof(sepl_size) * 3);
    }
}

SEPL_API void sepl__assign(SeplCompiler *com, sepl_size index,
                           sepl_size upvalue) {
    sepl_size oss = com->scope_size;

    sepl__nexttok(com);

    com->assign_type = upvalue;
    sepl__expr(com);
    com->assign_type = 0;

    if (upvalue <= SEPL__ASSIGN_UPS)
        sepl__writesized(com, SEPL_BC_SET, (com->scope_size - index));
    else
        sepl__writesized(com, SEPL_BC_SET_UP, index);

    com->scope_size = oss;
}

SEPL_API void sepl__identifier(SeplCompiler *com) {
    SeplToken iden;
    sepl_size index, upvalue;
    SeplToken next;

    sepl__check_tok(com, SEPL_TOK_IDENTIFIER);
    iden = sepl__currtok(com);
    index = sepl__findvar(com, iden, &upvalue);

    sepl__check(com);
    next = sepl__peektok(com);
    if (next.type == SEPL_TOK_ASSIGN) {
        if (index < com->env.predef_len) {
            sepl_err_new(&com->error, SEPL_ERR_PREDEF);
            return;
        }

        sepl__nexttok(com);
        sepl__assign(com, index, upvalue);
    } else {
        if (upvalue <= SEPL__ASSIGN_UPS)
            sepl__writesized(com, SEPL_BC_GET, (com->scope_size - index));
        else
            sepl__writesized(com, SEPL_BC_GET_UP, index);
        com->scope_size++;
        sepl__check_vlimit(com);
    }
}

SEPL_API void sepl__variable(SeplCompiler *com) {
    SeplToken iden = sepl__nexttok(com);
    sepl_size oss = com->scope_size;
    sepl__check_tok(com, SEPL_TOK_IDENTIFIER);

    /* If the identifier already exists within the current scope */
    if (sepl__existsvar(com, iden)) {
        sepl_err_iden(&com->error, SEPL_ERR_IDEN_RDEF, iden);
        return;
    }

    sepl__markvar(com, iden.start);
    sepl__check(com);

    sepl__writebyte(com, SEPL_BC_NONE);
    sepl__identifier(com);

    /* Encountered Get instead of Set */
    if (com->scope_size - oss == 2) {
        /* Remove Get instruction */
        com->mod->bpos -= 1 + sizeof(sepl_size);
        com->scope_size--;
    }
}

SEPL_API void sepl__params(SeplCompiler *com, unsigned char *pcount) {
    sepl_size offset = 0;
    sepl__check_tok(com, SEPL_TOK_LPAREN);

    while (sepl__nexttok(com).type != SEPL_TOK_RPAREN) {
        SeplToken tok = sepl__currtok(com);
        sepl__check_tok(com, SEPL_TOK_IDENTIFIER);
        sepl__markvar(com, tok.start);
        offset++;

        sepl__nexttok(com);
        if (sepl__currtok(com).type == SEPL_TOK_RPAREN)
            break;
        sepl__check_tok(com, SEPL_TOK_COMMA);
    }

    *(sepl_size *)pcount = offset;
    sepl__check_tok(com, SEPL_TOK_RPAREN);
}

SEPL_API void sepl__func(SeplCompiler *com) {
    unsigned char *skip, *params;
    sepl_size oss = com->scope_size, ovp = com->mod->vpos;

    if (com->func_block) {
        sepl_err_new(&com->error, SEPL_ERR_CLOSURE);
        return;
    }
    if (com->assign_type >= SEPL__ASSIGN_UPS) {
        sepl_err_new(&com->error, SEPL_ERR_FUNC_UPV);
        return;
    }

    sepl__writebyte(com, SEPL_BC_FUNC);
    skip = sepl__writeplaceholder(com);
    params = sepl__writeplaceholder(com);
    sepl__markfunc(com);

    sepl__nexttok(com);
    sepl__params(com, params);
    sepl__nexttok(com);

    com->func_block = 1;
    sepl__block(com);
    com->func_block = 0;

    sepl__writebyte(com, SEPL_BC_RETURN);
    sepl__setpholder(com, skip);

    com->scope_size = oss + 1;
    com->mod->vpos = ovp;
    sepl__check_vlimit(com);
}

SEPL_API void sepl__call(SeplCompiler *com) {
    sepl_size curr_ss = com->scope_size;

    while (sepl__nexttok(com).type != SEPL_TOK_RPAREN) {
        sepl__expr(com);
        sepl__nexttok(com);
        if (sepl__currtok(com).type == SEPL_TOK_RPAREN)
            break;
        sepl__check_tok(com, SEPL_TOK_COMMA);
    }

    sepl__writesized(com, SEPL_BC_CALL, com->scope_size - curr_ss);
    sepl__check_tok(com, SEPL_TOK_RPAREN);

    com->scope_size = curr_ss;
}

SEPL_API void sepl__grouping(SeplCompiler *com) {
    sepl__check_tok(com, SEPL_TOK_LPAREN);
    sepl__nexttok(com);
    if (sepl__currtok(com).type == SEPL_TOK_RPAREN) {
        return;
    }

    sepl__expr(com);
    sepl__check(com);
    sepl__nexttok(com);
    sepl__check_tok(com, SEPL_TOK_RPAREN);
}

SEPL_API void sepl__expr(SeplCompiler *com) { sepl__parse(com, SEPL_PRE_EXPR); }

SEPL_API void sepl__return(SeplCompiler *com) {
    sepl_size old_ss = com->scope_size;
    SeplToken peek = sepl__peektok(com);

    if (peek.type != SEPL_TOK_SEMICOLON) {
        sepl__nexttok(com);
        sepl__expr(com);
        if (old_ss == com->scope_size)
            sepl__writebyte(com, SEPL_BC_NONE);
    } else {
        sepl__writebyte(com, SEPL_BC_NONE);
    }
    sepl__writebyte(com, SEPL_BC_RETURN);
    com->scope_size = old_ss + 1;
}

SEPL_API void sepl__else(SeplCompiler *com, unsigned char *if_jump,
                         unsigned char **end_jump) {
    unsigned char *pop_skip;
    sepl__writepop(com);

    if (sepl__peektok(com).type != SEPL_TOK_ELSE) {
        sepl__writebyte(com, SEPL_BC_JUMP);
        pop_skip = sepl__writeplaceholder(com);
        sepl__setpholder(com, if_jump);
        sepl__setpholder(com, pop_skip);
        return;
    }

    sepl__writebyte(com, SEPL_BC_JUMP);
    *end_jump = sepl__writeplaceholder(com);

    sepl__nexttok(com);
    sepl__nexttok(com);

    sepl__setpholder(com, if_jump);

    if (sepl__currtok(com).type == SEPL_TOK_IF) {
        sepl__if(com);
    } else {
        sepl__block(com);
        if (com->block_ret) {
            /* Propagate return value */
            sepl__writebyte(com, SEPL_BC_RETURN);
        } else {
            sepl__writepop(com);
        }
    }

    sepl__check(com);
    sepl__setpholder(com, *end_jump);
}

SEPL_API void sepl__if(SeplCompiler *com) {
    sepl_size oss = com->scope_size;
    unsigned char *if_jump = SEPL_NULL;
    unsigned char *end_jump = SEPL_NULL;

    sepl__nexttok(com);
    sepl__grouping(com);
    sepl__nexttok(com);
    sepl__check(com);

    sepl__writebyte(com, SEPL_BC_JUMPIF);
    if_jump = sepl__writeplaceholder(com);
    com->scope_size--;

    if (com->scope_size == oss - 1) {
        sepl_err_new(&com->error, SEPL_ERR_EEXPR);
        return;
    }

    sepl__block(com);
    sepl__check(com);

    if (com->block_ret) {
        /* Propagate return value */
        sepl__writebyte(com, SEPL_BC_RETURN);
    }

    sepl__else(com, if_jump, &end_jump);
}

SEPL_API void sepl__while(SeplCompiler *com) {
    sepl_size oss = com->scope_size;
    unsigned char *cond_jump = SEPL_NULL;
    sepl_size loop_start = com->mod->bpos, scope_jump;

    sepl__nexttok(com);
    sepl__grouping(com);
    sepl__nexttok(com);
    sepl__check(com);

    sepl__writebyte(com, SEPL_BC_JUMPIF);
    cond_jump = sepl__writeplaceholder(com);
    com->scope_size--;

    if (com->scope_size == oss - 1) {
        sepl_err_new(&com->error, SEPL_ERR_EEXPR);
        return;
    }
    scope_jump = com->mod->bpos;

    sepl__block(com);
    sepl__check(com);

    if (com->block_ret) {
        sepl__writebyte(com, SEPL_BC_RETURN);
    } else if (!com->block_ret && com->inner_ret) {
        /* Remove implicit RETURN NONE */
        com->mod->bpos -= 1 + 1;

        /* Simulate RETURN for while loop end */
        while (com->block_size != oss) {
            sepl__writebyte(com, SEPL_BC_POP);
            com->block_size--;
        }
        com->scope_size--;
        sepl__writesized(com, SEPL_BC_JUMP, loop_start);

        /* Return the value from other control structures to outer scope */
        *(sepl_size *)(com->mod->bytes + scope_jump + 1) = com->mod->bpos;
        sepl__writebyte(com, SEPL_BC_RETURN);
    } else {
        sepl__writepop(com);
        sepl__writesized(com, SEPL_BC_JUMP, loop_start);
    }

    sepl__setpholder(com, cond_jump);
}

SEPL_API void sepl__stmt(SeplCompiler *com) {
    SeplToken curr = sepl__currtok(com);

    /* Empty Statement */
    if (curr.type == SEPL_TOK_SEMICOLON) {
        return;
    }

    if (curr.type == SEPL_TOK_RETURN) {
        sepl__return(com);
    } else if (curr.type == SEPL_TOK_IF) {
        sepl__if(com);
        return;
    } else if (curr.type == SEPL_TOK_WHILE) {
        sepl__while(com);
        return;
    } else if (curr.type == SEPL_TOK_VAR) {
        sepl__variable(com);
    } else if (curr.type != SEPL_TOK_EOF) {
        sepl_size oss = com->scope_size;
        sepl__expr(com);
        if (com->scope_size - oss)
            sepl__writepop(com);
    }

    sepl__check(com);
    sepl__nexttok(com);
    sepl__check_tok(com, SEPL_TOK_SEMICOLON);
}

SEPL_API void sepl__block(SeplCompiler *com) {
    char ret = 0;
    SeplToken tok;
    sepl_size oss = com->scope_size, ovp = com->mod->vpos;
    unsigned char *scope_end = SEPL_NULL;

    sepl__check_tok(com, SEPL_TOK_LCURLY);
    tok = sepl__nexttok(com);

    sepl__writebyte(com, SEPL_BC_SCOPE);
    scope_end = sepl__writeplaceholder(com);
    sepl__markscope(com);
    com->inner_ret = 0;

    while (tok.type != SEPL_TOK_RCURLY) {
        sepl__stmt(com);
        sepl__check(com);
        com->inner_ret = com->inner_ret || com->block_ret;

        if (tok.type == SEPL_TOK_RETURN) {
            /* Ignore all statements after return */
            do {
                sepl__nexttok(com);
            } while (sepl__currtok(com).type != SEPL_TOK_RCURLY &&
                     sepl__currtok(com).type != SEPL_TOK_EOF);
            ret = 1;
            break;
        }
        tok = sepl__nexttok(com);
    }
    sepl__check_tok(com, SEPL_TOK_RCURLY);

    if (!ret) {
        sepl__writebyte(com, SEPL_BC_NONE);
        sepl__writebyte(com, SEPL_BC_RETURN);
    }

    com->block_size = com->scope_size;
    com->scope_size = oss + 1;
    com->mod->vpos = ovp;
    com->block_ret = ret;
    sepl__setpholder(com, scope_end);
    sepl__check_vlimit(com);
}

SEPL_API void module(SeplCompiler *com) {
    while (sepl__currtok(com).type != SEPL_TOK_EOF) {
        if (sepl__currtok(com).type == SEPL_TOK_IDENTIFIER &&
            sepl__peektok(com).type == SEPL_TOK_ASSIGN) {
            sepl__identifier(com);
        } else {
            sepl__variable(com);
        }
        sepl__check(com);
        sepl__nexttok(com);
        sepl__check_tok(com, SEPL_TOK_SEMICOLON);
        sepl__nexttok(com);
    }
}

SEPL_API SeplCompiler sepl__initcom(const char *source, SeplModule *mod,
                                    SeplEnv env) {
    SeplCompiler com = {0};
    sepl_size i;

    com.lex = sepl_lex_init(source);
    com.mod = mod;
    com.env = env;
    for (i = 0; i < env.predef_len; i++) {
        sepl__markvar(&com, env.predef[i].key);
    }
    for (i = 0; i < mod->esize; i++) {
        sepl__markvar(&com, mod->exports[i]);
    }

    return com;
}

SEPL_API void sepl__resetcom(SeplCompiler *com, SeplModule *mod) {
    if (com->error.code != SEPL_ERR_OK) {
        com->error.line = com->lex.line;
    }
    mod->vpos = 0;
}

SEPL_LIB SeplError sepl_com_module(const char *source, SeplModule *mod,
                                   SeplEnv env) {
    SeplCompiler com = sepl__initcom(source, mod, env);

    sepl__nexttok(&com);
    module(&com);

    sepl__resetcom(&com, mod);
    return com.error;
}

SEPL_LIB SeplError sepl_com_block(const char *source, SeplModule *mod,
                                  SeplEnv env) {
    SeplCompiler com = sepl__initcom(source, mod, env);

    sepl__nexttok(&com);
    sepl__block(&com);

    sepl__resetcom(&com, mod);
    return com.error;
}

SEPL_LIB SeplError sepl_com_expr(const char *source, SeplModule *mod,
                                 SeplEnv env) {
    SeplCompiler com = sepl__initcom(source, mod, env);
    unsigned char *scope_end;

    sepl__writebyte(&com, SEPL_BC_SCOPE);
    scope_end = sepl__writeplaceholder(&com);
    sepl__markscope(&com);

    sepl__nexttok(&com);
    if (sepl__currtok(&com).type == SEPL_TOK_EOF)
        sepl__writebyte(&com, SEPL_BC_NONE);
    else
        sepl__expr(&com);
    sepl__writebyte(&com, SEPL_BC_RETURN);

    sepl__setpholder(&com, scope_end);

    sepl__resetcom(&com, mod);
    return com.error;
}
