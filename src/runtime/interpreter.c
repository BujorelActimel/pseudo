#include "pseudo/runtime.h"
#include "pseudo/runtime_internal.h"
#include "pseudo/debugger.h"
#include "pseudo/parser.h"
#include "pseudo/environment.h"
#include "pseudo/value.h"
#include "pseudo/linter.h"
#include "pseudo/string.h"
#include <tree_sitter/api.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declarations
static value_t* eval_expr(runtime_t* rt, TSNode expr_node);

// === Stack Management ===

static void stack_push(runtime_t* rt, frame_type_t type, TSNode node) {
    assert(rt->stack_top < MAX_STACK_DEPTH - 1);
    rt->stack_top++;
    exec_frame_t* frame = &rt->exec_stack[rt->stack_top];
    frame->type = type;
    frame->node = node;
    frame->phase = 0;
    frame->child_idx = 0;
    frame->loop_current = 0;
    frame->loop_end = 0;
    frame->loop_step = 1;
    frame->loop_var = NULL;
    frame->condition_result = false;
    frame->in_else = false;
}

static void stack_pop(runtime_t* rt) {
    if (rt->stack_top >= 0) {
        exec_frame_t* frame = &rt->exec_stack[rt->stack_top];
        if (frame->loop_var) {
            string_destroy(frame->loop_var);
            frame->loop_var = NULL;
        }
        rt->stack_top--;
    }
}

static exec_frame_t* stack_top(runtime_t* rt) {
    if (rt->stack_top < 0) return NULL;
    return &rt->exec_stack[rt->stack_top];
}

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
    rt->stack_top = -1;
    rt->debug_mode = false;

    // Condition visualization
    rt->last_condition_text = NULL;
    rt->last_condition_result = false;
    rt->has_condition_info = false;

    // Initialize debugger state
    for (int i = 0; i < MAX_SNAPSHOTS; i++) {
        rt->snapshots[i] = NULL;
    }
    rt->snapshot_head = 0;
    rt->snapshot_count = 0;

    return rt;
}

