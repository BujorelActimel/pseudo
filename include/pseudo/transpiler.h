#ifndef PSEUDO_TRANSPILER_H
#define PSEUDO_TRANSPILER_H

#include "pseudo/parser.h"
#include "pseudo/string.h"

typedef enum {
    TRANSPILE_C,
    TRANSPILE_CPP,
    TRANSPILE_PASCAL,
} transpile_lang_t;

// Parse "c", "cpp", "pascal" → enum. Returns -1 if unrecognized.
int transpile_lang_from_str(const char* lang);

// Transpile an already-parsed source. The parser must be in a valid state.
// Returns a malloc'd NUL-terminated string; caller calls free().
// Returns NULL on allocation failure.
char* transpile(parser_t* parser, transpile_lang_t lang);

// Full pipeline: lint → parse → transpile from raw source.
// Returns a malloc'd NUL-terminated string, or NULL on error.
// On error, if error_out is non-NULL, a malloc'd error message is written there.
char* transpile_source(const char* source, transpile_lang_t lang, char** error_out);

#endif // PSEUDO_TRANSPILER_H
