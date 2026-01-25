#include "pseudo/runtime.h"
#include "pseudo/parser.h"
#include "pseudo/environment.h"
#include "pseudo/value.h"
#include "pseudo/linter.h"
#include "pseudo/string.h"
#include <tree_sitter/api.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

struct runtime {
    parser_t* parser;
    io_t* io;
    environment_t* env;
    
    exec_state_t state;
    string_t* error_msg;
    
    TSNode program_root;
    uint32_t next_stmt_index;
};

// Forward declarations
static value_t* eval_expr(runtime_t* rt, TSNode expr_node);
static void exec_stmt(runtime_t* rt, TSNode stmt_node);

// === Lifecycle ===

runtime_t* runtime_create(io_t* io) {
    runtime_t* rt = calloc(1, sizeof(runtime_t));
    if (!rt) return NULL;
    
    rt->parser = parser_create();
    if (!rt->parser) {
        free(rt);
        return NULL;
    }
    
    rt->env = env_create();
    if (!rt->env) {
        parser_destroy(rt->parser);
        free(rt);
        return NULL;
    }
    
    rt->io = io;
    rt->state = EXEC_DONE;
    
    return rt;
}

void runtime_destroy(runtime_t* rt) {
    if (!rt) return;
    
    parser_destroy(rt->parser);
    env_destroy(rt->env);
    if (rt->error_msg) string_destroy(rt->error_msg);
    free(rt);
}

// === Loading ===

bool runtime_load(runtime_t* rt, const char* source) {
    assert(rt);
    assert(source);
    
    if (rt->error_msg) {
        string_destroy(rt->error_msg);
        rt->error_msg = NULL;
    }
    
    string_t* source_str = string_create_from(source);
    string_t* linted = lint(source_str);
    string_destroy(source_str);
    
    parser_error_t err = parser_parse(rt->parser, linted);
    string_destroy(linted);
    
    if (err.type != PARSER_OK) {
        rt->error_msg = err.message;
        rt->state = EXEC_ERROR;
        return false;
    }
    
    rt->program_root = parser_root(rt->parser);
    rt->next_stmt_index = 0;
    rt->state = EXEC_CONTINUE;
    env_clear(rt->env);
    
    return true;
}

// === Expression evaluation ===

static value_t* eval_atom(runtime_t* rt, TSNode atom_node) {
    assert(parser_node_is_type(atom_node, NODE_ATOM));
    
    TSNode child = ts_node_child(atom_node, 0);
    const char* type = ts_node_type(child);
    
    if (strcmp(type, NODE_IDENTIFIER) == 0) {
        string_t* name = parser_node_text(rt->parser, child);
        value_t* val = env_get(rt->env, name);
        if (!val) {
            val = value_create_int(0);
            env_set(rt->env, name, val);
        }
        value_t* copy = value_clone(val);
        string_destroy(name);
        return copy;
    }
    
    if (strcmp(type, NODE_NUMBER) == 0) {
        string_t* text = parser_node_text(rt->parser, child);
        const char* str = string_cstr(text);
        
        if (strchr(str, '.')) {
            double val = strtod(str, NULL);
            string_destroy(text);
            return value_create_float(val);
        } else {
            long long val = strtoll(str, NULL, 10);
            string_destroy(text);
            return value_create_int(val);
        }
    }
    
    if (strcmp(type, NODE_STRING) == 0) {
        string_t* text = parser_node_text(rt->parser, child);
        const char* str = string_cstr(text);
        size_t len = string_length(text);
        value_t* val = value_create_string_buf(str + 1, len - 2);
        string_destroy(text);
        return val;
    }
    
    return NULL;
}

