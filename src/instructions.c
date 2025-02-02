/* Individual instruction implementations for x86_64 target of UYB.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <api.h>
#include <stdio.h>

void add_build(uint64_t val1, uint64_t val2, String *fnbuf) {
    (void) val1;
    (void) val2;
}

void sub_build(uint64_t val1, uint64_t val2, String *fnbuf) {
    (void) val1;
    (void) val2;
}

void div_build(uint64_t val1, uint64_t val2, String *fnbuf) {
    (void) val1;
    (void) val2;
}

void mul_build(uint64_t val1, uint64_t val2, String *fnbuf) {
    (void) val1;
    (void) val2;
}

void copy_build(uint64_t val1, uint64_t val2, String *fnbuf) {
    (void) val1;
    (void) val2;
}

void ret_build(uint64_t val1, uint64_t val2, String *fnbuf) {
    (void) val1;
    (void) val2;
    string_push(fnbuf, "\tret\n");
}
