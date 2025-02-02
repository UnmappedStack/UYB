#include <api.h>

void (*instructions[])(uint64_t, uint64_t) = {
    add_build, sub_build, div_build, mul_build,
    copy_build, ret_build,
};
