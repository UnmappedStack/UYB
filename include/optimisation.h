#pragma once
#include <api.h>
#include <stddef.h>

typedef struct {
    char *label;
    size_t val;
    ValType type;
} CopyVal;

void optimise(Function *IR, size_t num_functions);

/* Specific optimisations */
void opt_fold(Function *IR, size_t num_functions);
void opt_copy_elim(Function *IR, size_t num_functions);
void opt_unused_label_elim(Function *IR, size_t num_functions);
