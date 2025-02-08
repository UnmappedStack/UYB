/* Main file of UYB for testing the library.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <stdio.h>
#include <vector.h>
#include <api.h>

int main() {
    // Define globals
    Global globals[] = {
        {
            .name = "message",
            .type = StrLit,
            .val = (uint64_t) "Hello world from UYB, where there's always something in your backend!\\n"
        },
    };
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
                    .type = Bits64,
                    .vals = {5},
                    .val_types = {Number, Empty},
                },
                (Statement) {
                    .label = "val2",
                    .instruction = COPY,
                    .type = Bits64,
                    .vals = {2},
                    .val_types = {Number, Empty},
                },
                (Statement) {
                    .label = "ret",
                    .instruction = DIV,
                    .type = Bits64,
                    .vals = {(uint64_t) "val1", (uint64_t) "val2"},
                    .val_types = {Label, Label},
                },
                (Statement) {
                    .label = NULL, // it doesn't save the result in any label
                    .instruction = RET,
                    .type = None, // type not specified since it's not saving a value in a label
                    .vals = {(uint64_t) "ret", 0},
                    .val_types = {Label, Empty},
                },
            },
            .num_statements = 4,
        },
    };
    FILE *f = fopen("out.S", "w");
    build_program(IR, sizeof(IR) / sizeof(IR[0]), globals, sizeof(globals) / sizeof(globals[0]), f);
    fclose(f);
}
