/* Textual IR parser for the UYB compiler backend project.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <parser.h>
#include <math.h>
#include <vector.h>
#include <ctype.h>
#include <api.h>
#include <assert.h>
#include <string.h>
#include <arena.h>

// WARNING: Edits the original string
void str_toupper(char* str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

size_t bytes_from_size(Type sz) {
    switch (sz) {
        case Bits8:  return 1;
        case Bits16: return 2;
        case Bits32: return 4;
        default:     return 8;
    }
}

// really messy, there's probably a cleaner way to do this. Or at least, move it into another file.
Instruction parse_instruction(char *instr, size_t line, Type *type) {
    str_toupper(instr);
    if      (!strcmp(instr, "ADD"   )) return ADD;
    else if (!strcmp(instr, "SUB"   )) return SUB;
    else if (!strcmp(instr, "DIV"   )) return DIV;
    else if (!strcmp(instr, "MUL"   )) return MUL;
    else if (!strcmp(instr, "COPY"  )) return COPY;
    else if (!strcmp(instr, "RET"   )) return RET;
    else if (!strcmp(instr, "CALL"  )) return CALL;
    else if (!strcmp(instr, "JZ"    )) return JZ;
    else if (!strcmp(instr, "NEG"   )) return NEG;
    else if (!strcmp(instr, "UDIV"  )) return UDIV;
    else if (!memcmp(instr, "STORE", 5)) {
        if (strlen(instr) > 5)
            *type = char_to_type(tolower(instr[5]));
        return STORE;
    }
    else if (!memcmp(instr, "LOAD", 4)) {
        if (strlen(instr) > 5)
            *type = char_to_type(tolower(instr[5]));
        return LOAD;
    }
    else if (!strcmp(instr, "BLIT"    )) return BLIT;
    else if (!strcmp(instr, "ALLOC"   )) return ALLOC;
    else if (!memcmp(instr+1, "EQ", 2 )) return EQ;
    else if (!memcmp(instr+1, "NE", 2 )) return NE;
    else if (!memcmp(instr+1, "SGE", 3)) return SGE;
    else if (!memcmp(instr+1, "SGT", 3)) return SGT;
    else if (!memcmp(instr+1, "SLE", 3)) return SLE;
    else if (!memcmp(instr+1, "SLT", 3)) return SLT;
    else if (!memcmp(instr+1, "UGE", 3)) return UGE;
    else if (!memcmp(instr+1, "UGT", 3)) return UGT;
    else if (!memcmp(instr+1, "ULE", 3)) return ULE;
    else if (!memcmp(instr+1, "ULT", 3)) return ULT;
    else if (!memcmp(instr, "EXT", 3  )) return EXT;
    else if (!strcmp(instr, "HLT"     )) return HLT;
    else if (!strcmp(instr, "BLKLBL"  )) return BLKLBL;
    else if (!strcmp(instr, "JMP"     )) return JMP;
    else if (!strcmp(instr, "JNZ"     )) return JNZ;
    else if (!strcmp(instr, "SHL"     )) return SHL;
    else if (!strcmp(instr, "SHR"     )) return SHR;
    else if (!strcmp(instr, "OR"      )) return OR;
    else if (!strcmp(instr, "AND"     )) return AND;
    else if (!strcmp(instr, "PHI"     )) return PHI;
    else if (!strcmp(instr, "VASTART" )) return VASTART;
    else if (!strcmp(instr, "VAARG"   )) return VAARG;
    else if (!strcmp(instr, ".LOC"    )) return LOC;
    else if (!strcmp(instr, "ASM"     )) return ASM;
    else {
        printf("Invalid instruction on line %zu: %s\n", line, instr);
        exit(1);
    }
}

ValType tok_as_valtype(TokenType tok, size_t line) {
    if      (tok == TokInteger)     return Number;
    else if (tok == TokLabel)       return Label;
    else if (tok == TokRawStr)      return Str;
    else if (tok == TokBlockLabel)  return BlkLbl;
    else if (tok == TokStrLit)      return StrLit;
    else {
        printf("Token can't be converted to ValType: Invalid instruction value on line %zu\n", line);
        exit(1);
    }
}

void parse_statement_parameters(Token *toks, size_t at, Statement *ret) {
    size_t num_args = 0;
    size_t v = 0;
    for (size_t i = 0; v <= 3 && toks[at + i].type != TokNewLine; i++) {
        if (toks[at + i].type == TokComma) {
            continue;
        }
        ret->vals[v] = toks[at + i].val;
        ret->val_types[v] = tok_as_valtype(toks[at + i].type, toks[at + i].line);
        num_args++;
        v++;
    }
    for (size_t i = num_args; i < 3; i++) {
        ret->val_types[i] = Empty;
    }
}

void parse_phi_parameters(Token *toks, size_t at, Statement *ret) {
    if (toks[at].type != TokBlockLabel || toks[at + 3].type != TokBlockLabel) {
        printf("Phi instruction format is not correct, expected a block label on line %zu\n", toks->line);
        exit(1);
    }
    if (toks[at + 2].type != TokComma) {
        printf("Expected comma between phi node values on line %zu\n", toks->line);
        exit(1);
    }
    ret->vals[0] = (size_t) aalloc(sizeof(PhiVal));
    ret->vals[1] = (size_t) aalloc(sizeof(PhiVal));
    *((PhiVal*) ret->vals[0]) = (PhiVal) {
        .blklbl_name = (char*) toks[at].val,
        .val = toks[at + 1].val,
        .type = tok_as_valtype(toks[at + 1].type, toks[at + 1].line),
    };
    *((PhiVal*) ret->vals[1]) = (PhiVal) {
        .blklbl_name = (char*) toks[at + 3].val,
        .val = toks[at + 4].val,
        .type = tok_as_valtype(toks[at + 4].type, toks[at + 4].line),
    };
    ret->val_types[0] = PhiArg;
    ret->val_types[1] = PhiArg;
    ret->val_types[2] = Empty;
}

// returns number of tokens to skip
size_t parse_asm_io(Token *toks, size_t at, InlineAsmIO ***io_vec_buf) {
    size_t at_start = at;
    at++;
    *io_vec_buf = vec_new(sizeof(InlineAsmIO));
    while (toks[at].type == TokLabel) {
        if (toks[at + 1].type != TokBar) {
            printf("Expected vertical bar (|) after label in I/O list for inline assembly on line %zu.\n", toks[at + 1].line);
            exit(1);
        }
        if (toks[at + 2].type != TokStrLit) {
            printf("Expected string literal referring to register in I/O list for inline assembly on line %zu.\n", toks[at + 2].line);
            exit(1);
        }
        vec_push(*io_vec_buf, ((InlineAsmIO) {
            .reg   = (char*) toks[at + 2].val,
            .label = (char*) toks[at].val,
            .type  = tok_as_valtype(toks[at].type, toks[at].line),
        }));
        at += 3;
        if (toks[at].type == TokComma) at++;
    }
    return at - at_start;
}

void parse_asm_clobbers(Token *toks, size_t at, char** **clobbers_buf_vec) {
    *clobbers_buf_vec = vec_new(sizeof(char*));
    at++;
    while (toks[at].type != TokRParen) {
        if (toks[at].type == TokComma) at++;
        else if (toks[at].type == TokStrLit)
            vec_push(*clobbers_buf_vec, (char*) toks[at++].val);
        else {
            printf("Invalid token in inline assembly clobber list, expected string literal or comma on line %zu.\n", toks[at].line);
            exit(1);
        }
    }
}

void parse_asm_parameters(Token *toks, size_t at, Statement *ret) {
    ret->val_types[0] = InlineAssembly;
    ret->val_types[1] = ret->val_types[2] = Empty;
    InlineAsm *buf = (InlineAsm*) malloc(sizeof(InlineAsm));
    // get the assembly itself
    if (toks[at].type != TokLParen) {
        printf("Expected left parenthesis after ASM instruction keyword on line %zu\n", toks[at].line);
        exit(1);
    }
    if (toks[at + 1].type != TokStrLit) {
        printf("Expected string literal after \"asm(\" on line %zu\n", toks[at + 1].line);
        exit(1);
    }
    buf->assembly = (char*) toks[at + 1].val;
    // replace instances of \t and \n with their correct values
    size_t len = strlen(buf->assembly);
    for (size_t c = 0; c < len; c++) {
        if (buf->assembly[c] != '\\') continue;
        if (buf->assembly[c + 1] == 'n')
            buf->assembly[c] = 10; // 10 is newline
        else if (buf->assembly[c + 1] == 't')
            buf->assembly[c] = 9; // 9 is carriage return
        else {
            printf("Unknown escape sequence (only \\t and \\n can be used in UYB)\n");
            exit(1);
        }
        memmove(&buf->assembly[c + 1], &buf->assembly[c + 2], len - c);
        c--;
    }
    at += 2;
    // get the inputs
    if (toks[at].type == TokColon)
        at += parse_asm_io(toks, at, &buf->inputs_vec);
    else
        goto end_asm_parse;
    // get the outputs
    if (toks[at].type == TokColon)
        at += parse_asm_io(toks, at, &buf->outputs_vec);
    else
        goto end_asm_parse;
    // get the clobbers
    if (toks[at].type == TokColon)
        parse_asm_clobbers(toks, at, &buf->clobbers_vec);
end_asm_parse:
    ret->vals[0] = (uint64_t) buf;
}

void parse_call_parameters(Token *toks, size_t at, Statement *ret) {
    if (toks[at].type != TokRawStr) {
        printf("Expected function name after CALL instruction on line %zu.\n", toks[at].line);
        exit(1);
    }
    if (toks[at + 1].type != TokLParen) {
        printf("Expected function arguments within parenthesis for CALL instruction on line %zu.\n", toks[at + 1].line);
        exit(1);
    }
    ret->vals[0] = toks[at].val;
    at += 2;
    char* **args = vec_new(sizeof(char*));
    Type **arg_sizes = vec_new(sizeof(Type));
    char* **arg_struct_types = vec_new(sizeof(char*));
    bool **args_are_structs = vec_new(sizeof(bool));
    ValType **arg_types= vec_new(sizeof(ValType));
    while (toks[at].type != TokRParen) {
        if (toks[at].type == TokComma) {
            at++;
            continue;
        }
        if (toks[at].type == TokTripleDot) {
            at += 2;
            continue;
        }
        if ((toks[at].type != TokRawStr || ((char*) toks[at].val)[1] != 0) && toks[at].type != TokAggType) {
            printf("Expected argument type before argument in argument list in CALL instruction parameters on line %zu.\n", toks[at].line);
            exit(1);
        }
        if (toks[at + 1].type != TokLabel && toks[at + 1].type != TokRawStr && toks[at + 1].type != TokInteger) {
            printf("Expected label, integer literal, or global in argument list for CALL instruction on line %zu.\n", toks[at + 1].line);
            exit(1);
        }
        if (toks[at].type == TokRawStr) {
            vec_push(arg_sizes, char_to_type(((char*) toks[at].val)[0]));
            vec_push(arg_struct_types, 0);
            vec_push(args_are_structs, (bool) false);
        } else {
            vec_push(arg_sizes, 0);
            vec_push(arg_struct_types, (char*) toks[at].val);
            vec_push(args_are_structs, (bool) true);
        }
        vec_push(args, (char*) toks[at + 1].val);
        vec_push(arg_types, tok_as_valtype(toks[at + 1].type, toks[at + 1].line));
        at += 2;
    }
    ret->vals[1] = (uint64_t) aalloc(sizeof(FunctionArgList));
    *((FunctionArgList*) ret->vals[1]) = (FunctionArgList) {
        .args = *args,
        .arg_sizes = *arg_sizes,
        .arg_struct_types = *arg_struct_types,
        .args_are_structs = *args_are_structs,
        .arg_types = *arg_types,
        .num_args = vec_size(args),
    };
    ret->val_types[0] = Str;
    ret->val_types[1] = FunctionArgs;
    ret->val_types[2] = Empty;
}

Type instruction_remove_size(char *instr) {
    while (*instr) {
        if (*instr >= '0' && *instr <= '9') {
            *instr = 0;
            return Bits64;
        }
        instr++;
    }
    return 50;
}

// Expects tokens to end with TokNewLine
Statement parse_statement(Token *toks) {
    if (toks[0].type == TokNewLine) toks++;
    if (toks[0].type == TokBlockLabel) {
        return (Statement) {
            .label = NULL,
            .instruction = BLKLBL,
            .vals = {toks[0].val},
            .val_types = {Str, Empty, Empty},
        };
    }
    Statement ret = {0};
    size_t at = 0;
    if (toks[0].type == TokLabel) {
        ret.label = (char*) toks[0].val;
        ret.type = toks[1].val;
        at = 2;
    } else {
        ret.label = NULL;
    }
    if (toks[at].type != TokRawStr) {
        printf("Expected instruction in statement on line %zu, got %s instead.\n", toks[at].line, token_to_str(toks[at].type));
        exit(1);
    }
    size_t new_size = instruction_remove_size((char*) toks[at].val);
    if (new_size != 50)
        ret.type = new_size;
    ret.instruction = parse_instruction((char*) toks[at].val, toks[at].line, &ret.type);
    at++;
    if (ret.instruction == CALL)
        parse_call_parameters(toks, at, &ret);
    else if (ret.instruction == PHI)
        parse_phi_parameters(toks, at, &ret);
    else if (ret.instruction == ASM)
        parse_asm_parameters(toks, at, &ret);
    else
        parse_statement_parameters(toks, at, &ret);
    return ret;
}

// returns number of tokens to skip
size_t parse_function(Token **toks, size_t loc, Function *buf) {
    buf->is_global = (*toks)[loc].type == TokExport;
    size_t skip = 1 + loc;
    if ((*toks)[skip].type == TokNewLine) skip++;
    if (buf->is_global) skip++;
    if (((*toks)[skip].type != TokRawStr || ((char*) (*toks)[skip].val)[1])
            && (*toks)[skip].type != TokAggType) {
        printf("Not a valid function return type on line %zu.\n", (*toks)[skip].line);
        exit(1);
    }
    if ((*toks)[skip].type == TokRawStr) {
        buf->return_type = char_to_type(((char*) (*toks)[skip].val)[0]);
        buf->ret_is_struct = false;
    } else {
        buf->return_struct = (char*) (*toks)[skip].val;
        buf->ret_is_struct = true;
    }
    skip++;
    if ((*toks)[skip].type != TokRawStr) {
        printf("Expected function name on line %zu.\n", (*toks)[skip].line);
        exit(1);
    }
    buf->name = (char*) (*toks)[skip].val;
    if ((*toks)[skip + 1].type != TokLParen) {
        printf("Expected left parenthesis after function name in function definition on line %zu, got %s instead.\n", (*toks)[skip + 1].line, token_to_str((*toks)[skip + 1].type));
        exit(1);
    }
    skip += 2;
    FunctionArgument **args = vec_new(sizeof(FunctionArgument));
    buf->is_variadic = false;
    while ((*toks)[skip].type != TokRParen) {
        if ((*toks)[skip].type == TokComma) {
            skip++;
            continue;
        }
        if ((*toks)[skip].type == TokTripleDot) {
            buf->is_variadic = true;
            skip++;
            continue;
        }
        if (((*toks)[skip].type != TokRawStr || ((char*) (*toks)[skip].val)[1] != 0) && (*toks)[skip].type != TokAggType) {
            printf("Expected argument type as character (l,w,d,b), got something else instead on line %zu.\n", (*toks)[skip].line);
            exit(1);
        }
        if ((*toks)[skip + 1].type != TokLabel) {
            printf("Argument value isn't a label on line %zu.\n", (*toks)[skip + 1].line);
            exit(1);
        }
        FunctionArgument arg;
        arg.label = (char*) (*toks)[skip + 1].val;
        if ((*toks)[skip].type == TokRawStr) {
            arg.type_is_struct = false;
            arg.type = char_to_type(((char*) (*toks)[skip].val)[0]);
        } else {
            arg.type_is_struct = true;
            arg.type_struct = (char*) (*toks)[skip].val;
        }
        vec_push(args, arg);
        skip += 2;
    }
    buf->num_args = vec_size(args);
    buf->args = *args;
    skip++;
    if ((*toks)[skip].type != TokLBrace) {
        printf("Expected brace after function signature on line %zu\n", (*toks)[skip].line);
        exit(1);
    }
    skip++;
    if ((*toks)[skip].type != TokNewLine) {
        printf("Expected new line after left brace in function declaration on line %zu\n", (*toks)[skip].line);
        exit(1);
    }
    skip++;
    size_t depth = 1;
    size_t start = skip;
    Statement **statements = vec_new(sizeof(Statement));
    buf->num_statements = 0;
    for (;;) {
        if ((*toks)[skip].type == TokLBrace) {
            depth++;
        } else if ((*toks)[skip].type == TokRBrace) {
            depth--;
            if (!depth) {
                skip++;
                break;
            }
        } else if ((*toks)[skip].type == TokNewLine) {
            buf->num_statements++;
            vec_push(statements, parse_statement(&(*toks)[start]));
            start = skip;
            if ((*toks)[start + 1].type != TokRBrace) start++;
        }
        skip++;
    }
    buf->statements = *statements;
    return skip + 1 - loc;
}

// returns number of tokens to skip
size_t parse_global(Token **toks, size_t loc, Global *buf) {
    size_t start_loc = loc;
    if ((*toks)[loc].type == TokSection) {
        loc++;
        if ((*toks)[loc].type != TokStrLit) {
            printf("Expected string literal after section keyword on line %zu\n", (*toks)[loc].line);
            exit(1);
        }
        buf->section = (char*) (*toks)[loc].val;
        loc += 2;
    } else {
        buf->section = NULL;
    }
    if ((*toks)[loc].type != TokData) {
        printf("Expected data global definition after section specification on line %zu\n", (*toks)[loc].line);
        exit(1);
    }
    if ((*toks)[loc + 1].type != TokRawStr) {
        printf("Expected name of global after data keyword on line %zu, got %s instead, data = %s\n", (*toks)[loc + 1].line, token_to_str((*toks)[loc + 1].type), (char*) (*toks)[loc + 1].val);
        exit(1);
    }
    buf->name = (char*) (*toks)[loc + 1].val;
    if ((*toks)[loc + 2].type != TokEqu) {
        printf("Expected = after global label name on line %zu\n", (*toks)[loc + 2].line);
        exit(1);
    }
    if ((*toks)[loc + 3].type == TokAlign) {
        if ((*toks)[loc + 4].type != TokInteger) {
            printf("Expected integer literal after Align token on line %zu\n", (*toks)[loc + 4].line);
            exit(1);
        }
        buf->alignment = (*toks)[loc + 4].val;
        loc += 2;
    } else
        buf->alignment = 1;
    if ((*toks)[loc + 3].type != TokLBrace) {
        printf("Expected left brace ({) after = on line %zu\n", (*toks)[loc + 3].line);
        exit(1);
    }
    loc += 4;
    Type **sizes = vec_new(sizeof(Type));
    size_t **vals = vec_new(sizeof(size_t));
    ValType **types= vec_new(sizeof(ValType));
    while ((*toks)[loc].type != TokRBrace) {
        if ((*toks)[loc].type == TokComma) {
            loc++;
            continue;
        }
        if ((*toks)[loc].type != TokRawStr || ((char*) (*toks)[loc].val)[1] != 0) {
            printf("Invalid type in global declaration on line %zu\n", (*toks)[loc].line);
            exit(1);
        }
        vec_push(sizes, char_to_type(((char*) (*toks)[loc].val)[0]));
        if ((*toks)[loc + 1].type == TokInteger) vec_push(types, Number);
        else if ((*toks)[loc + 1].type == TokStrLit) vec_push(types, StrLit);
        else {
            printf("Global values can only be a number or a strlit token on line %zu, got something else.\n", (*toks)[loc + 1].line);
            exit(1);
        }
        vec_push(vals, (*toks)[loc + 1].val);
        loc += 2;
    }
    buf->num_vals = vec_size(vals);
    buf->vals  = *vals;
    buf->types = *types;
    buf->sizes = *sizes;
    return loc - start_loc;
}

size_t get_element_size(Token **toks, size_t *loc, AggregateType *buf) {
    if ((*toks)[*loc].type == TokRawStr && ((char*) (*toks)[*loc].val)[1] == 0) {
        // If it's a type element, like `l`
        size_t this_size = bytes_from_size(char_to_type(((char*) (*toks)[*loc].val)[0]));
        if (buf->alignment > this_size)
            return buf->alignment;
        else
            return this_size;
    } else if ((*toks)[*loc].type == TokInteger) {
        // If it's an opaque type just specifying the number of bytes, like `24`
        return (*toks)[*loc].val;
    } else if ((*toks)[*loc].type == TokLBrace) {
        // If it's an enum type, return the maximum size
        size_t max_size = 0;
        (*loc)++;
        while ((*toks)[*loc].type != TokRBrace) {
            if ((*toks)[*loc].type == TokComma) {
                (*loc)++;
                continue;
            }
            size_t this_size = get_element_size(toks, loc, buf); // eww recursion
            if (this_size > max_size)
                max_size = this_size;
            (*loc)++;
        }
        return max_size;
    } else {
        printf("Invalid element for aggregate type on line %zu.\n", (*toks)[*loc].line);
        exit(1);
    }
}

void parse_aggtype_size(Token **toks, size_t *loc, AggregateType *buf) {
    buf->size_bytes = 0;
    while ((*toks)[*loc].type != TokRBrace) {
        if ((*toks)[*loc].type == TokComma) {
            (*loc)++;
            continue;
        }
        buf->size_bytes += get_element_size(toks, loc, buf);
        (*loc)++;
    }
}

// return number of tokens to skip.
size_t parse_aggtype(Token **toks, size_t loc, AggregateType *buf) {
    size_t start_loc = loc;
    if ((*toks)[loc + 1].type != TokAggType) {
        printf("Expected type name after type token, got something else on line %zu\n", (*toks)[loc + 1].line);
        exit(1);
    }
    buf->name = (char*) (*toks)[loc + 1].val;
    if ((*toks)[loc + 2].type != TokEqu) {
        printf("Equal sign expected after type name in aggregate type definiton, got something else on line %zu\n", (*toks)[loc + 2].line);
        exit(1);
    }
    if ((*toks)[loc + 3].type == TokAlign) {
        if ((*toks)[loc + 4].type != TokInteger) {
            printf("Expected integer literal after Align token on line %zu\n", (*toks)[loc + 4].line);
            exit(1);
        }
        buf->alignment = (*toks)[loc + 4].val;
        loc += 2;
    } else
        buf->alignment = 1;
    if ((*toks)[loc + 3].type != TokLBrace) {
        printf("Expected left brace in aggregate type definition on line %zu\n", (*toks)[loc + 3].line);
        exit(1);
    }
    loc += 4;
    parse_aggtype_size(toks, &loc, buf);
    return loc - start_loc;
}

// returns number of tokens to skip
size_t parse_filedbg(Token **toks, size_t loc, FileDbg *filebuf) {
    size_t start_loc = loc;
    if ((*toks)[loc + 1].type != TokInteger) {
        printf("First argument of .file must be integer literal (file identification number)\n");
        exit(1);
    }
    if ((*toks)[loc + 2].type != TokStrLit) {
        printf("Second argument of .file must be string literal (file name)\n");
        exit(1);
    }
    filebuf->id = (*toks)[loc + 1].val;
    filebuf->fname = (char*) (*toks)[loc + 2].val;
    loc += 2;
    return loc - start_loc;
}

// Returns vector of functions
Function **parse_program(Token **toks, Global ***globals_buf, AggregateType ***aggtypes_buf, FileDbg ***filesdbg_buf) {
    size_t num_toks = vec_size(toks);
    Function **functions = vec_new(sizeof(Function));
    *globals_buf = vec_new(sizeof(Global));
    *aggtypes_buf = vec_new(sizeof(AggregateType));
    *filesdbg_buf = vec_new(sizeof(FileDbg));
    for (size_t tok = 0; tok < num_toks; tok++) {
        if ((*toks)[tok].type == TokFunction || (*toks)[tok].type == TokExport) {
            Function fnbuf;
            tok += parse_function(toks, tok, &fnbuf) - 1;
            vec_push(functions, fnbuf);
        } else if ((*toks)[tok].type == TokNewLine) {
            continue;
        } else if ((*toks)[tok].type == TokData || (*toks)[tok].type == TokSection) {
            Global newglobal;
            tok += parse_global(toks, tok, &newglobal);
            vec_push(*globals_buf, newglobal);
        } else if ((*toks)[tok].type == TokType) {
            AggregateType newtype;
            tok += parse_aggtype(toks, tok, &newtype);
            vec_push(*aggtypes_buf, newtype);
        } else if ((*toks)[tok].type == TokFile) {
            FileDbg newfile;
            tok += parse_filedbg(toks, tok, &newfile);
            vec_push(*filesdbg_buf, newfile);
        } else {
            printf("Something was found outside of a function body which isn't a constant definition on line %zu: %s, token id %u, val %p\n", (*toks)[tok].line, token_to_str((*toks)[tok].type), (*toks)[tok].type, (void*) (*toks)[tok].val);
            exit(1);
        }
    }
    return functions;
}
