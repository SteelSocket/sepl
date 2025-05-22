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

#ifndef SEPL_LIBRARY
#define SEPL_LIBRARY


#ifdef __cplusplus
extern "C" {
#endif


#define sepl__static_assert(cond, name) \
    typedef char sepl__assert_##name[(cond) ? 1 : -1]
#define sepl__is_unsigned(type) ((type)(-1) > (type)(0))

#ifndef SEPL_DEF_SIZE
typedef unsigned long sepl_size;
#else
typedef SEPL_DEF_SIZE sepl_size;
sepl__static_assert(sepl__is_unsigned(sepl_size), is_unsigned);
#endif

#ifndef SEPL_NULL
#ifdef __cplusplus
#define SEPL_NULL nullptr
#else
#define SEPL_NULL ((void *)0)
#endif
#endif

#ifndef SEPL_LIB
#define SEPL_LIB extern
#endif

#ifndef SEPL_API
#define SEPL_API static
#endif



typedef enum {
    SEPL_TOK_ERROR,
    SEPL_TOK_SEMICOLON,
    SEPL_TOK_COMMA,

    SEPL_TOK_NUM,
    SEPL_TOK_VAR,
    SEPL_TOK_FUNC,
    SEPL_TOK_NONE,
    SEPL_TOK_STRING,

    SEPL_TOK_ASSIGN,

    SEPL_TOK_ADD,
    SEPL_TOK_SUB,
    SEPL_TOK_MUL,
    SEPL_TOK_DIV,

    SEPL_TOK_NOT,
    SEPL_TOK_AND,
    SEPL_TOK_OR,

    SEPL_TOK_LT,
    SEPL_TOK_LTE,
    SEPL_TOK_GT,
    SEPL_TOK_GTE,
    SEPL_TOK_EQ,
    SEPL_TOK_NEQ,

    SEPL_TOK_LPAREN,
    SEPL_TOK_RPAREN,
    SEPL_TOK_LCURLY,
    SEPL_TOK_RCURLY,

    SEPL_TOK_IDENTIFIER,

    SEPL_TOK_RETURN,
    SEPL_TOK_IF,
    SEPL_TOK_ELSE,
    SEPL_TOK_WHILE,

    SEPL_TOK_EOF
} SeplTokenT;

typedef struct {
    SeplTokenT type;
    const char *start;
    const char *end;
    sepl_size line;
} SeplToken;

typedef struct {
    const char *source;
    sepl_size line;
    SeplToken current;
} SeplLexer;

SEPL_LIB char sepl_is_digit(char c);
SEPL_LIB char sepl_is_alpha(char c);
SEPL_LIB char sepl_is_delim(char c);
SEPL_LIB char sepl_is_identifier(char c);
SEPL_LIB int sepl_to_digit(char c);
SEPL_LIB char sepl_to_special(char c);

SEPL_LIB SeplLexer sepl_lex_init(const char *source);
SEPL_LIB SeplToken sepl_lex_next(SeplLexer *lex);
SEPL_LIB SeplToken sepl_lex_peek(SeplLexer lex);
SEPL_LIB double sepl_lex_num(SeplToken tok);



typedef enum {
    SEPL_ERR_OK,

    /* Value buffer errors */
    SEPL_ERR_VOVERFLOW,
    SEPL_ERR_VUNDERFLOW,

    /* Bytecode buffer errors */
    SEPL_ERR_BOVERFLOW,

    /* Compile errors */
    SEPL_ERR_UPTOK, /* unexpected token */
    SEPL_ERR_SYN,   /* generic syntax error */
    SEPL_ERR_EEXPR, /* expected expression */

    SEPL_ERR_IDEN_NDEF, /* identifier not defined */
    SEPL_ERR_IDEN_RDEF, /* identifier redefined */
    SEPL_ERR_PREDEF,    /* set predefined variable */

    SEPL_ERR_CLOSURE,  /* closure not supported */
    SEPL_ERR_FUNC_UPV, /* set function to upvalue */

    /* Runtime Errors */
    SEPL_ERR_BC,        /* undefined bytecode */
    SEPL_ERR_FUNC_RET,  /* returning a sepl function */
    SEPL_ERR_FUNC_CALL, /* calling a non function */
    SEPL_ERR_REFMOVE,   /* moving a reference to another variable */
    SEPL_ERR_OPER       /* invalid operations on value */
} SeplErrorCode;

typedef struct {
#ifndef NDEBUG
    sepl_size src_line;
#endif
    sepl_size line;
    SeplErrorCode code;
    union {
        /* SEPL_ERR_UPTOK, SEPL_ERR_SYN */
        struct {
            SeplToken up;
            SeplTokenT exp;
        } tokd;

        /* SEPL_ERR_IDEN_NDEF, SEPL_ERR_IDEN_RDEF */
        SeplToken iden;

        /* SEPL_ERR_BC */
        char bc;
    } info;

} SeplError;

#ifndef NDEBUG
#define sepl_err_new(e, ecode) ((e)->code = ecode, (e)->src_line = __LINE__)
#else
#define sepl_err_new(e, ecode) ((e)->code = ecode)
#endif

#define sepl_err_uptok(e, utok, etok)                           \
    (sepl_err_new(e, SEPL_ERR_UPTOK), (e)->info.tokd.up = utok, \
     (e)->info.tokd.exp = etok)
#define sepl_err_iden(e, code, i) (sepl_err_new(e, code), (e)->info.iden = i)



typedef struct SeplValue SeplValue;

typedef enum {
    SEPL_VAL_NONE,
    SEPL_VAL_SCOPE,
    SEPL_VAL_NUM,
    SEPL_VAL_STR,
    SEPL_VAL_FUNC,
    SEPL_VAL_CFUNC,
    SEPL_VAL_REF,
    SEPL_VAL_OBJ
} SeplValueType;

typedef struct {
    SeplValue *values;
    sepl_size size;
} SeplArgs;

typedef SeplValue (*sepl_c_func)(SeplArgs, SeplError *);
typedef void (*sepl_free_func)(SeplValue);

struct SeplValue {
    sepl_size type;
    union {
        /* SEPL_VAL_SCOPE, SEPL_VAL_FUNC */
        sepl_size pos;
        /* SEPL_VAL_NUM */
        double num;
        /* SEPL_VAL_CFUNC */
        sepl_c_func cfunc;
        /* SEPL_VAL_STR, SEPL_VAL_REF, SEPL_VAL_OBJ */
        void *obj;
    } as;
};

extern const SeplValue SEPL_NONE;

#define sepl_val_isnone(val) (val.type == SEPL_VAL_NONE)
#define sepl_val_isscp(val) (val.type == SEPL_VAL_SCOPE)
#define sepl_val_isnum(val) (val.type == SEPL_VAL_NUM)
#define sepl_val_isstr(val) (val.type == SEPL_VAL_STR)
#define sepl_val_isfun(val) (val.type == SEPL_VAL_FUNC)
#define sepl_val_iscfun(val) (val.type == SEPL_VAL_CFUNC)
#define sepl_val_isref(val) (val.type == SEPL_VAL_REF)
#define sepl_val_isobj(val) (val.type >= SEPL_VAL_OBJ)

SEPL_LIB SeplValue sepl_val_asref(void *v);
SEPL_LIB SeplValue sepl_val_scope(sepl_size pos);
SEPL_LIB SeplValue sepl_val_number(double vnum);
SEPL_LIB SeplValue sepl_val_str(char *str);
SEPL_LIB SeplValue sepl_val_func(sepl_size pos);
SEPL_LIB SeplValue sepl_val_cfunc(sepl_c_func cfunc);

SEPL_LIB SeplValue sepl_val_object(void *vobj);
SEPL_LIB SeplValue sepl_val_type(void *vobj, sepl_size custom_id);



typedef struct {
    const char *key;
    SeplValue value;
} SeplValuePair;

typedef struct {
    sepl_free_func free;

    SeplValuePair *predef;
    sepl_size predef_len;
} SeplEnv;



typedef enum {
    SEPL_BC_RETURN,
    SEPL_BC_JUMPIF,
    SEPL_BC_JUMP,

    SEPL_BC_CALL,
    SEPL_BC_POP,

    SEPL_BC_NONE,
    SEPL_BC_CONST,
    SEPL_BC_STR,
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
} SeplBC;

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
SEPL_LIB sepl_size sepl_mod_bc(SeplModule *mod, SeplBC bc, SeplError *e);
SEPL_LIB sepl_size sepl_mod_bcnum(SeplModule *mod, double n, SeplError *e);
SEPL_LIB sepl_size sepl_mod_bcsize(SeplModule *mod, sepl_size s, SeplError *e);
SEPL_LIB sepl_size sepl_mod_val(SeplModule *mod, SeplValue v, SeplError *e);

SEPL_LIB void sepl_mod_init(SeplModule *mod, SeplError *e, SeplEnv env);
SEPL_LIB void sepl_mod_cleanup(SeplModule *mod, SeplEnv env);
SEPL_LIB SeplValue sepl_mod_step(SeplModule *mod, SeplError *e, SeplEnv env);
SEPL_LIB SeplValue sepl_mod_exec(SeplModule *mod, SeplError *e, SeplEnv env);
SEPL_LIB void sepl_mod_initfunc(SeplModule *mod, SeplError *e, SeplValue func,
                                SeplArgs args);
SEPL_LIB SeplValue sepl_mod_getexport(SeplModule *mod, SeplEnv env,
                                      const char *key);



SEPL_LIB SeplError sepl_com_module(const char *source, SeplModule *mod,
                                   SeplEnv env);
SEPL_LIB SeplError sepl_com_block(const char *source, SeplModule *mod,
                                  SeplEnv env);
SEPL_LIB SeplError sepl_com_expr(const char *source, SeplModule *mod,
                                 SeplEnv env);

#ifdef __cplusplus
}
#endif

