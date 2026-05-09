#include "transpiler_internal.h"
#include "pseudo/parser.h"
#include <tree_sitter/api.h>
#include <string.h>
#include <stdio.h>

// ─── Hashmap helpers (uses string_t* keys/values) ────────────────────────────

void hmap_set_cstr(hashmap_t* map, const char* key, const char* val) {
    string_t* k = string_create_from(key);
    string_t* v = string_create_from(val);
    hashmap_set(map, k, v);
    string_destroy(k);
    string_destroy(v);
}

const char* hmap_get_cstr(hashmap_t* map, const char* key) {
    string_t* k = string_create_from(key);
    string_t* v = hashmap_get(map, k);
    string_destroy(k);
    return v ? string_cstr(v) : NULL;
}

bool hmap_has_cstr(hashmap_t* map, const char* key) {
    string_t* k = string_create_from(key);
    bool r = hashmap_has(map, k);
    string_destroy(k);
    return r;
}

// ─── Pass 1: Variable collection ─────────────────────────────────────────────

static bool node_contains_string(parser_t* parser, TSNode node);

static bool node_contains_string(parser_t* parser, TSNode node) {
    if (ts_node_is_null(node)) return false;
    if (strcmp(ts_node_type(node), NODE_STRING) == 0) return true;
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; i++) {
        if (node_contains_string(parser, ts_node_child(node, i))) return true;
    }
    return false;
}

static bool node_contains_float(parser_t* parser, TSNode node) {
    if (ts_node_is_null(node)) return false;
    if (strcmp(ts_node_type(node), NODE_NUMBER) == 0) {
        string_t* text = parser_node_text(parser, node);
        bool is_float  = strchr(string_cstr(text), '.') != NULL;
        string_destroy(text);
        if (is_float) return true;
    }
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; i++) {
        if (node_contains_float(parser, ts_node_child(node, i))) return true;
    }
    return false;
}

void collect_vars(transpiler_t* ctx, TSNode node) {
    if (ts_node_is_null(node)) return;
    const char* type = ts_node_type(node);

    if (strcmp(type, NODE_ASSIGN) == 0) {
        TSNode name_node  = parser_child_by_field(node, "name");
        TSNode value_node = parser_child_by_field(node, "value");
        string_t* name    = parser_node_text(ctx->parser, name_node);
        const char* n     = string_cstr(name);
        if (!hmap_has_cstr(ctx->var_types, n)) {
            if (node_contains_string(ctx->parser, value_node))
                hmap_set_cstr(ctx->var_types, n, "string");
            else if (node_contains_float(ctx->parser, value_node))
                hmap_set_cstr(ctx->var_types, n, "double");
            else
                hmap_set_cstr(ctx->var_types, n, "int");
        }
        string_destroy(name);
        collect_vars(ctx, value_node);
        return;
    }

    if (strcmp(type, NODE_FOR) == 0) {
        TSNode var_node = parser_child_by_field(node, "var");
        string_t* name  = parser_node_text(ctx->parser, var_node);
        // Loop variables are always int (unconditional override)
        hmap_set_cstr(ctx->var_types, string_cstr(name), "int");
        string_destroy(name);
        // Recurse into body
        uint32_t count = ts_node_child_count(node);
        for (uint32_t i = 0; i < count; i++)
            collect_vars(ctx, ts_node_child(node, i));
        return;
    }

    if (strcmp(type, NODE_READ) == 0) {
        TSNode names_node = parser_child_by_field(node, "names");
        uint32_t count    = ts_node_child_count(names_node);
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(names_node, i);
            if (strcmp(ts_node_type(child), NODE_IDENTIFIER) != 0) continue;
            string_t* name = parser_node_text(ctx->parser, child);
            if (!hmap_has_cstr(ctx->var_types, string_cstr(name)))
                hmap_set_cstr(ctx->var_types, string_cstr(name), "int");
            string_destroy(name);
        }
        return;
    }

    // Generic recursion for all other node types
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; i++)
        collect_vars(ctx, ts_node_child(node, i));
}

// ─── Pass 1.5: Pre-declare Pascal swap temp vars ─────────────────────────────

