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
    printf("size = %zu\n", vec_size(functs));
    build_program(*functs, vec_size(functs), globals, sizeof(globals) / sizeof(globals[0]), outf);
    fclose(outf);
    return 0;
    /* ### CODEGEN TEST ### */
    // Define globals
    // Define Function list
    FunctionArgList *argument_vals = malloc(sizeof(FunctionArgList));
    char *args[] = {"msg"};
    *argument_vals = (FunctionArgList) {
        .args = args,
        .num_args = 1,
    };
    Function IR[] = {
        (Function) {
            .is_global = true,
            .name = "main",
            .num_args = 0,
            .return_type = Bits64,
            .statements = (Statement[]) {
                (Statement) {
                    .label = "val1",
                    .instruction = COPY,
                    .type = Bits32,
                    .vals = {4},
                    .val_types = {Number, Empty, Empty},
                },
                (Statement) {
                    .label = "val2",
                    .instruction = COPY,
                    .type = Bits32,
                    .vals = {2},
                    .val_types = {Number, Empty, Empty},
                },
                (Statement) {
                    .label = "ret",
                    .instruction = DIV,
                    .type = Bits32,
                    .vals = {(uint64_t) "val1", (uint64_t) "val2"},
                    .val_types = {Label, Label, Empty},
                },
                (Statement) {
                    .label = "ret",
                    .instruction = EXT,
                    .type = Bits64,
                    .vals = {(uint64_t) "ret", false},
                    .val_types = {Label, Number, Empty},
                },
                (Statement) {
                    .label = NULL, // it doesn't save the result in any label
                    .instruction = RET,
                    .type = None, // type not specified since it's not saving a value in a label
                    .vals = {(uint64_t) "ret.1", 0},
                    .val_types = {Label, Empty, Empty},
                },
            },
            .num_statements = 5,
        },
    };
    FILE *f = fopen("out.S", "w");
    build_program(IR, sizeof(IR) / sizeof(IR[0]), globals, sizeof(globals) / sizeof(globals[0]), f);
    fclose(f);
}
