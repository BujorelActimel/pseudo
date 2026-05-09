#ifndef PSEUDO_EQUIVALENCE_H
#define PSEUDO_EQUIVALENCE_H

#include <stdint.h>

// Returns a malloc'd JSON array of all loops in source:
//   [{"type":"for","start_line":N,"end_line":N}, ...]
// Lines are 0-indexed. Caller frees with free().
char* equiv_get_all_loops(const char* source);

// Returns a malloc'd string with the full source, with the innermost loop
// containing (line, col) [0-indexed] replaced by an equivalent using
// target_type: "for" | "while" | "do_while" | "repeat".
// On error returns a malloc'd JSON: {"error":"message"}. Caller frees.
char* equiv_convert_loop(const char* source, uint32_t line, uint32_t col,
                         const char* target_type);

#endif // PSEUDO_EQUIVALENCE_H
