#include <stdio.h>
#include <string.h>
#include <arena.h>
#include <api.h>
#include <stdlib.h>

// TODO: Move all instances of this to a non-arch specific utils file
static char size_as_char(Type type) {
    if      (type == Bits8) return 'b';
    else if (type == Bits16) return 'h';
    else if (type == Bits32) return 'w';
    else return 'l';
}

static char *get_full_char_str(bool is_struct, Type type, char *type_struct) {
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

void build_function(Function IR, FILE *outf) {
    char *rettype = get_full_char_str(IR.ret_is_struct, IR.return_type, IR.return_struct);
    fprintf(outf, "%sfunction %s $%s(", (IR.is_global) ? "export " : "", rettype, IR.name);
    for (size_t arg = 0; arg < IR.num_args; arg++) {
        char *argtype = get_full_char_str(IR.args[arg].type_is_struct, IR.args[arg].type, IR.args[arg].type_struct);
        fprintf(outf, "%s %%%s", argtype, IR.args[arg].label);
        if (!(arg == IR.num_args - 1 || IR.is_variadic))
            fprintf(outf, ", ");
    }
    if (IR.is_variadic) fprintf(outf, "...");
    fprintf(outf, ") {\n");
    for (size_t s = 0; s < IR.num_statements; s++) {
        if (IR.statements[s].label) {
            fprintf(outf, "\t%%%s =%c ", IR.statements[s].label, size_as_char(IR.statements[s].type));
        } else {
            fprintf(outf, "\t");
        }
        instructions_IR[IR.statements[s].instruction](IR.statements[s].vals, IR.statements[s].val_types, IR.statements[s], outf);
    }
    fprintf(outf, "}\n\n");
}

void build_globals(Global *global_vars, size_t num_global_vars, FILE *outf) {
    for (size_t g = 0; g < num_global_vars; g++) {
        if (global_vars[g].section) {
            fprintf(outf, "section \"%s\"\n", global_vars[g].section);
        }
        fprintf(outf, "data $%s = align %zu {", global_vars[g].name, global_vars[g].alignment);
        for (size_t v = 0; v < global_vars[g].num_vals; v++) {
            fprintf(outf, "%c ", size_as_char(global_vars[g].sizes[v]));
            if (global_vars[g].types[v] == Number)
                fprintf(outf, "%zu", global_vars[g].vals[v]);
            else if (global_vars[g].types[v] == StrLit)
                fprintf(outf, "\"%s\"", (char*) global_vars[g].vals[v]);
            else {
                printf("Type for global var must either be Number or StrLit.\n");
                exit(1);
            }
            if (v != global_vars[g].num_vals - 1)
                fprintf(outf, ", ");
        }
        fprintf(outf, "}\n");
    }
}

void build_aggtypes(AggregateType *aggtypes, size_t num_aggtypes, FILE *outf) {
    for (size_t i = 0; i < num_aggtypes; i++) {
        fprintf(outf, "type :%s = align %zu {", aggtypes[i].name, aggtypes[i].alignment);
        for (size_t val = 0; val < aggtypes[i].num_members; val++) {
            fprintf(outf, "%c%s", 
                    size_as_char(aggtypes[i].types[val]), (val != aggtypes[i].num_members - 1) ? ", " : "");
        }
        fprintf(outf, "}\n");
    }
}

void build_program_IR(Function *IR, size_t num_functions, Global *global_vars, size_t num_global_vars, AggregateType *aggtypes, size_t num_aggtypes, FILE *outf) {
    fprintf(outf, "# Generated by UYB for UYB IR\n\n");
    build_globals(global_vars, num_global_vars, outf);
    build_aggtypes(aggtypes, num_aggtypes, outf);
    for (size_t f = 0; f < num_functions; f++) {
        build_function(IR[f], outf);
    }
}
