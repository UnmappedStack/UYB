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
    Type *arg_sizes;
    size_t num_args;
} FunctionArgList;

typedef enum {
    Label,
    Number,
    Str,
    StrLit,
    FunctionArgs,
    Empty,
} ValType;

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

void    add_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    sub_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    div_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    mul_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void   copy_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    ret_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void   call_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void     jz_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    neg_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void   udiv_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void   urem_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    rem_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void     or_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    xor_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    and_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    shl_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    shr_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void   load_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void  store_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void   blit_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void  alloc_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void     eq_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void     ne_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    sle_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    slt_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    sge_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    sgt_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    ule_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    ult_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    uge_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    ugt_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    ext_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void    hlt_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);
void blklbl_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf);

char *instruction_as_str(Instruction instr);
char *type_as_str(Type type);
void disasm_instr(String *fnbuf, Statement statement);
