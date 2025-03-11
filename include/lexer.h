/* Header for ../src/lexer.h, the lexer for the textual IR for UYB.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <api.h>

typedef enum {
    TokLabel,   // %labelname
    TokRawStr,  // value
    TokStrLit,  // "value"
    TokBlockLabel,
    TokInteger,
    TokAssign, // =t (with t being the type, stored in val (types defined in api.h))
    TokEqu, // just =, no type
    TokLBrace, TokRBrace,
    TokLParen, TokRParen,
    TokComma,
    TokNewLine,
    TokTripleDot,
    TokAggType,
    TokFunction, TokExport, TokData, TokSection, TokAlign, TokType, TokFile, // keywords
} TokenType;

typedef struct {
    size_t line;
    TokenType type;
    uint64_t val;
} Token;

void lex_line(char *str, size_t line_num, Token **ret);
Token **lex_file(FILE *f);
char *token_to_str(TokenType ttype);
Type char_to_type(char t_ch);
