/* Individual instruction implementations for x86_64 target of UYB.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <api.h>
#include <stdio.h>
#include <vector.h>
#include <register.h>
#include <strslice.h>
#include <stdlib.h>
#include <string.h>

char sizes[] = {
    'b', 'w', 'l', 'q'
};

// A quick alternative to reg_as_type since rax is used a lot
char *rax_versions[] = {
    "al", "ax", "eax", "rax"
};

char *instruction_as_str(Instruction instr) {
    if      (instr == ADD   ) return "ADD";
    else if (instr == SUB   ) return "SUB";
    else if (instr == DIV   ) return "DIV";
    else if (instr == MUL   ) return "MUL";
    else if (instr == COPY  ) return "COPY";
    else if (instr == RET   ) return "RET";
    else if (instr == CALL  ) return "CALL";
    else if (instr == JZ    ) return "JZ";
    else if (instr == NEG   ) return "NEG";
    else if (instr == UDIV  ) return "UDIV";
    else if (instr == STORE ) return "STORE";
    else if (instr == LOAD  ) return "LOAD";
    else if (instr == BLIT  ) return "BLIT";
    else if (instr == ALLOC ) return "ALLOC";
    else if (instr == EQ    ) return "EQ";
    else if (instr == NE    ) return "NE";
    else if (instr == SGE   ) return "SGE";
    else if (instr == SGT   ) return "SGT";
    else if (instr == SLE   ) return "SLE";
    else if (instr == SLT   ) return "SLT";
    else if (instr == UGE   ) return "UGE";
    else if (instr == UGT   ) return "UGT";
    else if (instr == ULE   ) return "ULE";
    else if (instr == ULT   ) return "ULT";
    else if (instr == EXT   ) return "EXT";
    else if (instr == HLT   ) return "HLT";
    else if (instr == BLKLBL) return "BLKLBL";
    else if (instr == JMP   ) return "JMP";
    else if (instr == JNZ   ) return "JNZ";
    else if (instr == SHR   ) return "SHR";
    else if (instr == SHL   ) return "SHL";
    else if (instr == AND   ) return "AND";
    else if (instr == OR    ) return "OR";
    else return "Unknown instruction";
}

static void print_val(String *fnbuf, uint64_t val, ValType type) {
    if      (type == Number      ) string_push_fmt(fnbuf, "$%llu", val);
    else if (type == Label       ) string_push_fmt(fnbuf, "%%%s", (char*) val);
    else if (type == Str         ) string_push_fmt(fnbuf, "$%s", (char*) val);
    else if (type == FunctionArgs) string_push_fmt(fnbuf, "(function arguments)");
    else if (type == BlkLbl      ) string_push_fmt(fnbuf, "@%s", (char*) val);
    else {
        printf("Invalid value type\n");
        exit(1);
    }
}

void disasm_instr(String *fnbuf, Statement statement) {
    if (statement.instruction == BLKLBL) return;
    string_push(fnbuf, "\t// ");
    if (statement.label) {
        string_push_fmt(fnbuf, "%%%s =%s ", statement.label, type_as_str(statement.type));
    }
    string_push_fmt(fnbuf, "%s ", instruction_as_str(statement.instruction));
    if (statement.val_types[0] != Empty) print_val(fnbuf, statement.vals[0], statement.val_types[0]);
    if (statement.val_types[1] != Empty) {
        string_push(fnbuf, ", ");
        print_val(fnbuf, statement.vals[1], statement.val_types[1]);
    }
    if (statement.val_types[2] != Empty) {
        string_push(fnbuf, ", ");
        print_val(fnbuf, statement.vals[2], statement.val_types[2]);
    }
    string_push(fnbuf, "\n");
}

void build_value_noresize(ValType type, uint64_t val, bool can_prepend_dollar, String *fnbuf) {
    if (type == Number) string_push_fmt(fnbuf, "$%llu", val);
    else if (type == BlkLbl) string_push_fmt(fnbuf, ".%s", (char*) val);
    else if (type == Label ) string_push_fmt(fnbuf, "%s", label_to_reg_noresize((char*) val, false));
    else if (type == Str   ) string_push_fmt(fnbuf, "%s%s", (can_prepend_dollar) ? "$" : "", (char*) val);
}

void build_value(ValType type, uint64_t val, bool can_prepend_dollar, String *fnbuf) {
    if (type == Number) string_push_fmt(fnbuf, "$%llu", val);
    else if (type == BlkLbl) string_push_fmt(fnbuf, ".%s", (char*) val);
    else if (type == Label ) string_push_fmt(fnbuf, "%s", label_to_reg((char*) val, false));
    else if (type == Str   ) string_push_fmt(fnbuf, "%s%s", (can_prepend_dollar) ? "$" : "", (char*) val);
}

void operation_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf, char *operation) {
    char *label_loc = reg_alloc(statement.label, statement.type);
    if (label_loc[0] != '%') { // label stored in memory address on stack
        string_push_fmt(fnbuf, "\tmov%c ", sizes[statement.type]);
        build_value(types[0], vals[0], true, fnbuf);
        string_push_fmt(fnbuf, ", %%%s\n", rax_versions[statement.type]);
        string_push_fmt(fnbuf, "\t%s ", operation);
        build_value(types[1], vals[1], true, fnbuf);
        string_push_fmt(fnbuf, ", %%%s\n", rax_versions[statement.type]);
        string_push_fmt(fnbuf, "\tmov%c %%%s, %s\n", sizes[statement.type], rax_versions[statement.type], label_loc);
    } else { // stored in register
        string_push_fmt(fnbuf, "\tmov%c ", sizes[statement.type]);
        build_value(types[0], vals[0], true, fnbuf);
        string_push_fmt(fnbuf, ", %s\n", label_loc);
        string_push_fmt(fnbuf, "\t%s%c ", operation, sizes[statement.type]);
        build_value(types[1], vals[1], true, fnbuf);
        string_push_fmt(fnbuf, ", %s\n", label_loc);
    }
}

void add_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    operation_build(vals, types, statement, fnbuf, "add");
}

void sub_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    operation_build(vals, types, statement, fnbuf, "sub");
}

void and_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    operation_build(vals, types, statement, fnbuf, "and");
}

void or_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    operation_build(vals, types, statement, fnbuf, "or");
}

void xor_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    operation_build(vals, types, statement, fnbuf, "xor");
}

void div_both_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf, bool is_signed, bool get_remainder) {
    char *label_loc = reg_alloc(statement.label, statement.type);
    string_push_fmt(fnbuf, "\tmov%c ", sizes[statement.type]);
    build_value(types[0], vals[0], true, fnbuf);
    string_push_fmt(fnbuf, ", %%%s\n"
                       "\txor %rdx, %rdx\n", rax_versions[statement.type]);
    string_push_fmt(fnbuf, "\t%s%c ", (is_signed) ? "idiv" : "div", sizes[statement.type]);
    build_value(types[1], vals[1], true, fnbuf);
    string_push(fnbuf, "\n");
    string_push_fmt(fnbuf, "\tmov %%%s, %s\n", (get_remainder) ? reg_as_size("%rdx", statement.type) : rax_versions[statement.type], label_loc);
}

void div_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    div_both_build(vals, types, statement, fnbuf, true, false);
}

void udiv_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    div_both_build(vals, types, statement, fnbuf, false, false);
}

void rem_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    div_both_build(vals, types, statement, fnbuf, true, true);
}

void urem_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    div_both_build(vals, types, statement, fnbuf, false, true);
}

void mul_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    char *label_loc = reg_alloc(statement.label, statement.type);
    bool is_imm = types[1] == Number || types[1] == Str; 
    if (is_imm) {
        string_push_fmt(fnbuf, "\tmov%c ", sizes[statement.type]);
        build_value(types[1], vals[1], true, fnbuf);
        string_push_fmt(fnbuf, ", %s\n", reg_as_size("%rdi", statement.type)); 
    }
    string_push_fmt(fnbuf, "\tmov%c ", sizes[statement.type]);
    build_value(types[0], vals[0], true, fnbuf);
    string_push_fmt(fnbuf, ", %%%s\n", rax_versions[statement.type]);
    string_push_fmt(fnbuf, "\tmul%c ", sizes[statement.type]);
    if (is_imm)
        string_push_fmt(fnbuf, "%s", reg_as_size("%rdi", statement.type)); 
    else
        build_value(types[1], vals[1], true, fnbuf);
    string_push(fnbuf, "\n");
    string_push_fmt(fnbuf, "\tmov%c %%%s, %s\n", sizes[statement.type], rax_versions[statement.type], label_loc);
}

void copy_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    char *label_loc = reg_alloc(statement.label, statement.type);
    string_push_fmt(fnbuf, "\tmov%c ", sizes[statement.type]);
    build_value(types[0], vals[0], true, fnbuf);
    if (label_loc[0] == '%') // stored in reg
        string_push_fmt(fnbuf, ", %s\n", label_loc);
    else { // stored in memory
        string_push_fmt(fnbuf, ", %%%s\n", rax_versions[statement.type]);
        string_push_fmt(fnbuf, "\tmov%c %%%s, %s\n", sizes[statement.type], rax_versions[statement.type], label_loc);
    }
}

void ret_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    if (types[0] == Empty || (types[0] == Number && !vals[0])) {
        string_push(fnbuf, "\txor %rax, %rax\n");
    } else {
        string_push(fnbuf, "\tmov ");
        build_value_noresize(types[0], vals[0], true, fnbuf);
        string_push(fnbuf, ", %rax\n");
    }
    string_push_fmt(fnbuf, "\tmov %rbp, %rsp\n\tpop %rbp\n\tret\n", bytes_rip_pad);
}

void call_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    size_t pop_bytes = 0;
    if (((FunctionArgList*) vals[1])->num_args > 6 && ((FunctionArgList*) vals[1])->num_args & 1) {
        string_push(fnbuf, "\tsub $8, %rsp\n");
    }
    for (size_t arg = 0; arg < ((FunctionArgList*) vals[1])->num_args; arg++) {
        char *label_loc = label_to_reg_noresize(((FunctionArgList*) vals[1])->args[arg], true);
        if (label_loc && arg < 6  && !strcmp(label_loc, reg_as_size(arg_regs[arg], get_reg_size(label_loc, ((FunctionArgList*) vals[1])->args[arg])))) continue;
        if (arg < 6) {
            if (((FunctionArgList*) vals[1])->arg_types[arg] == Label && label_loc[0] == '%') {
                label_to_reg_noresize(((FunctionArgList*) vals[1])->args[arg], true);
                string_push_fmt(fnbuf, "\tmov%c %s, %s\n", sizes[((FunctionArgList*) vals[1])->arg_sizes[arg]], reg_as_size(label_loc, ((FunctionArgList*) vals[1])->arg_sizes[arg]), reg_as_size(arg_regs[arg], ((FunctionArgList*) vals[1])->arg_sizes[arg]));
            } else {
                string_push_fmt(fnbuf, "\tmov%c ", sizes[((FunctionArgList*) vals[1])->arg_sizes[arg]]);
                build_value(((FunctionArgList*) vals[1])->arg_types[arg], (uint64_t) ((FunctionArgList*) vals[1])->args[arg], true, fnbuf);
                string_push_fmt(fnbuf, ", %s // arg = %zu\n", reg_as_size(arg_regs[arg], ((FunctionArgList*) vals[1])->arg_sizes[arg]), arg);
            }
        } else {
            pop_bytes += 8;
            string_push_fmt(fnbuf, "\tpush ");
            build_value(((FunctionArgList*) vals[1])->arg_types[arg], (uint64_t) ((FunctionArgList*) vals[1])->args[arg], true, fnbuf);
            string_push_fmt(fnbuf, " // arg = %zu\n", arg);
        }
    }
    string_push(fnbuf, "\tcall ");
    build_value(types[0], vals[0], false, fnbuf);
    string_push(fnbuf, "\n");
    if (((FunctionArgList*) vals[1])->num_args > 6 && ((FunctionArgList*) vals[1])->num_args & 1)
        pop_bytes += 8;
    if (pop_bytes)
        string_push_fmt(fnbuf, "\tadd $%zu, %rsp\n", pop_bytes);
    if (statement.label) {
        char *label_loc = reg_alloc(statement.label, statement.type);
        string_push_fmt(fnbuf, "\tmov %%%s, %s\n", rax_versions[statement.type], label_loc);
    }
}

void jz_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    if (types[0] != Label) {
        printf("First value of JZ instruction must be a label.\n");
        exit(1);
    }
    string_push_fmt(fnbuf, "\tcmp $0, %s\n"
                           "\tje ", label_to_reg((char*) vals[0], false));
    build_value(types[1], vals[1], false, fnbuf);
    string_push_fmt(fnbuf, "\n");
}

void jmp_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    string_push_fmt(fnbuf, "\tjmp ");
    build_value(types[0], vals[0], false, fnbuf);
    string_push_fmt(fnbuf, "\n");
}

void jnz_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    if (types[0] != Label) {
        printf("First value of JZ instruction must be a label.\n");
        exit(1);
    }
    if (types[1] == Empty || types[2] == Empty) {
        printf("Expected two labels in JNZ instruction.\n");
        exit(1);
    }
    char *loc = label_to_reg_noresize((char*) vals[0], false);
    Type sz = get_reg_size(loc, (char*) vals[0]);
    string_push_fmt(fnbuf, "\tcmp%c $0, %s\n"
                           "\tjne ", sizes[sz], reg_as_size(loc, sz));
    build_value(types[1], vals[1], false, fnbuf);
    string_push_fmt(fnbuf, "\n\tjmp ");
    build_value(types[2], vals[2], false, fnbuf);
    string_push_fmt(fnbuf, "\n");
}

void neg_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    char *label_loc = reg_alloc(statement.label, statement.type);
    string_push(fnbuf, "\tmov ");
    build_value(types[0], vals[0], true, fnbuf);
    string_push_fmt(fnbuf, ", %s\n"
                           "\tneg%c %s\n", sizes[statement.type], label_loc, label_loc);
}

void shift_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf, char direction) {
    if (types[0] != Label) {
        printf("First value of shift operation must be a label.\n");
        exit(1);
    }
    char *label_loc = reg_alloc(statement.label, statement.type);
    char *first_val = label_to_reg((char*) vals[0], false);
    string_push_fmt(fnbuf, "\tmov%c ", sizes[statement.type]);
    build_value(types[1], vals[1], true, fnbuf);
    string_push_fmt(fnbuf, ", %s\n", reg_as_size("%rcx", statement.type));
    string_push_fmt(fnbuf, "\tsh%c%c %%cl, %s\n"
                       "\tmov %s, %s\n"
                       "\tmov %s, %s\n",
        direction, sizes[statement.type], first_val, 
        first_val, reg_as_size("%rdi", statement.type), 
        reg_as_size("%rdi", statement.type), label_loc);
}

void shl_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    shift_build(vals, types, statement, fnbuf, 'l');
}

void shr_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    shift_build(vals, types, statement, fnbuf, 'r');
}

void store_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    string_push_fmt(fnbuf, "\tmov%c ", sizes[statement.type]);
    build_value(types[0], vals[0], true, fnbuf);
    string_push_fmt(fnbuf, ", %s\n", reg_as_size("%rdi", statement.type));
    string_push_fmt(fnbuf, "\tmov%c %s", sizes[statement.type], reg_as_size("%rdi", statement.type));
    char *reg = label_to_reg((char*) vals[1], false);
    if (reg[0] == '%')
        string_push_fmt(fnbuf, ", (%s) // addr of %s\n", reg, (char*) vals[1]);
    else
        string_push_fmt(fnbuf, ", %s\n", reg);
}

void load_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    char *label_loc = reg_alloc(statement.label, statement.type);
    char *addr = label_to_reg((char*) vals[0], false);
    bool use_brackets = addr[0] == '%';
    if (use_brackets) {
        // is a register that stores the address
        string_push_fmt(fnbuf, "\tmovq (%s), %%rdi\n", addr);
        string_push_fmt(fnbuf, "\tmov%c (%%rdi), %s\n", sizes[statement.type], label_loc);
    } else {
        // address is on the stack
        string_push_fmt(fnbuf, "\tmovq %s, %%rdi\n", addr);
        string_push_fmt(fnbuf, "\tmovq (%rdi), %%rdi\n");
        string_push_fmt(fnbuf, "\tmov%c %s, %s\n", sizes[statement.type], reg_as_size("%rdi", statement.type), label_loc);
    }
}

void blit_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    string_push(fnbuf, "\tmovq ");
    build_value(types[1], vals[1], true, fnbuf);
    string_push(fnbuf, ", %rdi\n");
    string_push(fnbuf, "\tmovq ");
    build_value(types[0], vals[0], true, fnbuf);
    string_push(fnbuf, ", %rsi\n");
    string_push(fnbuf, "\tmovq ");
    build_value(types[2], vals[2], true, fnbuf);
    string_push(fnbuf, ", %rcx\n");
    string_push(fnbuf, "\trep movsb\n");
}

void alloc_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    if (types[0] != Number) {
        printf("ALLOC's argument must be a number literal.\n");
        exit(1);
    }
    char *label_loc = reg_alloc(statement.label, statement.type);
    bytes_rip_pad += 8 * vals[0];
    string_push_fmt(fnbuf, "\tlea -%llu(%rbp), %s\n"
                           "\tmov %s, %s\n",
            bytes_rip_pad, reg_as_size("%rdi", statement.type), reg_as_size("%rdi", statement.type), label_loc);
}

void comparison_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf, char *instr) {
    char *label_loc = reg_alloc_noresize(statement.label, statement.type);
    string_push_fmt(fnbuf, "\tmov %s, %s\n"
                           "\tcmp%c %s, %s\n", reg_as_size(label_to_reg_noresize((char*) vals[1], false), statement.type), reg_as_size("%rdi", statement.type), sizes[statement.type], reg_as_size("%rdi", statement.type), reg_as_size(label_to_reg_noresize((char*) vals[0], false), statement.type));
    if (label_loc[0] == '%') { // label in reg
        char *sized_label = reg_as_size(label_loc, Bits8);
        string_push_fmt(fnbuf, "\t%s %s\n", instr, sized_label);
        string_push_fmt(fnbuf, "\tmovzb%c %s, %s\n", sizes[statement.type], sized_label, reg_as_size(label_loc, statement.type));
    } else { // on stack
        string_push_fmt(fnbuf, "\t%s %%al\n", instr);
        string_push_fmt(fnbuf, "\tmovzb%c %%al, %%%s\n", sizes[statement.type], rax_versions[statement.type]);
        string_push_fmt(fnbuf, "\tmov%c %%%s, %s\n", sizes[statement.type], rax_versions[statement.type], label_loc);
    }
}

void eq_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "sete");
}

void ne_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "setne");
}

void sge_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "setge");
}

void sgt_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "setg");
}

void sle_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "setle");
}

void slt_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "setl");
}

void uge_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "setae");
}

void ugt_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "seta");
}

void ule_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "setbe");
}

void ult_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    comparison_build(vals, types, statement, fnbuf, "setb");
}

void blklbl_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    if (types[0] != Str) {
        printf("Expected label to have value RawStr, got something else instead.\n");
        exit(1);
    }
    string_push_fmt(fnbuf, ".%s:\n", (char*) vals[0]);
}

// second val dictates whether or not it's a signed operation (signed if true).
// TODO: properly handle zero extensions for when it isn't signed
void ext_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    char *label_loc = reg_alloc_noresize(statement.label, statement.type);
    string_push_fmt(fnbuf, "\tmovsx %s, %s\n", label_to_reg((char*) vals[0], false), reg_as_size("%rdx", statement.type));
    string_push_fmt(fnbuf, "\tmov%c %s, %s\n", sizes[statement.type], reg_as_size("%rdx", statement.type), label_loc);
}

void hlt_build(uint64_t vals[2], ValType types[2], Statement statement, String *fnbuf) {
    string_push(fnbuf, "\tjmp .\n");
}
