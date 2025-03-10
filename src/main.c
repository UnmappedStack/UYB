/* Main file of UYB for parsing command line arguments and calling the rest of the compiler.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <stdio.h>
#include <vector.h>
#include <string.h>
#include <api.h>
#include <lexer.h>
#include <parser.h>
#include <arena.h>
#include <version.h>
#include <optimisation.h>

typedef enum {
    X86_64,
    IR,
} Target;

void (*targets[])(Function*, size_t, Global*, size_t, AggregateType*, size_t, FILE*) = {
    build_program_x86_64,
    build_program_IR,
};

void help(char *cmd) {
    printf("%s [options] <inputfile>\n", cmd);
    printf("Options:\n"
           "  --help      Display this information.\n"
           "  --version   Check the version of this copy of UYB.\n"
           "  --targets   List targets supported by UYB which the IR can be compiled to.\n"
           "  -o <file>   Specify that the resulting assembly should be outputted to <file>.\n"
           "  -t <target> Specify that assembly should be generated specifically for <target>.\n");
}

void targets_help() {
    printf("Use `-t <target_name>` to specify a target. Supported targets:\n");
    printf("  - x86_64\n"
           "  - IR\n");
}

Target str_as_target(char *cmd, char *s) {
    if (!strcmp(s, "x86_64")) return X86_64;
    else if (!strcmp(s, "IR")) return IR;
    else {
        printf("No such target: %s. To list all targets, run:\n"
               "%s --targets\n", s, cmd);
        exit(1);
    }
}

int main(int argc, char **argv) {
    char *input_fname = NULL;
    char *output_fname = NULL;
    Target target = X86_64;
    for (size_t arg = 1; arg < argc; arg++) {
        if (argv[arg][0] != '-') {
            if (input_fname) {
                printf("More than one input file passed, not allowed.\n");
                return 1;
            }
            input_fname = argv[arg];
            continue;
        }
        if (argv[arg][1] == '-') argv[arg]++;
        if (!strcmp(argv[arg], "-o")) {
            if (output_fname) {
                printf("Output file provided more than once, not allowed.\n");
                return 1;
            }
            if (arg == argc - 1) {
                printf("Output file was expected to be provided after -o, got end of command instead.\n");
                return 1;
            }
            output_fname = argv[arg + 1];
            arg++;
            continue;
        } else if (!strcmp(argv[arg], "-t")) {
            if (argc == argc - 1) {
                printf("Target was expected to be provided after -t, got end of command instead.\n");
                return 1;
            }
            target = str_as_target(argv[0], argv[arg + 1]);
            arg++;
        } else if (!strcmp(argv[arg], "-targets")) {
            targets_help();
            return 0;
        } else if (!strcmp(argv[arg], "-version")) {
            printf("UYB compiler backend version beta %s.\n"
                   "Copyright (C) 2025 UnmappedStack (Jake Steinburger) under the Mozilla Public License 2.0.\n", COMMIT);
            return 0;
        } else if (!strcmp(argv[arg], "-help")) {
            help(argv[0]);
            return 0;
        } else {
            printf("Invalid argument: %s\n", argv[arg]);
            help(argv[0]);
        }
    }
    if (!input_fname) {
        printf("No input files provided. Usage:\n");
        help(argv[0]);
        return 1;
    }
    if (!output_fname) {
        output_fname = "out.S";
    }
    FILE *inf = fopen(input_fname, "r");
    if (!inf) {
        printf("Failed to open %s\n", input_fname);
        return 1;
    }
    init_arena();
    Token **toks = lex_file(inf);
    fclose(inf);
    Global **globals;
    AggregateType **aggs;
    Function **functs = parse_program(toks, &globals, &aggs);
    FILE *outf = fopen(output_fname, "w");
    if (!outf) {
        printf("Failed to open out.S\n");
        exit(1);
    }
    size_t num_functions = vec_size(functs);
    optimise(*functs, num_functions);
    // Assembly codegen
    targets[target](*functs, num_functions, *globals, vec_size(globals), *aggs, vec_size(aggs), outf);
    fclose(outf);
    delete_arenas();
    return 0;
}
