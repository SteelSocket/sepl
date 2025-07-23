#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef ROOT_DIR
#define ROOT_DIR "../"
#endif
#define SRC_DIR ROOT_DIR"/sepl/"

const char *header_files[] = {
    SRC_DIR "def.h",
    SRC_DIR "lex.h",
    SRC_DIR "err.h",
    SRC_DIR "val.h",
    SRC_DIR "env.h",
    SRC_DIR "mod.h",
};

const char *src_files[] = {
    SRC_DIR "lex.c",
    SRC_DIR "val.c",
    SRC_DIR "mod.c",
};

typedef struct {
    char *text;
    size_t len;
} Text;

void free_text(Text *text) {
    text->len = 0;
    free(text->text);
}

Text read_file(const char *path) {
    Text t = {0};

    FILE *f = fopen(path, "r");
    fseek(f, 0, SEEK_END);
    t.len = ftell(f);
    fseek(f, 0, SEEK_SET);

    t.text = malloc(sizeof(char) * t.len);
    size_t end = fread(t.text, sizeof(char), t.len, f);
    t.text[end] = '\0';

    fclose(f);
    return t;
}

void append_license(FILE *dst) {
    FILE *file = fopen("./LICENSE", "r");
    char line[1024];

    fprintf(dst, "/*\n");
    while (fgets(line, sizeof(line), file)) {
        fprintf(dst, " * %s", line);
    }
    fprintf(dst, " */\n\n");

    fclose(file);
}

void append_headers(FILE *dst) {

    fprintf(dst, "\n\n#ifdef __cplusplus\nextern \"C\" {\n#endif\n\n");

    size_t hfiles_len = sizeof(header_files)/sizeof(header_files[0]);
    for (int i=0; i < hfiles_len;i++) {
        FILE *file = fopen(header_files[i], "r");
        char line[1024];
        int new_lines = 0;

        fgets(line, sizeof(line), file);
        fgets(line, sizeof(line), file);
        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "#include \"", 10) == 0) {
                continue;
            }
            fprintf(dst, "%s", line);
        }
        fseek(dst, -8, SEEK_CUR);

        fclose(file);
    }

    fprintf(dst, "#ifdef __cplusplus\n}\n#endif\n\n");
    fflush(dst);
}

void append_sources(FILE *dst) {

    fprintf(dst, "#ifdef SEPL_IMPLEMENTATION\n\n");
    size_t hfiles_len = sizeof(src_files)/sizeof(src_files[0]);
    for (int i=0; i < hfiles_len;i++) {
        FILE *file = fopen(src_files[i], "r");
        char line[1024];

        while (fgets(line, sizeof(line), file)) {
            if (strncmp(line, "#include \"", 10) == 0) {
                continue;
            }
            fprintf(dst, "%s", line);
        }

        fclose(file);
    }
    fprintf(dst, "\n#endif");
    fflush(dst);
}

int main() {
    FILE *header = fopen(ROOT_DIR "sepl.h", "w");
    append_license(header);
    fprintf(header, "#ifndef SEPL_LIBRARY\n#define SEPL_LIBRARY\n");
    append_headers(header);
    append_sources(header);
    fprintf(header, "\n#endif");
    fclose(header);
    return 0;
}
