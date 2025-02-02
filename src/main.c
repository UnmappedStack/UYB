#include <stdio.h>
#include <vector.h>

int main() {
    // Vector test
    int **vec = vec_new(sizeof(int));
    vec_push(vec, 8);
    vec_push(vec, 2);
    printf("len: %zu\n", vec_size(vec));
    printf("first = %i, second = %i\n", (*vec)[0], (*vec)[1]);
}
