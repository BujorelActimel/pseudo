#include "transpiler_internal.h"
#include "pseudo/linter.h"
#include <tree_sitter/api.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

// ─── Language table ───────────────────────────────────────────────────────────

static lang_ops_t make_ops(transpile_lang_t lang) {
    lang_ops_t o = {0};
    o.sqrt_fn    = "sqrt(";
    switch (lang) {
        case TRANSPILE_C:
            o.assign_op  = "=";
            o.eq_op      = "==";
            o.ne_op      = "!=";
            o.or_op      = "||";
            o.and_op     = "&&";
            o.not_kw     = "!";
            o.floor_open = "(int)(";
            o.floor_close= ")";
            o.is_pascal  = false;
            o.is_cpp     = false;
            break;
        case TRANSPILE_CPP:
            o.assign_op  = "=";
            o.eq_op      = "==";
            o.ne_op      = "!=";
            o.or_op      = "||";
            o.and_op     = "&&";
            o.not_kw     = "!";
            o.floor_open = "(int)(";
            o.floor_close= ")";
            o.is_pascal  = false;
            o.is_cpp     = true;
            break;
        case TRANSPILE_PASCAL:
            o.assign_op  = ":=";
            o.eq_op      = "=";
            o.ne_op      = "<>";
            o.or_op      = "or";
            o.and_op     = "and";
            o.not_kw     = "not ";
            o.floor_open = "trunc(";
            o.floor_close= ")";
            o.is_pascal  = true;
            o.is_cpp     = false;
            break;
    }
    return o;
}

// ─── Emit helpers ─────────────────────────────────────────────────────────────

static void emit(transpiler_t* ctx, const char* text) {
    string_append(ctx->out, text);
}

static void emit_indent(transpiler_t* ctx) {
    for (int i = 0; i < ctx->indent; i++)
        string_append(ctx->out, "    ");
}

static void emit_fmt(transpiler_t* ctx, const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    string_append(ctx->out, buf);
}

// ─── Inline declaration helpers (C/C++ only) ─────────────────────────────────

// Emit the C/C++ type keyword for a variable (no trailing space).
static void emit_var_type(transpiler_t* ctx, const char* var_name) {
    const char* t = hmap_get_cstr(ctx->var_types, var_name);
    if (!t) t = "int";
    if (ctx->ops.is_cpp && strcmp(t, "string") == 0) emit(ctx, "string");
    else if (strcmp(t, "double") == 0)               emit(ctx, "double");
    else if (strcmp(t, "string") == 0)               emit_fmt(ctx, "char %s[256]", var_name); // special: C string includes name
    else                                              emit(ctx, "int");
}

// Emit inline declaration prefix for a C/C++ variable if not yet declared.
// Returns true if the declaration was emitted (caller should NOT emit var name again for C strings).
static bool maybe_declare(transpiler_t* ctx, const char* var_name) {
    if (ctx->ops.is_pascal || !ctx->declared_vars) return false;
    if (hmap_has_cstr(ctx->declared_vars, var_name)) return false;
    hmap_set_cstr(ctx->declared_vars, var_name, "1");
    const char* t = hmap_get_cstr(ctx->var_types, var_name);
    if (!t) t = "int";
    if (!ctx->ops.is_cpp && strcmp(t, "string") == 0) {
        // C char array: special form — caller handles the rest
        emit_fmt(ctx, "char %s[256]", var_name);
        return true;
    }
    if (ctx->ops.is_cpp && strcmp(t, "string") == 0) emit(ctx, "string ");
    else if (strcmp(t, "double") == 0)               emit(ctx, "double ");
    else                                              emit(ctx, "int ");
    return false;
}

// ─── Expression helpers ──────────────────────────────────────────────────────

// Unwrap transparent NODE_EXPR single-child wrappers.
static TSNode unwrap_expr(TSNode node) {
    while (!ts_node_is_null(node) &&
           strcmp(ts_node_type(node), NODE_EXPR) == 0 &&
           ts_node_child_count(node) == 1)
        node = ts_node_child(node, 0);
    return node;
}

// Return true if `node` (possibly wrapped) is the integer literal 1.
static bool node_is_literal_one(transpiler_t* ctx, TSNode node) {
    TSNode cur = unwrap_expr(node);
    if (!ts_node_is_null(cur) && strcmp(ts_node_type(cur), NODE_ATOM) == 0 && ts_node_child_count(cur) > 0)
        cur = ts_node_child(cur, 0);
    if (ts_node_is_null(cur) || strcmp(ts_node_type(cur), NODE_NUMBER) != 0) return false;
    string_t* text = parser_node_text(ctx->parser, cur);
    bool is_one = strcmp(string_cstr(text), "1") == 0;
    string_destroy(text);
    return is_one;
}

// Return true if `node` (possibly wrapped in EXPR/ATOM) is the identifier `var_name`.
static bool node_is_ident(transpiler_t* ctx, TSNode node, const char* var_name) {
    TSNode cur = unwrap_expr(node);
    if (!ts_node_is_null(cur) && strcmp(ts_node_type(cur), NODE_ATOM) == 0 && ts_node_child_count(cur) > 0)
        cur = ts_node_child(cur, 0);
    if (ts_node_is_null(cur) || strcmp(ts_node_type(cur), NODE_IDENTIFIER) != 0) return false;
    string_t* text = parser_node_text(ctx->parser, cur);
    bool match = strcmp(string_cstr(text), var_name) == 0;
    string_destroy(text);
    return match;
}

// ─── Hoisting helpers (C/C++ scope correctness) ──────────────────────────────