#ifdef SEPL_IMPLEMENTATION



SEPL_LIB char sepl_is_digit(char c) { return c >= '0' && c <= '9'; }

SEPL_LIB char sepl_is_alpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}
SEPL_LIB char sepl_is_delim(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

SEPL_LIB char sepl_is_identifier(char c) {
    return sepl_is_digit(c) || sepl_is_alpha(c) || c == '_';
}

SEPL_LIB int sepl_to_digit(char c) { return c - '0'; }

SEPL_LIB char sepl_to_special(char c) {
    switch (c) {
        case 'n':
            return '\n';
        case 't':
            return '\t';
        case 'r':
            return '\r';
        case 'b':
            return '\b';
        case 'f':
            return '\f';
        case 'v':
            return '\v';
        case 'a':
            return '\a';
        case '\\':
            return '\\';
        case '\'':
            return '\'';
        case '\"':
            return '\"';
        case '?':
            return '\?';
        case '0':
            return '\0';
    }
    return c;
}

SEPL_API SeplToken sepl__make_tok(SeplLexer *lex, SeplTokenT type) {
    SeplToken tok;
    tok.type = type;
    tok.start = lex->source - 1;
    tok.end = lex->source;
    return tok;
}

SEPL_API SeplToken sepl__make_number(SeplLexer *lex) {
    SeplToken tok;
    char dot_found = 0;

    tok.type = SEPL_TOK_NUM;
    tok.line = lex->line;
    tok.start = lex->source - 1;
    tok.end = lex->source;

    while (1) {
        char c = *lex->source++;

        if (sepl_is_digit(c)) {
            ++tok.end;
            continue;
        }
        if (c == '.') {
            if (dot_found)
                goto invalid_num;
            dot_found = 1;
            ++tok.end;
            continue;
        }
        if (sepl_is_alpha(c))
            goto invalid_num;

        --lex->source;
        break;
    }

    return tok;

invalid_num:
    tok.type = SEPL_TOK_ERROR;
    tok.start = "Invalid number";
    tok.line = lex->line;
    return tok;
}

SEPL_API SeplToken sepl__make_string(SeplLexer *lex) {
    SeplToken tok;
    tok.type = SEPL_TOK_STRING;
    tok.start = lex->source - 1;
    tok.end = lex->source;

    while (*tok.end != '"' && *tok.end != '\0') {
        if (*tok.end++ == '\\') {
            tok.end++;
        }
    }

    if (*tok.end == '\0') {
        tok.type = SEPL_TOK_ERROR;
        tok.start = "Unexpected EOF while parsing string";
        tok.line = lex->line;
        return tok;
    }

    lex->source = ++tok.end;
    return tok;
}

SEPL_API SeplToken sepl__make_identifier(SeplLexer *lex) {
    SeplToken tok;
    tok.type = SEPL_TOK_IDENTIFIER;
    tok.start = lex->source - 1;
    tok.end = lex->source;

    while (sepl_is_identifier(*tok.end++));

    lex->source = --tok.end;
    return tok;
}

SEPL_API SeplToken sepl__make_keyword(SeplLexer *lex, const char *keyword,
                                      SeplTokenT type) {
    SeplToken iden = sepl__make_identifier(lex);
    const char *start = iden.start;

    while (start != iden.end) {
        if (*start++ != *keyword++)
            return iden;
    }
    if (*keyword != '\0')
        return iden;

    iden.type = type;
    return iden;
}

SEPL_API SeplToken next_token(SeplLexer *lex) {
    char c = *lex->source++;

    switch (c) {
        case '\n':
            ++lex->line;
            return next_token(lex);
        case ' ':
        case '\r':
        case '\t':
            return next_token(lex);
        case ';':
            return sepl__make_tok(lex, SEPL_TOK_SEMICOLON);
        case ',':
            return sepl__make_tok(lex, SEPL_TOK_COMMA);

        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            return sepl__make_number(lex);

        case '=':
            if (*lex->source == '=') {
                lex->source++;
                return sepl__make_tok(lex, SEPL_TOK_EQ);
            }
            return sepl__make_tok(lex, SEPL_TOK_ASSIGN);

        case '+':
            return sepl__make_tok(lex, SEPL_TOK_ADD);
        case '-':
            return sepl__make_tok(lex, SEPL_TOK_SUB);
        case '*':
            return sepl__make_tok(lex, SEPL_TOK_MUL);
        case '/':
            return sepl__make_tok(lex, SEPL_TOK_DIV);

        case '!':
            if (*lex->source == '=') {
                lex->source++;
                return sepl__make_tok(lex, SEPL_TOK_NEQ);
            }
            return sepl__make_tok(lex, SEPL_TOK_NOT);

        case '&':
            if (*lex->source == '&') {
                lex->source++;
                return sepl__make_tok(lex, SEPL_TOK_AND);
            }
            break;
        case '|':
            if (*lex->source == '|') {
                lex->source++;
                return sepl__make_tok(lex, SEPL_TOK_OR);
            }
            break;
        case '>':
            if (*lex->source == '=') {
                lex->source++;
                return sepl__make_tok(lex, SEPL_TOK_GTE);
            }
            return sepl__make_tok(lex, SEPL_TOK_GT);

        case '<':
            if (*lex->source == '=') {
                lex->source++;
                return sepl__make_tok(lex, SEPL_TOK_LTE);
            }
            return sepl__make_tok(lex, SEPL_TOK_LT);

        case '(':
            return sepl__make_tok(lex, SEPL_TOK_LPAREN);
        case ')':
            return sepl__make_tok(lex, SEPL_TOK_RPAREN);
        case '{':
            return sepl__make_tok(lex, SEPL_TOK_LCURLY);
        case '}':
            return sepl__make_tok(lex, SEPL_TOK_RCURLY);

        case '@':
            return sepl__make_tok(lex, SEPL_TOK_VAR);
        case '$':
            return sepl__make_tok(lex, SEPL_TOK_FUNC);
        case '"':
            return sepl__make_string(lex);

        case 'e':
            return sepl__make_keyword(lex, "else", SEPL_TOK_ELSE);
        case 'i':
            return sepl__make_keyword(lex, "if", SEPL_TOK_IF);
        case 'N':
            return sepl__make_keyword(lex, "NONE", SEPL_TOK_NONE);
        case 'r':
            return sepl__make_keyword(lex, "return", SEPL_TOK_RETURN);
        case 'w':
            return sepl__make_keyword(lex, "while", SEPL_TOK_WHILE);

        case '_':
            return sepl__make_identifier(lex);

        case '\0': {
            lex->source--; /* Ensure lexer always returns EOF after first EOF*/
            return sepl__make_tok(lex, SEPL_TOK_EOF);
        }

        default:
            break;
    }

    if (sepl_is_alpha(c)) {
        return sepl__make_identifier(lex);
    }

    return sepl__make_tok(lex, SEPL_TOK_ERROR);
}

SEPL_LIB SeplLexer sepl_lex_init(const char *source) {
    SeplLexer lex = {0};
    lex.source = source;
    return lex;
}

SEPL_LIB SeplToken sepl_lex_next(SeplLexer *lex) {
    SeplToken tok = next_token(lex);
    lex->current = tok;
    return tok;
}

SEPL_LIB SeplToken sepl_lex_peek(SeplLexer lex) { return next_token(&lex); }

SEPL_LIB double sepl_lex_num(SeplToken tok) {
    double result = 0.0;
    sepl_size factor = 10;

    for (; tok.start != tok.end;) {
        char c = *tok.start++;
        if (c == '.') {
            break;
        }
        result = result * 10 + sepl_to_digit(c);
    }

    for (; tok.start != tok.end;) {
        char c = *tok.start++;
        result = result + ((double)sepl_to_digit(c) / factor);
        factor *= 10;
    }

    return result;
}

const SeplValue SEPL_NONE = {0};

SEPL_LIB SeplValue sepl_val_asref(void *v) {
    SeplValue r;
    r.type = SEPL_VAL_REF;
    r.as.obj = v;
    return r;
}

SEPL_LIB SeplValue sepl_val_scope(sepl_size pos) {
    SeplValue r;
    r.type = SEPL_VAL_SCOPE;
    r.as.pos = pos;
    return r;
}

SEPL_LIB SeplValue sepl_val_number(double vnum) {
    SeplValue r;
    r.type = SEPL_VAL_NUM;
    r.as.num = vnum;
    return r;
}

SEPL_LIB SeplValue sepl_val_str(char *str) {
    SeplValue r;
    r.type = SEPL_VAL_STR;
    r.as.obj = str;
    return r;
}

SEPL_LIB SeplValue sepl_val_func(sepl_size pos) {
    SeplValue r;
    r.type = SEPL_VAL_FUNC;
    r.as.pos = pos;
    return r;
}

SEPL_LIB SeplValue sepl_val_cfunc(sepl_c_func cfunc) {
    SeplValue r;
    r.type = SEPL_VAL_CFUNC;
    r.as.cfunc = cfunc;
    return r;
}

SEPL_LIB SeplValue sepl_val_object(void *vobj) {
    return sepl_val_type(vobj, 0);
}

SEPL_LIB SeplValue sepl_val_type(void *vobj, sepl_size custom_id) {
    SeplValue r;
    r.type = SEPL_VAL_OBJ + custom_id;
    r.as.obj = vobj;
    return r;
}

SEPL_LIB SeplModule sepl_mod_new(unsigned char bytes[], sepl_size bsize,
                                 SeplValue values[], sepl_size vsize) {
    SeplModule mod = {0};
    mod.bytes = bytes;
    mod.bsize = bsize;
    mod.values = values;
    mod.vsize = vsize;
    return mod;
}

SEPL_LIB sepl_size sepl_mod_bc(SeplModule *mod, SeplBC bc, SeplError *e) {
    if (mod->bpos + 1 > mod->bsize) {
        sepl_err_new(e, SEPL_ERR_BOVERFLOW);
        return 0;
    }
    mod->bytes[mod->bpos] = bc;
    return mod->bpos++;
}

SEPL_LIB sepl_size sepl_mod_bcnum(SeplModule *mod, double n, SeplError *e) {
    if (mod->bpos + sizeof(n) > mod->bsize) {
        sepl_err_new(e, SEPL_ERR_BOVERFLOW);
        return 0;
    }
    *(double *)(mod->bytes + mod->bpos) = n;
    mod->bpos += sizeof(n);
    return mod->bpos - sizeof(n);
}

SEPL_LIB sepl_size sepl_mod_bcsize(SeplModule *mod, sepl_size s, SeplError *e) {
    if (mod->bpos + sizeof(s) > mod->bsize) {
        sepl_err_new(e, SEPL_ERR_BOVERFLOW);
        return 0;
    }
    *(sepl_size *)(mod->bytes + mod->bpos) = s;
    mod->bpos += sizeof(s);
    return mod->bpos - sizeof(s);
}

SEPL_LIB sepl_size sepl_mod_val(SeplModule *mod, SeplValue v, SeplError *e) {
    if (mod->vpos >= mod->vsize) {
        sepl_err_new(e, SEPL_ERR_VOVERFLOW);
        return 0;
    }

    mod->values[mod->vpos] = v;
    return mod->vpos++;
}

SEPL_API void sepl__free(SeplValue _) { (void)_; }

SEPL_LIB void sepl_mod_init(SeplModule *mod, SeplError *e, SeplEnv env) {
    sepl_size i;
    mod->vpos = 0;
    e->code = SEPL_ERR_OK;

    if (env.free == SEPL_NULL) {
        env.free = sepl__free;
    }

    for (i = 0; i < env.predef_len; i++) {
        sepl_mod_val(mod, env.predef[i].value, e);
    }
    for (i = 0; i < mod->esize; i++) {
        sepl_mod_val(mod, SEPL_NONE, e);
    }
}

SEPL_LIB void sepl_mod_cleanup(SeplModule *mod, SeplEnv env) {
    sepl_size i;
    for (i = mod->vpos; i-- > env.predef_len;) {
        SeplValue v = mod->values[i];
        if (sepl_val_isobj(v))
            env.free(v);
    }
}

SEPL_API double sepl__todbl(SeplError *err, SeplValue v) {
    if (sepl_val_isnum(v)) {
        return v.as.num;
    } else if (sepl_val_isobj(v)) {
        return v.as.num;
    } else if (sepl_val_isref(v)) {
        return sepl__todbl(err, *(SeplValue *)v.as.obj);
    }
    sepl_err_new(err, SEPL_ERR_OPER);
    return 0.0;
}

SEPL_LIB SeplValue sepl_mod_step(SeplModule *mod, SeplError *e, SeplEnv env) {
#define sepl__rddbl()           \
    (mod->pc += sizeof(double), \
     *(double *)(mod->bytes + mod->pc - sizeof(double)))
#define sepl__rdsz()               \
    (mod->pc += sizeof(sepl_size), \
     *(sepl_size *)(mod->bytes + mod->pc - sizeof(sepl_size)))

#define sepl__pushv(val) (sepl_mod_val(mod, val, e))
#define sepl__popv()                \
    (mod->vpos > env.predef_len     \
         ? mod->values[--mod->vpos] \
         : (sepl_err_new(e, SEPL_ERR_VUNDERFLOW), SEPL_NONE))
#define sepl__peekv(offset) (mod->values[mod->vpos - offset - 1])
#define sepl__popd()                                 \
    (sepl_val_isobj(sepl__peekv(0))                  \
         ? (env.free(sepl__popv()), sepl__peekv(-1)) \
         : sepl__popv())

#define sepl__unaryop(op)                   \
    do {                                    \
        SeplValue v = sepl__popv();         \
        double d = sepl__todbl(e, v);       \
        if (e->code)                        \
            break;                          \
        sepl__pushv(sepl_val_number(op d)); \
    } while (0)
#define sepl__binaryop(op)                                       \
    do {                                                         \
        SeplValue v2 = sepl__popv(), v1 = sepl__popv();          \
        double d1 = sepl__todbl(e, v1), d2 = sepl__todbl(e, v2); \
        if (e->code)                                             \
            break;                                               \
        sepl__pushv(sepl_val_number(d1 op d2));                  \
    } while (0)

    SeplBC bc = (SeplBC)mod->bytes[mod->pc++];

    switch (bc) {
        case SEPL_BC_RETURN: {
            SeplValue retv, v;
            e->code = SEPL_ERR_OK;
            if (mod->vpos <= env.predef_len)
                return SEPL_NONE;

            retv = sepl__popv();

            if (sepl_val_isfun(retv)) {
                sepl_err_new(e, SEPL_ERR_FUNC_RET);
                return SEPL_NONE;
            }

            /* Pop all values until scope block end */
            while (1) {
                v = sepl__popv();

                /* Prevent the return ref object from being freed */
                if (sepl_val_isref(retv) &&
                    retv.as.obj == (mod->values + mod->vpos)) {
                    retv = v; /* Deference */
                    continue;
                } else if (sepl_val_isobj(v)) {
                    env.free(v);
                } else if (sepl_val_isscp(v)) {
                    break;
                }
            }

            if (v.as.pos >= mod->bpos) {
                mod->pc = mod->bpos;
                return retv;
            }

            sepl__pushv(retv);
            mod->pc = v.as.pos;
            break;
        }
        case SEPL_BC_JUMP: {
            sepl_size jump = sepl__rdsz();
            mod->pc = jump;
            break;
        }
        case SEPL_BC_JUMPIF: {
            sepl_size jump = sepl__rdsz();
            SeplValue cond = sepl__peekv(0);
            if (!cond.as.num) {
                mod->pc = jump;
            }
            sepl__popd();
            break;
        }

        case SEPL_BC_CALL: {
            sepl_size offset = sepl__rdsz();
            SeplValue v = mod->values[mod->vpos - offset - 1];

            if (sepl_val_iscfun(v)) {
                SeplArgs args;
                SeplValue result;
                args.values = mod->values + mod->vpos - offset;
                args.size = offset;
                result = v.as.cfunc(args, e);

                /* Pop arguments */
                while (offset--) sepl__popd();
                mod->vpos--; /* Pop function variable */
                sepl__pushv(result);
            } else if (sepl_val_isfun(v)) {
                sepl_size param_c;
                mod->values[mod->vpos - offset - 1] = sepl_val_scope(mod->pc);
                mod->pc = v.as.pos;
                param_c = sepl__rdsz();

                if (param_c < offset) {
                    while (param_c++ != offset) sepl__popd();
                } else if (param_c > offset) {
                    while (param_c-- != offset) sepl__pushv(SEPL_NONE);
                }

            } else {
                sepl_err_new(e, SEPL_ERR_FUNC_CALL);
                return SEPL_NONE;
            }
            break;
        }
        case SEPL_BC_POP: {
            sepl__popd();
            break;
        }

        case SEPL_BC_SET: {
            sepl_size i = sepl__rdsz();
            SeplValue peek = sepl__peekv(0);
            sepl_size offset = mod->vpos - i;

            /* Reference is assign to another variable */
            if (sepl_val_isref(peek)) {
                sepl_err_new(e, SEPL_ERR_REFMOVE);
                break;
            }
            if (sepl_val_isobj(mod->values[offset]))
                env.free(mod->values[offset]);
            mod->values[offset] = sepl__popv();
            break;
        }
        case SEPL_BC_GET: {
            sepl_size i = sepl__rdsz();
            sepl_size offset = mod->vpos - i;
            SeplValue v = mod->values[offset];

            if (sepl_val_isobj(v)) {
                sepl__pushv(sepl_val_asref(&mod->values[offset]));
                break;
            }
            sepl__pushv(v);
            break;
        }

        case SEPL_BC_SET_UP: {
            sepl_size offset = sepl__rdsz();
            SeplValue peek = sepl__peekv(0);

            /* Reference is assign to another variable */
            if (sepl_val_isref(peek)) {
                sepl_err_new(e, SEPL_ERR_REFMOVE);
                break;
            }
            if (sepl_val_isobj(mod->values[offset]))
                env.free(mod->values[offset]);
            mod->values[offset] = sepl__popv();
            break;
        }
        case SEPL_BC_GET_UP: {
            sepl_size offset = sepl__rdsz();
            SeplValue v = mod->values[offset];

            if (sepl_val_isobj(v)) {
                sepl__pushv(sepl_val_asref(&mod->values[offset]));
                break;
            }
            sepl__pushv(v);
            break;
        }

        case SEPL_BC_NONE: {
            sepl__pushv(SEPL_NONE);
            break;
        }
        case SEPL_BC_CONST: {
            double v = sepl__rddbl();
            sepl__pushv(sepl_val_number(v));
            break;
        }
        case SEPL_BC_STR: {
            sepl_size len = sepl__rdsz();
            sepl__pushv(sepl_val_str((char *)(mod->bytes + mod->pc)));
            mod->pc += len + 1;
            break;
        }
        case SEPL_BC_SCOPE: {
            sepl_size end_pos = sepl__rdsz();
            sepl__pushv(sepl_val_scope(end_pos));
            break;
        }
        case SEPL_BC_FUNC: {
            sepl_size skip_pos = sepl__rdsz();
            sepl__pushv(sepl_val_func(mod->pc));
            mod->pc = skip_pos;
            break;
        }

        case SEPL_BC_NEG: {
            sepl__unaryop(-);
            break;
        }

        case SEPL_BC_ADD: {
            sepl__binaryop(+);
            break;
        }
        case SEPL_BC_SUB: {
            sepl__binaryop(-);
            break;
        }
        case SEPL_BC_MUL: {
            sepl__binaryop(*);
            break;
        }
        case SEPL_BC_DIV: {
            sepl__binaryop(/);
            break;
        }
        case SEPL_BC_NOT: {
            sepl__unaryop(!);
            break;
        }

        case SEPL_BC_LT: {
            sepl__binaryop(<);
            break;
        }
        case SEPL_BC_LTE: {
            sepl__binaryop(<=);
            break;
        }
        case SEPL_BC_GT: {
            sepl__binaryop(>);
            break;
        }
        case SEPL_BC_GTE: {
            sepl__binaryop(>=);
            break;
        }
        case SEPL_BC_EQ: {
            sepl__binaryop(==);
            break;
        }
        case SEPL_BC_NEQ: {
            sepl__binaryop(!=);
            break;
        }

        default: {
            sepl_err_new(e, SEPL_ERR_BC);
            e->info.bc = bc;
            return SEPL_NONE;
        }
    }
    return SEPL_NONE;
}

SEPL_LIB SeplValue sepl_mod_exec(SeplModule *mod, SeplError *e, SeplEnv env) {
    SeplValue retv = SEPL_NONE;

    while (mod->pc < mod->bpos) {
        retv = sepl_mod_step(mod, e, env);
        if (e->code != SEPL_ERR_OK)
            return SEPL_NONE;
    }
    return retv;
}

SEPL_LIB void sepl_mod_initfunc(SeplModule *mod, SeplError *e, SeplValue func,
                                SeplArgs args) {
    sepl_size args_count, i;
    if (!sepl_val_isfun(func)) {
        sepl_err_new(e, SEPL_ERR_FUNC_CALL);
        return;
    }

    mod->pc = func.as.pos;
    args_count = sepl__rdsz();

    sepl_mod_val(mod, sepl_val_scope(mod->bpos), e);
    for (i = 0; i < args.size; i++) {
        sepl_mod_val(mod, args.values[i], e);
    }
    if (e->code != SEPL_ERR_OK)
        return;

    if (args_count > args.size) {
        for (i = 0; i < args_count - args.size; i++) {
            sepl_mod_val(mod, SEPL_NONE, e);
        }
    } else if (args_count < args.size) {
        mod->vpos -= args.size - args_count;
    }
    if (e->code != SEPL_ERR_OK)
        return;
}

SEPL_LIB SeplValue sepl_mod_getexport(SeplModule *mod, SeplEnv env,
                                      const char *key) {
    sepl_size i;
    for (i = 0; i < mod->esize; i++) {
        const char *e_key = mod->exports[i];
        const char *k_key = key;
        char match = 1;

        while (*e_key && *k_key) {
            match = *e_key++ == *k_key++;
            if (!match)
                break;
        }
        if (match && *e_key == *k_key) {
            return mod->values[env.predef_len + i];
        }
    }

    return SEPL_NONE;
}


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

#endif
#endif