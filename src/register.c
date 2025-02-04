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
uintptr_t reg_alloc_tab[][2] = {
    {(uintptr_t) "%rsi", 0},
    {(uintptr_t) "%rdi", 0},
    {(uintptr_t) "%rdx", 0},
    {(uintptr_t) "%rcx", 0},
    {(uintptr_t)  "%r8", 0},
    {(uintptr_t)  "%r9", 0},
    {(uintptr_t) "%r10", 0},
    {(uintptr_t) "%r11", 0},
};

char *label_reg_tab[][2] = {
    {"%rsi", 0},
    {"%rdi", 0},
    {"%rdx", 0},
    {"%rcx", 0},
    { "%r8", 0},
    { "%r9", 0},
    {"%r10", 0},
    {"%r11", 0},
};

size_t bytes_rip_pad = 0; // for local variables
Function fn;
size_t fn_statement_num = 0;
size_t* **labels_as_offsets;

void reg_init_fn(Function func) {
    bytes_rip_pad = 0;
    for (size_t i = 0; i < sizeof(reg_alloc_tab) / sizeof(reg_alloc_tab[0]); i++)
        reg_alloc_tab[i][1] = 0;
    fn = func;
    labels_as_offsets = vec_new(sizeof(size_t) * 2);
}

char *reg_alloc(char *label) {
    for (size_t i = 0; i < sizeof(reg_alloc_tab) / sizeof(reg_alloc_tab[0]); i++) {
        if (!reg_alloc_tab[i][1]) {
            for (size_t s = fn_statement_num; s < fn.num_statements; s++) {
                // TODO/FIXME: This won't work with function call arguments.
                if ((fn.statements[s].val_types[0] == Label && strcmp((char*) fn.statements[s].vals[0], label)) || 
                        (fn.statements[s].val_types[1] == Label && strcmp((char*) fn.statements[s].vals[1], label)))
                    reg_alloc_tab[i][1]++;
            }
            label_reg_tab[i][1] = malloc(strlen(label) + 1);
            strcpy(label_reg_tab[i][1], label);
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
        if (!strcmp(label_reg_tab[i][1], label)) return label_reg_tab[i][0];
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
