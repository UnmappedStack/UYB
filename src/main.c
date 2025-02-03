/* Main file of UYB for testing the library.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <stdio.h>
#include <vector.h>
#include <api.h>

int main() {
    Function IR[] = {
        (Function) {
            .is_global = true,
            .name = "main",
            .args = (FunctionArgument[]) {
                (FunctionArgument) {
                    .type  = Bits64,
                    .label = "argc",
                },
                (FunctionArgument) {
                    .type  = Bits64,
                    .label = "argv",
                },
            },
            .num_args = 2,
            .return_type = Bits32,
            .statements = (Statement[]) {
                (Statement) {
                    .label = "r0",
                    .instruction = COPY,
                    .type = Bits64,
                    .vals = {3, 0},
                    .val_types = {Number, Empty},
                },
                (Statement) {
                    .label = "r1",
                    .instruction = COPY,
                    .type = Bits64,
                    .vals = {2, 0},
                    .val_types = {Number, Empty},
                },
                (Statement) {
                    .label = "sum",
                    .instruction = ADD,
                    .type = Bits64,
                    .vals = {(uint64_t) "r0", (uint64_t) "r1"},
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
            .num_statements = 4,
        },
    };
    FILE *f = fopen("out.S", "w");
    build_program(IR, sizeof(IR) / sizeof(IR[0]), f);
    fclose(f);
}
