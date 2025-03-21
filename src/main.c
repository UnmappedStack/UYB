/* Main file of UYB for parsing command line arguments and calling the rest of the compiler.
 * Copyright (C) 2025 Jake Steinburger (UnmappedStack) under MPL2.0, see /LICENSE for details. */
#define ARENA_IMPLEMENTATION
#include <arena.h>
#include <stdio.h>
#include <signal.h>
#include <vector.h>
#include <string.h>
#include <api.h>
#include <lexer.h>
#include <parser.h>
#include <arena.h>
#include <version.h>
#include <optimisation.h>

Arena arena;
int is_position_independent = 1;

typedef enum {
    X86_64,
    IR,
} Target;

void (*targets[])(Function*, size_t, Global*, size_t, AggregateType*, size_t, FileDbg*, size_t, FILE*) = {
    build_program_x86_64,
    build_program_IR,
};

void help(char *cmd) {
    printf("%s [options] <inputfile>\n", cmd);
    printf("Options:\n"
           "  --help      Display this information.\n"
           "  --version   Check the version of this copy of UYB.\n"
           "  --targets   List targets supported by UYB which the IR can be compiled to.\n"
           "  --no-pie    Ensure that the generated program is not position independent.\n"
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

void sigsegv_handler(int sig, siginfo_t *si, void *unused) {
    printf(":( Something went very wrong and UYB cannot continue (segmentation fault).\n\n"
           "Please report an issue for the bug on the GitHub repository (https://github.com/UnmappedStack) and describe what you did that caused this.\n"
           "Signal: %d, address: %p\n", sig, si->si_addr);
    exit(1);
}

void setup_sigsev() {
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sigsegv_handler;
    sigaction(SIGSEGV, &sa, NULL);
}

int main(int argc, char **argv) {
    setup_sigsev();
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
        } else if (!strcmp(argv[arg], "-no-pie")) {
            is_position_independent = 0;
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
    FILE *inf = stdin;
    if (input_fname) {
        inf = fopen(input_fname, "r");
        if (!inf) {
            printf("Failed to open %s\n", input_fname);
            return 1;
        }
    }
    Token **toks = lex_file(inf);
    fclose(inf);
    Global **globals;
    AggregateType **aggs;
    FileDbg **files_dbg;
    Function **functs = parse_program(toks, &globals, &aggs, &files_dbg);
    FILE *outf = stdout;
    if (output_fname) {
        outf = fopen(output_fname, "w");
        if (!outf) {
            printf("Failed to open out.S\n");
            exit(1);
        }
    }
    size_t num_functions = vec_size(functs);
    optimise(*functs, num_functions);
    // Assembly codegen
    targets[target](*functs, num_functions, *globals, vec_size(globals), *aggs, vec_size(aggs), *files_dbg, vec_size(files_dbg), outf);
    fclose(outf);
    delete_arenas();
    return 0;
}