// Emit a standalone variable declaration at current indent and mark as declared.
static void emit_var_decl(transpiler_t* ctx, const char* n) {
    const char* t = hmap_get_cstr(ctx->var_types, n);
    if (!t) t = "int";
    emit_indent(ctx);
    if (!ctx->ops.is_cpp && strcmp(t, "string") == 0)
        emit_fmt(ctx, "char %s[256];\n", n);
    else if (ctx->ops.is_cpp && strcmp(t, "string") == 0)
        emit_fmt(ctx, "string %s;\n", n);
    else if (strcmp(t, "double") == 0)
        emit_fmt(ctx, "double %s;\n", n);
    else
        emit_fmt(ctx, "int %s;\n", n);
    hmap_set_cstr(ctx->declared_vars, n, "1");
}

// Scan `node` recursively; emit declarations for any variable first assigned or
// read within it that isn't yet in declared_vars.  Stops at NODE_FOR so the loop
// variable stays in the for-header (for (int i = ...)).  No-op for Pascal.
static void hoist_vars(transpiler_t* ctx, TSNode node) {
    if (ts_node_is_null(node) || ctx->ops.is_pascal || !ctx->declared_vars) return;
    const char* type = ts_node_type(node);

    if (strcmp(type, NODE_ASSIGN) == 0) {
        TSNode name_node = parser_child_by_field(node, "name");
        string_t* name   = parser_node_text(ctx->parser, name_node);
        if (!hmap_has_cstr(ctx->declared_vars, string_cstr(name)))
            emit_var_decl(ctx, string_cstr(name));
        string_destroy(name);
        return;
    }
    if (strcmp(type, NODE_READ) == 0) {
        TSNode names_node = parser_child_by_field(node, "names");
        uint32_t cnt = ts_node_child_count(names_node);
        for (uint32_t i = 0; i < cnt; i++) {
            TSNode child = ts_node_child(names_node, i);
            if (strcmp(ts_node_type(child), NODE_IDENTIFIER) != 0) continue;
            string_t* name = parser_node_text(ctx->parser, child);
            if (!hmap_has_cstr(ctx->declared_vars, string_cstr(name)))
                emit_var_decl(ctx, string_cstr(name));
            string_destroy(name);
        }
        return;
    }
    if (strcmp(type, NODE_FOR) == 0) return; // loop var declared inline in for-header

    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; i++)
        hoist_vars(ctx, ts_node_child(node, i));
}

// ─── Pass 2: Expression emitter ──────────────────────────────────────────────

static void gen_expr(transpiler_t* ctx, TSNode node);

static void gen_atom(transpiler_t* ctx, TSNode atom_node) {
    TSNode child = ts_node_child(atom_node, 0);
    const char* type = ts_node_type(child);
    string_t* text = parser_node_text(ctx->parser, child);
    const char* s  = string_cstr(text);

    if (strcmp(type, NODE_STRING) == 0) {
        size_t len = string_length(text);
        if (ctx->ops.is_pascal) {
            // Emit single-quoted Pascal string; escape embedded single-quotes as ''
            emit(ctx, "'");
            for (size_t i = 1; i + 1 < len; i++) {
                if (s[i] == '\'') emit(ctx, "''");
                else { char c[2] = { s[i], 0 }; emit(ctx, c); }
            }
            emit(ctx, "'");
        } else {
            // C/C++: always emit double-quoted string regardless of source quotes
            emit(ctx, "\"");
            for (size_t i = 1; i + 1 < len; i++) {
                if (s[i] == '"') emit(ctx, "\\\"");
                else { char c[2] = { s[i], 0 }; emit(ctx, c); }
            }
            emit(ctx, "\"");
        }
    } else {
        emit(ctx, s);
    }
    string_destroy(text);
}

