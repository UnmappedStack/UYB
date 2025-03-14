/* Register allocator for UYB project.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <target/x86_64/register.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector.h>
#include <arena.h>

/* all the scratch registers:
 *  {reg_name, num_refs, reg_size} 
 * num_refs is the number of references to the label corresponding to that register
 * *after* the current instruction. */
intptr_t reg_alloc_tab[5][3] = {
    {(uintptr_t) "%rbx", 0, 0},
    {(uintptr_t) "%r12", 0, 0},
    {(uintptr_t) "%r13", 0, 0},
    {(uintptr_t) "%r14", 0, 0},
    {(uintptr_t) "%r15", 0, 0},
};

// Left side is register, middle is assigned label, right is number of instances of that label
char *label_reg_tab[5][3] = {
    {"%rbx", 0, 0},
    {"%r12", 0, 0},
    {"%r13", 0, 0},
    {"%r14", 0, 0},
    {"%r15", 0, 0},
};

RegAlloc regalloc;

bool check_label_in_args(char *label) {
    for (size_t i = 0; i < regalloc.current_fn->num_args; i++) {
        if (!strcmp(label, regalloc.current_fn->args[i].label)) return true;
    }
    return false;
}

char *reg_as_size_inner(char *reg, Type size) {
    reg++;
    if (reg[0] == 'r' && /* is digit: */ (reg[1] >= '0' && reg[1] <= '9')) {
        String *str = string_from(reg);
             if (size == Bits8 ) string_push(str, "b");
        else if (size == Bits16) string_push(str, "w");
        else if (size == Bits32) string_push(str, "d");
        return str->data;
    }
    if (size == Bits8) {
             if (!strcmp(reg, "rsi")) return "sil";
        else if (!strcmp(reg, "rdi")) return "dil";
        char *buf = aalloc(4);
        memcpy(buf, &reg[1], 3);
        buf[1] = 'l';
        return buf;
    } else if (size == Bits16) {
        return &reg[1];
    } else if (size == Bits32) {
        char *buf = aalloc(4);
        memcpy(buf, &reg[0], 4);
        buf[0] = 'e';
        return buf;
    } else return reg;
}

Type size_from_reg(char *reg) {
    reg++;
    char last = reg[strlen(reg) - 1];
    if (reg[0] == 'r' && /* is digit: */ (reg[1] >= '0' && reg[1] <= '9')) {
             if (last == 'b') return Bits8;
        else if (last == 'w') return Bits16;
        else if (last == 'd') return Bits32;
        else                  return Bits64;
    }
    if (reg[0] == 'e') return Bits32;
    if (reg[0] == 'r') return Bits64;
    if (last == 'i' || last == 'x') return Bits16;
    return Bits8;
}

char *reg_as_size(char *reg, Type size) {
    if (reg[0] != '%') return reg;
    char *buf = aalloc(5);
    buf[0] = '%';
    strcpy(&buf[1], reg_as_size_inner(reg, size));
    return buf;
}

void reg_init_fn(Function func) {
    regalloc.bytes_rip_pad = 0;
    for (size_t i = 0; i < sizeof(reg_alloc_tab) / sizeof(reg_alloc_tab[0]); i++)
        reg_alloc_tab[i][1] = 0;
    regalloc.current_fn = (Function*) aalloc(sizeof(Function));
    *regalloc.current_fn = func;
    regalloc.labels_as_offsets = vec_new(sizeof(size_t) * 3);
    regalloc.used_regs_vec = vec_new(sizeof(char*));
    regalloc.statement_idx = 0;
}

