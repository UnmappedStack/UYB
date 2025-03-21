/* Textual IR lexer for the UYB compiler backend project.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <lexer.h>
#include <vector.h>
#include <string.h>
#include <ctype.h>
#include <arena.h>
#include <utils.h>

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

char *token_to_str(TokenType ttype) {
    if      (ttype == TokFunction)   return "TokFunction";
    else if (ttype == TokExport)     return "TokExport";
    else if (ttype == TokNewLine)    return "TokNewLine";
    else if (ttype == TokLabel)      return "TokLabel";
    else if (ttype == TokStrLit)     return "TokStrLit";
    else if (ttype == TokRawStr)     return "TokRawStr";
    else if (ttype == TokInteger)    return "TokInteger";
    else if (ttype == TokLabel)      return "TokLabel";
    else if (ttype == TokLParen)     return "TokLParen";
    else if (ttype == TokRParen)     return "TokRParen";
    else if (ttype == TokLBrace)     return "TokLBrace";
    else if (ttype == TokRBrace)     return "TokRBrace";
    else if (ttype == TokData)       return "TokData";
    else if (ttype == TokSection)    return "TokSection";
    else if (ttype == TokBlockLabel) return "TokBlkLbl";
    else if (ttype == TokTripleDot)  return "TokTripleDot";
    else if (ttype == TokAlign)      return "TokAlign";
    else if (ttype == TokAggType)    return "TokAggType";
    else if (ttype == TokType)       return "TokType";
    else if (ttype == TokComma)      return "TokComma";
    else if (ttype == TokColon)      return "TokColon";
    else if (ttype == TokBar)        return "TokBar";
    else return "TokInvalid";
}

// `ret` argument is a buffer for a vector which all the tokens will be pushed to.
void lex_line(char *str, size_t line_num, Token **ret) {
    size_t len = strlen(str);
    for (size_t i = 0; i < len; i++) {
        if      (str[i] == '\t' || str[i] == ' ' || str[i] == '\r' || str[i] == 0) continue;
        else if (str[i] == '#') break;
        else if (str[i] == '(') vec_push(ret, ((Token) {.line=line_num,.type=TokLParen,.val=0}));
        else if (str[i] == ')') vec_push(ret, ((Token) {.line=line_num,.type=TokRParen,.val=0}));
        else if (str[i] == '{') vec_push(ret, ((Token) {.line=line_num,.type=TokLBrace,.val=0}));
        else if (str[i] == '}') vec_push(ret, ((Token) {.line=line_num,.type=TokRBrace,.val=0}));
        else if (str[i] == ',') vec_push(ret, ((Token) {.line=line_num,.type=TokComma,.val=0}));
        else if (str[i] == ':') vec_push(ret, ((Token) {.line=line_num,.type=TokColon,.val=0}));
        else if (str[i] == '|') vec_push(ret, ((Token) {.line=line_num,.type=TokBar,.val=0}));
        else if (!memcmp(&str[i], "...", 3)) {
            vec_push(ret, ((Token) {.line=line_num,.type=TokTripleDot,.val=0}));
            i += 2;
        }
        else if (str[i] == '=' && isalpha(str[i + 1])) {
            vec_push(ret, ((Token) {.line=line_num,.type=TokAssign,.val=char_to_type(str[i+1])}));
            i++;
        } else if (str[i] == '=') {
            vec_push(ret, ((Token) {.line=line_num,.type=TokEqu,.val=0}));
        } else if (isdigit(str[i]) || str[i] == '-') {
            size_t dig = 0;
            for (; isdigit(str[i + dig]) || (str[i + dig] == '-' && dig == 0); dig++);
            char *buf = aalloc(dig + 1); // perhaps I should move this to a fixed size buffer?
            memcpy(buf, &str[i], dig);
            buf[dig] = 0;
            int negative_flag = 0;
            if (str[i] == '-') {
                negative_flag = true;
                buf++;
            }
            uint64_t val = strtoll(buf,NULL,10);
            if (negative_flag) {
                val = -val;
            }
            vec_push(ret, ((Token) {.line=line_num,.type=TokInteger,.val=val}));
            i += dig - 1;
        } else if (str[i] == '"') {
            size_t dig = 0;
            for (; !(str[i + dig] == '"' && dig); dig++);
            char *buf = aalloc(dig + 1);
            memcpy(buf, &str[i + 1], dig);
            buf[dig - 1] = 0;
            vec_push(ret, ((Token) {.line=line_num,.type=TokStrLit,.val=(uint64_t) buf}));
            i += dig;
        } else if (str[i] == '%' || str[i] == '$' || str[i] == '@' || str[i] == ':') {
            i++;
            size_t dig = 0;
            for (; valid_label_char(str[i + dig]); dig++);
            char *buf = aalloc(dig + 2);
            memcpy(buf, &str[i], dig + 1);
            buf[dig] = 0;
            if (str[i - 1] == '%')
                vec_push(ret, ((Token) {.line=line_num,.type=TokLabel,.val=(uint64_t) buf}));
            else if (str[i - 1] == '$')
                vec_push(ret, ((Token) {.line=line_num,.type=TokRawStr,.val=(uint64_t) buf}));
            else if (str[i - 1] == '@')
                vec_push(ret, ((Token) {.line=line_num,.type=TokBlockLabel,.val=(uint64_t) buf}));
            else if (str[i - 1] == ':')
                vec_push(ret, ((Token) {.line=line_num,.type=TokAggType,.val=(uint64_t) buf}));
            i += dig - 1;
        } else if (valid_label_char(str[i])) {
            size_t dig = 0;
            for (; valid_label_char(str[i + dig]); dig++);
            char *buf = aalloc(dig + 1);
            memcpy(buf, &str[i], dig);
            buf[dig] = 0;
            if (!strcmp(buf, "function")) {
                vec_push(ret, ((Token) {.line=line_num,.type=TokFunction,.val=0}));
            } else if (!strcmp(buf, "export")) {
                vec_push(ret, ((Token) {.line=line_num,.type=TokExport,.val=0}));
            } else if (!strcmp(buf, "data")) {
                vec_push(ret, ((Token) {.line=line_num,.type=TokData,.val=0}));
            } else if (!strcmp(buf, "section")) {
                vec_push(ret, ((Token) {.line=line_num,.type=TokSection,.val=0}));
            } else if (!strcmp(buf, "align")) {
                vec_push(ret, ((Token) {.line=line_num,.type=TokAlign,.val=0}));
            } else if (!strcmp(buf, "type")) {
                vec_push(ret, ((Token) {.line=line_num,.type=TokType,.val=0}));
            } else if (!strcmp(buf, ".file")) {
                vec_push(ret, ((Token) {.line=line_num,.type=TokFile,.val=0}));
            } else {
                vec_push(ret, ((Token) {.line=line_num,.type=TokRawStr,.val=(uint64_t) buf}));
            }
            i += dig - 1;
        } else {
            printf("Invalid token on line %zu: %c (%u)\n", line_num, str[i], str[i]);
            exit(1);
        }
    }
}

Token **lex_file(FILE *f) {
    ssize_t sz;
    char *contents;
    Token **ret = vec_new(sizeof(Token));
    size_t ln = 1;
    size_t start = 0;
    size_t end = 0;
    if (f == stdin) {
        contents = read_full_stdin();
        sz = strlen(contents);
        goto end_readfile;
    }
    fseek(f, 0, SEEK_END);
    if ((sz = ftell(f)) < 0) {
        printf("Failed to get file length (ftell error).\n");
        exit(1);
    }
    fseek(f, 0, SEEK_SET);
    contents = aalloc(sz + 1);
    if (!fread(contents, sz, 1, f)) {
        printf("Failed to read from file.\n");
        exit(1);
    }
end_readfile:
    for (; end <= sz; end++) {
        if (contents[end] == '\n') {
            contents[end] = 0;
            lex_line(&contents[start], ln, ret);
            vec_push(ret, ((Token) {.line=ln,.type=TokNewLine,.val=0}));
            start = end + 1;
            ln++;
        }
    }
    if (f == stdin) free(contents);
    return ret;
}
