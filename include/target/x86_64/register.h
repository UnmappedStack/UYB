/* Header file for ../src/register.c, the register allocator for the UYB compiler backend.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#pragma once
#include <stdint.h>
#include <api.h>

#define update_regalloc() regalloc.statement_idx++

static char *arg_regs[] = {
    "%rdi",
    "%rsi",
    "%rdx",
    "%rcx",
    "%r8",
    "%r9",
};

typedef struct {
    size_t bytes_rip_pad;
    char* **used_regs_vec;
    Function *current_fn;
    size_t statement_idx;
    size_t* **labels_as_offsets;
} RegAlloc;

extern RegAlloc regalloc;

extern char *label_reg_tab[5][3];
extern intptr_t reg_alloc_tab[5][3];
void reg_init_fn(Function func);
char *reg_alloc(char *label, Type reg_size);
char *label_to_reg(size_t offset, char *label, bool allow_noexist);
char *reg_as_size(char *reg, Type size);
Type size_from_reg(char *reg);
char *label_to_reg_noresize(size_t offset, char *label, bool allow_noexist);
char *reg_alloc_noresize(char *label, Type reg_size);
Type get_reg_size(char *reg, char *expected_label);
