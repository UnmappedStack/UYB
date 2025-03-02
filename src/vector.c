/* Part of vector implementationf for UYB compiler backend project, see ../include/vector.h for the
 * rest of the code and an explanation on how to use the full thing.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under the MPL2.0 license, see /LICENSE for more information. */
#include <vector.h>

void *vec_new(size_t data_size) {
    Vec *vec = (Vec*) malloc(sizeof(Vec));
    *vec = (Vec) {
        .len = 0,
        .capacity = 1,
        .data_size = data_size,
        .data = (uint8_t*) malloc(data_size),
    };
    return &vec->data;
}

size_t vec_size(void *vec_data) {
    Vec *vec = (Vec*) ((uint64_t) vec_data - (sizeof(Vec) - sizeof(void*)));
    return vec->len;
}

int vec_contains(void *vec_data, size_t val) {
    Vec *vec = (Vec*) ((uint64_t) vec_data - (sizeof(Vec) - sizeof(void*)));
    for (size_t i = 0; i < vec->len; i++) {
        size_t masked_vec = (*((size_t**) vec_data))[i] & ((1ULL << (8 * vec->data_size)) - 1);
        size_t masked_val = val & ((1ULL << (8 * vec->data_size)) - 1);
        if (masked_vec == masked_val) return 1;
    }
    return 0;
}