static void gen_expr(transpiler_t* ctx, TSNode node) {
    if (ts_node_is_null(node)) return;
    const char* type = ts_node_type(node);

    // Unwrap transparent wrappers
    if (strcmp(type, NODE_EXPR) == 0 && ts_node_child_count(node) == 1) {
        gen_expr(ctx, ts_node_child(node, 0));
        return;
    }
    if (strcmp(type, NODE_ATOM) == 0) {
        gen_atom(ctx, node);
        return;
    }
    if (strcmp(type, NODE_PAREN) == 0) {
        emit(ctx, "(");
        gen_expr(ctx, ts_node_child(node, 1));
        emit(ctx, ")");
        return;
    }

    if (strcmp(type, NODE_OR_EXPR) == 0) {
        if (ctx->ops.is_pascal) {
            // Pascal: and/or have higher precedence than comparison operators,
            // so each operand must be parenthesized.
            emit(ctx, "(");
            gen_expr(ctx, parser_child_by_field(node, "left"));
            emit_fmt(ctx, ") %s (", ctx->ops.or_op);
            gen_expr(ctx, parser_child_by_field(node, "right"));
            emit(ctx, ")");
        } else {
            gen_expr(ctx, parser_child_by_field(node, "left"));
            emit_fmt(ctx, " %s ", ctx->ops.or_op);
            gen_expr(ctx, parser_child_by_field(node, "right"));
        }
        return;
    }
    if (strcmp(type, NODE_AND_EXPR) == 0) {
        if (ctx->ops.is_pascal) {
            emit(ctx, "(");
            gen_expr(ctx, parser_child_by_field(node, "left"));
            emit_fmt(ctx, ") %s (", ctx->ops.and_op);
            gen_expr(ctx, parser_child_by_field(node, "right"));
            emit(ctx, ")");
        } else {
            gen_expr(ctx, parser_child_by_field(node, "left"));
            emit_fmt(ctx, " %s ", ctx->ops.and_op);
            gen_expr(ctx, parser_child_by_field(node, "right"));
        }
        return;
    }
    if (strcmp(type, NODE_NOT_EXPR) == 0) {
        emit(ctx, ctx->ops.not_kw);
        emit(ctx, "(");
        gen_expr(ctx, parser_child_by_field(node, "operand"));
        emit(ctx, ")");
        return;
    }

    if (strcmp(type, NODE_ADD_EXPR) == 0 || strcmp(type, NODE_MUL_EXPR) == 0 ||
        strcmp(type, NODE_COMPARE_EXPR) == 0) {
        TSNode left_node = parser_child_by_field(node, "left");
        TSNode op_node   = parser_child_by_field(node, "op");
        TSNode right_node= parser_child_by_field(node, "right");
        string_t* op_str = parser_node_text(ctx->parser, op_node);
        const char* op   = string_cstr(op_str);

        const char* mapped_op = op;
        if (strcmp(op, "=") == 0)  mapped_op = ctx->ops.eq_op;
        if (strcmp(op, "!=") == 0) mapped_op = ctx->ops.ne_op;

        if (strcmp(op, "%") == 0) {
            if (ctx->ops.is_pascal) {
                gen_expr(ctx, left_node);
                emit(ctx, " mod ");
                gen_expr(ctx, right_node);
            } else {
                var_type_t lt = infer_expr_type(ctx, left_node);
                var_type_t rt = infer_expr_type(ctx, right_node);
                if (lt == VAR_DOUBLE || rt == VAR_DOUBLE) {
                    emit(ctx, "fmod(");
                    gen_expr(ctx, left_node);
                    emit(ctx, ", ");
                    gen_expr(ctx, right_node);
                    emit(ctx, ")");
                } else {
                    gen_expr(ctx, left_node);
                    emit(ctx, " % ");
                    gen_expr(ctx, right_node);
                }
            }
        } else {
            gen_expr(ctx, left_node);
            emit_fmt(ctx, " %s ", mapped_op);
            gen_expr(ctx, right_node);
        }
        string_destroy(op_str);
        return;
    }

    if (strcmp(type, NODE_NEG_EXPR) == 0) {
        emit(ctx, "-");
        gen_atom(ctx, ts_node_child(node, 1));
        return;
    }
    if (strcmp(type, NODE_SQRT_EXPR) == 0) {
        emit(ctx, ctx->ops.sqrt_fn);
        gen_expr(ctx, parser_child_by_field(node, "operand"));
        emit(ctx, ")");
        return;
    }
    if (strcmp(type, NODE_FLOOR) == 0) {
        TSNode inner = parser_child_by_field(node, "operand");
        if (ctx->ops.is_pascal) {
            // Unwrap NODE_EXPR wrappers to find the actual node
            TSNode unwrapped = inner;
            while (strcmp(ts_node_type(unwrapped), NODE_EXPR) == 0 &&
                   ts_node_child_count(unwrapped) == 1)
                unwrapped = ts_node_child(unwrapped, 0);
            // If inner is a division, emit as (left div right)
            bool is_div = false;
            if (strcmp(ts_node_type(unwrapped), NODE_MUL_EXPR) == 0) {
                TSNode op_node = parser_child_by_field(unwrapped, "op");
                if (!ts_node_is_null(op_node)) {
                    string_t* op_text = parser_node_text(ctx->parser, op_node);
                    is_div = strcmp(string_cstr(op_text), "/") == 0;
                    string_destroy(op_text);
                }
            }
            if (is_div) {
                emit(ctx, "(");
                gen_expr(ctx, parser_child_by_field(unwrapped, "left"));
                emit(ctx, " div ");
                gen_expr(ctx, parser_child_by_field(unwrapped, "right"));
                emit(ctx, ")");
            } else {
                emit(ctx, "trunc(");
                gen_expr(ctx, inner);
                emit(ctx, ")");
            }
        } else {
            // For int/int division the (int) cast is redundant — emit (a / b) directly
            TSNode inner_op = unwrap_expr(inner);
            bool int_div = false;
            if (strcmp(ts_node_type(inner_op), NODE_MUL_EXPR) == 0) {
                TSNode op_n = parser_child_by_field(inner_op, "op");
                string_t* op_t = parser_node_text(ctx->parser, op_n);
                if (strcmp(string_cstr(op_t), "/") == 0) {
                    TSNode left  = parser_child_by_field(inner_op, "left");
                    TSNode right = parser_child_by_field(inner_op, "right");
                    int_div = (infer_expr_type(ctx, left)  == VAR_INT &&
                               infer_expr_type(ctx, right) == VAR_INT);
                }
                string_destroy(op_t);
            }
            if (int_div) {
                emit(ctx, "(");
                gen_expr(ctx, inner);
                emit(ctx, ")");
            } else {
                emit(ctx, ctx->ops.floor_open);
                gen_expr(ctx, inner);
                emit(ctx, ctx->ops.floor_close);
            }
        }
        return;
    }

    // Fallback: emit source text
    string_t* text = parser_node_text(ctx->parser, node);
    emit(ctx, string_cstr(text));
    string_destroy(text);
}

// ─── Statement emitter helpers ────────────────────────────────────────────────

static void gen_stmt(transpiler_t* ctx, TSNode node);
static void gen_block(transpiler_t* ctx, TSNode parent);
static void gen_expr(transpiler_t* ctx, TSNode node);

