#include "pseudo/linter.h"
#include "pseudo/parser.h"
#include "pseudo/runtime.h"
#include "pseudo/io.h"
#include "pseudo/string.h"
#include <tree_sitter/api.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char* prog_name) {
    printf("Usage: %s <command> [options]\n\n", prog_name);
    printf("Commands:\n");
    printf("  run <file>     Execute pseudocode file\n");
    printf("  lint <file>    Lint pseudocode file\n");
    printf("  parse <file>   Parse and show syntax tree\n");
    printf("  debug <file>   Debug tree (shows all nodes + ERROR/MISSING)\n");
    printf("  help           Show this help message\n");
    printf("\nExample:\n");
    printf("  %s run program.pseudo\n", prog_name);
    printf("  %s lint input.txt\n", prog_name);
    printf("  %s parse input.txt\n", prog_name);
}

static string_t* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Eroare: Nu se poate deschide fisierul '%s'\n", filename);
        return NULL;
    }

    string_t* content = string_create();
    char buffer[4096];
    size_t bytes_read;

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        string_append_buf(content, buffer, bytes_read);
    }

    fclose(file);
    return content;
}

static int cmd_lint(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Eroare: comanda lint necesita un fisier\n\n");
        print_usage(argv[0]);
        return 1;
    }

    string_t* input = read_file(argv[2]);
    if (!input) {
        return 1;
    }

    string_t* output = lint(input);
    printf("%s", string_cstr(output));

    string_destroy(input);
    string_destroy(output);

    return 0;
}

static int cmd_parse(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Eroare: comanda parse necesita un fisier\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // Read file
    string_t* input = read_file(argv[2]);
    if (!input) {
        return 1;
    }

    // Lint first
    string_t* linted = lint(input);
    string_destroy(input);

    // Parse
    parser_t* parser = parser_create();
    if (!parser) {
        fprintf(stderr, "Eroare: Nu s-a putut crea parser-ul\n");
        string_destroy(linted);
        return 1;
    }

    parser_error_t error = parser_parse(parser, linted);

    if (error.type != PARSER_OK) {
        fprintf(stderr, "Eroare de sintaxa la linia %u, coloana %u:\n\n",
                error.line + 1, error.column + 1);
        if (error.message) {
            fprintf(stderr, "%s\n", string_cstr(error.message));
        }
        parser_error_free(&error);
        parser_destroy(parser);
        string_destroy(linted);
        return 1;
    }

    // Print tree
    string_t* tree_str = parser_pretty_tree(parser);
    printf("%s", string_cstr(tree_str));
    string_destroy(tree_str);

    parser_destroy(parser);
    string_destroy(linted);

    return 0;
}

static int cmd_debug(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Eroare: comanda debug necesita un fisier\n\n");
        print_usage(argv[0]);
        return 1;
    }

    // Read file
    string_t* input = read_file(argv[2]);
    if (!input) {
        return 1;
    }

    // Lint first
    string_t* linted = lint(input);
    string_destroy(input);

    // Parse (ignore errors - we want to see the tree anyway)
    parser_t* parser = parser_create();
    if (!parser) {
        fprintf(stderr, "Eroare: Nu s-a putut crea parser-ul\n");
        string_destroy(linted);
        return 1;
    }

    parser_error_t error = parser_parse(parser, linted);
    parser_error_free(&error);  // Ignore errors

    // Print debug tree
    string_t* tree_str = parser_debug_tree(parser);
    printf("%s", string_cstr(tree_str));
    string_destroy(tree_str);

    parser_destroy(parser);
    string_destroy(linted);

    return 0;
}

static int cmd_run(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Eroare: comanda run necesita un fisier\n\n");
        print_usage(argv[0]);
        return 1;
    }

    string_t* input = read_file(argv[2]);
    if (!input) {
        return 1;
    }

    // Create runtime with stdio backend
    io_t* io = io_stdio_create();
    if (!io) {
        fprintf(stderr, "Eroare: Nu s-a putut crea interfata I/O\n");
        string_destroy(input);
        return 1;
    }

    runtime_t* rt = runtime_create(io);
    if (!rt) {
        fprintf(stderr, "Eroare: Nu s-a putut crea runtime-ul\n");
        io_destroy(io);
        string_destroy(input);
        return 1;
    }

    // Load and run
    if (!runtime_load(rt, string_cstr(input))) {
        fprintf(stderr, "%s\n", runtime_get_error(rt));
        runtime_destroy(rt);
        io_destroy(io);
        string_destroy(input);
        return 1;
    }

    exec_state_t state = runtime_run(rt);

    if (state == EXEC_ERROR) {
        fprintf(stderr, "\nEroare: %s\n", runtime_get_error(rt));
        runtime_destroy(rt);
        io_destroy(io);
        string_destroy(input);
        return 1;
    }

    runtime_destroy(rt);
    io_destroy(io);
    string_destroy(input);

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char* command = argv[1];

    if (strcmp(command, "run") == 0) {
        return cmd_run(argc, argv);
    } else if (strcmp(command, "lint") == 0) {
        return cmd_lint(argc, argv);
    } else if (strcmp(command, "parse") == 0) {
        return cmd_parse(argc, argv);
    } else if (strcmp(command, "debug") == 0) {
        return cmd_debug(argc, argv);
    } else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    } else {
        fprintf(stderr, "Eroare: Comanda necunoscuta '%s'\n\n", command);
        print_usage(argv[0]);
        return 1;
    }
}
