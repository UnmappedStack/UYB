#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef enum {
    TokLabel,   // %labelname
    TokRawStr,  // value
    TokStrLit,  // "value"
    TokInteger,
    TokAssign, // t= (with t being the type, stored in val (types defined in api.h))
    TokLBrace, TokRBrace,
    TokLParen, TokRParen,
    TokComma,
    TokNewLine,
    TokFunction, TokExport, // keywords
} TokenType;

typedef struct {
    size_t line;
    TokenType type;
    uint64_t val;
} Token;

void lex_line(char *str, size_t line_num, Token **ret);
Token **lex_file(FILE *f);
