/* Individual instruction implementations for x86_64 target of UYB.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <api.h>
#include <stdio.h>
#include <register.h>

void build_value(ValType type, uint64_t val, String *fnbuf) {
    if (type == Number) string_push_fmt(fnbuf, "$%llu", val);
    if (type == Label) string_push_fmt(fnbuf, "%s", label_to_reg((char*) val));
}

void add_build(uint64_t vals[2], ValType types[2], String *fnbuf) {
    (void) vals;
}

void sub_build(uint64_t vals[2], ValType types[2], String *fnbuf) {
    (void) vals;
}

void div_build(uint64_t vals[2], ValType types[2], String *fnbuf) {
    (void) vals;
}

void mul_build(uint64_t vals[2], ValType types[2], String *fnbuf) {
    (void) vals;
}

void copy_build(uint64_t vals[2], ValType types[2], String *fnbuf) {
    string_push(fnbuf, "\tmov ");
    build_value(types[0], vals[0], fnbuf);
    string_push(fnbuf, ", %rax\n");
}

void ret_build(uint64_t vals[2], ValType types[2], String *fnbuf) {
    if (types[0] != Empty) {
        string_push(fnbuf, "\tmov ");
        build_value(types[0], vals[0], fnbuf);
        string_push(fnbuf, ", %rax\n");
    }
    string_push(fnbuf, "\tret\n");
}