static value_t* eval_expr(runtime_t* rt, TSNode expr_node) {
    const char* type = ts_node_type(expr_node);
    
    // If it's a generic "expr" node, unwrap to the actual expression inside
    if (strcmp(type, NODE_EXPR) == 0 && ts_node_child_count(expr_node) == 1) {
        TSNode child = ts_node_child(expr_node, 0);
        return eval_expr(rt, child);
    }
    
    if (strcmp(type, NODE_ATOM) == 0) {
        return eval_atom(rt, expr_node);
    }
    
    if (strcmp(type, NODE_PAREN) == 0) {
        TSNode inner = ts_node_child(expr_node, 1);
        return eval_expr(rt, inner);
    }
    
    if (strcmp(type, NODE_FLOOR) == 0) {
        TSNode operand = parser_child_by_field(expr_node, "operand");
        value_t* val = eval_expr(rt, operand);
        value_error_t err = VALUE_OK;
        value_t* result = value_floor(val, &err);
        value_destroy(val);
        if (err != VALUE_OK) {
            rt->state = EXEC_ERROR;
            rt->error_msg = string_create_from(value_error_string(err));
        }
        return result;
    }
    
    if (strcmp(type, NODE_OR_EXPR) == 0) {
        TSNode left = parser_child_by_field(expr_node, "left");
        value_t* left_val = eval_expr(rt, left);
        if (value_to_bool(left_val)) {
            value_destroy(left_val);
            return value_create_int(1);
        }
        value_destroy(left_val);
        TSNode right = parser_child_by_field(expr_node, "right");
        value_t* right_val = eval_expr(rt, right);
        int result = value_to_bool(right_val);
        value_destroy(right_val);
        return value_create_int(result);
    }
    
    if (strcmp(type, NODE_AND_EXPR) == 0) {
        TSNode left = parser_child_by_field(expr_node, "left");
        value_t* left_val = eval_expr(rt, left);
        if (!value_to_bool(left_val)) {
            value_destroy(left_val);
            return value_create_int(0);
        }
        value_destroy(left_val);
        TSNode right = parser_child_by_field(expr_node, "right");
        value_t* right_val = eval_expr(rt, right);
        int result = value_to_bool(right_val);
        value_destroy(right_val);
        return value_create_int(result);
    }
    
    if (strcmp(type, NODE_ADD_EXPR) == 0 || strcmp(type, NODE_MUL_EXPR) == 0 || 
        strcmp(type, NODE_COMPARE_EXPR) == 0) {
        TSNode left = parser_child_by_field(expr_node, "left");
        TSNode op_node = parser_child_by_field(expr_node, "op");
        TSNode right = parser_child_by_field(expr_node, "right");
        
        string_t* op_str = parser_node_text(rt->parser, op_node);
        const char* op = string_cstr(op_str);
        
        value_t* left_val = eval_expr(rt, left);
        value_t* right_val = eval_expr(rt, right);
        value_t* result = NULL;
        value_error_t err = VALUE_OK;
        
        if (strcmp(op, "+") == 0) result = value_add(left_val, right_val, &err);
        else if (strcmp(op, "-") == 0) result = value_sub(left_val, right_val, &err);
        else if (strcmp(op, "*") == 0) result = value_mul(left_val, right_val, &err);
        else if (strcmp(op, "/") == 0) result = value_div(left_val, right_val, &err);
        else if (strcmp(op, "%") == 0) result = value_mod(left_val, right_val, &err);
        else if (strcmp(op, "=") == 0) result = value_eq(left_val, right_val, &err);
        else if (strcmp(op, "!=") == 0) result = value_ne(left_val, right_val, &err);
        else if (strcmp(op, "<") == 0) result = value_lt(left_val, right_val, &err);
        else if (strcmp(op, "<=") == 0) result = value_le(left_val, right_val, &err);
        else if (strcmp(op, ">") == 0) result = value_gt(left_val, right_val, &err);
        else if (strcmp(op, ">=") == 0) result = value_ge(left_val, right_val, &err);
        
        value_destroy(left_val);
        value_destroy(right_val);
        string_destroy(op_str);
        
        if (err != VALUE_OK) {
            rt->state = EXEC_ERROR;
            rt->error_msg = string_create_from(value_error_string(err));
        }
        
        return result;
    }
    
    if (strcmp(type, NODE_NOT_EXPR) == 0) {
        TSNode operand = parser_child_by_field(expr_node, "operand");
        value_t* val = eval_expr(rt, operand);
        value_t* result = value_not(val);
        value_destroy(val);
        return result;
    }
    
    if (strcmp(type, NODE_NEG_EXPR) == 0) {
        TSNode operand = ts_node_child(expr_node, 1);
        value_t* val = eval_atom(rt, operand);
        value_error_t err = VALUE_OK;
        value_t* result = value_neg(val, &err);
        value_destroy(val);
        if (err != VALUE_OK) {
            rt->state = EXEC_ERROR;
            rt->error_msg = string_create_from(value_error_string(err));
        }
        return result;
    }
    
    if (strcmp(type, NODE_SQRT_EXPR) == 0) {
        TSNode operand = parser_child_by_field(expr_node, "operand");
        value_t* val = eval_atom(rt, operand);
        value_error_t err = VALUE_OK;
        value_t* result = value_sqrt(val, &err);
        value_destroy(val);
        if (err != VALUE_OK) {
            rt->state = EXEC_ERROR;
            rt->error_msg = string_create_from(value_error_string(err));
        }
        return result;
    }
    
    return NULL;
}