char *reg_alloc_noresize(char *label, Type reg_size) {
    for (size_t l = 0; l < sizeof(label_reg_tab) / sizeof(label_reg_tab[0]); l++) {
        if (!label_reg_tab[l][1] || strcmp(label_reg_tab[l][1], label)) continue;
        size_t new_label_sz = strlen(label) + 5;
        char *new_label = (char*) aalloc(new_label_sz);
        label_reg_tab[l][2]++;
        snprintf(new_label, new_label_sz, "%s.%zu", label, (size_t) label_reg_tab[l][2]);
        label = new_label;
    }
    for (size_t i = 0; i < sizeof(reg_alloc_tab) / sizeof(reg_alloc_tab[0]); i++) {
        if (reg_alloc_tab[i][1]) continue;
        for (size_t s = regalloc.statement_idx; s < regalloc.current_fn->num_statements; s++) {
            if (regalloc.current_fn->statements[s].instruction == JMP || regalloc.current_fn->statements[s].instruction == JNZ) {
                reg_alloc_tab[i][1] = -1;
                break;
            }
            if (regalloc.current_fn->statements[s].val_types[1] == FunctionArgs) {
                for (size_t arg = 0; arg < ((FunctionArgList*) regalloc.current_fn->statements[s].vals[1])->num_args; arg++) {
                    if (((FunctionArgList*) regalloc.current_fn->statements[s].vals[1])->arg_types[arg] != Number &&
                            strcmp(label, ((FunctionArgList*) regalloc.current_fn->statements[s].vals[1])->args[arg])) continue;
                    reg_alloc_tab[i][1] += 2;
                }
            }
            if (regalloc.current_fn->statements[s].instruction == ASM) {
                InlineAsm *info = (InlineAsm*) regalloc.current_fn->statements[s].vals[0];
                for (size_t in = 0; in < vec_size(info->inputs_vec); in++) {
                    if (strcmp((*info->inputs_vec)[in].label, label)) continue;
                    reg_alloc_tab[i][1]++;
                }
            }
            if ((regalloc.current_fn->statements[s].val_types[0] == Label && !strcmp((char*) regalloc.current_fn->statements[s].vals[0], label)) || 
                    (regalloc.current_fn->statements[s].val_types[1] == Label && !strcmp((char*) regalloc.current_fn->statements[s].vals[1], label)) ||
                    (regalloc.current_fn->statements[s].val_types[2] == Label && !strcmp((char*) regalloc.current_fn->statements[s].vals[2], label))) {
                reg_alloc_tab[i][1]++;
            }
        }
        if (check_label_in_args(label) && reg_alloc_tab[i][1]) reg_alloc_tab[i][1]++;
        label_reg_tab[i][1] = aalloc(strlen(label) + 1);
        strcpy(label_reg_tab[i][1], label);
        size_t used_sz = vec_size(regalloc.used_regs_vec);
        bool do_push = true;
        for (size_t y = 0; y < used_sz; y++) {
            if (strcmp((*regalloc.used_regs_vec)[y], (char*) reg_alloc_tab[i][0])) continue;
        }
        if (reg_alloc_tab[i][1]) {
            if (do_push)
                vec_push(regalloc.used_regs_vec, (char*) reg_alloc_tab[i][0]);
            regalloc.bytes_rip_pad += 8;
        }
        reg_alloc_tab[i][2] = reg_size;
        return (char*) reg_alloc_tab[i][0];
    }
    regalloc.bytes_rip_pad += 8;
    char *fmt = "-%llu(%%rbp)";
    size_t buf_sz = strlen("-(%rbp)") + 5;
    char *buf = (char*) aalloc(buf_sz + 1);
    snprintf(buf, buf_sz, fmt, regalloc.bytes_rip_pad);
    size_t *new_vec_val = aalloc(sizeof(size_t) * 3);
    new_vec_val[0] = (size_t) label;
    new_vec_val[1] = regalloc.bytes_rip_pad;
    new_vec_val[2] = reg_size;
    vec_push(regalloc.labels_as_offsets, new_vec_val);
    return buf;
}

char *reg_alloc(char *label, Type reg_size) {
    char *reg = reg_alloc_noresize(label, reg_size);
    if (reg[0] == '%')
        return reg_as_size((char*) reg, reg_size);
    else
        return reg;
}

char *label_to_reg_noresize(size_t offset, char *label, bool allow_noexist) {
    for (size_t i = 0; i < sizeof(label_reg_tab) / sizeof(label_reg_tab[1]); i++) {
        if (!label_reg_tab[i][1] || strcmp(label_reg_tab[i][1], label)) continue;
        if (reg_alloc_tab[i][1])
            reg_alloc_tab[i][1]--;
        if (!reg_alloc_tab[i][1])
            label_reg_tab[i][1] = 0;
        return label_reg_tab[i][0];
    }
    size_t label_offset_list_len = vec_size(regalloc.labels_as_offsets);
    for (size_t l = 0; l < label_offset_list_len; l++) {
        if (strcmp((char*) (*regalloc.labels_as_offsets)[l][0], label)) continue;
        char *fmt = "-%llu(%%rbp)";
        size_t buf_sz = strlen("-(%rbp)") + 5;
        char *buf = (char*) aalloc(buf_sz + 1);
        snprintf(buf, buf_sz, fmt, (*regalloc.labels_as_offsets)[l][1] + offset);
        return buf;
    }
    if (allow_noexist) return NULL;
    printf("Tried to use non-defined label: %s\n", label);
    exit(1);
}

Type get_reg_size(char *reg, char *expected_label) {
    for (size_t i = 0; i < sizeof(reg_alloc_tab) / sizeof(reg_alloc_tab[0]); i++) {
        if (strcmp(reg, (char*) reg_alloc_tab[i][0])) continue;
        return reg_alloc_tab[i][2];
    }
    size_t len = vec_size(regalloc.labels_as_offsets);
    for (size_t i = 0; i < len; i++) {
        if (strcmp(expected_label, (char*) (*regalloc.labels_as_offsets)[i][0])) continue;
        return (Type) (*regalloc.labels_as_offsets)[i][2];
    }
    printf("Invalid register in get_reg_size: %s\n", reg);
    exit(1);
}

// I think this is kinda slow
char *label_to_reg(size_t offset, char *label, bool allow_noexist) {
    char *reg = label_to_reg_noresize(0, label, allow_noexist);
    if (!reg && allow_noexist) return NULL;
    for (size_t i = 0; i < sizeof(reg_alloc_tab) / sizeof(reg_alloc_tab[0]); i++) {
        if (strcmp(reg, (char*) reg_alloc_tab[i][0])) continue;
        if (!reg_alloc_tab[i][1] && allow_noexist) return NULL;
        return reg_as_size(reg, (Type) reg_alloc_tab[i][2]);
    }
    return reg;
}
