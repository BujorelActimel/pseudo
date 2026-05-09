#include "pseudo/equivalence.h"
#include "pseudo/parser.h"
#include "pseudo/linter.h"
#include "pseudo/string.h"
#include <tree_sitter/api.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

// ─── Internal helpers ─────────────────────────────────────────────────────

static char* make_error(const char* msg) {
    size_t len = strlen(msg) + 32;
    char* buf = (char*)malloc(len);
    if (buf) snprintf(buf, len, "{\"error\":\"%s\"}", msg);
    return buf;
}

static bool is_loop_type(const char* type) {
    return strcmp(type, "for") == 0 || strcmp(type, "while") == 0 ||
           strcmp(type, "do_while") == 0 || strcmp(type, "repeat") == 0;
}

// Find the first loop node (DFS order) that starts on target_line (0-indexed).
static TSNode find_loop_at_line(TSNode node, uint32_t target_line) {
    if (is_loop_type(ts_node_type(node))) {
        if (ts_node_start_point(node).row == target_line) return node;
    }
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; i++) {
        TSNode result = find_loop_at_line(ts_node_child(node, i), target_line);
        if (!ts_node_is_null(result)) return result;
    }
    TSNode null_node = {0};
    return null_node;
}

typedef struct { TSNode* nodes; uint32_t count; uint32_t cap; } loop_list_t;

static void loops_push(loop_list_t* l, TSNode node) {
    if (l->count >= l->cap) {
        l->cap = l->cap ? l->cap * 2 : 8;
        l->nodes = (TSNode*)realloc(l->nodes, l->cap * sizeof(TSNode));
    }
    l->nodes[l->count++] = node;
}

static void collect_loops(TSNode node, loop_list_t* list) {
    if (is_loop_type(ts_node_type(node))) loops_push(list, node);
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; i++) collect_loops(ts_node_child(node, i), list);
}

// ─── Indentation detection ────────────────────────────────────────────────

// Extract whitespace before loop_start_byte on its source line → loop indent (LI).
static void get_loop_indent(const char* src, uint32_t loop_start,
                             char* buf, size_t buf_size) {
    uint32_t line_s = loop_start;
    while (line_s > 0 && src[line_s - 1] != '\n') line_s--;
    size_t len = loop_start - line_s;
    if (len >= buf_size) len = buf_size - 1;
    memcpy(buf, src + line_s, len);
    buf[len] = '\0';
}

// Extract indentation of the first non-empty line in body_text → body indent (BI).
static void get_body_indent(const char* body_text, size_t body_len,
                             char* buf, size_t buf_size) {
    size_t i = 0;
    while (i < body_len && body_text[i] == '\n') i++;
    size_t start = i;
    while (i < body_len && (body_text[i] == ' ' || body_text[i] == '\t') &&
           (i - start) < buf_size - 1) {
        buf[i - start] = body_text[i];
        i++;
    }
    buf[i - start] = '\0';
}

// ─── Body range extraction ────────────────────────────────────────────────

// Trim body_end back to just after the last '\n' before the closing keyword,
// so body_text does not include the leading whitespace of the closing keyword's line.
static uint32_t trim_body_end(const char* src, uint32_t body_start, uint32_t body_end) {
    if (body_end <= body_start) return body_end;
    uint32_t pos = body_end;
    while (pos > body_start && src[pos - 1] != '\n') pos--;
    return (pos > body_start) ? pos : body_end;
}

typedef struct {
    uint32_t start_byte; // end of opening keyword token
    uint32_t end_byte;   // start of closing keyword token (untrimmed)
} body_range_t;

static body_range_t get_body_range(TSNode loop_node, const char* open_tok,
                                   const char* close_tok) {
    body_range_t r = {0};
    uint32_t count = ts_node_child_count(loop_node);
    bool past_open = false;

    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(loop_node, i);
        const char* type = ts_node_type(child);

        if (!past_open && strcmp(type, open_tok) == 0) {
            r.start_byte = ts_node_end_byte(child);
            past_open = true;
            continue;
        }
        if (past_open && strcmp(type, close_tok) == 0) {
            r.end_byte = ts_node_start_byte(child);
            break;
        }
    }
    return r;
}

