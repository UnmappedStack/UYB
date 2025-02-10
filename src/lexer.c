#include <lexer.h>
#include <vector.h>
#include <string.h>
#include <api.h>
#include <ctype.h>

#define valid_label_char(ch) (ch == '.' || ch == '_' || isdigit(ch) || isalpha(ch))

Type char_to_type(char t_ch) {
    if      (t_ch == 'b') return Bits8;
    else if (t_ch == 'h') return Bits16;
    else if (t_ch == 'w') return Bits32;
    else if (t_ch == 'l') return Bits64;
    else {
        printf("Invalid type: %c\n", t_ch);
        exit(1);
    }
}

// `ret` argument is a buffer for a vector which all the tokens will be pushed to.
void lex_line(char *str, size_t line_num, Token **ret) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        if      (str[i] == '\t' || str[i] == ' ' || str[i] == '\r') continue;
        else if (str[i] == '#') break;
        else if (str[i] == '(') vec_push(ret, ((Token) {.line=line_num,.type=TokLParen,.val=0}));
        else if (str[i] == ')') vec_push(ret, ((Token) {.line=line_num,.type=TokRParen,.val=0}));
        else if (str[i] == '{') vec_push(ret, ((Token) {.line=line_num,.type=TokLBrace,.val=0}));
        else if (str[i] == '}') vec_push(ret, ((Token) {.line=line_num,.type=TokRBrace,.val=0}));
        else if (str[i] == ',') vec_push(ret, ((Token) {.line=line_num,.type=TokComma,.val=0}));
        else if (str[i] == '=') {
            vec_push(ret, ((Token) {.line=line_num,.type=TokAssign,.val=char_to_type(str[i+1])}));
        } else if (isdigit(str[i])) {
            // TODO: Support for lexing signed values
            size_t dig = 0;
            for (; isdigit(str[i + dig]); dig++);
            char *buf = malloc(dig + 1); // perhaps I should move this to a fixed size buffer?
            memcpy(buf, &str[i], dig);
            buf[dig] = 0;
            vec_push(ret, ((Token) {.line=line_num,.type=TokInteger,.val=strtoll(buf,NULL,10)}));
            free(buf);
            i += dig - 1;
        } else if (str[i] == '"') {
            size_t dig = 0;
            for (; !(str[i + dig] == '"' && dig); dig++);
            char *buf = malloc(dig + 1);
            memcpy(buf, &str[i + 1], dig);
            buf[dig - 1] = 0;
            vec_push(ret, ((Token) {.line=line_num,.type=TokStrLit,.val=(uint64_t) buf}));
            i += dig;
        } else if (str[i] == '%' || str[i] == '$') {
            i++;
            size_t dig = 0;
            for (; valid_label_char(str[i + dig]); dig++);
            char *buf = malloc(dig + 2);
            memcpy(buf, &str[i], dig + 1);
            buf[dig - 1] = 0;
            if (str[-1] == '%')
                vec_push(ret, ((Token) {.line=line_num,.type=TokLabel,.val=(uint64_t) buf}));
            else if (str[-1] == '$')
                vec_push(ret, ((Token) {.line=line_num,.type=TokRawStr,.val=(uint64_t) buf}));
            i += dig;
        } else if (isalpha(str[i])) {
            size_t dig = 0;
            for (; valid_label_char(str[i + dig]); dig++);
            char *buf = malloc(dig + 1);
            memcpy(buf, &str[i + dig], dig);
            buf[dig] = 0;
            if (!strcmp(buf, "function")) {
                free(buf);
                vec_push(ret, ((Token) {.line=line_num,.type=TokFunction,.val=0}));
            } else if (!strcmp(buf, "export")) {
                free(buf);
                vec_push(ret, ((Token) {.line=line_num,.type=TokExport,.val=0}));
            } else {
                vec_push(ret, ((Token) {.line=line_num,.type=TokRawStr,.val=(uint64_t) buf}));
            }
            i += dig - 1;
        } else {
            printf("Invalid token on line %zu: %c\n", line_num, str[i]);
            exit(1);
        }
    }
}

Token **lex_file(FILE *f) {
    ssize_t sz;
    fseek(f, 0, SEEK_END);
    if ((sz = ftell(f)) < 0) {
        printf("Failed to get file length (ftell error).\n");
        exit(1);
    }
    fseek(f, 0, SEEK_SET);
    char *contents = malloc(sz + 1);
    if (!fread(contents, sz, 1, f)) {
        printf("Failed to read from file.\n");
        exit(1);
    }
    Token **ret = vec_new(sizeof(Token));
    size_t ln = 1;
    size_t start = 0;
    size_t end = 0;
    for (; end <= sz; end++) {
        if (contents[end] == '\n') {
            contents[end] = 0;
            lex_line(&contents[start], ln, ret);
            vec_push(ret, ((Token) {.line=ln,.type=TokNewLine,.val=0}));
            start = end + 1;
            ln++;
        }
    }   
    return ret;
}
