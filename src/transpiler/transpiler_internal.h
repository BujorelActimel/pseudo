#ifndef TRANSPILER_INTERNAL_H
#define TRANSPILER_INTERNAL_H

#include "pseudo/transpiler.h"
#include "pseudo/hashmap.h"
#include "pseudo/string.h"
#include "pseudo/parser.h"
#include <stdbool.h>

typedef enum { VAR_DOUBLE, VAR_INT, VAR_STRING } var_type_t;

typedef struct {
    const char* assign_op;
    const char* eq_op;
    const char* ne_op;
    const char* or_op;
    const char* and_op;
    const char* not_kw;
    const char* sqrt_fn;
    const char* floor_open;
    const char* floor_close;
    bool is_pascal;
    bool is_cpp;
} lang_ops_t;

typedef struct {
    parser_t*        parser;
    transpile_lang_t lang;
    lang_ops_t       ops;
    hashmap_t*       var_types;
    hashmap_t*       declared_vars;
    string_t*        out;
    int              indent;
    int              tmp_count;
} transpiler_t;

// Hashmap helpers (shared)
void hmap_set_cstr(hashmap_t* map, const char* key, const char* val);
const char* hmap_get_cstr(hashmap_t* map, const char* key);
bool hmap_has_cstr(hashmap_t* map, const char* key);

// Variable collection passes (from transpiler_collect.c, used by transpiler.c)
void collect_vars(transpiler_t* ctx, TSNode node);
void collect_swap_temps(transpiler_t* ctx, TSNode node);

// Type inference (from transpiler_collect.c, used by transpiler.c)
var_type_t infer_expr_type(transpiler_t* ctx, TSNode node);

#endif