// ─── Body indent helper ───────────────────────────────────────────────────

// Appends body text to r, adding one extra '\t' after each '\n' for non-empty lines.
// Used when the body must be nested one level deeper (e.g. inside a daca guard).
static void append_indented_body(string_t* r, const char* bt, size_t bln) {
    for (size_t i = 0; i < bln; i++) {
        string_append_char(r, bt[i]);
        if (bt[i] == '\n' && i + 1 < bln && bt[i + 1] != '\n') {
            string_append_char(r, '\t');
        }
    }
}

// ─── Conversion helpers ───────────────────────────────────────────────────

static string_t* convert_for(parser_t* parser, TSNode node,
                              const char* target, const char* src) {
    TSNode var_n   = parser_child_by_field(node, "var");
    TSNode start_n = parser_child_by_field(node, "start");
    TSNode end_n   = parser_child_by_field(node, "end");
    TSNode step_n  = parser_child_by_field(node, "step");

    string_t* var_s   = parser_node_text(parser, var_n);
    string_t* start_s = parser_node_text(parser, start_n);
    string_t* end_s   = parser_node_text(parser, end_n);
    string_t* step_s  = ts_node_is_null(step_n)
                       ? string_create_from("1")
                       : parser_node_text(parser, step_n);

    const char* var   = string_cstr(var_s);
    const char* start = string_cstr(start_s);
    const char* end   = string_cstr(end_s);
    const char* step  = string_cstr(step_s);

    bool step_neg   = (step[0] == '-');
    const char* cmp = step_neg ? ">=" : "<=";
    const char* ex  = step_neg ? "<"  : ">";

    body_range_t raw  = get_body_range(node, "executa", "sf");
    uint32_t body_end = trim_body_end(src, raw.start_byte, raw.end_byte);
    const char* bt    = src + raw.start_byte;
    size_t bln        = body_end > raw.start_byte ? body_end - raw.start_byte : 0;

    char li[128] = ""; get_loop_indent(src, ts_node_start_byte(node), li, sizeof(li));
    char bi[128] = ""; get_body_indent(bt, bln, bi, sizeof(bi));

    string_t* r = string_create();

    if (strcmp(target, "while") == 0) {
        string_append(r, var); string_append(r, " <- "); string_append(r, start);
        string_append_char(r, '\n'); string_append(r, li);
        string_append(r, "cat timp "); string_append(r, var);
        string_append_char(r, ' '); string_append(r, cmp); string_append_char(r, ' ');
        string_append(r, end); string_append(r, " executa");
        string_append_buf(r, bt, bln);
        string_append(r, bi);
        string_append(r, var); string_append(r, " <- "); string_append(r, var);
        string_append(r, " + "); string_append(r, step); string_append_char(r, '\n');
        string_append(r, li); string_append(r, "sf");

    } else if (strcmp(target, "do_while") == 0) {
        string_append(r, var); string_append(r, " <- "); string_append(r, start);
        string_append_char(r, '\n'); string_append(r, li);
        string_append(r, "executa");
        string_append_buf(r, bt, bln);
        string_append(r, bi);
        string_append(r, var); string_append(r, " <- "); string_append(r, var);
        string_append(r, " + "); string_append(r, step); string_append_char(r, '\n');
        string_append(r, li);
        string_append(r, "cat timp "); string_append(r, var);
        string_append_char(r, ' '); string_append(r, cmp); string_append_char(r, ' ');
        string_append(r, end);

    } else if (strcmp(target, "repeat") == 0) {
        string_append(r, var); string_append(r, " <- "); string_append(r, start);
        string_append_char(r, '\n'); string_append(r, li);
        string_append(r, "repeta");
        string_append_buf(r, bt, bln);
        string_append(r, bi);
        string_append(r, var); string_append(r, " <- "); string_append(r, var);
        string_append(r, " + "); string_append(r, step); string_append_char(r, '\n');
        string_append(r, li);
        string_append(r, "pana cand "); string_append(r, var);
        string_append_char(r, ' '); string_append(r, ex); string_append_char(r, ' ');
        string_append(r, end);

    } else {
        string_destroy(r); r = NULL;
    }

    string_destroy(var_s); string_destroy(start_s);
    string_destroy(end_s); string_destroy(step_s);
    return r;
}

