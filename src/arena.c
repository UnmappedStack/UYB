#include <arena.h>
#include <string.h>
#include <vector.h>

Arena **arena_vec;

void init_arena() {
    arena_vec = vec_new(sizeof(Arena));
}

void *arena_alloc(size_t len) {
    return malloc(len); // TODO: remove this and fix the arena allocator
    size_t sz = vec_size(arena_vec);
    if (!sz || (*arena_vec)[sz - 1].upto + len >= (*arena_vec)[sz - 1].len) {
        Arena new = {0};
        new.data = malloc(len * ARENA_MULTIPLIER);
        new.len = len * ARENA_MULTIPLIER;
        vec_push(arena_vec, new);
        memset(new.data, 0, len);
        return new.data;
    } else {
        void *ret = ((*arena_vec)[sz - 1].data) + (*arena_vec)[sz - 1].upto;
        (*arena_vec)[sz - 1].upto += len + 1;
        memset(ret, 0, len);
        return ret;
    }
}

void delete_arenas() {
    size_t sz = vec_size(arena_vec);
    for (size_t i = 0; i < sz; i++) {
        free((*arena_vec)[i].data);
    }
}
