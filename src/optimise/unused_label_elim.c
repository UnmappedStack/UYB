#include <optimisation.h>
#include <vector.h>

void elim_unused_labels_fn(Function *IR) {
    char* **used_labels = vec_new(sizeof(char*));
    Statement **statement_vec = vec_new(sizeof(Statement));
    for (ssize_t s = IR->num_statements - 1; s >= 0; s--) {
        if (IR->statements[s].label && IR->statements[s].instruction != CALL) {
            for (size_t i = 0; i < vec_size(used_labels); i++) {
                if (!vec_contains(used_labels, (size_t) IR->statements[s].label)) continue;
            }
        }
        if (IR->statements[s].instruction == CALL) {
            FunctionArgList *args = (FunctionArgList*) IR->statements[s].vals[1];
            for (size_t a = 0; a < args->num_args; a++) {
                if (args->arg_types[a] == Label) vec_push(used_labels, args->args[a]);
            }
        } else {
            for (size_t i = 0; i < 3; i++) {
                if (IR->statements[s].val_types[i] == Label) vec_push(used_labels, IR->statements[s].vals[i]);
            }
        }
        vec_push(statement_vec, IR->statements[s]);
    }
    // reverse it cos the previous thing inserts statements backwards
    for (size_t i = 0; i < vec_size(statement_vec) / 2; i++) {
        Statement tmp = (*statement_vec)[i];
        (*statement_vec)[i] = (*statement_vec)[vec_size(statement_vec) - 1 - i];
        (*statement_vec)[vec_size(statement_vec) - 1 - i] = tmp;
    }
    IR->statements = *statement_vec;
    IR->num_statements = vec_size(statement_vec);
}

void opt_unused_label_elim(Function *IR, size_t num_functions) {
    for (size_t fn = 0; fn < num_functions; fn++) {
        elim_unused_labels_fn(&IR[fn]);
    }
}
