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

#endif
#endif