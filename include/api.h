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
    PHI,
    VASTART,
    VAARG,
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
    PhiArg,
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
    size_t alignment; // default is 1
} Global;

typedef struct {
    char *label; // to store result in (NULL if none (only if it's a function or something))
    Instruction instruction;
    Type type;
    uint64_t vals[3];
    ValType val_types[3];
} Statement;

typedef struct {
    bool type_is_struct;
    union {
        Type type;
        char *type_struct;
    };
    char *label;
} FunctionArgument;

typedef struct {
    bool is_global;
    char *name;
    FunctionArgument *args;
    size_t num_args;
    bool ret_is_struct;
    union {
        Type return_type;
        char *return_struct;
    };
    Statement *statements;
    size_t num_statements;
    bool is_variadic;
} Function;

typedef struct {
    char *name;
    size_t alignment; // default is size of largest value
    Type *types;
    size_t num_members;
} AggregateType;

typedef struct {
    char *blklbl_name;
    size_t val;
    ValType type;
} PhiVal;

// for each target
void build_program_x86_64(Function *IR, size_t num_functions, Global *global_vars, size_t num_global_vars, AggregateType *aggtypes, size_t num_aggtypes, FILE *outf);
void     build_program_IR(Function *IR, size_t num_functions, Global *global_vars, size_t num_global_vars, AggregateType *aggtypes, size_t num_aggtypes, FILE *outf);

extern void (*instructions_x86_64[39])(uint64_t[2], ValType[2], Statement, String*);
extern void (*instructions_IR[])(uint64_t[2], ValType[2], Statement, FILE*);
char *instruction_as_str(Instruction instr);
char *type_as_str(Type type);
void disasm_instr(String *fnbuf, Statement statement);