// WHILE → DO-WHILE or REPEAT:
//   Not directly equivalent — WHILE may execute 0 times, DO-WHILE/REPEAT always
//   execute at least once. Wrap in a guard: daca cond atunci ... sf
//   Body is nested one level deeper, so append_indented_body adds one extra tab.
//
//   Caller uses line_start as the replacement offset, so new_loop must start with
//   the full LI prefix (it replaces the entire loop line, not just the keyword).
static string_t* convert_while(parser_t* parser, TSNode node,
                                const char* target, const char* src) {
    TSNode cond_n = parser_child_by_field(node, "condition");
    string_t* cond_s = parser_node_text(parser, cond_n);
    const char* cond = string_cstr(cond_s);

    body_range_t raw  = get_body_range(node, "executa", "sf");
    uint32_t body_end = trim_body_end(src, raw.start_byte, raw.end_byte);
    const char* bt    = src + raw.start_byte;
    size_t bln        = body_end > raw.start_byte ? body_end - raw.start_byte : 0;

    char li[128] = ""; get_loop_indent(src, ts_node_start_byte(node), li, sizeof(li));

    string_t* r = string_create();

    if (strcmp(target, "do_while") == 0) {
        string_append(r, li);
        string_append(r, "daca "); string_append(r, cond); string_append(r, " atunci\n");
        string_append(r, li); string_append_char(r, '\t');
        string_append(r, "executa");
        append_indented_body(r, bt, bln);
        string_append(r, li); string_append_char(r, '\t');
        string_append(r, "cat timp "); string_append(r, cond); string_append_char(r, '\n');
        string_append(r, li);
        string_append(r, "sf");

    } else if (strcmp(target, "repeat") == 0) {
        string_append(r, li);
        string_append(r, "daca "); string_append(r, cond); string_append(r, " atunci\n");
        string_append(r, li); string_append_char(r, '\t');
        string_append(r, "repeta");
        append_indented_body(r, bt, bln);
        string_append(r, li); string_append_char(r, '\t');
        string_append(r, "pana cand not ("); string_append(r, cond);
        string_append_char(r, ')'); string_append_char(r, '\n');
        string_append(r, li);
        string_append(r, "sf");

    } else {
        string_destroy(r); r = NULL;
    }

    string_destroy(cond_s);
    return r;
}

