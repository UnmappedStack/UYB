/* Main file of UYB for testing the library.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <stdio.h>
#include <vector.h>
#include <api.h>

int main() {
    FunctionArgList *argument_vals = malloc(sizeof(FunctionArgList));
    char *args[2] = {"message", "sum"};
    *argument_vals = (FunctionArgList) {
        .args = args,
        .num_args = 2,
    };
    Function IR[] = {
        (Function) {
            .is_global = true,
            .name = "sum",
            .args = (FunctionArgument[]) {
                (FunctionArgument) {
                    .type  = Bits64,
                    .label = "x",
                },
                (FunctionArgument) {
                    .type  = Bits64,
                    .label = "y",
                },
            },
            .num_args = 2,
            .return_type = Bits64,
            .statements = (Statement[]) {
                (Statement) {
                    .label = NULL,
                    .instruction = CALL,
                    .type = None,
                    .vals = {(uint64_t) "printf", (uint64_t) argument_vals},
                    .val_types = {Str, FunctionArgs},
                },
                (Statement) {
                    .label = "sum",
                    .instruction = ADD,
                    .type = Bits64,
                    .vals = {(uint64_t) "x", (uint64_t) "y"},
                    .val_types = {Label, Label},
                },
                (Statement) {
                    .label = NULL, // it doesn't save the result in any label
                    .instruction = RET,
                    .type = None, // type not specified since it's not saving a value in a label
                    .vals = {(uint64_t) "sum", 0},
                    .val_types = {Label, Empty},
                },
            },
            .num_statements = 2,
        },
    };
    FILE *f = fopen("out.S", "w");
    build_program(IR, sizeof(IR) / sizeof(IR[0]), f);
    fclose(f);
}
