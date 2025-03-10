#include <utils.h>
#include <vector.h>
#include <arena.h>
#include <string.h>

char size_as_char(Type type) {
    return ((char[]){'b', 'h', 'w', 'l'})[type];
    if      (type == Bits8) return 'b';
    else if (type == Bits16) return 'h';
    else if (type == Bits32) return 'w';
    else return 'l';
}

/* returns 1 or 0 depending on if it was found. if it was found it stores the result in val_buf unless
 * val_buf is null */
int find_sizet_in_copyvals(CopyVal **copyvals, char *label, size_t *val_buf) {
    for (size_t i = 0; i < vec_size(copyvals); i++) {
        if (!strcmp((*copyvals)[i].label, label)) {
            if (val_buf)
                *val_buf = (*copyvals)[i].val;
            return 1;
        }
    }
    return 0;
}

int find_copyval(CopyVal **copyvals, char *label, CopyVal *val_buf) {
    for (size_t i = 0; i < vec_size(copyvals); i++) {
        if (!strcmp((*copyvals)[i].label, label)) {
            if (val_buf)
                *val_buf = (*copyvals)[i];
            return 1;
        }
    }
    return 0;
}

char *get_full_char_str(bool is_struct, Type type, char *type_struct) {
    char *rettype;
    if (is_struct) {
        rettype = (char*) aalloc(strlen(type_struct) + 2);
        sprintf(rettype, ":%s", type_struct);
    } else {
        rettype = (char*) aalloc(2);
        rettype[0] = size_as_char(type);
        rettype[1] = 0;
    }
    return rettype;
}

// Returns a pointer to an aggregate type from an array of aggregate types
AggregateType *find_aggtype(char *name, AggregateType *aggtypes, size_t num_aggtypes) {
    for (size_t i = 0; i < num_aggtypes; i++) {
        if (!strcmp(name, aggtypes[i].name)) return &aggtypes[i];
    }
    printf("Tried to use undefined aggregate type.\n");
    exit(1);
}

/* Caller is expected to free return value.
 * Reads a full line of stdin. */
char *read_full_stdin() {
    size_t pos = 0, size = 1025, nread;
    char *buf0 = malloc(size);
    char *buf = buf0;
    for (;;) {
        if (buf == NULL) {
            fprintf(stderr, "Not enough memory for %zu bytes in read_full_stdin()\n", size);
            free(buf0);
            return NULL;
        }
        nread = fread(buf + pos, 1, size - pos - 1, stdin);
        if (nread == 0) break;
        pos += nread;
        if (size - pos < size / 2)
            size += size / 2 + size / 8;
        buf = realloc(buf0 = buf, size);
    }
    buf[pos] = '\0';
    return buf;
}