// DO-WHILE → WHILE:
//   Not directly equivalent — DO-WHILE always executes at least once, WHILE may not.
//   Copy the body once before the WHILE loop (unconditional first execution).
//
//   Caller uses line_start as the replacement offset. bt starts with '\n'; skip it
//   for the first copy so there is no stray blank line between the prefix and the body.
static string_t* convert_do_while(parser_t* parser, TSNode node,
                                   const char* target, const char* src) {
    TSNode cond_n = parser_child_by_field(node, "condition");
    string_t* cond_s = parser_node_text(parser, cond_n);
    const char* cond = string_cstr(cond_s);

    body_range_t raw  = get_body_range(node, "executa", "cat");
    uint32_t body_end = trim_body_end(src, raw.start_byte, raw.end_byte);
    const char* bt    = src + raw.start_byte;
    size_t bln        = body_end > raw.start_byte ? body_end - raw.start_byte : 0;

    char li[128] = ""; get_loop_indent(src, ts_node_start_byte(node), li, sizeof(li));

    string_t* r = string_create();

    if (strcmp(target, "while") == 0) {
        // First copy of body (skip leading '\n' — prefix already ends with '\n')
        const char* bt_body = (bln > 0 && bt[0] == '\n') ? bt + 1 : bt;
        size_t      bln_body = (bln > 0 && bt[0] == '\n') ? bln - 1 : bln;
        string_append_buf(r, bt_body, bln_body);
        string_append(r, li);
        string_append(r, "cat timp "); string_append(r, cond); string_append(r, " executa");
        string_append_buf(r, bt, bln);   // second copy with leading '\n'
        string_append(r, li); string_append(r, "sf");

    } else if (strcmp(target, "repeat") == 0) {
        // DO-WHILE ↔ REPEAT are both "at-least-once" — directly equivalent
        string_append(r, "repeta");
        string_append_buf(r, bt, bln);
        string_append(r, li);
        string_append(r, "pana cand not ("); string_append(r, cond);
        string_append_char(r, ')');

    } else {
        string_destroy(r); r = NULL;
    }

    string_destroy(cond_s);
    return r;
}

// REPEAT → WHILE:
//   Same asymmetry as DO-WHILE → WHILE: copy body once before the loop.
static string_t* convert_repeat(parser_t* parser, TSNode node,
                                 const char* target, const char* src) {
    TSNode cond_n = parser_child_by_field(node, "condition");
    string_t* cond_s = parser_node_text(parser, cond_n);
    const char* cond = string_cstr(cond_s);

    body_range_t raw  = get_body_range(node, "repeta", "pana");
    uint32_t body_end = trim_body_end(src, raw.start_byte, raw.end_byte);
    const char* bt    = src + raw.start_byte;
    size_t bln        = body_end > raw.start_byte ? body_end - raw.start_byte : 0;

    char li[128] = ""; get_loop_indent(src, ts_node_start_byte(node), li, sizeof(li));

    string_t* r = string_create();

    if (strcmp(target, "while") == 0) {
        const char* bt_body = (bln > 0 && bt[0] == '\n') ? bt + 1 : bt;
        size_t      bln_body = (bln > 0 && bt[0] == '\n') ? bln - 1 : bln;
        string_append_buf(r, bt_body, bln_body);
        string_append(r, li);
        string_append(r, "cat timp not ("); string_append(r, cond); string_append(r, ") executa");
        string_append_buf(r, bt, bln);
        string_append(r, li); string_append(r, "sf");

    } else if (strcmp(target, "do_while") == 0) {
        // REPEAT ↔ DO-WHILE are both "at-least-once" — directly equivalent
        string_append(r, "executa");
        string_append_buf(r, bt, bln);
        string_append(r, li);
        string_append(r, "cat timp not ("); string_append(r, cond);
        string_append_char(r, ')');

    } else {
        string_destroy(r); r = NULL;
    }

    string_destroy(cond_s);
    return r;
}

// ─── Parse helper ─────────────────────────────────────────────────────────

static parser_t* parse_linted(const char* source, string_t** linted_out) {
    string_t* raw    = string_create_from(source);
    string_t* linted = lint(raw);
    string_destroy(raw);

    parser_t* parser = parser_create();
    if (!parser) { string_destroy(linted); return NULL; }

    parser_error_t err = parser_parse(parser, linted);
    parser_error_free(&err);

    *linted_out = linted;
    return parser;
}

// ─── Public API ───────────────────────────────────────────────────────────

