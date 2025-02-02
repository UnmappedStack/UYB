#include <stdio.h>
#include <vector.h>

int main() {
    // Vector test
    Vec *vec = vec_new(sizeof(int));
    vec_push(vec, (int) 8);
    vec_push(vec, (int) 12);
    printf("len: %zu\n", vec->len);
    printf("first = %i, second = %i\n", ((int*) vec->data)[0], ((int*) vec->data)[1]);
}
