#pragma once
#include <api.h>

#define update_regalloc() fn_statement_num++

extern char *label_reg_tab[][2];
extern uintptr_t reg_alloc_tab[][2];
extern size_t fn_statement_num;
extern size_t bytes_rip_pad;
extern size_t* **labels_as_offsets;
void reg_init_fn(Function func);
char *reg_alloc(char *label);
char *label_to_reg(char *label);
