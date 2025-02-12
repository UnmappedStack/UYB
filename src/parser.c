#include <parser.h>
#include <vector.h>
#include <api.h>
#include <assert.h>
#include <string.h>

Instruction parse_instruction(char *instr, size_t line) {
    if (!strcmp(instr, "ret")) return RET;
    else {
        printf("Invalid instruction on line %zu (check it's listed in parse_instruction())\n", line);
        exit(1);
    }
}

ValType tok_as_valtype(TokenType tok, size_t line) {
    if      (tok == TokInteger) return Number;
    else if (tok == TokLabel)   return Label;
    else if (tok == TokRawStr)  return Str;
    else if (tok == TokStrLit)  return StrLit;
    else {
        printf("Token can't be converted to ValType: Invalid instruction value on line %zu\n", line);
        exit(1);
    }
}

// Expects tokens to end with TokNewLine
Statement parse_statement(Token *toks) {
    Statement ret = {0};
    size_t at = 0;
    if (toks[0].type == TokLabel) {
        ret.label = (char*) toks[0].val;
        ret.type = toks[1].val;
        at = 2;
    }
    if (toks[at].type != TokRawStr) {
        printf("Expected instruction in statement on line %zu\n", toks[at].line);
        exit(1);
    }
    ret.instruction = parse_instruction((char*) toks[at].val, toks[at].line);
    at++;
    for (size_t i = 0; i < 3 && toks[at + i].type != TokNewLine; i++) {
        if (toks[at + i].type == TokComma) {
            i--;
            continue;
        }
        ret.vals[i] = toks[at + i].val;
        ret.val_types[i] = tok_as_valtype(toks[at + i].type, toks[at + i].type);
    }
    return ret;
}

// returns number of tokens to skip
size_t parse_function(Token **toks, size_t loc, Function *buf) {
    buf->is_global = (*toks)[loc].type == TokExport;
    size_t skip = 1 + loc;
    if (buf->is_global) skip++;
    if ((*toks)[skip].type != TokRawStr || ((char*) (*toks)[skip].val)[1]) {
        printf("Not a valid function return type on line %zu.\n", (*toks)[skip].line);
        exit(1);
    }
    buf->return_type = char_to_type(((char*) (*toks)[skip].val)[0]);
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
    while ((*toks)[skip].type != TokRParen) {
        printf("Function arguments not supported yet in parser (TODO)\n");
        exit(1);
    }
    buf->num_args = 0;
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
        }
        skip++;
    }
    buf->statements = *statements;
    return skip + 1 - loc;
}

void parse_program(Token **toks) {
    size_t num_toks = vec_size(toks);
    for (size_t tok = 0; tok < num_toks; tok++) {
        if ((*toks)[tok].type == TokFunction || (*toks)[tok].type == TokExport) {
            Function fnbuf;
            tok += parse_function(toks, tok, &fnbuf) - 1;
        } else if ((*toks)[tok].type == TokNewLine) {
            continue;
        } else if ((*toks)[tok].type == TokLabel) {
            printf("TODO: Constant definitions are not yet implemented.\n");
            exit(1);
        } else {
            printf("Something was found outside of a function body which isn't a constant definition on line %zu\n", (*toks)[tok].line);
        }
    }
}