// === Statement execution ===

static void exec_assign(runtime_t* rt, TSNode assign_node) {
    TSNode name_node = parser_child_by_field(assign_node, "name");
    TSNode value_node = parser_child_by_field(assign_node, "value");
    
    string_t* name = parser_get_identifier(rt->parser, name_node);
    value_t* val = eval_expr(rt, value_node);
    
    if (val && rt->state != EXEC_ERROR) {
        env_set(rt->env, name, val);
    }
    
    string_destroy(name);
}

static void exec_swap(runtime_t* rt, TSNode swap_node) {
    TSNode left_node = parser_child_by_field(swap_node, "left");
    TSNode right_node = parser_child_by_field(swap_node, "right");
    
    string_t* left_name = parser_get_identifier(rt->parser, left_node);
    string_t* right_name = parser_get_identifier(rt->parser, right_node);
    
    value_t* left_val = env_get(rt->env, left_name);
    value_t* right_val = env_get(rt->env, right_name);
    
    if (!left_val) {
        left_val = value_create_int(0);
        env_set(rt->env, left_name, left_val);
        left_val = env_get(rt->env, left_name);
    }
    if (!right_val) {
        right_val = value_create_int(0);
        env_set(rt->env, right_name, right_val);
        right_val = env_get(rt->env, right_name);
    }
    
    value_t* temp = value_clone(left_val);
    env_set(rt->env, left_name, value_clone(right_val));
    env_set(rt->env, right_name, temp);
    
    string_destroy(left_name);
    string_destroy(right_name);
}

static void exec_read(runtime_t* rt, TSNode read_node) {
    TSNode name_list = parser_child_by_field(read_node, "names");
    
    uint32_t count = ts_node_child_count(name_list);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(name_list, i);
        if (!parser_node_is_type(child, NODE_IDENTIFIER)) continue;
        
        string_t* name = parser_get_identifier(rt->parser, child);
        
        const char* input = rt->io->ops.read(rt->io);
        if (!input) {
            rt->state = EXEC_NEEDS_INPUT;
            string_destroy(name);
            return;
        }
        
        value_t* val;
        char* endptr;
        
        long long int_val = strtoll(input, &endptr, 10);
        if (*endptr == '\0') {
            val = value_create_int(int_val);
        } else {
            double float_val = strtod(input, &endptr);
            if (*endptr == '\0') {
                val = value_create_float(float_val);
            } else {
                val = value_create_string_from(input);
            }
        }
        
        env_set(rt->env, name, val);
        string_destroy(name);
    }
}

static void exec_write(runtime_t* rt, TSNode write_node) {
    TSNode expr_list = parser_child_by_field(write_node, "values");
    
    string_t* output = string_create();
    
    uint32_t count = ts_node_child_count(expr_list);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(expr_list, i);
        if (strcmp(ts_node_type(child), ",") == 0) continue;
        
        value_t* val = eval_expr(rt, child);
        if (!val || rt->state == EXEC_ERROR) {
            string_destroy(output);
            return;
        }
        
        string_t* val_str = value_to_string(val);
        string_append_string(output, val_str);
        string_destroy(val_str);
        value_destroy(val);
    }
    
    rt->io->ops.write(rt->io, string_cstr(output));
    string_destroy(output);
}