// For Pascal only: scan for NODE_SWAP and pre-allocate _tN temp names in var_types
// so they appear in the var block. Must run after collect_vars, before emit_preamble.
// tmp_count is then reset to 0 so Pass 2 generates the same names in the same order.
void collect_swap_temps(transpiler_t* ctx, TSNode node) {
    if (ts_node_is_null(node)) return;
    const char* type = ts_node_type(node);

    if (strcmp(type, NODE_SWAP) == 0) {
        TSNode left_node = parser_child_by_field(node, "left");
        string_t* left   = parser_node_text(ctx->parser, left_node);
        const char* t    = hmap_get_cstr(ctx->var_types, string_cstr(left));
        string_destroy(left);
        char tmp[32];
        snprintf(tmp, sizeof(tmp), "_t%d", ctx->tmp_count++);
        hmap_set_cstr(ctx->var_types, tmp, t ? t : "int");
        return;
    }

    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; i++)
        collect_swap_temps(ctx, ts_node_child(node, i));
}

// ─── Type inference for expressions (for printf format) ──────────────────────

var_type_t infer_expr_type(transpiler_t* ctx, TSNode node) {
    if (ts_node_is_null(node)) return VAR_INT;
    const char* type = ts_node_type(node);

    if (strcmp(type, NODE_EXPR) == 0 && ts_node_child_count(node) == 1)
        return infer_expr_type(ctx, ts_node_child(node, 0));
    if (strcmp(type, NODE_ATOM) == 0)
        return infer_expr_type(ctx, ts_node_child(node, 0));
    if (strcmp(type, NODE_PAREN) == 0)
        return infer_expr_type(ctx, ts_node_child(node, 1));

    if (strcmp(type, NODE_STRING) == 0) return VAR_STRING;

    if (strcmp(type, NODE_IDENTIFIER) == 0) {
        string_t* name = parser_node_text(ctx->parser, node);
        const char* t  = hmap_get_cstr(ctx->var_types, string_cstr(name));
        string_destroy(name);
        if (t && strcmp(t, "double") == 0) return VAR_DOUBLE;
        if (t && strcmp(t, "string") == 0) return VAR_STRING;
        return VAR_INT;
    }
    if (strcmp(type, NODE_NUMBER) == 0) {
        string_t* text = parser_node_text(ctx->parser, node);
        bool is_float  = strchr(string_cstr(text), '.') != NULL;
        string_destroy(text);
        return is_float ? VAR_DOUBLE : VAR_INT;
    }
    if (strcmp(type, NODE_ADD_EXPR) == 0 || strcmp(type, NODE_MUL_EXPR) == 0) {
        TSNode left_n  = parser_child_by_field(node, "left");
        TSNode right_n = parser_child_by_field(node, "right");
        TSNode op_n    = parser_child_by_field(node, "op");
        if (!ts_node_is_null(op_n)) {
            string_t* op_text = parser_node_text(ctx->parser, op_n);
            bool is_div = strcmp(string_cstr(op_text), "/") == 0;
            string_destroy(op_text);
            if (is_div) return VAR_DOUBLE;
        }
        var_type_t lt = infer_expr_type(ctx, left_n);
        var_type_t rt = infer_expr_type(ctx, right_n);
        if (lt == VAR_DOUBLE || rt == VAR_DOUBLE) return VAR_DOUBLE;
        if (lt == VAR_STRING || rt == VAR_STRING) return VAR_STRING;
        return VAR_INT;
    }
    if (strcmp(type, NODE_NEG_EXPR) == 0)
        return infer_expr_type(ctx, ts_node_child(node, 1));
    if (strcmp(type, NODE_FLOOR) == 0)   return VAR_INT;
    if (strcmp(type, NODE_SQRT_EXPR) == 0) return VAR_DOUBLE;
    // comparisons, logical ops → int (0/1 in C, boolean in Pascal)
    if (strcmp(type, NODE_COMPARE_EXPR) == 0 || strcmp(type, NODE_AND_EXPR) == 0 ||
        strcmp(type, NODE_OR_EXPR) == 0  || strcmp(type, NODE_NOT_EXPR) == 0)
        return VAR_INT;

    return VAR_INT;
}
