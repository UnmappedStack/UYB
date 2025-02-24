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
    JZ,
    NEG,
    UDIV,
    REM,
    UREM,
    AND,
    OR,
    XOR,
    SHL,
    SHR,
    STORE,
    LOAD,
    BLIT,
    ALLOC,
    EQ,
    NE,
    SLE, // less than or equal (signed)
    SLT, // less than (signed)
    SGE, // higher than or equal (signed)
    SGT, // higher than (signed)
    ULE, // less than or equal (unsigned)
    ULT, // less than (unsigned)
    UGE, // higher than or equal (unsigned)
    UGT, // higher than (unsigned)
    EXT,
    HLT,
    BLKLBL,
    JMP,
    JNZ,
} Instruction;

typedef enum {
    Bits8,
    Bits16,
    Bits32,
    Bits64,
    None,
} Type;

typedef enum {
    Label,
    Number,
    Str,
    StrLit,
    FunctionArgs,
    BlkLbl,
    Empty,
} ValType;

typedef struct {
    char **args;
    Type *arg_sizes;
    ValType *arg_types;
    size_t num_args;
} FunctionArgList;

typedef struct {
    char *section; // NULL if in data section
    char *name;
    ValType *types; // Can only be StrLit or number. Anything else should panic.
    Type *sizes;
    size_t *vals;
    size_t num_vals;
} Global;

typedef struct {
    char *label; // to store result in (NULL if none (only if it's a function or something))
    Instruction instruction;
    Type type;
    uint64_t vals[3];
    ValType val_types[3];
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

void build_program(Function *IR, size_t num_functions, Global *global_vars, size_t num_global_vars, FILE *outf);

extern void (*instructions[36])(uint64_t[2], ValType[2], Statement, String*);
char *instruction_as_str(Instruction instr);
char *type_as_str(Type type);
void disasm_instr(String *fnbuf, Statement statement);
