/* Header for ../src/parser.c, the parser for the textual IR for UYB.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#pragma once
#include <lexer.h>

Function **parse_program(Token **toks, Global ***globals_buf, AggregateType ***aggtypes_buf, FileDbg ***filesdbg_buf);
