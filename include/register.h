#pragma once
#include <api.h>

#define update_regalloc() fn_statement_num++

extern size_t fn_statement_num;
void reg_init_fn(Function func);
char *reg_alloc(char *label);
char *label_to_reg(char *label);
