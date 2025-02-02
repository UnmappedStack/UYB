#include <api.h>
#include <vector.h>
#include <strslice.h>

void (*instructions[])(uint64_t, uint64_t) = {
    add_build, sub_build, div_build, mul_build,
    copy_build, ret_build,
};

static char *type_as_str(Type type) {
    if (type == Bits8) return "byte";
    else if (type == Bits16) return "word";
    else if (type == Bits32) return "dword";
    else if (type == Bits64) return "qword";
    else return "invalid_type";
}

String *build_function(Function IR) {
    String *fnbuf = string_from("\n");
    string_push_fmt(fnbuf, "// %s %s(", type_as_str(IR.return_type), IR.name);
    for (size_t arg = 0; arg < IR.num_args; arg++) {
        string_push_fmt(fnbuf, "%s %%%s", type_as_str(IR.args[arg].type), IR.args[arg].label);
        if (arg != IR.num_args - 1) string_push(fnbuf, ", ");
    }
    string_push(fnbuf, ") {\n");
    string_push(fnbuf, "\t// Function body\n");
    string_push(fnbuf, "// }\n");
    return fnbuf;
}

void build_program(Function *IR, size_t num_functions) {
    char* **globals = vec_new(sizeof(char*));
    String* ** function_statements = vec_new(sizeof(String**));
    for (size_t f = 0; f < num_functions; f++) {
        if (IR[f].is_global) vec_push(globals, IR[f].name);
        vec_push(function_statements, build_function(IR[f]));
    }
    
    printf("\n.text\n");
    for (size_t i = 0; i < vec_size(globals); i++)
        printf(".globl %s\n", (*globals)[i]);
    for (size_t i = 0; i < vec_size(function_statements); i++)
        printf("%s", (*function_statements)[i]->data);
}
