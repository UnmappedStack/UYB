/* Header file for ../src/register.c, the register allocator for the UYB compiler backend.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#pragma once
#include <api.h>

#define update_regalloc() fn_statement_num++

static char *arg_regs[] = {
    "%rdi",
    "%rsi",
    "%rdx",
    "%rcx",
    "%r8",
    "%r9",
};

extern char* **used_regs_vec;
extern char *label_reg_tab[][3];
extern intptr_t reg_alloc_tab[][3];
extern size_t fn_statement_num;
extern size_t bytes_rip_pad;
extern size_t* **labels_as_offsets;
void reg_init_fn(Function func);
char *reg_alloc(char *label, Type reg_size);
char *label_to_reg(char *label, bool allow_noexist);
char *reg_as_size(char *reg, Type size);
Type size_from_reg(char *reg);
char *label_to_reg_noresize(char *label, bool allow_noexist);
char *reg_alloc_noresize(char *label, Type reg_size);
Type get_reg_size(char *reg, char *expected_label);
