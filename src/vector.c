/* Part of vector implementationf for UYB compiler backend project, see ../include/vector.h for the
 * rest of the code and an explanation on how to use the full thing.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under the MPL2.0 license, see /LICENSE for more information. */
#include <vector.h>

Vec *vec_new(size_t data_size) {
    Vec *vec = (Vec*) malloc(sizeof(Vec));
    *vec = (Vec) {
        .data = (uint8_t*) malloc(data_size),
        .len = 0,
        .data_size = data_size,
    };
    return vec;
}
