#pragma once
#include <stddef.h>

typedef struct {
    char *data;
    size_t len;
} String;

String *string_from(char *from);
void string_push(String *str, char *new);
void string_push_fmt(String *str, char *fmt, ...);