// If value_node matches "var_name OP expr" (or "expr OP var_name" for +/*),
// and the var is already declared, emit "var_name OP= rhs;\n" and return true.
// Also handles "[var_name / int_expr]" → "var_name /= int_expr;\n".
// Caller has already emitted indentation; this function emits the rest of the line.
// Returns false if the pattern doesn't match (caller should emit normally).
static bool try_emit_compound(transpiler_t* ctx, const char* var_name, TSNode value_node) {
    if (ctx->ops.is_pascal || !ctx->declared_vars) return false;
    if (!hmap_has_cstr(ctx->declared_vars, var_name)) return false;

    TSNode val = unwrap_expr(value_node);

    // Pattern: [var / int_expr] → var /= int_expr
    if (strcmp(ts_node_type(val), NODE_FLOOR) == 0) {
        TSNode inner = unwrap_expr(parser_child_by_field(val, "operand"));
        if (strcmp(ts_node_type(inner), NODE_MUL_EXPR) == 0) {
            TSNode op_n = parser_child_by_field(inner, "op");
            string_t* op_t = parser_node_text(ctx->parser, op_n);
            bool is_div = strcmp(string_cstr(op_t), "/") == 0;
            string_destroy(op_t);
            if (is_div) {
                TSNode left  = parser_child_by_field(inner, "left");
                TSNode right = parser_child_by_field(inner, "right");
                if (node_is_ident(ctx, left, var_name) &&
                    infer_expr_type(ctx, left)  == VAR_INT &&
                    infer_expr_type(ctx, right) == VAR_INT) {
                    emit_fmt(ctx, "%s /= ", var_name);
                    gen_expr(ctx, right);
                    emit(ctx, ";\n");
                    return true;
                }
            }
        }
        return false;
    }

    // Pattern: var OP expr  or  expr OP var (for +, *)
    if (strcmp(ts_node_type(val), NODE_ADD_EXPR) != 0 &&
        strcmp(ts_node_type(val), NODE_MUL_EXPR) != 0) return false;

    TSNode left_n  = parser_child_by_field(val, "left");
    TSNode op_n    = parser_child_by_field(val, "op");
    TSNode right_n = parser_child_by_field(val, "right");
    string_t* op_t = parser_node_text(ctx->parser, op_n);
    const char* op = string_cstr(op_t);

    const char* compound   = NULL;
    bool        commutative = false;
    if      (strcmp(op, "+") == 0) { compound = "+="; commutative = true; }
    else if (strcmp(op, "-") == 0) { compound = "-="; }
    else if (strcmp(op, "*") == 0) { compound = "*="; commutative = true; }
    else if (strcmp(op, "%") == 0) {
        var_type_t lt = infer_expr_type(ctx, left_n);
        var_type_t rt = infer_expr_type(ctx, right_n);
        if (lt != VAR_DOUBLE && rt != VAR_DOUBLE) compound = "%=";
    }

    bool result = false;
    if (compound) {
        if (node_is_ident(ctx, left_n, var_name)) {
            bool rhs_one = node_is_literal_one(ctx, right_n);
            if (rhs_one && strcmp(compound, "+=") == 0)
                emit_fmt(ctx, "%s++", var_name);
            else if (rhs_one && strcmp(compound, "-=") == 0)
                emit_fmt(ctx, "%s--", var_name);
            else {
                emit_fmt(ctx, "%s %s ", var_name, compound);
                gen_expr(ctx, right_n);
            }
            emit(ctx, ";\n");
            result = true;
        } else if (commutative && node_is_ident(ctx, right_n, var_name)) {
            if (strcmp(compound, "+=") == 0 && node_is_literal_one(ctx, left_n))
                emit_fmt(ctx, "%s++", var_name);
            else {
                emit_fmt(ctx, "%s %s ", var_name, compound);
                gen_expr(ctx, left_n);
            }
            emit(ctx, ";\n");
            result = true;
        }
    }

    string_destroy(op_t);
    return result;
}

// Get the C type string for a variable
static const char* c_type_for(transpiler_t* ctx, const char* var_name) {
    const char* t = hmap_get_cstr(ctx->var_types, var_name);
    if (!t) return "double";
    if (strcmp(t, "int") == 0) return "int";
    if (strcmp(t, "string") == 0) return ctx->ops.is_cpp ? "string" : "char";
    return "double";
}

static void emit_assign(transpiler_t* ctx, TSNode node) {
    TSNode name_node  = parser_child_by_field(node, "name");
    TSNode value_node = parser_child_by_field(node, "value");
    string_t* name    = parser_node_text(ctx->parser, name_node);
    const char* n     = string_cstr(name);
    emit_indent(ctx);

    // C strings use strcpy, not assignment
    if (!ctx->ops.is_pascal && !ctx->ops.is_cpp) {
        const char* t = hmap_get_cstr(ctx->var_types, n);
        if (t && strcmp(t, "string") == 0) {
            if (!hmap_has_cstr(ctx->declared_vars, n)) {
                emit_fmt(ctx, "char %s[256] = ", n);
                gen_expr(ctx, value_node);
                emit(ctx, ";\n");
                hmap_set_cstr(ctx->declared_vars, n, "1");
            } else {
                emit_fmt(ctx, "strcpy(%s, ", n);
                gen_expr(ctx, value_node);
                emit(ctx, ");\n");
            }
            string_destroy(name);
            return;
        }
    }

    // Try compound assignment (x += y, x *= y, x /= y, etc.) for already-declared vars
    if (try_emit_compound(ctx, n, value_node)) {
        string_destroy(name);
        return;
    }

    bool was_c_str = maybe_declare(ctx, n);
    (void)was_c_str;
    emit(ctx, n);
    emit_fmt(ctx, " %s ", ctx->ops.assign_op);
    gen_expr(ctx, value_node);
    emit(ctx, ";\n");
    string_destroy(name);
}

