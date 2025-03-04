#include <api.h>
#include <stdlib.h>

static void build_value(uint64_t val, ValType type, FILE *outf) {
    if      (type == Number) fprintf(outf, "%zu",  val);
    else if (type == BlkLbl) fprintf(outf, "@%s",  (char*) val);
    else if (type == Label ) fprintf(outf, "%%%s", (char*) val);
    else if (type == Str   ) fprintf(outf, "$%s",  (char*) val);
    else if (type == PhiArg) {
        fprintf(outf, "@%s ", ((PhiVal*) val)->blklbl_name);
        build_value(((PhiVal*) val)->val, ((PhiVal*) val)->type, outf);
    }
}

static char size_as_char(Type type) {
    if      (type == Bits8) return 'b';
    else if (type == Bits16) return 'h';
    else if (type == Bits32) return 'w';
    else return 'l';
}

static void add_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "add ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void sub_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "sub ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void div_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "div ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void udiv_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "udiv ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void rem_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "rem ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void urem_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "urem ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void mul_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "mul ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void copy_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "copy ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, "\n");
}

static void ret_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "ret ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, "\n");
}

static void jmp_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "jmp ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, "\n");
}

static void jz_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "jz ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void and_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "and ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void or_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "or ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void blit_build(uint64_t vals[3], ValType types[3], Statement statement, FILE* outf) {
    fprintf(outf, "blit ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, ", ");
    build_value(vals[2], types[2], outf);
    fprintf(outf, "\n");
}

static void jnz_build(uint64_t vals[3], ValType types[3], Statement statement, FILE* outf) {
    fprintf(outf, "jnz ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, ", ");
    build_value(vals[2], types[2], outf);
    fprintf(outf, "\n");
}

static void xor_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "xor ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void shl_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "shl ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void shr_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "shr ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void store_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "store%c ", size_as_char(statement.type));
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void load_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "load%c ", size_as_char(statement.type));
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void neg_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "neg ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, "\n");
}

static void alloc_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "alloc ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, "\n");
}

static void call_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "call ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, "(");
    FunctionArgList *args = (FunctionArgList*) vals[1];
    size_t num_args = args->num_args;
    for (size_t arg = 0; arg < num_args; arg++) {
        fprintf(outf, "%c ", size_as_char(args->arg_sizes[arg]));
        build_value((uint64_t) args->args[arg], args->arg_types[arg], outf);
        if (arg != num_args - 1)
            fprintf(outf, ", ");
    }
    fprintf(outf, ")\n");
}

static void eq_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "ceq ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void ne_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "cne ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void sle_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "csle ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void slt_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "cslt ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void sge_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "csge ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void sgt_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "csgt ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void ule_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "cule ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void ult_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "cult ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void uge_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "cuge ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void ugt_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "cugt ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void ext_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "ext ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, "\n");
}

static void hlt_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "hlt\n");
}

static void blklbl_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "@%s\n", (char*) vals[0]);
}

static void phi_build(uint64_t vals[2], ValType types[2], Statement statement, FILE* outf) {
    fprintf(outf, "phi ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, ", ");
    build_value(vals[1], types[1], outf);
    fprintf(outf, "\n");
}

static void vastart_build(uint64_t vals[2], ValType types[2], Statement statement, FILE *outf) {
    fprintf(outf, "vastart ");
    build_value(vals[0], types[0], outf);
    fprintf(outf, "\n");
}

void (*instructions_IR[])(uint64_t[2], ValType[2], Statement, FILE*) = {
    add_build, sub_build, div_build, mul_build, copy_build, ret_build, call_build, jz_build, 
    neg_build, udiv_build, rem_build, urem_build, and_build, or_build, xor_build, shl_build, shr_build, 
    store_build, load_build, blit_build, alloc_build, eq_build, ne_build, sle_build, slt_build, sge_build, sgt_build, ule_build, ult_build, 
    uge_build, ugt_build, ext_build, hlt_build, blklbl_build, jmp_build, jnz_build, phi_build, vastart_build, 
};
