#include <optimisation.h>
#include <string.h>
#include <vector.h>

typedef struct {
    char *label;
    size_t val;
} CopyVal;

/* returns 1 or 0 depending on if it was found. if it was found it stores the result in val_buf unless
 * val_buf is null */
int find_val(CopyVal **copyvals, char *label, size_t *val_buf) {
    for (size_t i = 0; i < vec_size(copyvals); i++) {
        if (!strcmp((*copyvals)[i].label, label)) {
            if (val_buf)
                *val_buf = (*copyvals)[i].val;
            return 1;
        }
    }
    return 0;
}

size_t get_val(ValType type, size_t val, size_t label_val) {
    if      (type == Number) return val;
    else if (type == Label) return label_val;
    else return 0;
}

void fold_funct(Function *fn) {
    CopyVal **copyvals = vec_new(sizeof(CopyVal));
    for (size_t s = 0; s < fn->num_statements; s++) {
        ValType *valtypes = fn->statements[s].val_types;
        size_t *vals = fn->statements[s].vals;
        Instruction instr = fn->statements[s].instruction;
        // If it's a COPY, save the value
        if (instr == COPY && valtypes[0] == Number) {
            vec_push(copyvals, ((CopyVal) {
                .label = fn->statements[s].label,
                .val = fn->statements[s].vals[0],
            }));
            continue;
        }
        size_t in_vals[2];
        if ((valtypes[0] == Str || valtypes[0] == BlkLbl || (valtypes[0] != Number && !(valtypes[0] == Label && find_val(copyvals, (char*) vals[0], &in_vals[0]))) ||
             valtypes[1] == Str || valtypes[1] == BlkLbl || (valtypes[1] != Number && !(valtypes[1] == Label && find_val(copyvals, (char*) vals[1], &in_vals[1])))) && valtypes[1] != Empty) {
            // it can't constant fold it if the values can't be found at compile time
            continue;
        }
        // Now solve for the value and replace it with a COPY.
        size_t params[] = {get_val(valtypes[0], vals[0], in_vals[0]), get_val(valtypes[1], vals[1], in_vals[1])};
        if (instr == ADD) {
            fn->statements[s].vals[0] = params[0] + params[1];
        } else if (instr == MUL) {
            fn->statements[s].vals[0] = params[0] * params[1];
        } else if (instr == DIV) {
            fn->statements[s].vals[0] = params[0] / params[1];
        } else if (instr == SUB) {
            fn->statements[s].vals[0] = params[0] - params[1];
        } else if (instr == SHL) {
            fn->statements[s].vals[0] = params[0] << params[1];
        } else if (instr == SHR) {
            fn->statements[s].vals[0] = params[0] >> params[1];
        } else if (instr == EQ) {
            fn->statements[s].vals[0] = params[0] == params[1];
        } else if (instr == NE) {
            fn->statements[s].vals[0] = params[0] != params[1];
        } else if (instr == OR) {
            fn->statements[s].vals[0] = params[0] | params[1];
        } else if (instr == AND) {
            fn->statements[s].vals[0] = params[0] & params[1];
        } else if (instr == XOR) {
            fn->statements[s].vals[0] = params[0] ^ params[1];
        } else if (instr == NEG) {
            fn->statements[s].vals[0] = -params[0];
        } else {
            return;
        }
        fn->statements[s].instruction = COPY;
        fn->statements[s].val_types[0] = Number;
        fn->statements[s].val_types[1] = Empty;
    }
}

void opt_fold(Function *IR, size_t num_functions) {
    for (size_t fn = 0; fn < num_functions; fn++) {
        fold_funct(&IR[fn]);
    }
}
