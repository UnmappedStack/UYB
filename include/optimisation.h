#pragma once
#include <api.h>
#include <stddef.h>

void optimise(Function *IR, size_t num_functions);

/* Specific optimisations */
void opt_fold(Function *IR, size_t num_functions);