char* equiv_get_all_loops(const char* source) {
    if (!source) return NULL;

    string_t* linted = NULL;
    parser_t* parser = parse_linted(source, &linted);
    if (!parser) return NULL;

    loop_list_t list = {0};
    collect_loops(parser_root(parser), &list);

    string_t* json = string_create();
    string_append(json, "[");
    for (uint32_t i = 0; i < list.count; i++) {
        TSNode loop     = list.nodes[i];
        TSPoint start   = ts_node_start_point(loop);
        TSPoint end_pt  = ts_node_end_point(loop);
        const char* typ = ts_node_type(loop);

        char buf[256];
        snprintf(buf, sizeof(buf),
                 "%s{\"type\":\"%s\",\"start_line\":%u,\"end_line\":%u}",
                 i > 0 ? "," : "", typ, start.row, end_pt.row);
        string_append(json, buf);
    }
    string_append(json, "]");

    parser_destroy(parser);
    string_destroy(linted);
    free(list.nodes);

    size_t len = string_length(json);
    char* result = (char*)malloc(len + 1);
    if (result) memcpy(result, string_cstr(json), len + 1);
    string_destroy(json);
    return result;
}

char* equiv_convert_loop(const char* source, uint32_t line, uint32_t col,
                         const char* target_type) {
    if (!source || !target_type) return make_error("Argumente lipsa");

    string_t* linted = NULL;
    parser_t* parser = parse_linted(source, &linted);
    if (!parser) return make_error("Initializare parser esuata");

    const char* linted_src = string_cstr(linted);
    (void)col; // col unused: we match by line

    TSNode loop_node = find_loop_at_line(parser_root(parser), line);

    if (ts_node_is_null(loop_node)) {
        parser_destroy(parser); string_destroy(linted);
        return make_error("Nicio bucla la pozitia indicata");
    }

    const char* loop_type = ts_node_type(loop_node);

    if (strcmp(loop_type, target_type) == 0) {
        parser_destroy(parser); string_destroy(linted);
        return make_error("Bucla este deja de tipul ales");
    }

    string_t* new_loop = NULL;
    if      (strcmp(loop_type, "for")      == 0)
        new_loop = convert_for(parser, loop_node, target_type, linted_src);
    else if (strcmp(loop_type, "while")    == 0)
        new_loop = convert_while(parser, loop_node, target_type, linted_src);
    else if (strcmp(loop_type, "do_while") == 0)
        new_loop = convert_do_while(parser, loop_node, target_type, linted_src);
    else if (strcmp(loop_type, "repeat")   == 0)
        new_loop = convert_repeat(parser, loop_node, target_type, linted_src);

    if (!new_loop) {
        parser_destroy(parser); string_destroy(linted);
        return make_error("Conversie nesuportata");
    }

    uint32_t loop_start = ts_node_start_byte(loop_node);
    uint32_t loop_end   = ts_node_end_byte(loop_node);
    size_t   src_len    = strlen(linted_src);

    // Conversions that emit content before the loop keyword (body-copy or if-guard)
    // must replace from the start of the loop's line so the LI whitespace in the
    // source prefix is not duplicated. The new_loop text already includes the LI.
    bool uses_line_start =
        (strcmp(loop_type, "while")    == 0 && strcmp(target_type, "do_while") == 0) ||
        (strcmp(loop_type, "while")    == 0 && strcmp(target_type, "repeat")   == 0) ||
        (strcmp(loop_type, "do_while") == 0 && strcmp(target_type, "while")    == 0) ||
        (strcmp(loop_type, "repeat")   == 0 && strcmp(target_type, "while")    == 0);

    uint32_t replace_start = loop_start;
    if (uses_line_start) {
        while (replace_start > 0 && linted_src[replace_start - 1] != '\n')
            replace_start--;
    }

    string_t* result = string_create();
    string_append_buf(result, linted_src, replace_start);
    string_append_string(result, new_loop);
    if (loop_end < src_len)
        string_append_buf(result, linted_src + loop_end, src_len - loop_end);

    string_destroy(new_loop);
    parser_destroy(parser);
    string_destroy(linted);

    size_t len = string_length(result);
    char* out = (char*)malloc(len + 1);
    if (out) memcpy(out, string_cstr(result), len + 1);
    string_destroy(result);
    return out;
}
