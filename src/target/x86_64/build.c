/* Main code generation file for UYB x86_64 target.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <api.h>
#include <vector.h>
#include <strslice.h>
#include <string.h>
#include <arena.h>
#include <target/x86_64/register.h>
#include <utils.h>

AggregateType *aggregate_types; /* TODO: Move all global vars (including those in register.c) */
size_t num_aggregate_types;     /* into a single structure. */

size_t type_to_size(Type type) {
    if (type == Bits8) return 1;
    else if (type == Bits8) return 2;
    else if (type == Bits16) return 4;
    else if (type == Bits64) return 8;
    return 0;
}

char *global_sizes[] = {
    ".byte", ".value", ".long", ".quad",
};

char *type_as_str(Type type, char *struct_type, bool is_struct) {
    if (is_struct) {
        char *buf = aalloc(strlen(struct_type) + 2);
        sprintf(buf, ":%s", struct_type);
        return buf;
    }
    if (type == Bits8) return "byte";
    else if (type == Bits16) return "word";
    else if (type == Bits32) return "dword";
    else if (type == Bits64) return "qword";
    else {
        printf("Invalid type: %u\n", type);
        return "invalid_type";
    }
}

static String *build_function(Function IR) {
    reg_init_fn(IR);
    String *fnbuf0 = string_from("\n");
    string_push_fmt(fnbuf0, "// %s %s(", type_as_str(IR.return_type, IR.return_struct, IR.ret_is_struct), IR.name);
    for (size_t arg = 0; arg < IR.num_args; arg++) {
        string_push_fmt(fnbuf0, "%s %%%s", type_as_str(IR.args[arg].type, IR.args[arg].type_struct, IR.args[arg].type_is_struct), IR.args[arg].label);
        if (arg != IR.num_args - 1) string_push(fnbuf0, ", ");
    }
    string_push_fmt(fnbuf0, ") {\n%s", IR.name);
    String *fnbuf = string_from(":\n");
    String *structarg_buf = string_from("\n");
    size_t reg_arg_off = 0;
    for (size_t arg = 0; arg < IR.num_args; arg++) {
        if (IR.args[arg].type_is_struct) {
            if (arg > 4) {
                printf("Only the first 5 arguments accepted by a function can be structures. (TODO)\n");
                exit(1);
            }
            AggregateType *aggtype = find_aggtype(IR.args[arg].type_struct, aggregate_types, num_aggregate_types);
            char *label_loc = reg_alloc(IR.args[arg].label, Bits64);
            if (aggtype->size_bytes <= 16) {
                // allocate space on the stack for it
                bytes_rip_pad += (aggtype->size_bytes <= 8) ? 1 : 2;
                string_push_fmt(structarg_buf, "\tlea -%llu(%rbp), %%rdi\n"
                                       "\tmov %%rdi, %s\n",
                        bytes_rip_pad, label_loc);
                // copy the data
                string_push_fmt(structarg_buf, "\tmov %s, (%s)\n", arg_regs[arg], label_loc);
                if (aggtype->size_bytes > 8) {
                    // copy the second byte
                    string_push_fmt(structarg_buf, "\tmov %s, 8(%s)\n", arg_regs[arg + 1], label_loc);
                }
            } else {
                bytes_rip_pad += aggtype->size_bytes;
                // copy all the data
                string_push_fmt(structarg_buf, "\tmov %s, %%rsi\n", arg_regs[arg + 1]);
                string_push_fmt(structarg_buf, "\tmov %zu, %%rdi\n", bytes_rip_pad);
                string_push_fmt(structarg_buf, "\tmov %zu, %%rcx\n", aggtype->size_bytes);
                string_push(structarg_buf,     "\trep movsb\n");
                string_push_fmt(structarg_buf, "\tmov %zu, %s\n", bytes_rip_pad, label_loc);
            }
        } else if (arg > 5) {
            // it's on the stack
            size_t *new_vec_val = aalloc(sizeof(size_t) * 2);
            new_vec_val[0] = (size_t) IR.args[arg].label;
            reg_arg_off += type_to_size(IR.args[arg].type);
            new_vec_val[1] = reg_arg_off + 8;
            vec_push(labels_as_offsets, new_vec_val);
        } else {
            reg_alloc(IR.args[arg].label, IR.args[arg].type);
            for (size_t i = 0; i < sizeof(label_reg_tab) / sizeof(label_reg_tab[0]); i++) {
                if (label_reg_tab[i][1] && !strcmp(IR.args[arg].label, label_reg_tab[i][1])) reg_alloc_tab[i][1]++;
            }
        }
    }
    for (size_t s = 0; s < IR.num_statements; s++) {
        update_regalloc();
        disasm_instr(fnbuf, IR.statements[s]);
        // expects result in rax
        instructions_x86_64[IR.statements[s].instruction](IR.statements[s].vals, IR.statements[s].val_types, IR.statements[s], fnbuf); 
    }
    size_t sz = vec_size(used_regs_vec);
    if ((bytes_rip_pad & 0b11111) != 0b10000) bytes_rip_pad += 8;
    if (sz & 1) bytes_rip_pad += 8;
    string_push(fnbuf, "// }\n");
    string_push(fnbuf0, ":\n");
    if (IR.is_variadic) {
        string_push(fnbuf0, "\t // Start pushing all variadic argument registers\n");
        for (ssize_t arg = sizeof(arg_regs) / sizeof(arg_regs[0]) - 1; arg >= 0; arg--)
            string_push_fmt(fnbuf0, "\tpush %s\n", arg_regs[arg]);
        string_push(fnbuf0, "\t // End var args\n");
        bytes_rip_pad += 8;
    }
    string_push(fnbuf0, "\tpush %rbp\n\tmov %rsp, %rbp\n");
    if (bytes_rip_pad)
        string_push_fmt(fnbuf0, "\tsub $%llu, %%rsp\n", bytes_rip_pad);
    for (size_t i = 0; i < sz; i++)
        string_push_fmt(fnbuf0, "\tpush %s // used reg\n", (*used_regs_vec)[i]);
    char **argregs_at = arg_regs;
    for (size_t arg = 0; arg < IR.num_args; arg++) {
        if (IR.args[arg].type_is_struct) {
            AggregateType *aggtype = find_aggtype(IR.args[arg].type_struct, aggregate_types, num_aggregate_types);
            if (aggtype->size_bytes <= 8 || aggtype->size_bytes > 16) 
                argregs_at++;
            else
                argregs_at += 2;
            continue;
        }
        char *reg = label_to_reg(IR.args[arg].label, true);
        if (reg)
            string_push_fmt(fnbuf0, "\tmov %s, %s\n", reg_as_size(*argregs_at, IR.args[arg].type), reg); // TODO: fix with >6 args
        argregs_at++;
    }
    string_push(fnbuf0, structarg_buf->data + 1);
    string_push(fnbuf0, fnbuf->data + 2);
    return fnbuf0;
}