static void emit_swap(transpiler_t* ctx, TSNode node) {
    TSNode left_node  = parser_child_by_field(node, "left");
    TSNode right_node = parser_child_by_field(node, "right");
    string_t* left    = parser_node_text(ctx->parser, left_node);
    string_t* right   = parser_node_text(ctx->parser, right_node);
    const char* ln    = string_cstr(left);
    const char* rn    = string_cstr(right);
    const char* ttype = c_type_for(ctx, ln);

    char tmp[32];
    snprintf(tmp, sizeof(tmp), "_t%d", ctx->tmp_count++);

    if (ctx->ops.is_pascal) {
        // Temp var is pre-declared in the var block by collect_swap_temps.
        emit_indent(ctx); emit(ctx, "begin\n");
        ctx->indent++;
        emit_indent(ctx); emit_fmt(ctx, "%s := %s;\n", tmp, ln);
        emit_indent(ctx); emit_fmt(ctx, "%s := %s;\n", ln, rn);
        emit_indent(ctx); emit_fmt(ctx, "%s := %s;\n", rn, tmp);
        ctx->indent--;
        emit_indent(ctx); emit(ctx, "end;\n");
    } else {
        // C/C++: inline block
        // For C, use the inferred type; for C++ with string use std::swap
        if (ctx->ops.is_cpp && strcmp(ttype, "string") == 0) {
            emit_indent(ctx);
            emit_fmt(ctx, "swap(%s, %s);\n", ln, rn);
        } else {
            // Determine full type string for char array
            bool is_str = (strcmp(ttype, "char") == 0);
            emit_indent(ctx);
            if (is_str)
                emit_fmt(ctx, "{ char %s[256]; strcpy(%s, %s); strcpy(%s, %s); strcpy(%s, %s); }\n",
                    tmp, tmp, ln, ln, rn, rn, tmp);
            else
                emit_fmt(ctx, "{ %s %s = %s; %s = %s; %s = %s; }\n",
                    ttype, tmp, ln, ln, rn, rn, tmp);
        }
    }
    string_destroy(left);
    string_destroy(right);
}

static void emit_read(transpiler_t* ctx, TSNode node) {
    TSNode names_node = parser_child_by_field(node, "names");
    uint32_t count    = ts_node_child_count(names_node);

    if (ctx->ops.is_pascal) {
        // Pascal: read(a, b, c);
        emit_indent(ctx);
        emit(ctx, "read(");
        bool first = true;
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(names_node, i);
            if (strcmp(ts_node_type(child), NODE_IDENTIFIER) != 0) continue;
            if (!first) emit(ctx, ", ");
            first = false;
            string_t* name = parser_node_text(ctx->parser, child);
            emit(ctx, string_cstr(name));
            string_destroy(name);
        }
        emit(ctx, ");\n");
    } else if (ctx->ops.is_cpp) {
        // C++: declare undeclared vars first, then cin >> a >> b >> c;
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(names_node, i);
            if (strcmp(ts_node_type(child), NODE_IDENTIFIER) != 0) continue;
            string_t* name = parser_node_text(ctx->parser, child);
            const char* n  = string_cstr(name);
            if (ctx->declared_vars && !hmap_has_cstr(ctx->declared_vars, n)) {
                const char* t = hmap_get_cstr(ctx->var_types, n);
                emit_indent(ctx);
                if (t && strcmp(t, "string") == 0) emit_fmt(ctx, "string %s;\n", n);
                else if (t && strcmp(t, "double") == 0) emit_fmt(ctx, "double %s;\n", n);
                else emit_fmt(ctx, "int %s;\n", n);
                hmap_set_cstr(ctx->declared_vars, n, "1");
            }
            string_destroy(name);
        }
        emit_indent(ctx);
        emit(ctx, "cin");
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(names_node, i);
            if (strcmp(ts_node_type(child), NODE_IDENTIFIER) != 0) continue;
            string_t* name = parser_node_text(ctx->parser, child);
            emit_fmt(ctx, " >> %s", string_cstr(name));
            string_destroy(name);
        }
        emit(ctx, ";\n");
    } else {
        // C: one scanf per variable, declare inline if needed
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(names_node, i);
            if (strcmp(ts_node_type(child), NODE_IDENTIFIER) != 0) continue;
            string_t* name = parser_node_text(ctx->parser, child);
            const char* n  = string_cstr(name);
            const char* t  = hmap_get_cstr(ctx->var_types, n);
            bool is_str    = t && strcmp(t, "string") == 0;
            bool is_int    = t && strcmp(t, "int") == 0;
            if (ctx->declared_vars && !hmap_has_cstr(ctx->declared_vars, n)) {
                emit_indent(ctx);
                if (is_str)       emit_fmt(ctx, "char %s[256];\n", n);
                else if (is_int)  emit_fmt(ctx, "int %s;\n", n);
                else              emit_fmt(ctx, "double %s;\n", n);
                hmap_set_cstr(ctx->declared_vars, n, "1");
            }
            emit_indent(ctx);
            if (is_str)       emit_fmt(ctx, "scanf(\"%%s\", %s);\n", n);
            else if (is_int)  emit_fmt(ctx, "scanf(\"%%d\", &%s);\n", n);
            else              emit_fmt(ctx, "scanf(\"%%lf\", &%s);\n", n);
            string_destroy(name);
        }
    }
}

