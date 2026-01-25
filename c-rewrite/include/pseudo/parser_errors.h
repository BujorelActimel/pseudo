#ifndef PSEUDO_PARSER_ERRORS_H
#define PSEUDO_PARSER_ERRORS_H

#include "pseudo/string.h"
#include <tree_sitter/api.h>
#include <stdint.h>
#include <stdbool.h>

// Error node information
typedef struct {
    TSNode node;
    TSPoint point;
    bool is_missing;
    bool found;
} error_info_t;

// Find the first error in the tree (MISSING preferred over ERROR)
void find_first_error(TSNode root, const char* source, error_info_t* info);

// Build a detailed Romanian error message from error info
string_t* build_error_message(const string_t* source, error_info_t* info);

// Helper: translate tree-sitter node types to Romanian
const char* translate_node_type(const char* type);

#endif // PSEUDO_PARSER_ERRORS_H
