/* String slice implementation for UYB.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <strslice.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

String *string_from(char *from) {
    String *str = (String*) malloc(sizeof(String));
    str->len  = strlen(from);
    str->data = (char*) malloc(str->len + 1);
    strcpy(str->data, from);
    return str;
}

void string_push(String *str, char *new) {
    size_t new_len = str->len + strlen(new);
    str->data = realloc(str->data, new_len + 1);
    strcpy(str->data + str->len, new);
    str->len = new_len;
}

void string_push_fmt(String *str, char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int length = vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    size_t new_len = str->len + length;
    str->data = realloc(str->data, new_len + 1);
    va_start(args, fmt);
    vsnprintf(str->data + str->len, length + 1, fmt, args);
    va_end(args);
    str->len = new_len;
}
