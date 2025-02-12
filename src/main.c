/* Main file of UYB for testing the library.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <stdio.h>
#include <vector.h>
#include <api.h>
#include <lexer.h>
#include <parser.h>

Global globals[] = {
    {
        .name = "message",
        .type = StrLit,
        .val = (uint64_t) "Hello world from UYB, where there's always something in your backend!\\n"
    },
};

int main() {
    /* ### LEXER TEST ### */
    FILE *inf = fopen("test.ssa", "r");
    if (!inf) {
        printf("Failed to open file.\n");
        return 1;
    }
    Token **toks = lex_file(inf);
    fclose(inf);
    Function **functs = parse_program(toks);
    FILE *outf = fopen("out.S", "w");
    if (!outf) {
        printf("Failed to open out.S\n");
        exit(1);
    }
    printf("size = %zu\n", vec_size(functs));
    build_program(*functs, vec_size(functs), globals, sizeof(globals) / sizeof(globals[0]), outf);
    fclose(outf);
    return 0;
}
