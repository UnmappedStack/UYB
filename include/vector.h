/* Part of vector implementationf for UYB compiler backend project, see ../src/vector.c for the
 * rest of the code.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under the MPL2.0 license, see /LICENSE for more information. */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    size_t len;
    size_t capacity;
    size_t data_size;
    void *data;
} __attribute__((packed)) Vec;

void *vec_new(size_t data_size);
size_t vec_size(void *vec_data);

#define vec_push(vec_data, val) \
    do { \
        Vec *vec_internal = (Vec*) ((uintptr_t) vec_data - (sizeof(Vec) - sizeof(void*))); \
        ((typeof(val)*) vec_internal->data)[vec_internal->len] = val; \
        vec_internal->len++; \
        if (vec_internal->capacity == vec_internal->len) { \
            vec_internal->data = realloc(vec_internal->data, (vec_internal->len + 1) * sizeof(val) * 2); \
            vec_internal->capacity *= 2; \
        } \
    } while (0)

/* Usage of this header:
 *  - To create a new vector, use vec_new():
 *      data_type **vec = vec_new(sizeof(data_type));
 *    (replace `data_type` with the type that the vector is for, for example uint64_t)
 *  - To append an element to a vector, use vec_push():
 *      vec_push(vec, new_value);
 *  - To access elements of the vector, including writing/reading specific elements, access it like a normal array but dereference vec:
 *      value = (*vec)[8];
 *  - To get the length of a vector, use vec_size():
 *      length_of_vector = vec_size(vec);
 */