static void exec_if(runtime_t* rt, TSNode if_node) {
    TSNode cond = parser_child_by_field(if_node, "condition");
    value_t* cond_val = eval_expr(rt, cond);
    if (!cond_val || rt->state == EXEC_ERROR) return;
    bool is_true = value_to_bool(cond_val);
    value_destroy(cond_val);
    
    bool in_else = false;
    uint32_t count = ts_node_child_count(if_node);
    
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(if_node, i);
        const char* type = ts_node_type(child);
        
        if (strcmp(type, "altfel") == 0) {
            in_else = true;
            continue;
        }
        
        if (strcmp(type, "sf") == 0) break;
        
        if (parser_node_is_type(child, NODE_STMT)) {
            if ((!in_else && is_true) || (in_else && !is_true)) {
                exec_stmt(rt, child);
                if (rt->state == EXEC_ERROR) return;
            }
        }
    }
}

static void exec_for(runtime_t* rt, TSNode for_node) {
    TSNode var_node = parser_child_by_field(for_node, "var");
    TSNode start_node = parser_child_by_field(for_node, "start");
    TSNode end_node = parser_child_by_field(for_node, "end");
    TSNode step_node = parser_child_by_field(for_node, "step");
    
    string_t* var_name = parser_get_identifier(rt->parser, var_node);
    
    value_t* start_val = eval_expr(rt, start_node);
    value_t* end_val = eval_expr(rt, end_node);
    
    int64_t start = value_to_int(start_val);
    int64_t end = value_to_int(end_val);
    int64_t step = 1;
    
    if (!ts_node_is_null(step_node)) {
        value_t* step_val = eval_expr(rt, step_node);
        step = value_to_int(step_val);
        value_destroy(step_val);
    }
    
    value_destroy(start_val);
    value_destroy(end_val);
    
    int64_t current = start;
    while ((step > 0 && current <= end) || (step < 0 && current >= end)) {
        env_set(rt->env, var_name, value_create_int(current));
        
        uint32_t count = ts_node_child_count(for_node);
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(for_node, i);
            if (parser_node_is_type(child, NODE_STMT)) {
                exec_stmt(rt, child);
                if (rt->state == EXEC_ERROR) {
                    string_destroy(var_name);
                    return;
                }
            }
        }
        
        current += step;
    }
    
    string_destroy(var_name);
}

static void exec_while(runtime_t* rt, TSNode while_node) {
    TSNode cond = parser_child_by_field(while_node, "condition");
    
    while (true) {
        value_t* cond_val = eval_expr(rt, cond);
        if (!cond_val || rt->state == EXEC_ERROR) return;
        bool is_true = value_to_bool(cond_val);
        value_destroy(cond_val);
        
        if (!is_true) break;
        
        uint32_t count = ts_node_child_count(while_node);
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(while_node, i);
            if (parser_node_is_type(child, NODE_STMT)) {
                exec_stmt(rt, child);
                if (rt->state == EXEC_ERROR) return;
            }
        }
    }
}

static void exec_do_while(runtime_t* rt, TSNode do_while_node) {
    TSNode cond = parser_child_by_field(do_while_node, "condition");
    
    do {
        uint32_t count = ts_node_child_count(do_while_node);
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(do_while_node, i);
            if (parser_node_is_type(child, NODE_STMT)) {
                exec_stmt(rt, child);
                if (rt->state == EXEC_ERROR) return;
            }
        }
        
        value_t* cond_val = eval_expr(rt, cond);
        if (!cond_val || rt->state == EXEC_ERROR) return;
        bool is_true = value_to_bool(cond_val);
        value_destroy(cond_val);
        
        if (!is_true) break;
    } while (true);
}

static void exec_repeat(runtime_t* rt, TSNode repeat_node) {
    TSNode cond = parser_child_by_field(repeat_node, "condition");
    
    do {
        uint32_t count = ts_node_child_count(repeat_node);
        for (uint32_t i = 0; i < count; i++) {
            TSNode child = ts_node_child(repeat_node, i);
            if (parser_node_is_type(child, NODE_STMT)) {
                exec_stmt(rt, child);
                if (rt->state == EXEC_ERROR) return;
            }
        }
        
        value_t* cond_val = eval_expr(rt, cond);
        if (!cond_val || rt->state == EXEC_ERROR) return;
        bool is_true = value_to_bool(cond_val);
        value_destroy(cond_val);
        
        if (is_true) break;
    } while (true);
}

