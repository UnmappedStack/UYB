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

// Returns vector.
Token **lex_line(char *str, size_t line_num) {
    Token **ret = vec_new(sizeof(Token));
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        if      (str[i] == '(') vec_push(ret, ((Token) {.line=line_num,.type=TokLParen,.val=0}));
        else if (str[i] == ')') vec_push(ret, ((Token) {.line=line_num,.type=TokRParen,.val=0}));
        else if (str[i] == '{') vec_push(ret, ((Token) {.line=line_num,.type=TokLBrace,.val=0}));
        else if (str[i] == '}') vec_push(ret, ((Token) {.line=line_num,.type=TokRBrace,.val=0}));
        else if (str[i] == ',') vec_push(ret, ((Token) {.line=line_num,.type=TokComma,.val=0}));
        else if (str[i+1] == '=' && len == 2) {
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
        } else if (str[i] == '%') {
            i++;
            size_t dig = 0;
            for (; valid_label_char(str[i + dig]); dig++);
            char *buf = malloc(dig + 2);
            memcpy(buf, &str[i], dig + 1);
            buf[dig - 1] = 0;
            vec_push(ret, ((Token) {.line=line_num,.type=TokLabel,.val=(uint64_t) buf}));
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
        }
    }
    return ret;
}