void runtime_destroy(runtime_t* rt) {
    if (!rt) return;

    // Clean up stack frames
    while (rt->stack_top >= 0) {
        stack_pop(rt);
    }

    runtime_clear_snapshots(rt);
    parser_destroy(rt->parser);
    env_destroy(rt->env);
    if (rt->error_msg) string_destroy(rt->error_msg);
    if (rt->last_condition_text) string_destroy(rt->last_condition_text);
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

    // Clean up any existing stack
    while (rt->stack_top >= 0) {
        stack_pop(rt);
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
    rt->read_var_index = 0;
    rt->has_pending_read = false;
    rt->stop_requested = false;
    rt->current_line = 0;
    rt->state = EXEC_CONTINUE;
    env_clear(rt->env);

    // Clear condition info
    if (rt->last_condition_text) {
        string_destroy(rt->last_condition_text);
        rt->last_condition_text = NULL;
    }
    rt->has_condition_info = false;

    // Initialize stack with program frame
    stack_push(rt, FRAME_PROGRAM, rt->program_root);

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
        value_t* val = eval_expr(rt, operand);
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

// === Simple statement execution (atomic operations) ===

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

static bool exec_read_one(runtime_t* rt, TSNode read_node) {
    TSNode name_list = parser_child_by_field(read_node, "names");
    uint32_t count = ts_node_child_count(name_list);
    uint32_t var_index = 0;

    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(name_list, i);
        if (!parser_node_is_type(child, NODE_IDENTIFIER)) continue;

        if (var_index < rt->read_var_index) {
            var_index++;
            continue;
        }

        string_t* name = parser_get_identifier(rt->parser, child);
        const char* input = rt->io->ops.read(rt->io);

        if (!input) {
            rt->state = EXEC_NEEDS_INPUT;
            rt->has_pending_read = true;
            rt->pending_read_node = read_node;
            string_destroy(name);
            return false;
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

        rt->read_var_index++;
        var_index++;
    }

    rt->read_var_index = 0;
    rt->has_pending_read = false;
    return true;
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

// === Find next statement child in a node ===

static TSNode find_next_stmt_child(TSNode parent, uint32_t* idx) {
    uint32_t count = ts_node_child_count(parent);
    while (*idx < count) {
        TSNode child = ts_node_child(parent, *idx);
        if (parser_node_is_type(child, NODE_STMT)) {
            return child;
        }
        (*idx)++;
    }
    return ts_node_child(parent, 0); // Return invalid node
}

// === Execute simple statement directly ===

static bool exec_simple_stmt(runtime_t* rt, TSNode node) {
    const char* type = ts_node_type(node);

    rt->current_line = ts_node_start_point(node).row;

    if (strcmp(type, NODE_ASSIGN) == 0) {
        exec_assign(rt, node);
        return true;
    }
    if (strcmp(type, NODE_SWAP) == 0) {
        exec_swap(rt, node);
        return true;
    }
    if (strcmp(type, NODE_READ) == 0) {
        return exec_read_one(rt, node);
    }
    if (strcmp(type, NODE_WRITE) == 0) {
        exec_write(rt, node);
        return true;
    }
    return false;
}

// === Condition info helper ===

static void save_condition_info(runtime_t* rt, TSNode cond_node, bool result) {
    if (rt->last_condition_text) {
        string_destroy(rt->last_condition_text);
    }
    rt->last_condition_text = parser_node_text(rt->parser, cond_node);
    rt->last_condition_result = result;
    rt->has_condition_info = true;
}

static void clear_condition_info(runtime_t* rt) {
    rt->has_condition_info = false;
}

// === Stack-based step execution ===

// Internal step that processes one phase. Returns true if a visible action occurred.
static bool runtime_step_internal(runtime_t* rt) {
    assert(rt);

    if (rt->state != EXEC_CONTINUE) {
        return true;  // Done/error/input is a terminal state
    }

    if (rt->stop_requested) {
        rt->state = EXEC_ERROR;
        rt->error_msg = string_create_from("Program stopped");
        return true;
    }

    // Handle pending read - if successful, this counts as a visible action
    if (rt->has_pending_read) {
        if (!exec_read_one(rt, rt->pending_read_node)) {
            return true;  // Still waiting for input - visible (shows input prompt)
        }
        // Read completed successfully - advance past the read statement
        if (rt->stack_top >= 0) {
            rt->exec_stack[rt->stack_top].child_idx++;
        }
        clear_condition_info(rt);
        return true;  // Visible: input was received
    }

    exec_frame_t* frame = stack_top(rt);
    if (!frame) {
        rt->state = EXEC_DONE;
        return true;
    }

    switch (frame->type) {
        case FRAME_PROGRAM: {
            // Find next statement in program
            uint32_t count = ts_node_child_count(frame->node);
            while (frame->child_idx < count) {
                TSNode child = ts_node_child(frame->node, frame->child_idx);
                frame->child_idx++;

                if (!parser_node_is_type(child, NODE_STMT)) continue;

                TSNode actual = ts_node_child(child, 0);
                const char* type = ts_node_type(actual);

                rt->current_line = ts_node_start_point(actual).row;
                clear_condition_info(rt);

                // Simple statements - execute directly (VISIBLE)
                if (strcmp(type, NODE_ASSIGN) == 0 ||
                    strcmp(type, NODE_SWAP) == 0 ||
                    strcmp(type, NODE_READ) == 0 ||
                    strcmp(type, NODE_WRITE) == 0) {
                    if (!exec_simple_stmt(rt, actual)) {
                        frame->child_idx--; // Retry this statement
                    }
                    return true;  // Visible: statement executed
                }

                // Multi-statement - NOT visible, just setup
                if (strcmp(type, NODE_MULTI_STMT) == 0) {
                    stack_push(rt, FRAME_BLOCK, actual);
                    return false;  // Not visible: just pushed frame
                }

                // Control structures - push frame (NOT visible yet)
                if (strcmp(type, NODE_IF) == 0) {
                    stack_push(rt, FRAME_IF, actual);
                    return false;  // Not visible: will evaluate condition next
                }
                if (strcmp(type, NODE_FOR) == 0) {
                    stack_push(rt, FRAME_FOR, actual);
                    return false;  // Not visible: will init loop next
                }
                if (strcmp(type, NODE_WHILE) == 0) {
                    stack_push(rt, FRAME_WHILE, actual);
                    return false;  // Not visible: will check condition next
                }
                if (strcmp(type, NODE_DO_WHILE) == 0) {
                    stack_push(rt, FRAME_DO_WHILE, actual);
                    return false;  // Not visible: will execute body next
                }
                if (strcmp(type, NODE_REPEAT) == 0) {
                    stack_push(rt, FRAME_REPEAT, actual);
                    return false;  // Not visible: will execute body next
                }
            }
            // No more statements - NOT visible, just cleanup
            stack_pop(rt);
            if (rt->stack_top < 0) {
                rt->state = EXEC_DONE;
                return true;  // Visible: program ended
            }
            return false;  // Not visible: popped empty frame
        }

        case FRAME_BLOCK: {
            // Iterate through statements in a block (multi_stmt or loop body)
            uint32_t count = ts_node_child_count(frame->node);
            while (frame->child_idx < count) {
                TSNode child = ts_node_child(frame->node, frame->child_idx);
                const char* ctype = ts_node_type(child);
                frame->child_idx++;

                if (strcmp(ctype, ";") == 0) continue;

                rt->current_line = ts_node_start_point(child).row;
                clear_condition_info(rt);

                // In multi_stmt, children are direct statements (VISIBLE)
                if (strcmp(ctype, NODE_ASSIGN) == 0 ||
                    strcmp(ctype, NODE_SWAP) == 0 ||
                    strcmp(ctype, NODE_READ) == 0 ||
                    strcmp(ctype, NODE_WRITE) == 0) {
                    if (!exec_simple_stmt(rt, child)) {
                        frame->child_idx--;
                    }
                    return true;  // Visible: statement executed
                }
            }
            stack_pop(rt);
            return false;  // Not visible: just popped block
        }

        case FRAME_IF: {
            if (frame->phase == 0) {
                // Evaluate condition (VISIBLE)
                TSNode cond = parser_child_by_field(frame->node, "condition");
                rt->current_line = ts_node_start_point(frame->node).row;

                value_t* cond_val = eval_expr(rt, cond);
                if (!cond_val || rt->state == EXEC_ERROR) {
                    stack_pop(rt);
                    return true;  // Visible: error occurred
                }
                frame->condition_result = value_to_bool(cond_val);
                value_destroy(cond_val);

                // Save condition info for visualization
                save_condition_info(rt, cond, frame->condition_result);

                frame->phase = 1;
                frame->child_idx = 0;
                return true;  // Visible: condition evaluated
            }

            // Find and execute statements in appropriate branch
            uint32_t count = ts_node_child_count(frame->node);

            while (frame->child_idx < count) {
                TSNode child = ts_node_child(frame->node, frame->child_idx);
                const char* ctype = ts_node_type(child);
                frame->child_idx++;

                if (strcmp(ctype, "altfel") == 0) {
                    frame->in_else = true;
                    continue;
                }
                if (strcmp(ctype, "sf") == 0) break;

                if (parser_node_is_type(child, NODE_STMT)) {
                    bool should_exec = (!frame->in_else && frame->condition_result) ||
                                       (frame->in_else && !frame->condition_result);
                    if (should_exec) {
                        TSNode actual = ts_node_child(child, 0);
                        const char* type = ts_node_type(actual);

                        rt->current_line = ts_node_start_point(actual).row;
                        clear_condition_info(rt);

                        if (strcmp(type, NODE_ASSIGN) == 0 ||
                            strcmp(type, NODE_SWAP) == 0 ||
                            strcmp(type, NODE_READ) == 0 ||
                            strcmp(type, NODE_WRITE) == 0) {
                            if (!exec_simple_stmt(rt, actual)) {
                                frame->child_idx--;
                            }
                            return true;  // Visible: statement executed
                        }
                        if (strcmp(type, NODE_MULTI_STMT) == 0) {
                            stack_push(rt, FRAME_BLOCK, actual);
                            return false;  // Not visible: just pushed frame
                        }
                        if (strcmp(type, NODE_IF) == 0) {
                            stack_push(rt, FRAME_IF, actual);
                            return false;
                        }
                        if (strcmp(type, NODE_FOR) == 0) {
                            stack_push(rt, FRAME_FOR, actual);
                            return false;
                        }
                        if (strcmp(type, NODE_WHILE) == 0) {
                            stack_push(rt, FRAME_WHILE, actual);
                            return false;
                        }
                        if (strcmp(type, NODE_DO_WHILE) == 0) {
                            stack_push(rt, FRAME_DO_WHILE, actual);
                            return false;
                        }
                        if (strcmp(type, NODE_REPEAT) == 0) {
                            stack_push(rt, FRAME_REPEAT, actual);
                            return false;
                        }
                    }
                }
            }
            stack_pop(rt);
            return false;  // Not visible: just popped if frame
        }

        case FRAME_FOR: {
            if (frame->phase == 0) {
                // Initialize loop (VISIBLE - shows loop start with i=start)
                TSNode var_node = parser_child_by_field(frame->node, "var");
                TSNode start_node = parser_child_by_field(frame->node, "start");
                TSNode end_node = parser_child_by_field(frame->node, "end");
                TSNode step_node = parser_child_by_field(frame->node, "step");

                rt->current_line = ts_node_start_point(frame->node).row;

                frame->loop_var = parser_get_identifier(rt->parser, var_node);

                value_t* start_val = eval_expr(rt, start_node);
                value_t* end_val = eval_expr(rt, end_node);

                frame->loop_current = value_to_int(start_val);
                frame->loop_end = value_to_int(end_val);
                frame->loop_step = 1;

                if (!ts_node_is_null(step_node)) {
                    value_t* step_val = eval_expr(rt, step_node);
                    frame->loop_step = value_to_int(step_val);
                    value_destroy(step_val);
                }

                value_destroy(start_val);
                value_destroy(end_val);

                // Set initial loop variable
                env_set(rt->env, frame->loop_var, value_create_int(frame->loop_current));

                // Show condition in visualization
                if (rt->last_condition_text) string_destroy(rt->last_condition_text);
                char buf[128];
                bool will_continue = (frame->loop_step > 0 && frame->loop_current <= frame->loop_end) ||
                                     (frame->loop_step < 0 && frame->loop_current >= frame->loop_end);
                snprintf(buf, sizeof(buf), "%s = %lld, %s %s %lld",
                    string_cstr(frame->loop_var), (long long)frame->loop_current,
                    string_cstr(frame->loop_var),
                    frame->loop_step > 0 ? "<=" : ">=",
                    (long long)frame->loop_end);
                rt->last_condition_text = string_create_from(buf);
                rt->last_condition_result = will_continue;
                rt->has_condition_info = true;

                if (!will_continue) {
                    stack_pop(rt);
                    return true;  // Visible: loop condition false from start
                }

                frame->phase = 2;
                frame->child_idx = 0;
                return true;  // Visible: loop initialized
            }

            if (frame->phase == 1) {
                // Check loop condition (VISIBLE - shows condition check)
                bool continue_loop = (frame->loop_step > 0 && frame->loop_current <= frame->loop_end) ||
                                     (frame->loop_step < 0 && frame->loop_current >= frame->loop_end);

                rt->current_line = ts_node_start_point(frame->node).row;

                // Show condition in visualization
                if (rt->last_condition_text) string_destroy(rt->last_condition_text);
                char buf[128];
                snprintf(buf, sizeof(buf), "%s = %lld, %s %s %lld",
                    string_cstr(frame->loop_var), (long long)frame->loop_current,
                    string_cstr(frame->loop_var),
                    frame->loop_step > 0 ? "<=" : ">=",
                    (long long)frame->loop_end);
                rt->last_condition_text = string_create_from(buf);
                rt->last_condition_result = continue_loop;
                rt->has_condition_info = true;

                if (!continue_loop) {
                    stack_pop(rt);
                    return true;  // Visible: loop ended
                }

                // Set loop variable for this iteration
                env_set(rt->env, frame->loop_var, value_create_int(frame->loop_current));
                frame->phase = 2;
                frame->child_idx = 0;
                return true;  // Visible: starting new iteration
            }

            if (frame->phase == 2) {
                // Execute body statements
                uint32_t count = ts_node_child_count(frame->node);
                while (frame->child_idx < count) {
                    TSNode child = ts_node_child(frame->node, frame->child_idx);
                    frame->child_idx++;

                    if (!parser_node_is_type(child, NODE_STMT)) continue;

                    TSNode actual = ts_node_child(child, 0);
                    const char* type = ts_node_type(actual);

                    rt->current_line = ts_node_start_point(actual).row;
                    clear_condition_info(rt);

                    if (strcmp(type, NODE_ASSIGN) == 0 ||
                        strcmp(type, NODE_SWAP) == 0 ||
                        strcmp(type, NODE_READ) == 0 ||
                        strcmp(type, NODE_WRITE) == 0) {
                        if (!exec_simple_stmt(rt, actual)) {
                            frame->child_idx--;
                        }
                        return true;  // Visible: statement executed
                    }
                    if (strcmp(type, NODE_MULTI_STMT) == 0) {
                        stack_push(rt, FRAME_BLOCK, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_IF) == 0) {
                        stack_push(rt, FRAME_IF, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_FOR) == 0) {
                        stack_push(rt, FRAME_FOR, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_WHILE) == 0) {
                        stack_push(rt, FRAME_WHILE, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_DO_WHILE) == 0) {
                        stack_push(rt, FRAME_DO_WHILE, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_REPEAT) == 0) {
                        stack_push(rt, FRAME_REPEAT, actual);
                        return false;
                    }
                }

                // Body done, increment and loop back
                frame->loop_current += frame->loop_step;
                frame->phase = 1;
                frame->child_idx = 0;
                return false;  // Not visible: just incrementing counter
            }
            break;
        }

        case FRAME_WHILE: {
            if (frame->phase == 0) {
                // Check condition (VISIBLE)
                TSNode cond = parser_child_by_field(frame->node, "condition");
                rt->current_line = ts_node_start_point(frame->node).row;

                value_t* cond_val = eval_expr(rt, cond);
                if (!cond_val || rt->state == EXEC_ERROR) {
                    stack_pop(rt);
                    return true;  // Visible: error
                }
                bool is_true = value_to_bool(cond_val);
                value_destroy(cond_val);

                // Save condition info for visualization
                save_condition_info(rt, cond, is_true);

                if (!is_true) {
                    stack_pop(rt);
                    return true;  // Visible: loop condition false
                }

                frame->phase = 1;
                frame->child_idx = 0;
                return true;  // Visible: condition evaluated
            }

            if (frame->phase == 1) {
                // Execute body
                uint32_t count = ts_node_child_count(frame->node);
                while (frame->child_idx < count) {
                    TSNode child = ts_node_child(frame->node, frame->child_idx);
                    frame->child_idx++;

                    if (!parser_node_is_type(child, NODE_STMT)) continue;

                    TSNode actual = ts_node_child(child, 0);
                    const char* type = ts_node_type(actual);

                    rt->current_line = ts_node_start_point(actual).row;
                    clear_condition_info(rt);

                    if (strcmp(type, NODE_ASSIGN) == 0 ||
                        strcmp(type, NODE_SWAP) == 0 ||
                        strcmp(type, NODE_READ) == 0 ||
                        strcmp(type, NODE_WRITE) == 0) {
                        if (!exec_simple_stmt(rt, actual)) {
                            frame->child_idx--;
                        }
                        return true;  // Visible: statement executed
                    }
                    if (strcmp(type, NODE_MULTI_STMT) == 0) {
                        stack_push(rt, FRAME_BLOCK, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_IF) == 0) {
                        stack_push(rt, FRAME_IF, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_FOR) == 0) {
                        stack_push(rt, FRAME_FOR, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_WHILE) == 0) {
                        stack_push(rt, FRAME_WHILE, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_DO_WHILE) == 0) {
                        stack_push(rt, FRAME_DO_WHILE, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_REPEAT) == 0) {
                        stack_push(rt, FRAME_REPEAT, actual);
                        return false;
                    }
                }

                // Body done, check condition again
                frame->phase = 0;
                frame->child_idx = 0;
                return false;  // Not visible: looping back to condition check
            }
            break;
        }

        case FRAME_DO_WHILE: {
            if (frame->phase == 0) {
                // Execute body first
                uint32_t count = ts_node_child_count(frame->node);
                while (frame->child_idx < count) {
                    TSNode child = ts_node_child(frame->node, frame->child_idx);
                    frame->child_idx++;

                    if (!parser_node_is_type(child, NODE_STMT)) continue;

                    TSNode actual = ts_node_child(child, 0);
                    const char* type = ts_node_type(actual);

                    rt->current_line = ts_node_start_point(actual).row;
                    clear_condition_info(rt);

                    if (strcmp(type, NODE_ASSIGN) == 0 ||
                        strcmp(type, NODE_SWAP) == 0 ||
                        strcmp(type, NODE_READ) == 0 ||
                        strcmp(type, NODE_WRITE) == 0) {
                        if (!exec_simple_stmt(rt, actual)) {
                            frame->child_idx--;
                        }
                        return true;  // Visible: statement executed
                    }
                    if (strcmp(type, NODE_MULTI_STMT) == 0) {
                        stack_push(rt, FRAME_BLOCK, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_IF) == 0) {
                        stack_push(rt, FRAME_IF, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_FOR) == 0) {
                        stack_push(rt, FRAME_FOR, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_WHILE) == 0) {
                        stack_push(rt, FRAME_WHILE, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_DO_WHILE) == 0) {
                        stack_push(rt, FRAME_DO_WHILE, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_REPEAT) == 0) {
                        stack_push(rt, FRAME_REPEAT, actual);
                        return false;
                    }
                }

                frame->phase = 1;
                return false;  // Not visible: transitioning to condition check
            }

            if (frame->phase == 1) {
                // Check condition (VISIBLE)
                TSNode cond = parser_child_by_field(frame->node, "condition");
                rt->current_line = ts_node_start_point(cond).row;

                value_t* cond_val = eval_expr(rt, cond);
                if (!cond_val || rt->state == EXEC_ERROR) {
                    stack_pop(rt);
                    return true;  // Visible: error
                }
                bool is_true = value_to_bool(cond_val);
                value_destroy(cond_val);

                // Save condition info for visualization
                save_condition_info(rt, cond, is_true);

                if (!is_true) {
                    stack_pop(rt);
                    return true;  // Visible: loop ended
                }

                // Continue loop
                frame->phase = 0;
                frame->child_idx = 0;
                return true;  // Visible: condition evaluated, continuing
            }
            break;
        }

        case FRAME_REPEAT: {
            if (frame->phase == 0) {
                // Execute body first
                uint32_t count = ts_node_child_count(frame->node);
                while (frame->child_idx < count) {
                    TSNode child = ts_node_child(frame->node, frame->child_idx);
                    frame->child_idx++;

                    if (!parser_node_is_type(child, NODE_STMT)) continue;

                    TSNode actual = ts_node_child(child, 0);
                    const char* type = ts_node_type(actual);

                    rt->current_line = ts_node_start_point(actual).row;
                    clear_condition_info(rt);

                    if (strcmp(type, NODE_ASSIGN) == 0 ||
                        strcmp(type, NODE_SWAP) == 0 ||
                        strcmp(type, NODE_READ) == 0 ||
                        strcmp(type, NODE_WRITE) == 0) {
                        if (!exec_simple_stmt(rt, actual)) {
                            frame->child_idx--;
                        }
                        return true;  // Visible: statement executed
                    }
                    if (strcmp(type, NODE_MULTI_STMT) == 0) {
                        stack_push(rt, FRAME_BLOCK, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_IF) == 0) {
                        stack_push(rt, FRAME_IF, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_FOR) == 0) {
                        stack_push(rt, FRAME_FOR, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_WHILE) == 0) {
                        stack_push(rt, FRAME_WHILE, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_DO_WHILE) == 0) {
                        stack_push(rt, FRAME_DO_WHILE, actual);
                        return false;
                    }
                    if (strcmp(type, NODE_REPEAT) == 0) {
                        stack_push(rt, FRAME_REPEAT, actual);
                        return false;
                    }
                }

                frame->phase = 1;
                return false;  // Not visible: transitioning to condition check
            }

            if (frame->phase == 1) {
                // Check condition (repeat UNTIL condition is true) (VISIBLE)
                TSNode cond = parser_child_by_field(frame->node, "condition");
                rt->current_line = ts_node_start_point(cond).row;

                value_t* cond_val = eval_expr(rt, cond);
                if (!cond_val || rt->state == EXEC_ERROR) {
                    stack_pop(rt);
                    return true;  // Visible: error
                }
                bool is_true = value_to_bool(cond_val);
                value_destroy(cond_val);

                // Save condition info for visualization
                save_condition_info(rt, cond, is_true);

                if (is_true) {
                    // Condition true = stop loop
                    stack_pop(rt);
                    return true;  // Visible: loop ended
                }

                // Continue loop
                frame->phase = 0;
                frame->child_idx = 0;
                return true;  // Visible: condition evaluated, continuing
            }
            break;
        }
    }

    return false;  // Fallthrough - should not reach here
}

// Public step function - loops until a visible action occurs
exec_state_t runtime_step(runtime_t* rt) {
    assert(rt);

    // Keep stepping internally until we do something visible
    // or reach a terminal state (done/error/input)
    while (rt->state == EXEC_CONTINUE) {
        bool visible = runtime_step_internal(rt);
        if (visible || rt->state != EXEC_CONTINUE) {
            break;
        }
    }

    return rt->state;
}

// Step over - execute until we return to the same or lower stack depth
exec_state_t runtime_step_over(runtime_t* rt) {
    assert(rt);

    int initial_depth = rt->stack_top;

    // First, do one step
    runtime_step(rt);

    // If we're still running and went deeper, keep going until we return
    while (rt->state == EXEC_CONTINUE && rt->stack_top > initial_depth) {
        runtime_step(rt);
    }

    return rt->state;
}

int runtime_get_stack_depth(runtime_t* rt) {
    return rt ? rt->stack_top : -1;
}

exec_state_t runtime_run(runtime_t* rt) {
    // Fast execution path - runs until done/error/input
    // Just keep calling step_internal without any overhead
    while (rt->state == EXEC_CONTINUE && !rt->stop_requested) {
        runtime_step_internal(rt);
    }

    if (rt->stop_requested && rt->state == EXEC_CONTINUE) {
        rt->state = EXEC_ERROR;
        rt->error_msg = string_create_from("Program stopped");
    }

    return rt->state;
}

void runtime_resume(runtime_t* rt) {
    if (rt && rt->state == EXEC_NEEDS_INPUT) {
        rt->state = EXEC_CONTINUE;
    }
}

void runtime_request_stop(runtime_t* rt) {
    if (rt) {
        rt->stop_requested = true;
    }
}

const char* runtime_get_error(runtime_t* rt) {
    return rt->error_msg ? string_cstr(rt->error_msg) : NULL;
}

uint32_t runtime_get_current_line(runtime_t* rt) {
    return rt ? rt->current_line : 0;
}

uint32_t runtime_get_current_column(runtime_t* rt) {
    return 0; // Not tracking column for now
}

void runtime_set_debug_mode(runtime_t* rt, bool enabled) {
    if (rt) {
        rt->debug_mode = enabled;
    }
}

bool runtime_get_debug_mode(runtime_t* rt) {
    return rt ? rt->debug_mode : false;
}

condition_info_t runtime_get_last_condition(runtime_t* rt) {
    condition_info_t info = { NULL, false, false };
    if (rt && rt->has_condition_info && rt->last_condition_text) {
        info.condition_text = string_cstr(rt->last_condition_text);
        info.result = rt->last_condition_result;
        info.valid = true;
    }
    return info;
}
