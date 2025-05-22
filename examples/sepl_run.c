#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define SEPL_IMPLEMENTATION
#include "../sepl.h"

#define MAX_BC_BUF 1024 * 10
#define MAX_VAL_BUF 200

#define array_len(sarray) sizeof(sarray) / sizeof(sarray[0])

void print_help() {
    printf("A simple sepl interpreter with input output support\n");
    printf("sepl_run: [file]\n");
}

void print_tok(SeplTokenT tok) {
    switch (tok) {
        case SEPL_TOK_SEMICOLON:
            printf(";\n");
            break;
        case SEPL_TOK_COMMA:
            printf(",\n");
            break;

        case SEPL_TOK_NUM:
            printf("{number}");
            break;
        case SEPL_TOK_VAR:
            printf("@\n");
            break;
        case SEPL_TOK_ASSIGN:
            printf("=\n");
            break;

        case SEPL_TOK_ADD:
            printf("+\n");
            break;
        case SEPL_TOK_SUB:
            printf("-\n");
            break;
        case SEPL_TOK_MUL:
            printf("*\n");
            break;
        case SEPL_TOK_DIV:
            printf("/\n");
            break;

        case SEPL_TOK_NOT:
            printf("!\n");
            break;
        case SEPL_TOK_AND:
            printf("&&\n");
            break;
        case SEPL_TOK_OR:
            printf("||\n");
            break;

        case SEPL_TOK_LT:
            printf("<\n");
            break;
        case SEPL_TOK_LTE:
            printf("<=\n");
            break;
        case SEPL_TOK_GT:
            printf(">\n");
            break;
        case SEPL_TOK_GTE:
            printf(">=\n");
            break;
        case SEPL_TOK_EQ:
            printf("==\n");
            break;
        case SEPL_TOK_NEQ:
            printf("!=\n");
            break;

        case SEPL_TOK_LPAREN:
            printf("(\n");
            break;
        case SEPL_TOK_RPAREN:
            printf(")\n");
            break;
        case SEPL_TOK_LCURLY:
            printf("{\n");
            break;
        case SEPL_TOK_RCURLY:
            printf("}\n");
            break;

        case SEPL_TOK_IDENTIFIER:
            printf("{identifier}\n");
            break;
        case SEPL_TOK_RETURN:
            printf("return\n");
            break;
        case SEPL_TOK_IF:
            printf("if\n");
            break;
        case SEPL_TOK_ELSE:
            printf("else\n");
            break;
        case SEPL_TOK_WHILE:
            printf("while\n");
            break;

        case SEPL_TOK_EOF:
            printf("END OF FILE\n");
            break;

        case SEPL_TOK_FUNC:
            printf("$\n");
            break;

        case SEPL_TOK_NONE:
            printf("NONE\n");
            break;

        default:
            break;
    }
}

void print_error(SeplError error) {
    printf("At line %ld\n\t", error.line);

    switch (error.code) {
        case SEPL_ERR_OK:
            break;

        case SEPL_ERR_SYN: {
            const char *s = error.info.tokd.up.start;
            const char *e = error.info.tokd.up.end;
            printf("Syntax error: %.*s\n", (int)(e - s), s);
            break;
        }
        case SEPL_ERR_OPER:
            printf("Operation not supported on given value\n");
            break;
        case SEPL_ERR_VOVERFLOW:
            printf("Value buffer overflow\n");
            break;
        case SEPL_ERR_VUNDERFLOW:
            printf("Value buffer underflow\n");
            break;
        case SEPL_ERR_BOVERFLOW:
            printf("Bytecode buffer overflow\n");
            break;
        case SEPL_ERR_UPTOK:
            printf("Unexpected token: ");
            print_tok(error.info.tokd.up.type);
            printf("Expected token: ");
            print_tok(error.info.tokd.exp);
            break;
        case SEPL_ERR_BC:
            printf("Unexpected bytecode: %d\n", error.info.bc);
            break;
        case SEPL_ERR_EEXPR:
            printf("Expected expression\n");
            break;
        case SEPL_ERR_IDEN_NDEF:
            printf("Identifier not defined\n");
            break;
        case SEPL_ERR_IDEN_RDEF:
            printf("Identifier redefined\n");
            break;
        case SEPL_ERR_PREDEF:
            printf("Attempting to set predefined variable\n");
            break;

        case SEPL_ERR_CLOSURE:
            printf("Closure not supported \n");
            break;

        case SEPL_ERR_FUNC_UPV:
            printf("Attempting to set upvalue from function\n");
            break;

        case SEPL_ERR_FUNC_RET:
            printf("Attempting to return a sepl function\n");
            break;

        case SEPL_ERR_FUNC_CALL:
            printf("Attempting to call a non function\n");
            break;

        case SEPL_ERR_REFMOVE:
            printf("Attempting to set a reference to a variable\n");
            break;
    }
}

