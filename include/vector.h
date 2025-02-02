/* Part of vector implementationf for UYB compiler backend project, see ../src/vector.c for the
 * rest of the code.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under the MPL2.0 license, see /LICENSE for more information. */
#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
    void *data;
    size_t len;
    size_t data_size;
} Vec;

Vec *vec_new(size_t data_size);

#define vec_push(vec, val) \
    do { \
        if (vec->data_size != sizeof(val)) { \
            printf("Data size of vector is not equal to val argument to vec_push() macro\n"); \
            exit(1); \
        } \
        ((typeof(val)*) vec->data)[vec->len] = val; \
        vec->len++; \
        vec->data = realloc(vec->data, (vec->len + 1) * sizeof(val)); \
    } while (0)


/* Usage of this header:
 *  - To create a new vector, use vec_new():
 *      Vec *vec = vec_new(sizeof(data_type));
 *    (replace `data_type` with the type that the vector is for, for example uint64_t)
 *  - To append an element to a vector, use vec_push():
 *      vec_push(vec, (data_type) new_value);
 *    (cast is done to ensure that it's the right type. If the wrong type is detected, then
 *    a runtime error will be thrown)
 *  - To access elements of the vector, including writing/reading specific elements, use the `data` field:
 *      ((data_type*) vec->data)[3] = 12;
 *      value = (data-type*)vec->data))[8];
 *  - To get the length of a vector, read the `len` field:
 *      length_of_vector = vec->len;
 */
