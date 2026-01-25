#include "pseudo/parser.h"
#include "pseudo/parser_errors.h"
#include "pseudo/string.h"
#include <tree_sitter/api.h>
#include <tree_sitter/tree-sitter-pseudo.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct parser {
    TSParser* ts_parser;
    TSTree* tree;
    string_t* source;
};

parser_t* parser_create(void) {
    parser_t* parser = malloc(sizeof(parser_t));
    if (!parser) return NULL;

    parser->ts_parser = ts_parser_new();
    if (!parser->ts_parser) {
        free(parser);
        return NULL;
    }

    const TSLanguage* lang = tree_sitter_pseudo();
    if (!lang) {
        ts_parser_delete(parser->ts_parser);
        free(parser);
        return NULL;
    }

    if (!ts_parser_set_language(parser->ts_parser, lang)) {
        ts_parser_delete(parser->ts_parser);
        free(parser);
        return NULL;
    }

    parser->tree = NULL;
    parser->source = NULL;

    return parser;
}

void parser_destroy(parser_t* parser) {
    if (!parser) return;

    if (parser->tree) {
        ts_tree_delete(parser->tree);
    }
    if (parser->source) {
        string_destroy(parser->source);
    }
    ts_parser_delete(parser->ts_parser);
    free(parser);
}

parser_error_t parser_parse(parser_t* parser, const string_t* source) {
    assert(parser);
    assert(source);

    parser_error_t result = {
        .type = PARSER_OK,
        .line = 0,
        .column = 0,
        .message = NULL
    };

    // Free previous parse
    if (parser->tree) {
        ts_tree_delete(parser->tree);
        parser->tree = NULL;
    }
    if (parser->source) {
        string_destroy(parser->source);
    }

    // Copy source
    parser->source = string_create_from_string(source);
    if (!parser->source) {
        result.type = PARSER_ERR_MEMORY;
        result.message = string_create_from("Nu s-a putut aloca memorie pentru sursa");
        return result;
    }

    // Parse
    const char* src = string_cstr(parser->source);
    size_t len = string_length(parser->source);
    parser->tree = ts_parser_parse_string(parser->ts_parser, NULL, src, len);

    if (!parser->tree) {
        result.type = PARSER_ERR_MEMORY;
        result.message = string_create_from("Nu s-a putut aloca memorie pentru arborele sintactic");
        return result;
    }

    // Check for syntax errors
    TSNode root = ts_tree_root_node(parser->tree);
    error_info_t info = { .found = false };
    find_first_error(root, string_cstr(parser->source), &info);

    if (info.found) {
        result.type = PARSER_ERR_SYNTAX;
        result.line = info.point.row;
        result.column = info.point.column;
        result.message = build_error_message(parser->source, &info);
    }

    return result;
}

void parser_error_free(parser_error_t* error) {
    if (error && error->message) {
        string_destroy(error->message);
        error->message = NULL;
    }
}

TSNode parser_root(parser_t* parser) {
    assert(parser);
    assert(parser->tree);
    return ts_tree_root_node(parser->tree);
}

const string_t* parser_source(parser_t* parser) {
    assert(parser);
    return parser->source;
}

bool parser_has_error(parser_t* parser) {
    assert(parser);
    if (!parser->tree) return true;

    TSNode root = ts_tree_root_node(parser->tree);
    error_info_t info = { .found = false };
    find_first_error(root, string_cstr(parser->source), &info);
    return info.found;
}

string_t* parser_node_text(parser_t* parser, TSNode node) {
    assert(parser);
    assert(parser->source);

    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    const char* src = string_cstr(parser->source);

    return string_create_from_buf(src + start, end - start);
}

// Pretty print helper
static void print_tree_recursive(TSNode node, const char* source, int indent, string_t* out) {
    // Indentation
    for (int i = 0; i < indent; i++) {
        string_append(out, "  ");
    }

    const char* type = ts_node_type(node);
    bool is_named = ts_node_is_named(node);

    if (is_named) {
        string_append(out, "(");
        string_append(out, type);

        // For leaf nodes, show the text
        uint32_t child_count = ts_node_child_count(node);
        if (child_count == 0) {
            uint32_t start = ts_node_start_byte(node);
            uint32_t end = ts_node_end_byte(node);
            if (end > start && end - start < 50) {
                string_append(out, " \"");
                for (uint32_t i = start; i < end; i++) {
                    char c = source[i];
                    if (c == '\n') {
                        string_append(out, "\\n");
                    } else if (c == '"') {
                        string_append(out, "\\\"");
                    } else {
                        string_append_char(out, c);
                    }
                }
                string_append(out, "\"");
            }
            string_append(out, ")\n");
        } else {
            string_append(out, "\n");
            for (uint32_t i = 0; i < child_count; i++) {
                TSNode child = ts_node_child(node, i);
                print_tree_recursive(child, source, indent + 1, out);
            }
            for (int i = 0; i < indent; i++) {
                string_append(out, "  ");
            }
            string_append(out, ")\n");
        }
    } else {
        // Anonymous nodes (keywords, operators) - just recurse into children
        uint32_t child_count = ts_node_child_count(node);
        for (uint32_t i = 0; i < child_count; i++) {
            TSNode child = ts_node_child(node, i);
            print_tree_recursive(child, source, indent, out);
        }
    }
}

