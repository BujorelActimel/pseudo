#include "pseudo/parser_errors.h"
#include "pseudo/string.h"
#include <tree_sitter/api.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Helper to find MISSING nodes (more specific errors)
static void find_missing_node(TSNode node, error_info_t* info) {
    if (ts_node_is_missing(node)) {
        // Only update if this is the first one or earlier in the file
        if (!info->found) {
            info->node = node;
            info->point = ts_node_start_point(node);
            info->is_missing = true;
            info->found = true;
        }
        return;
    }

    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(node, i);
        find_missing_node(child, info);
        if (info->found) return;
    }
}

// Find position after last stmt in ERROR node (where sf should be)
static TSPoint find_missing_sf_position(TSNode error_node, const char* source) {
    TSPoint best_pos = ts_node_start_point(error_node);
    uint32_t child_count = ts_node_child_count(error_node);

    // Look for altfel followed by stmt, or atunci followed by stmt
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(error_node, i);
        const char* type = ts_node_type(child);

        // After finding altfel or atunci, look for the next stmt
        if (strcmp(type, "altfel") == 0 || strcmp(type, "atunci") == 0) {
            // Find the stmt after this keyword
            for (uint32_t j = i + 1; j < child_count; j++) {
                TSNode next = ts_node_child(error_node, j);
                const char* next_type = ts_node_type(next);
                if (strcmp(next_type, "stmt") == 0) {
                    // Position should be right after this statement
                    best_pos = ts_node_end_point(next);
                    if (strcmp(type, "altfel") == 0) {
                        // This is the best position for sf after altfel
                        return best_pos;
                    }
                    break;
                }
            }
        }
    }

    // Fallback: check if any identifier looks like "pana" - sf should be before it
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(error_node, i);
        const char* type = ts_node_type(child);
        if (strcmp(type, "identifier") == 0) {
            uint32_t start = ts_node_start_byte(child);
            uint32_t end = ts_node_end_byte(child);
            if (end - start == 4 && strncmp(source + start, "pana", 4) == 0) {
                // sf should be before this pana
                return ts_node_start_point(child);
            }
        } else if (strcmp(type, "pana") == 0) {
            return ts_node_start_point(child);
        }
    }

    return best_pos;
}

// Helper to find ERROR nodes (fallback)
static void find_error_node(TSNode node, const char* source, error_info_t* info) {
    if (ts_node_is_error(node)) {
        if (!info->found) {
            info->node = node;
            // Try to find a better position based on content
            info->point = find_missing_sf_position(node, source);
            info->is_missing = false;
            info->found = true;
        }
        return;
    }

    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(node, i);
        find_error_node(child, source, info);
        if (info->found) return;
    }
}

// Find the best error to report - prefer MISSING over ERROR
void find_first_error(TSNode node, const char* source, error_info_t* info) {
    // First try to find a MISSING node anywhere in the tree
    find_missing_node(node, info);
    if (info->found) return;

    // Fall back to ERROR nodes
    find_error_node(node, source, info);
}

// Extract a specific line from source
static string_t* get_source_line(const string_t* source, uint32_t line_num) {
    const char* src = string_cstr(source);
    uint32_t current_line = 0;
    const char* line_start = src;

    // Find the start of the requested line
    while (*src && current_line < line_num) {
        if (*src == '\n') {
            current_line++;
            line_start = src + 1;
        }
        src++;
    }

    // Find the end of the line
    const char* line_end = line_start;
    while (*line_end && *line_end != '\n') {
        line_end++;
    }

    return string_create_from_buf(line_start, line_end - line_start);
}

// Translate node type to Romanian
const char* translate_node_type(const char* type) {
    if (strcmp(type, "stmt") == 0) return "instructiune";
    if (strcmp(type, "expr") == 0) return "expresie";
    if (strcmp(type, "identifier") == 0) return "nume de variabila";
    if (strcmp(type, "number") == 0) return "numar";
    if (strcmp(type, "string") == 0) return "sir de caractere";
    if (strcmp(type, "sf") == 0) return "'sf' (sfarsit bloc)";
    if (strcmp(type, "atunci") == 0) return "'atunci'";
    if (strcmp(type, "executa") == 0) return "'executa'";
    if (strcmp(type, "altfel") == 0) return "'altfel'";
    if (strcmp(type, "pana") == 0) return "'pana cand'";
    if (strcmp(type, "cat") == 0) return "'cat timp'";
    if (strcmp(type, "repeta") == 0) return "'repeta'";
    if (strcmp(type, "pentru") == 0) return "'pentru'";
    if (strcmp(type, "daca") == 0) return "'daca'";
    if (strcmp(type, "assign") == 0) return "atribuire (<-)";
    if (strcmp(type, "read") == 0) return "'citeste'";
    if (strcmp(type, "write") == 0) return "'scrie'";
    return type;
}