static void emit_write(transpiler_t* ctx, TSNode node) {
    TSNode expr_list = parser_child_by_field(node, "values");
    uint32_t count   = ts_node_child_count(expr_list);

    if (ctx->ops.is_pascal) {
        emit_indent(ctx);
        emit(ctx, "write(");
        bool first = true;
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(expr_list, i);
            if (strcmp(ts_node_type(child), ",") == 0) continue;
            if (!first) emit(ctx, ", ");
            first = false;
            gen_expr(ctx, child);
        }
        emit(ctx, ");\n");
    } else if (ctx->ops.is_cpp) {
        emit_indent(ctx);
        emit(ctx, "cout");
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(expr_list, i);
            if (strcmp(ts_node_type(child), ",") == 0) continue;
            emit(ctx, " << ");
            gen_expr(ctx, child);
        }
        emit(ctx, ";\n");
    } else {
        // C: one printf per expression
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(expr_list, i);
            if (strcmp(ts_node_type(child), ",") == 0) continue;
            var_type_t t = infer_expr_type(ctx, child);
            emit_indent(ctx);
            if (t == VAR_STRING) {
                // Check if it's a string literal — emit directly
                const char* ctype = ts_node_type(child);
                bool is_atom = strcmp(ctype, NODE_ATOM) == 0 || strcmp(ctype, NODE_EXPR) == 0;
                bool is_literal = false;
                if (is_atom) {
                    TSNode inner = child;
                    while (strcmp(ts_node_type(inner), NODE_ATOM) == 0 || strcmp(ts_node_type(inner), NODE_EXPR) == 0)
                        inner = ts_node_child(inner, 0);
                    is_literal = strcmp(ts_node_type(inner), NODE_STRING) == 0;
                }
                if (is_literal) {
                    emit(ctx, "printf(\"%s\", ");
                    gen_expr(ctx, child);
                    emit(ctx, ");\n");
                } else {
                    emit(ctx, "printf(\"%s\", ");
                    gen_expr(ctx, child);
                    emit(ctx, ");\n");
                }
            } else if (t == VAR_INT) {
                emit(ctx, "printf(\"%d\", ");
                gen_expr(ctx, child);
                emit(ctx, ");\n");
            } else {
                emit(ctx, "printf(\"%.15g\", ");
                gen_expr(ctx, child);
                emit(ctx, ");\n");
            }
        }
    }
}

// Emit the body of a control structure (iterates NODE_STMT children)
static void gen_block(transpiler_t* ctx, TSNode parent) {
    uint32_t count = ts_node_child_count(parent);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(parent, i);
        if (!parser_node_is_type(child, NODE_STMT)) continue;
        TSNode actual = ts_node_child(child, 0);
        gen_stmt(ctx, actual);
    }
}

// Emit the body of a MULTI_STMT (children are direct statements, no NODE_STMT wrapper)
static void gen_multi_block(transpiler_t* ctx, TSNode node) {
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(node, i);
        const char* t = ts_node_type(child);
        if (strcmp(t, ";") == 0) continue;
        if (strcmp(t, NODE_ASSIGN) == 0 || strcmp(t, NODE_SWAP) == 0 ||
            strcmp(t, NODE_READ) == 0  || strcmp(t, NODE_WRITE) == 0) {
            gen_stmt(ctx, child);
        }
    }
}

static void emit_open_block(transpiler_t* ctx) {
    if (ctx->ops.is_pascal) {
        emit(ctx, " begin\n");
    } else {
        emit(ctx, " {\n");
    }
    ctx->indent++;
}

static void emit_close_block(transpiler_t* ctx) {
    ctx->indent--;
    emit_indent(ctx);
    if (ctx->ops.is_pascal) {
        emit(ctx, "end");
    } else {
        emit(ctx, "}");
    }
}

