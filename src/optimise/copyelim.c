#include <optimisation.h>
#include <string.h>
#include <api.h>
#include <vector.h>
#include <utils.h>

void copy_elim_funct(Function *IR) {
    CopyVal val;
    Statement **statement_vec = vec_new(sizeof(Statement));
    CopyVal **copyvals = vec_new(sizeof(Statement));
    for (size_t s = 0; s < IR->num_statements; s++) {
        if (IR->statements[s].instruction == COPY) {
            vec_push(copyvals, ((CopyVal) {
                .label = IR->statements[s].label,
                .val = IR->statements[s].vals[0],
                .type = IR->statements[s].val_types[0],
            }));
        } else {
            if (IR->statements[s].instruction == CALL) {
                FunctionArgList *args = (FunctionArgList*) IR->statements[s].vals[1];
                for (size_t a = 0; a < args->num_args; a++) {
                    if (args->arg_types[a] != Label) continue;
                    if (!find_copyval(copyvals, (char*) args->args[a], &val)) continue;
                    args->args[a] = (char*) val.val;
                    args->arg_types[a] = val.type;
                }
                goto statement_end;
            } else if (IR->statements[s].instruction == ASM) {
                InlineAsm *info = (InlineAsm*) IR->statements[s].vals[0];
                for (size_t i = 0; i < vec_size(info->inputs_vec); i++) {
                    if (!find_copyval(copyvals, (char*) (*info->inputs_vec)[i].label, &val)) continue;
                    (*info->inputs_vec)[i].label = (char*) val.val;
                    (*info->inputs_vec)[i].type = val.type;
                }
            }
            for (size_t i = 0; i < 2; i++) {
                if (IR->statements[s].val_types[i] != Label) continue;
                if (!find_copyval(copyvals, (char*) IR->statements[s].vals[i], &val)) continue;
                IR->statements[s].val_types[i] = val.type;
                IR->statements[s].vals[i] = val.val;
            }
            statement_end:
            vec_push(statement_vec, IR->statements[s]);
        }
    }
    IR->statements = *statement_vec;
    IR->num_statements = vec_size(statement_vec);
}

void opt_copy_elim(Function *IR, size_t num_functions) {
    for (size_t fn = 0; fn < num_functions; fn++) {
        copy_elim_funct(&IR[fn]);
    }
}
