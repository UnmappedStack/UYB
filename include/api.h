/* This is the "library"/API file which is what is used to interact with the actual backend through
 * a non-textual representation.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <strslice.h>
#include <stdio.h>

typedef enum {
    ADD,
    SUB,
    DIV,
    MUL,
    COPY,
    RET,
    CALL,
} Instruction;

typedef enum {
    Bits8,
    Bits16,
    Bits32,
    Bits64,
    None,
} Type;

typedef struct {
    char **args;
    size_t num_args;
} FunctionArgList;

typedef enum {
    Label,
    Number,
    Str,
    FunctionArgs,
    Empty,
} ValType;

typedef struct {
    char *label; // to store result in (NULL if none (only if it's a function or something))
    Instruction instruction;
    Type type;
    uint64_t vals[2];
    ValType val_types[2];
} Statement;

typedef struct {
    Type type;
    char *label;
} FunctionArgument;

typedef struct {
    bool is_global;
    char *name;
    FunctionArgument *args;
    size_t num_args;
    Type return_type;
    Statement *statements;
    size_t num_statements;
} Function;

void build_program(Function *IR, size_t num_functions, FILE *outf);

void  add_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void  sub_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void  div_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void  mul_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void copy_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void  ret_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void call_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);

char *instruction_as_str(Instruction instr);
char *type_as_str(Type type);
void disasm_instr(String *fnbuf, Statement statement);
