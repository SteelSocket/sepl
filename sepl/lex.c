#include "lex.h"

#include "def.h"

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
