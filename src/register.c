/* Register allocator for UYB project.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <register.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector.h>

/* all the scratch registers:
 *  {reg_name, num_regs} 
 * num_refs is the number of references to the label corresponding to that register
 * *after* the current instruction. */
intptr_t reg_alloc_tab[][2] = {
    {(uintptr_t) "%rbx", 0},
    {(uintptr_t) "%r12", 0},
    {(uintptr_t) "%r13", 0},
    {(uintptr_t) "%r14", 0},
    {(uintptr_t)  "%15", 0},
};

// Left side is register, right side is assigned label
char *label_reg_tab[][2] = {
    {"%rbx", 0},
    {"%r12", 0},
    {"%r13", 0},
    {"%r14", 0},
    { "%15", 0},
};

size_t bytes_rip_pad = 0; // for local variables
char * **used_regs_vec;
Function fn;
size_t fn_statement_num = 0;
size_t* **labels_as_offsets;

void reg_init_fn(Function func) {
    bytes_rip_pad = 0;
    for (size_t i = 0; i < sizeof(reg_alloc_tab) / sizeof(reg_alloc_tab[0]); i++)
        reg_alloc_tab[i][1] = 0;
    fn = func;
    labels_as_offsets = vec_new(sizeof(size_t) * 2);
    used_regs_vec = vec_new(sizeof(char*));
}

// This is a lot of really bad indentation, FIXME/TODO
char *reg_alloc(char *label) {
    for (size_t i = 0; i < sizeof(reg_alloc_tab) / sizeof(reg_alloc_tab[0]); i++) {
        if (!reg_alloc_tab[i][1]) {
            for (size_t s = fn_statement_num; s < fn.num_statements; s++) {
                if (fn.statements[s].instruction == JZ) { // NOTE: All loop-based instructions must be added here
                    reg_alloc_tab[i][1] = -1;
                    break;
                }
                if (fn.statements[s].val_types[1] == FunctionArgs) {
                    for (size_t i = 0; i < ((FunctionArgList*) fn.statements[s].vals[1])->num_args; i++) {
                        if (!strcmp(label, ((FunctionArgList*) fn.statements[s].vals[1])->args[i]))
                            reg_alloc_tab[i][1] += 2;
                    }
                }
                if ((fn.statements[s].val_types[0] == Label && !strcmp((char*) fn.statements[s].vals[0], label)) || 
                        (fn.statements[s].val_types[1] == Label && !strcmp((char*) fn.statements[s].vals[1], label))) {
                    reg_alloc_tab[i][1]++;
                }
            }
            label_reg_tab[i][1] = malloc(strlen(label) + 1);
            strcpy(label_reg_tab[i][1], label);
            size_t used_sz = vec_size(used_regs_vec);
            bool do_push = true;
            for (size_t y = 0; y < used_sz; y++) {
                if (!strcmp((*used_regs_vec)[y], (char*) reg_alloc_tab[i][0])) {
                    do_push = false;
                    break;
                }
            }
            if (do_push)
                vec_push(used_regs_vec, (char*) reg_alloc_tab[i][0]);
            return (char*) reg_alloc_tab[i][0];
        }
    }
    char *fmt = "%llu(%%rbp)";
    size_t buf_sz = strlen("(%rbp)") + 5;
    char *buf = (char*) malloc(buf_sz + 1);
    snprintf(buf, buf_sz, fmt, bytes_rip_pad);
    size_t *new_vec_val = malloc(sizeof(size_t*));
    new_vec_val[0] = (size_t) label;
    new_vec_val[1] = bytes_rip_pad;
    bytes_rip_pad += 8;
    vec_push(labels_as_offsets, new_vec_val);
    return buf;
}

char *label_to_reg(char *label) {
    for (size_t i = 0; i < sizeof(label_reg_tab) / sizeof(label_reg_tab[1]); i++) {
        if (label_reg_tab[i][1] && !strcmp(label_reg_tab[i][1], label)) {
            reg_alloc_tab[i][1]--;
            if (!reg_alloc_tab[i][1])
                label_reg_tab[i][1] = 0;
            return label_reg_tab[i][0];
        }
    }
    size_t label_offset_list_len = vec_size(labels_as_offsets);
    for (size_t l = 0; l < label_offset_list_len; l++) {
        if (!strcmp((char*) (*labels_as_offsets)[l][0], label)) {
            char *fmt = "-%llu(%%rbp)";
            size_t buf_sz = strlen("-(%rbp)") + 5;
            char *buf = (char*) malloc(buf_sz + 1);
            snprintf(buf, buf_sz, fmt, (*labels_as_offsets)[l][1]);
            return buf;
        }
    }
    printf("Tried to use non-defined label: %s\n", label);
    exit(1);
}
