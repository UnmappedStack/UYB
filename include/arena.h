#pragma once
#include <stddef.h>

#define aalloc arena_alloc
#define ARENA_MULTIPLIER 50

typedef struct {
    void *data;
    size_t upto;
    size_t len;
} Arena;

void init_arena();
void *arena_alloc(size_t len);