static void gen_stmt(transpiler_t* ctx, TSNode node) {
    if (ts_node_is_null(node)) return;
    const char* type = ts_node_type(node);

    // Unwrap stmt wrapper if encountered
    if (strcmp(type, NODE_STMT) == 0) {
        gen_stmt(ctx, ts_node_child(node, 0));
        return;
    }

    if (strcmp(type, NODE_ASSIGN) == 0) { emit_assign(ctx, node); return; }
    if (strcmp(type, NODE_SWAP) == 0)   { emit_swap(ctx, node);   return; }
    if (strcmp(type, NODE_READ) == 0)   { emit_read(ctx, node);   return; }
    if (strcmp(type, NODE_WRITE) == 0)  { emit_write(ctx, node);  return; }
    if (strcmp(type, NODE_MULTI_STMT) == 0) { gen_multi_block(ctx, node); return; }

    if (strcmp(type, NODE_IF) == 0) {
        hoist_vars(ctx, node);
        TSNode cond = parser_child_by_field(node, "condition");
        emit_indent(ctx);
        if (ctx->ops.is_pascal) {
            emit(ctx, "if ");
            gen_expr(ctx, cond);
            emit(ctx, " then");
        } else {
            emit(ctx, "if (");
            gen_expr(ctx, cond);
            emit(ctx, ")");
        }
        emit_open_block(ctx);

        // Walk children: emit then-branch, handle altfel, stop at sf
        uint32_t count = ts_node_child_count(node);
        bool in_else = false;
        bool has_else = false;
        // First pass: check if there's an else branch
        for (uint32_t i = 0; i < count; i++) {
            if (strcmp(ts_node_type(ts_node_child(node, i)), "altfel") == 0) { has_else = true; break; }
        }

        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(node, i);
            const char* ct = ts_node_type(child);
            if (strcmp(ct, "altfel") == 0) {
                in_else = true;
                emit_close_block(ctx);
                if (ctx->ops.is_pascal) emit(ctx, "\n");
                else emit(ctx, "\n");
                emit_indent(ctx);
                emit(ctx, "else");
                emit_open_block(ctx);
                continue;
            }
            if (strcmp(ct, "sf") == 0) break;
            if (!parser_node_is_type(child, NODE_STMT)) continue;
            (void)in_else;
            TSNode actual = ts_node_child(child, 0);
            gen_stmt(ctx, actual);
        }
        emit_close_block(ctx);
        if (ctx->ops.is_pascal) emit(ctx, ";");
        emit(ctx, "\n");
        (void)has_else;
        return;
    }

    if (strcmp(type, NODE_FOR) == 0) {
        TSNode var_node   = parser_child_by_field(node, "var");
        TSNode start_node = parser_child_by_field(node, "start");
        TSNode end_node   = parser_child_by_field(node, "end");
        TSNode step_node  = parser_child_by_field(node, "step");
        string_t* var     = parser_node_text(ctx->parser, var_node);
        const char* vn    = string_cstr(var);

        bool has_step = !ts_node_is_null(step_node);
        // Check if step is a simple literal "1" or "-1"
        bool step_is_one = true;
        bool step_is_neg = false;
        if (has_step) {
            string_t* step_text = parser_node_text(ctx->parser, step_node);
            const char* st = string_cstr(step_text);
            step_is_neg = (strcmp(st, "-1") == 0);
            step_is_one = (strcmp(st, "1") == 0 || strcmp(st, "-1") == 0);
            string_destroy(step_text);
        }

        if (ctx->ops.is_pascal && has_step && !step_is_one) {
            // Pascal can't do arbitrary steps in for — emit while equivalent
            emit_indent(ctx);
            emit_fmt(ctx, "%s := ", vn);
            gen_expr(ctx, start_node);
            emit(ctx, ";\n");
            emit_indent(ctx);
            emit(ctx, "while ");
            emit_fmt(ctx, "%s <= ", vn);
            gen_expr(ctx, end_node);
            emit(ctx, " do");
            emit_open_block(ctx);
            gen_block(ctx, node);
            emit_indent(ctx); emit_fmt(ctx, "%s := %s + ", vn, vn);
            gen_expr(ctx, step_node);
            emit(ctx, ";\n");
            emit_close_block(ctx);
            emit(ctx, ";\n");
        } else if (ctx->ops.is_pascal) {
            emit_indent(ctx);
            emit_fmt(ctx, "for %s := ", vn);
            gen_expr(ctx, start_node);
            emit(ctx, step_is_neg ? " downto " : " to ");
            gen_expr(ctx, end_node);
            emit(ctx, " do");
            emit_open_block(ctx);
            gen_block(ctx, node);
            emit_close_block(ctx);
            emit(ctx, ";\n");
        } else {
            // C / C++
            const char* cmp_op = " <= ";
            if (has_step) {
                string_t* step_text = parser_node_text(ctx->parser, step_node);
                const char* st = string_cstr(step_text);
                if (st[0] == '-') cmp_op = " >= ";
                string_destroy(step_text);
            }
            emit_indent(ctx);
            // Always declare the loop variable in the for header so it is
            // scoped to the loop — multiple for-loops using the same index
            // variable each get their own "for (int i = ...)" declaration.
            if (ctx->declared_vars)
                emit(ctx, "for (int ");
            else
                emit(ctx, "for (");
            emit_fmt(ctx, "%s = ", vn);
            gen_expr(ctx, start_node);
            emit_fmt(ctx, "; %s%s", vn, cmp_op);
            gen_expr(ctx, end_node);
            if (!has_step || (step_is_one && !step_is_neg))
                emit_fmt(ctx, "; %s++", vn);
            else if (step_is_one && step_is_neg)
                emit_fmt(ctx, "; %s--", vn);
            else {
                emit_fmt(ctx, "; %s += ", vn);
                gen_expr(ctx, step_node);
            }
            emit(ctx, ")");
            emit_open_block(ctx);
            gen_block(ctx, node);
            emit_close_block(ctx);
            emit(ctx, "\n");
        }
        string_destroy(var);
        return;
    }

    if (strcmp(type, NODE_WHILE) == 0) {
        hoist_vars(ctx, node);
        TSNode cond = parser_child_by_field(node, "condition");
        emit_indent(ctx);
        if (ctx->ops.is_pascal) {
            emit(ctx, "while ");
            gen_expr(ctx, cond);
            emit(ctx, " do");
        } else {
            emit(ctx, "while (");
            gen_expr(ctx, cond);
            emit(ctx, ")");
        }
        emit_open_block(ctx);
        gen_block(ctx, node);
        emit_close_block(ctx);
        if (ctx->ops.is_pascal) emit(ctx, ";\n");
        else emit(ctx, "\n");
        return;
    }

    if (strcmp(type, NODE_DO_WHILE) == 0) {
        hoist_vars(ctx, node);
        TSNode cond = parser_child_by_field(node, "condition");
        if (ctx->ops.is_pascal) {
            emit_indent(ctx);
            emit(ctx, "repeat\n");
            ctx->indent++;
            gen_block(ctx, node);
            ctx->indent--;
            emit_indent(ctx);
            emit(ctx, "until ");
            emit(ctx, ctx->ops.not_kw);
            emit(ctx, "(");
            gen_expr(ctx, cond);
            emit(ctx, ");\n");
        } else {
            emit_indent(ctx);
            emit(ctx, "do {\n");
            ctx->indent++;
            gen_block(ctx, node);
            ctx->indent--;
            emit_indent(ctx);
            emit(ctx, "} while (");
            gen_expr(ctx, cond);
            emit(ctx, ");\n");
        }
        return;
    }

    if (strcmp(type, NODE_REPEAT) == 0) {
        hoist_vars(ctx, node);
        TSNode cond = parser_child_by_field(node, "condition");
        if (ctx->ops.is_pascal) {
            emit_indent(ctx);
            emit(ctx, "repeat\n");
            ctx->indent++;
            gen_block(ctx, node);
            ctx->indent--;
            emit_indent(ctx);
            emit(ctx, "until ");
            gen_expr(ctx, cond);
            emit(ctx, ";\n");
        } else {
            emit_indent(ctx);
            emit(ctx, "do {\n");
            ctx->indent++;
            gen_block(ctx, node);
            ctx->indent--;
            emit_indent(ctx);
            emit(ctx, "} while (!(");
            gen_expr(ctx, cond);
            emit(ctx, "));\n");
        }
        return;
    }
}

// ─── Pass 2: Preamble / postamble ────────────────────────────────────────────

typedef struct {
    transpiler_t* ctx;
    bool first;
} decl_ctx_t;

