#include <parser.h>
#include <vector.h>
#include <api.h>
#include <assert.h>

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
            printf("Parse line.\n");
            start = skip;
        }
        skip++;
    }
    (void) start;
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
