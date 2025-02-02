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
                    .label = NULL, // it doesn't save the result in any label
                    .instruction = RET,
                    .type = None, // type not specified since it's not saving a value in a label
                },
            },
            .num_statements = 0,
        },
    };
    (void) IR;
}
