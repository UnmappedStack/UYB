/* Individual instruction implementations for x86_64 target of UYB.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <api.h>
#include <stdio.h>
#include <register.h>
#include <strslice.h>

char *instruction_as_str(Instruction instr) {
    if      (instr == ADD ) return "ADD";
    else if (instr == SUB ) return "SUB";
    else if (instr == DIV ) return "DIV";
    else if (instr == MUL ) return "MUL";
    else if (instr == COPY) return "COPY";
    else if (instr == RET ) return "RET";
    else return "Unknown instruction";
}

static void print_val(String *fnbuf, uint64_t val, ValType type) {
    if      (type == Number) string_push_fmt(fnbuf, "$%llu", val);
    else if (type == Label ) string_push_fmt(fnbuf, "%%%s", (char*) val);
}

void disasm_instr(String *fnbuf, Statement statement) {
    string_push(fnbuf, "\t// ");
    if (statement.label) {
        string_push_fmt(fnbuf, "%%%s =%s ", statement.label, type_as_str(statement.type));
    }
    string_push_fmt(fnbuf, "%s ", instruction_as_str(statement.instruction));
    if (statement.val_types[0] != Empty) print_val(fnbuf, statement.vals[0], statement.val_types[0]);
    if (statement.val_types[1] != Empty) {
        string_push(fnbuf, ", ");
        print_val(fnbuf, statement.vals[1], statement.val_types[1]);
    }
    string_push(fnbuf, "\n");
}

void build_value(ValType type, uint64_t val, String *fnbuf) {
    if (type == Number) string_push_fmt(fnbuf, "$%llu", val);
    if (type == Label) string_push_fmt(fnbuf, "%s", label_to_reg((char*) val));
}

void add_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    char *label_loc = reg_alloc(statement.label);
    if (label_loc[0] != '%') { // label stored in memory address on stack
        string_push(fnbuf, "\tmov ");
        build_value(types[0], vals[0], fnbuf);
        string_push(fnbuf, ", %rax\n");
        string_push(fnbuf, "\tadd ");
        build_value(types[1], vals[1], fnbuf);
        string_push(fnbuf, ", %rax\n");
        string_push_fmt(fnbuf, "\tmov %rax, %s\n", label_loc);
    } else { // stored in register
        string_push(fnbuf, "\tmov ");
        build_value(types[0], vals[0], fnbuf);
        string_push_fmt(fnbuf, ", %s\n", label_loc);
        string_push(fnbuf, "\tadd ");
        build_value(types[1], vals[1], fnbuf);
        string_push_fmt(fnbuf, ", %s\n", label_loc);
    }
}

void sub_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    (void) vals;
}

void div_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    (void) vals;
}

void mul_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    (void) vals;
}

void copy_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    char *label_loc = reg_alloc(statement.label);
    string_push(fnbuf, "\tmov ");
    build_value(types[0], vals[0], fnbuf);
    if (label_loc[0] == '%') // stored in reg
        string_push_fmt(fnbuf, ", %s\n", label_loc);
    else { // stored in memory
        string_push_fmt(fnbuf, "%%rax, %s\n", label_loc);
        string_push_fmt(fnbuf, "\tmov %rax, %s\n", label_loc);
    }
}

void ret_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    if (types[0] != Empty) {
        string_push(fnbuf, "\tmov ");
        build_value(types[0], vals[0], fnbuf);
        string_push(fnbuf, ", %rax\n");
    }
    string_push_fmt(fnbuf, "\tpop %rbp\n\tadd $%zu, %rsp\n\tret\n", bytes_rip_pad);
}