char *file_read_all(const char *file_path) {
    FILE *f = fopen(file_path, "r");
    if (!f)
        return NULL;

    fseek(f, 0, SEEK_END);
    size_t size = ftell(f);
    rewind(f);

    char *buf = (char *)malloc(size + 1);
    if (!buf) {
        fclose(f);
        return NULL;
    }
    fread(buf, 1, size, f);
    buf[size] = '\0';
    fclose(f);
    return buf;
}

void assert_error(SeplError error, const char *msg) {
    if (error.code == SEPL_ERR_OK)
        return;
    printf("%s", msg);
    print_error(error);
    exit(1);
}

// Sepl Global Values

SeplValue gv_print(SeplArgs args, SeplError *_) {
    for (sepl_size i = 0; i < args.size; i++) {
        switch (args.values[i].type) {
            case SEPL_VAL_NONE:
                printf("NONE ");
                break;
            case SEPL_VAL_NUM:
                printf("%lf ", args.values[i].as.num);
                break;
            case SEPL_VAL_STR:
                printf("%s ", (const char *)args.values[i].as.obj);
                break;
            default:
                printf("%p ", args.values[i].as.obj);
                break;
        }
    }
    printf("\n");
    return SEPL_NONE;
}
SeplValue gv_input(SeplArgs args, SeplError *_) {
    double d;
    scanf("%lf", &d);
    return sepl_val_number(d);
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        print_help();
        return 0;
    }

    char *contents = file_read_all(argv[1]);
    if (contents == NULL) {
        printf("Failed to read the file \"%s\"\n", argv[1]);
        return 1;
    }

    // Setup the module
    unsigned char bc_buf[MAX_BC_BUF];
    SeplValue val_buf[MAX_VAL_BUF];
    SeplModule module = sepl_mod_new(bc_buf, MAX_BC_BUF, val_buf, MAX_VAL_BUF);

    SeplValuePair globals[] = {{"print", sepl_val_cfunc(gv_print)},
                               {"input", sepl_val_cfunc(gv_input)}};
    SeplEnv env = {NULL, globals, array_len(globals)};

    const char *exports[] = {"main"};
    module.exports = exports;
    module.esize = array_len(exports);

    // Compile the file as a module
    SeplError err = sepl_com_module(contents, &module, env);
    free(contents);
    assert_error(err, "sepl compilation error: ");

    // Execute the module to set up the main function
    sepl_mod_init(&module, &err, env);
    sepl_mod_exec(&module, &err, env);
    assert_error(err, "sepl runtime error: ");

    // Get the main function
    SeplValue main_func = sepl_mod_getexport(&module, env, "main");
    if (!sepl_val_isfun(main_func)) {
        printf("sepl error: No main function found\n");
        return 1;
    }

    // Pass the argv to sepl main
    // Unfortunatly sepl does not support variadic arguments
    // So cli tools using n arguments are not supported
    SeplArgs args;
    args.values = (SeplValue *)malloc(sizeof(SeplValue) * (argc + 1));
    args.size = argc + 1;
    args.values[0] = sepl_val_number(argc);
    for (int i=1; i < args.size; i++) {
        args.values[i] = sepl_val_str(argv[i - 1]);
    }

    // Load the main function into the module to be executed
    sepl_mod_initfunc(&module, &err, main_func, args);
    free(args.values);
    
    // Execute the main function
    SeplValue retv = sepl_mod_exec(&module, &err, env);
    assert_error(err, "sepl runtime error: ");

    // Cleanup any global variables
    sepl_mod_cleanup(&module, env);

    return (int)retv.as.num;
}
