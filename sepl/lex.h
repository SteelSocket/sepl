#ifndef SEPL_LEXER
#define SEPL_LEXER

#include "def.h"

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

#endif
