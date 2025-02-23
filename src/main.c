/* Main file of UYB for parsing command line arguments and calling the rest of the compiler.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#include <stdio.h>
#include <vector.h>
#include <string.h>
#include <api.h>
#include <lexer.h>
#include <parser.h>
#include <arena.h>

void help(char *cmd) {
    printf("%s [options] <inputfile>\n", cmd);
    printf("Options:\n"
           "  --help      Display this information.\n"
           "  --version   Check the version of this copy of UYB.\n"
           "  -o <file>   Specify that the resulting assembly should be outputted to <file>.\n");
}

int main(int argc, char **argv) {
    char *input_fname = NULL;
    char *output_fname = NULL;
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
        } else if (!strcmp(argv[arg], "-version")) {
            printf("UYB compiler backend, rolling release.\n"
                   "Copyright (C) 2025 UnmappedStack (Jake Steinburger) under the Mozilla Public License 2.0.\n");
            return 0;
        } else if (!strcmp(argv[arg], "-help")) {
            help(argv[0]);
            return 0;
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
    Function **functs = parse_program(toks, &globals);
    FILE *outf = fopen(output_fname, "w");
    if (!outf) {
        printf("Failed to open out.S\n");
        exit(1);
    }
    build_program(*functs, vec_size(functs), *globals, vec_size(globals), outf);
    fclose(outf);
    delete_arenas();
    return 0;
}
