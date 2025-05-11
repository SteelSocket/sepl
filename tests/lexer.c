#include <assert.h>
#include <stdbool.h>
#include <stddef.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#include "tests.h"

SeplLexer lex;

#define next() (sepl_lex_next(&lex))
#define peek() (sepl_lex_peek(lex))
#define curr() (lex.current)
#define first(src) (lex = sepl_lex_init(src), next())

void check_eof() {
    lex = sepl_lex_init("");
    assert(next().type == SEPL_TOK_EOF);
    assert(peek().type == SEPL_TOK_EOF);
    assert(curr().type == SEPL_TOK_EOF);
}

void check_toks() {
    assert(first("").type == SEPL_TOK_EOF);
    assert(first("&").type == SEPL_TOK_ERROR);
    assert(first(";").type == SEPL_TOK_SEMICOLON);
    assert(first(",").type == SEPL_TOK_COMMA);

    assert(first("10.32").type == SEPL_TOK_NUM);
    assert(first("@").type == SEPL_TOK_VAR);
    assert(first("$").type == SEPL_TOK_FUNC);
    assert(first("NONE").type == SEPL_TOK_NONE);
    assert(first("\"hello\"").type == SEPL_TOK_STRING);

    assert(first("=").type == SEPL_TOK_ASSIGN);

    assert(first("+").type == SEPL_TOK_ADD);
    assert(first("-").type == SEPL_TOK_SUB);
    assert(first("*").type == SEPL_TOK_MUL);
    assert(first("/").type == SEPL_TOK_DIV);

    assert(first("!").type == SEPL_TOK_NOT);
    assert(first("&&").type == SEPL_TOK_AND);
    assert(first("||").type == SEPL_TOK_OR);

    assert(first("<").type == SEPL_TOK_LT);
    assert(first("<=").type == SEPL_TOK_LTE);
    assert(first(">").type == SEPL_TOK_GT);
    assert(first(">=").type == SEPL_TOK_GTE);
    assert(first("==").type == SEPL_TOK_EQ);
    assert(first("!=").type == SEPL_TOK_NEQ);

    assert(first("(").type == SEPL_TOK_LPAREN);
    assert(first(")").type == SEPL_TOK_RPAREN);
    assert(first("{").type == SEPL_TOK_LCURLY);
    assert(first("}").type == SEPL_TOK_RCURLY);

    assert(first("hello").type == SEPL_TOK_IDENTIFIER);
    assert(first("return").type == SEPL_TOK_RETURN);
    assert(first("if").type == SEPL_TOK_IF);
    assert(first("else").type == SEPL_TOK_ELSE);

    assert(first("\b").type == SEPL_TOK_ERROR);
}

void check_num() {
    assert(first("1a3").type == SEPL_TOK_ERROR);
    assert(first("10.a").type == SEPL_TOK_ERROR);
    assert(first("10.1.").type == SEPL_TOK_ERROR);
    assert(first(".12").type == SEPL_TOK_ERROR);

    assert(sepl_lex_num(first("10")) == 10.0);
    assert(sepl_lex_num(first("3.14")) == 3.14);
    assert(
        sepl_lex_num(first(
            "1797693134862316300000000000000000000000000000000000000000000000"
            "0000000000000000000000000000000000000000000000000000000000000000"
            "0000000000000000000000000000000000000000000000000000000000000000"
            "0000000000000000000000000000000000000000000000000000000000000000"
            "00000000000000000000000000000000000000000000000000000")) ==
        1.0 / 0.0);
}

int cmp_strtok_(const char *s1, const char *s2) {
    SeplToken tok = first(s1);
    const char *start = tok.start + 1;

    while (start != tok.end && *s2 != '\0') {
        if (*start == '\\')
            start++;
        if (*start++ != *s2++)
            return 0;
    }

    return *start == '"' && *s2 == '\0';
}
#define cmp_strtok(expr, str) (cmp_strtok_(#expr, str))

void check_string() {
    assert(first("\"\"").type == SEPL_TOK_STRING);
    assert(first("\"return\"").type == SEPL_TOK_STRING);
    assert(first("\"@hello = 302;\"").type == SEPL_TOK_STRING);

    assert(first("\"hello").type == SEPL_TOK_ERROR);

    assert(cmp_strtok("hello", "hello"));
    assert(cmp_strtok("\"hello\"", "\"hello\""));

    // The lexer does not convert "\\b" into "\b"
    // The special character convertion is done at the
    // compilation stage
    assert(!cmp_strtok("\b", "\b"));
    assert(!cmp_strtok("\n", "\n"));
    assert(!cmp_strtok("\r", "\r"));
    assert(!cmp_strtok("\a", "\a"));
}

void check_identifier() {
    assert(first("retur").type == SEPL_TOK_IDENTIFIER);
    assert(first("returnq").type == SEPL_TOK_IDENTIFIER);
    assert(first("retur 1").type == SEPL_TOK_IDENTIFIER);
    assert(first("returnq 1").type == SEPL_TOK_IDENTIFIER);
}

SEPL_TEST_GROUP(check_eof, check_toks, check_num, check_string,
                check_identifier);