static void emit_c_decl(const string_t* key, const string_t* value, void* user_data) {
    transpiler_t* ctx = (transpiler_t*)user_data;
    const char* name  = string_cstr(key);
    const char* type  = string_cstr(value);
    emit_indent(ctx);
    if (strcmp(type, "int") == 0)
        emit_fmt(ctx, "int %s;\n", name);
    else if (strcmp(type, "string") == 0)
        emit_fmt(ctx, "char %s[256];\n", name);
    else
        emit_fmt(ctx, "double %s;\n", name);
}

static void emit_cpp_decl(const string_t* key, const string_t* value, void* user_data) {
    transpiler_t* ctx = (transpiler_t*)user_data;
    const char* name  = string_cstr(key);
    const char* type  = string_cstr(value);
    emit_indent(ctx);
    if (strcmp(type, "int") == 0)
        emit_fmt(ctx, "int %s;\n", name);
    else if (strcmp(type, "string") == 0)
        emit_fmt(ctx, "string %s;\n", name);
    else
        emit_fmt(ctx, "double %s;\n", name);
}

static void emit_pascal_decl(const string_t* key, const string_t* value, void* user_data) {
    transpiler_t* ctx = (transpiler_t*)user_data;
    const char* name  = string_cstr(key);
    const char* type  = string_cstr(value);
    emit_indent(ctx);
    if (strcmp(type, "string") == 0)
        emit_fmt(ctx, "%s: string;\n", name);
    else
        emit_fmt(ctx, "%s: longint;\n", name);
}

static void emit_preamble(transpiler_t* ctx) {
    switch (ctx->lang) {
        case TRANSPILE_C:
            emit(ctx, "#include <stdio.h>\n");
            emit(ctx, "#include <math.h>\n");
            emit(ctx, "\nint main(void) {\n");
            ctx->indent = 1;
            break;
        case TRANSPILE_CPP:
            emit(ctx, "#include <iostream>\n");
            emit(ctx, "#include <cmath>\n");
            emit(ctx, "#include <string>\n");
            emit(ctx, "using namespace std;\n");
            emit(ctx, "\nint main() {\n");
            ctx->indent = 1;
            break;
        case TRANSPILE_PASCAL:
            emit(ctx, "program Generated;\n");
            emit(ctx, "uses Math;\n");
            emit(ctx, "\nvar\n");
            ctx->indent = 1;
            hashmap_foreach(ctx->var_types, emit_pascal_decl, ctx);
            ctx->indent = 0;
            emit(ctx, "\nbegin\n");
            ctx->indent = 1;
            break;
    }
}

static void emit_postamble(transpiler_t* ctx) {
    ctx->indent = 0;
    if (ctx->ops.is_pascal) {
        emit(ctx, "end.\n");
    } else {
        emit(ctx, "    return 0;\n}\n");
    }
}

// ─── Public API ───────────────────────────────────────────────────────────────

int transpile_lang_from_str(const char* lang) {
    if (!lang) return -1;
    if (strcmp(lang, "c") == 0)      return TRANSPILE_C;
    if (strcmp(lang, "cpp") == 0)    return TRANSPILE_CPP;
    if (strcmp(lang, "pascal") == 0) return TRANSPILE_PASCAL;
    return -1;
}

char* transpile(parser_t* parser, transpile_lang_t lang) {
    transpiler_t ctx = {0};
    ctx.parser        = parser;
    ctx.lang          = lang;
    ctx.ops           = make_ops(lang);
    ctx.var_types     = hashmap_create(16);
    ctx.declared_vars = make_ops(lang).is_pascal ? NULL : hashmap_create(16);
    ctx.out           = string_create_with_capacity(4096);
    ctx.indent        = 0;
    ctx.tmp_count     = 0;

    TSNode root = parser_root(parser);

    // Pass 1: collect variable declarations
    collect_vars(&ctx, root);

    // Pass 1.5 (Pascal only): pre-declare swap temp vars so they appear in the var block
    if (ctx.ops.is_pascal) {
        collect_swap_temps(&ctx, root);
        ctx.tmp_count = 0;  // Reset so Pass 2 generates the same names in the same order
    }

    // Pass 2: emit code
    emit_preamble(&ctx);

    // Iterate program-level statements (root has NODE_STMT children)
    uint32_t count = ts_node_child_count(root);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(root, i);
        if (!parser_node_is_type(child, NODE_STMT)) continue;
        TSNode actual = ts_node_child(child, 0);
        gen_stmt(&ctx, actual);
    }

    emit_postamble(&ctx);

    // Convert to malloc'd char*
    size_t len = string_length(ctx.out);
    char* result = malloc(len + 1);
    if (result) memcpy(result, string_cstr(ctx.out), len + 1);

    hashmap_destroy(ctx.var_types);
    if (ctx.declared_vars) hashmap_destroy(ctx.declared_vars);
    string_destroy(ctx.out);

    return result;
}

char* transpile_source(const char* source, transpile_lang_t lang, char** error_out) {
    if (!source) return NULL;

    string_t* src    = string_create_from(source);
    string_t* linted = lint(src);
    string_destroy(src);

    parser_t* parser = parser_create();
    if (!parser) {
        string_destroy(linted);
        if (error_out) *error_out = strdup("Nu s-a putut crea parser-ul");
        return NULL;
    }

    parser_error_t err = parser_parse(parser, linted);
    string_destroy(linted);

    if (err.type != PARSER_OK) {
        if (error_out && err.message) {
            *error_out = strdup(string_cstr(err.message));
        } else if (error_out) {
            *error_out = strdup("Eroare de sintaxa");
        }
        parser_error_free(&err);
        parser_destroy(parser);
        return NULL;
    }
    parser_error_free(&err);

    char* result = transpile(parser, lang);
    parser_destroy(parser);
    return result;
}
