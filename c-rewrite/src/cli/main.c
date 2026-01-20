#include "pseudo/linter.h"
#include "pseudo/string.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char* prog_name) {
    printf("Usage: %s <command> [options]\n\n", prog_name);
    printf("Commands:\n");
    printf("  lint <file>    Lint pseudocode file\n");
    printf("  help           Show this help message\n");
    printf("\nExample:\n");
    printf("  %s lint input.txt\n", prog_name);
}

static string_t* read_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
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
        fprintf(stderr, "Error: lint command requires a file argument\n\n");
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

int main(int argc, char** argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char* command = argv[1];

    if (strcmp(command, "lint") == 0) {
        return cmd_lint(argc, argv);
    } else if (strcmp(command, "help") == 0 || strcmp(command, "--help") == 0 || strcmp(command, "-h") == 0) {
        print_usage(argv[0]);
        return 0;
    } else {
        fprintf(stderr, "Error: Unknown command '%s'\n\n", command);
        print_usage(argv[0]);
        return 1;
    }
}