string_t* parser_pretty_tree(parser_t* parser) {
    assert(parser);
    assert(parser->tree);
    assert(parser->source);

    string_t* out = string_create();
    TSNode root = ts_tree_root_node(parser->tree);
    print_tree_recursive(root, string_cstr(parser->source), 0, out);
    return out;
}

// Debug tree print - shows ALL nodes including anonymous ones, with ERROR/MISSING markers
static void print_debug_tree_recursive(TSNode node, const char* source, int indent, string_t* out) {
    for (int i = 0; i < indent; i++) {
        string_append(out, "  ");
    }

    const char* type = ts_node_type(node);
    bool is_error = strcmp(type, "ERROR") == 0;
    bool is_missing = ts_node_is_missing(node);

    string_append(out, type);
    if (is_error) string_append(out, " [ERROR]");
    if (is_missing) string_append(out, " [MISSING]");

    // Show position
    TSPoint start = ts_node_start_point(node);
    TSPoint end = ts_node_end_point(node);
    char pos[64];
    snprintf(pos, sizeof(pos), " (%u:%u-%u:%u)", start.row + 1, start.column, end.row + 1, end.column);
    string_append(out, pos);

    // Show text for leaf nodes
    uint32_t child_count = ts_node_child_count(node);
    if (child_count == 0) {
        uint32_t start_byte = ts_node_start_byte(node);
        uint32_t end_byte = ts_node_end_byte(node);
        if (end_byte > start_byte && end_byte - start_byte < 30) {
            string_append(out, " = \"");
            for (uint32_t i = start_byte; i < end_byte; i++) {
                char c = source[i];
                if (c == '\n') string_append(out, "\\n");
                else if (c == '"') string_append(out, "\\\"");
                else string_append_char(out, c);
            }
            string_append(out, "\"");
        }
    }
    string_append(out, "\n");

    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(node, i);
        print_debug_tree_recursive(child, source, indent + 1, out);
    }
}

string_t* parser_debug_tree(parser_t* parser) {
    assert(parser);
    assert(parser->tree);
    assert(parser->source);

    string_t* out = string_create();
    TSNode root = ts_tree_root_node(parser->tree);
    print_debug_tree_recursive(root, string_cstr(parser->source), 0, out);
    return out;
}

// ========== Interpreter Helper Functions ==========

// Node type constants
const char* const NODE_PROGRAM = "program";
const char* const NODE_STMT = "stmt";
const char* const NODE_ASSIGN = "assign";
const char* const NODE_SWAP = "swap";
const char* const NODE_READ = "read";
const char* const NODE_WRITE = "write";
const char* const NODE_IF = "if";
const char* const NODE_FOR = "for";
const char* const NODE_WHILE = "while";
const char* const NODE_DO_WHILE = "do_while";
const char* const NODE_REPEAT = "repeat";
const char* const NODE_MULTI_STMT = "multi_stmt";
const char* const NODE_EXPR = "expr";
const char* const NODE_OR_EXPR = "or_expr";
const char* const NODE_AND_EXPR = "and_expr";
const char* const NODE_NOT_EXPR = "not_expr";
const char* const NODE_COMPARE_EXPR = "compare_expr";
const char* const NODE_ADD_EXPR = "add_expr";
const char* const NODE_MUL_EXPR = "mul_expr";
const char* const NODE_NEG_EXPR = "neg_expr";
const char* const NODE_SQRT_EXPR = "sqrt_expr";
const char* const NODE_FLOOR = "floor";
const char* const NODE_PAREN = "paren";
const char* const NODE_ATOM = "atom";
const char* const NODE_IDENTIFIER = "identifier";
const char* const NODE_NUMBER = "number";
const char* const NODE_STRING = "string";
const char* const NODE_NAME_LIST = "name_list";
const char* const NODE_EXPR_LIST = "expr_list";

// Check if node is a specific type
bool parser_node_is_type(TSNode node, const char* type) {
    return strcmp(ts_node_type(node), type) == 0;
}

// Get child by field name
TSNode parser_child_by_field(TSNode node, const char* field_name) {
    return ts_node_child_by_field_name(node, field_name, strlen(field_name));
}

// Get identifier value
string_t* parser_get_identifier(parser_t* parser, TSNode node) {
    assert(parser);
    assert(parser_node_is_type(node, NODE_IDENTIFIER));
    return parser_node_text(parser, node);
}
