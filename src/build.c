/* Main code generation file for UYB x86_64 target.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <api.h>
#include <vector.h>
#include <strslice.h>
#include <register.h>
#include <string.h>

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

void (*instructions[])(uint64_t[2], ValType[2], Statement, String*) = {
    add_build, sub_build, div_build, mul_build,
    copy_build, ret_build, call_build, jz_build, neg_build,
    udiv_build, rem_build, urem_build, and_build, or_build, xor_build,
    shl_build, shr_build, store_build, load_build, blit_build, alloc_build,
    eq_build, ne_build, sle_build, slt_build, sge_build, sgt_build, ule_build, ult_build,
    uge_build, ugt_build, ext_build, hlt_build, blklbl_build, jmp_build, jnz_build, 
};

char *type_as_str(Type type) {
    if (type == Bits8) return "byte";
    else if (type == Bits16) return "word";
    else if (type == Bits32) return "dword";
    else if (type == Bits64) return "qword";
    else return "invalid_type";
}

String *build_function(Function IR) {
    reg_init_fn(IR);
    String *fnbuf0 = string_from("\n");
    string_push_fmt(fnbuf0, "// %s %s(", type_as_str(IR.return_type), IR.name);
    for (size_t arg = 0; arg < IR.num_args; arg++) {
        string_push_fmt(fnbuf0, "%s %%%s", type_as_str(IR.args[arg].type), IR.args[arg].label);
        if (arg != IR.num_args - 1) string_push(fnbuf0, ", ");
    }
    string_push_fmt(fnbuf0, ") {\n%s", IR.name);
    String *fnbuf = string_from(":\n");
    size_t reg_arg_off = 0;
    for (size_t arg = 0; arg < IR.num_args; arg++) {
        if (arg > 5) {
            // it's on the stack
            size_t *new_vec_val = malloc(sizeof(size_t) * 2);
            new_vec_val[0] = (size_t) IR.args[arg].label;
            reg_arg_off += type_to_size(IR.args[arg].type);
            new_vec_val[1] = reg_arg_off + 8;
            vec_push(labels_as_offsets, new_vec_val);
        } else {
            reg_alloc(IR.args[arg].label, IR.args[arg].type);
        }
    }
    for (size_t s = 0; s < IR.num_statements; s++) {
        update_regalloc();
        disasm_instr(fnbuf, IR.statements[s]);
        // expects result in rax
        instructions[IR.statements[s].instruction](IR.statements[s].vals, IR.statements[s].val_types, IR.statements[s], fnbuf); 
    }
    string_push(fnbuf, "// }\n");
    string_push_fmt(fnbuf0, ":\n\tpush %%rbp\n\tmov %rsp, %rbp\n\tsub $%llu, %%rsp\n", bytes_rip_pad);
    size_t sz = vec_size(used_regs_vec);
    for (size_t i = 0; i < sz; i++)
        string_push_fmt(fnbuf0, "\tpush %s // used reg\n", (*used_regs_vec)[i]);
    for (size_t arg = 0; arg < IR.num_args; arg++) {
        char *reg = label_to_reg(IR.args[arg].label, true);
        if (reg)
            string_push_fmt(fnbuf0, "\tmov %s, %s\n", arg_regs[arg], reg); // TODO: fix with >6 args
    }
    string_push(fnbuf0, fnbuf->data + 2);
    return fnbuf0;
}

void build_program(Function *IR, size_t num_functions, Global *global_vars, size_t num_global_vars, FILE *outf) {
    char* **globals = vec_new(sizeof(char*));
    String* ** function_statements = vec_new(sizeof(String**));
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
            if (global_vars[g].types[i] == Number)
                fprintf(outf, "\t%s %zu\n", global_sizes[global_vars[g].sizes[i]], global_vars[g].vals[i]);
            else if (global_vars[g].types[i] == StrLit)
                fprintf(outf, "\t.ascii \"%s\"\n", (char*) global_vars[g].vals[i]);
            else {
                printf("Type for global var must either be Number or StrLit.\n");
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
