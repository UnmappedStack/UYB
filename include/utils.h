#pragma once
#include <stdbool.h>
#include <api.h>
#include <optimisation.h>

char size_as_char(Type type);
char *get_full_char_str(bool is_struct, Type type, char *type_struct);
int find_copyval(CopyVal **copyvals, char *label, CopyVal *val_buf);
int find_sizet_in_copyvals(CopyVal **copyvals, char *label, size_t *val_buf);
AggregateType *find_aggtype(char *name, AggregateType *aggtypes, size_t num_aggtypes);
char *read_full_stdin();
