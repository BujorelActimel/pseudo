#ifndef PSEUDO_PARSER_H
#define PSEUDO_PARSER_H

#include "pseudo/string.h"
#include <tree_sitter/api.h>
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    PARSER_OK,
    PARSER_ERR_MEMORY,    // Memory allocation failed
    PARSER_ERR_SYNTAX,    // Syntax error in source
} parser_error_type_t;

typedef struct {
    parser_error_type_t type;
    uint32_t line;        // 0-indexed line number
    uint32_t column;      // 0-indexed column number
    string_t* message;    // Error message (owned, may be NULL)
} parser_error_t;

typedef struct parser parser_t;

parser_t* parser_create(void);
void parser_destroy(parser_t* parser);

// Parse source code, returns error info
parser_error_t parser_parse(parser_t* parser, const string_t* source);

// Free error message if allocated
void parser_error_free(parser_error_t* error);

// Get root node of parsed tree
TSNode parser_root(parser_t* parser);

// Get source text
const string_t* parser_source(parser_t* parser);

// Check if tree has errors
bool parser_has_error(parser_t* parser);

// Get node text as new string (caller frees)
string_t* parser_node_text(parser_t* parser, TSNode node);

// Get pretty-printed tree (caller frees)
string_t* parser_pretty_tree(parser_t* parser);

// Get debug tree showing all nodes with ERROR/MISSING markers (caller frees)
string_t* parser_debug_tree(parser_t* parser);

// ========== Interpreter Helper Functions ==========

// Node type constants (use these instead of string literals)
extern const char* const NODE_PROGRAM;
extern const char* const NODE_STMT;
extern const char* const NODE_ASSIGN;
extern const char* const NODE_SWAP;
extern const char* const NODE_READ;
extern const char* const NODE_WRITE;
extern const char* const NODE_IF;
extern const char* const NODE_FOR;
extern const char* const NODE_WHILE;
extern const char* const NODE_DO_WHILE;
extern const char* const NODE_REPEAT;
extern const char* const NODE_MULTI_STMT;
extern const char* const NODE_EXPR;
extern const char* const NODE_OR_EXPR;
extern const char* const NODE_AND_EXPR;
extern const char* const NODE_NOT_EXPR;
extern const char* const NODE_COMPARE_EXPR;
extern const char* const NODE_ADD_EXPR;
extern const char* const NODE_MUL_EXPR;
extern const char* const NODE_NEG_EXPR;
extern const char* const NODE_SQRT_EXPR;
extern const char* const NODE_FLOOR;
extern const char* const NODE_PAREN;
extern const char* const NODE_ATOM;
extern const char* const NODE_IDENTIFIER;
extern const char* const NODE_NUMBER;
extern const char* const NODE_STRING;
extern const char* const NODE_NAME_LIST;
extern const char* const NODE_EXPR_LIST;

// Check if node is a specific type
bool parser_node_is_type(TSNode node, const char* type);

// Get child by field name (wrapper for ts_node_child_by_field_name)
TSNode parser_child_by_field(TSNode node, const char* field_name);

// Get identifier value (asserts node type == "identifier")
string_t* parser_get_identifier(parser_t* parser, TSNode node);

#endif // PSEUDO_PARSER_H
