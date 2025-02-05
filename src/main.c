/* Main file of UYB for testing the library.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <stdio.h>
#include <vector.h>
#include <api.h>

int main() {
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