// Check if an ERROR node contains a specific keyword
static bool error_contains_keyword(TSNode error_node, const char* source, const char* keyword) {
    uint32_t child_count = ts_node_child_count(error_node);
    for (uint32_t i = 0; i < child_count; i++) {
        TSNode child = ts_node_child(error_node, i);
        const char* type = ts_node_type(child);
        if (strcmp(type, keyword) == 0) {
            return true;
        }
        // Also check anonymous nodes (identifiers that match keyword text)
        if (strcmp(type, "identifier") == 0) {
            uint32_t start = ts_node_start_byte(child);
            uint32_t end = ts_node_end_byte(child);
            size_t len = strlen(keyword);
            if (end - start == len && strncmp(source + start, keyword, len) == 0) {
                return true;
            }
        }
    }
    return false;
}

// Analyze ERROR node content to suggest what's missing
static string_t* analyze_error_content(TSNode error_node, const char* source) {
    string_t* suggestion = string_create();

    bool has_daca = error_contains_keyword(error_node, source, "daca");
    bool has_atunci = error_contains_keyword(error_node, source, "atunci");
    bool has_altfel = error_contains_keyword(error_node, source, "altfel");
    bool has_repeta = error_contains_keyword(error_node, source, "repeta");
    bool has_pana = error_contains_keyword(error_node, source, "pana");
    bool has_pentru = error_contains_keyword(error_node, source, "pentru");
    bool has_executa = error_contains_keyword(error_node, source, "executa");
    bool has_cat = error_contains_keyword(error_node, source, "cat");

    // Check for incomplete structures
    if (has_daca && has_atunci) {
        string_append(suggestion, "Structura 'daca...atunci");
        if (has_altfel) string_append(suggestion, "...altfel");
        string_append(suggestion, "' incompleta - lipseste 'sf' (sfarsit bloc)");
    } else if (has_repeta && has_pana) {
        // repeta...pana cand should have sf before pana cand for nested daca
        string_append(suggestion, "Structura 'repeta...pana cand' incompleta - verificati ca toate blocurile 'daca' au 'sf'");
    } else if (has_repeta) {
        string_append(suggestion, "Structura 'repeta' incompleta - lipseste 'pana cand' sau 'sf' pentru blocuri interioare");
    } else if (has_pentru && has_executa) {
        string_append(suggestion, "Structura 'pentru...executa' incompleta - lipseste 'sf' (sfarsit bloc)");
    } else if (has_cat && has_executa) {
        string_append(suggestion, "Structura 'cat timp...executa' incompleta - lipseste 'sf' (sfarsit bloc)");
    } else if (has_pana) {
        string_append(suggestion, "'pana cand' gasit fara 'repeta' corespunzator sau blocuri interioare neinchise");
    } else {
        string_append(suggestion, "Element neasteptat in cod - verificati structura blocurilor");
    }

    return suggestion;
}

// Build a detailed error message
string_t* build_error_message(const string_t* source, error_info_t* info) {
    string_t* msg = string_create();

    // Error description
    if (info->is_missing) {
        const char* type = ts_node_type(info->node);
        string_append(msg, "Lipseste: ");
        string_append(msg, translate_node_type(type));
    } else {
        // Analyze ERROR node content to provide better suggestions
        string_t* suggestion = analyze_error_content(info->node, string_cstr(source));
        string_append_string(msg, suggestion);
        string_destroy(suggestion);
    }
    string_append(msg, "\n\n");

    // Show the line of code
    string_t* line = get_source_line(source, info->point.row);
    char line_prefix[32];
    snprintf(line_prefix, sizeof(line_prefix), "  %u | ", info->point.row + 1);
    string_append(msg, line_prefix);
    string_append_string(msg, line);
    string_append(msg, "\n");

    // Show the caret pointing to the error
    size_t prefix_len = strlen(line_prefix);
    for (size_t i = 0; i < prefix_len; i++) {
        string_append_char(msg, ' ');
    }
    for (uint32_t i = 0; i < info->point.column; i++) {
        // Preserve tabs for alignment
        char c = string_at(line, i);
        string_append_char(msg, (c == '\t') ? '\t' : ' ');
    }
    string_append(msg, "^");
    string_destroy(line);

    return msg;
}