static void exec_multi_stmt(runtime_t* rt, TSNode multi_stmt_node) {
    uint32_t count = ts_node_child_count(multi_stmt_node);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(multi_stmt_node, i);
        const char* type = ts_node_type(child);
        
        if (strcmp(type, ";") == 0) continue;
        
        // Multi_stmt contains simple statements directly (not wrapped in stmt nodes)
        // So we execute them directly
        if (strcmp(type, NODE_ASSIGN) == 0) exec_assign(rt, child);
        else if (strcmp(type, NODE_SWAP) == 0) exec_swap(rt, child);
        else if (strcmp(type, NODE_READ) == 0) exec_read(rt, child);
        else if (strcmp(type, NODE_WRITE) == 0) exec_write(rt, child);
        
        if (rt->state == EXEC_ERROR) return;
    }
}

static void exec_stmt(runtime_t* rt, TSNode stmt_node) {
    assert(parser_node_is_type(stmt_node, NODE_STMT));
    
    TSNode actual = ts_node_child(stmt_node, 0);
    const char* type = ts_node_type(actual);
    
    if (strcmp(type, NODE_ASSIGN) == 0) exec_assign(rt, actual);
    else if (strcmp(type, NODE_SWAP) == 0) exec_swap(rt, actual);
    else if (strcmp(type, NODE_READ) == 0) exec_read(rt, actual);
    else if (strcmp(type, NODE_WRITE) == 0) exec_write(rt, actual);
    else if (strcmp(type, NODE_IF) == 0) exec_if(rt, actual);
    else if (strcmp(type, NODE_FOR) == 0) exec_for(rt, actual);
    else if (strcmp(type, NODE_WHILE) == 0) exec_while(rt, actual);
    else if (strcmp(type, NODE_DO_WHILE) == 0) exec_do_while(rt, actual);
    else if (strcmp(type, NODE_REPEAT) == 0) exec_repeat(rt, actual);
    else if (strcmp(type, NODE_MULTI_STMT) == 0) exec_multi_stmt(rt, actual);
}

// === Public API ===

exec_state_t runtime_step(runtime_t* rt) {
    assert(rt);
    
    if (rt->state != EXEC_CONTINUE) {
        return rt->state;
    }
    
    uint32_t child_count = ts_node_child_count(rt->program_root);
    
    // Skip non-statement nodes (comments, whitespace)
    while (rt->next_stmt_index < child_count) {
        TSNode child = ts_node_child(rt->program_root, rt->next_stmt_index);
        
        // Only process actual statement nodes
        if (parser_node_is_type(child, NODE_STMT)) {
            rt->next_stmt_index++;
            exec_stmt(rt, child);
            
            if (rt->state == EXEC_CONTINUE && rt->next_stmt_index >= child_count) {
                rt->state = EXEC_DONE;
            }
            
            return rt->state;
        }
        
        // Skip this non-statement node
        rt->next_stmt_index++;
    }
    
    // No more statements
    rt->state = EXEC_DONE;
    return EXEC_DONE;
}

exec_state_t runtime_run(runtime_t* rt) {
    while (rt->state == EXEC_CONTINUE) {
        runtime_step(rt);
    }
    return rt->state;
}

const char* runtime_get_error(runtime_t* rt) {
    return rt->error_msg ? string_cstr(rt->error_msg) : NULL;
}

uint32_t runtime_get_current_line(runtime_t* rt) {
    if (rt->next_stmt_index == 0) return 0;
    TSNode stmt = ts_node_child(rt->program_root, rt->next_stmt_index - 1);
    return ts_node_start_point(stmt).row;
}

uint32_t runtime_get_current_column(runtime_t* rt) {
    if (rt->next_stmt_index == 0) return 0;
    TSNode stmt = ts_node_child(rt->program_root, rt->next_stmt_index - 1);
    return ts_node_start_point(stmt).column;
}