void build_program_x86_64(Function *IR, size_t num_functions, Global *global_vars, size_t num_global_vars, AggregateType *aggtypes, size_t num_aggtypes, FILE *outf) {
    aggregate_types = aggtypes;
    num_aggregate_types = num_aggtypes;
    char* **globals = vec_new(sizeof(char*));
    String* **function_statements = vec_new(sizeof(String**));
    for (size_t f = 0; f < num_functions; f++) {
        if (IR[f].is_global) vec_push(globals, IR[f].name);
        vec_push(function_statements, build_function(IR[f]));
    }
    fprintf(outf, "// Generated by UYB for x86_64\n");
    fprintf(outf, ".data\n");
    for (size_t g = 0; g < num_global_vars; g++) {
        if (global_vars[g].section)
            fprintf(outf, ".section \"%s\"\n", global_vars[g].section);
        fprintf(outf, "%s:\n", global_vars[g].name);
        for (size_t i = 0; i < global_vars[g].num_vals; i++) {
            fprintf(outf, ".align %zu\n", global_vars[g].alignment);
            if (global_vars[g].types[i] == Number)
                fprintf(outf, "\t%s %zu\n", global_sizes[global_vars[g].sizes[i]], global_vars[g].vals[i]);
            else if (global_vars[g].types[i] == StrLit)
                fprintf(outf, "\t.ascii \"%s\"\n", (char*) global_vars[g].vals[i]);
            else {
                printf("Type for global var must either be Number or StrLit.\n");
                exit(1);
            }
        }
        if (global_vars[g].section)
            fprintf(outf, ".data\n");
    }
    fprintf(outf, "\n.text\n");
    for (size_t i = 0; i < vec_size(globals); i++)
        fprintf(outf, ".globl %s\n", (*globals)[i]);
    for (size_t i = 0; i < vec_size(function_statements); i++)
        fprintf(outf, "%s", (*function_statements)[i]->data);
}
