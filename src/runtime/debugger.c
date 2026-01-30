#include "pseudo/debugger.h"
#include "pseudo/runtime_internal.h"
#include "pseudo/environment.h"
#include "pseudo/value.h"
#include "pseudo/string.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Saved frame state for snapshots
typedef struct {
    frame_type_t type;
    int phase;
    uint32_t child_idx;
    int64_t loop_current;
    int64_t loop_end;
    int64_t loop_step;
    string_t* loop_var;
    bool condition_result;
    // Store node identity for restoration
    uint32_t node_id;  // We'll use start byte as ID
} saved_frame_t;

// Snapshot structure definition
struct runtime_snapshot {
    var_info_t* variables;
    size_t var_count;

    // Execution stack state
    saved_frame_t* frames;
    int frame_count;

    // Other runtime state
    uint32_t read_var_index;
    bool has_pending_read;
    uint32_t current_line;
    exec_state_t state;
};

// === Variable Inspection ===

typedef struct {
    var_info_t* vars;
    size_t count;
    size_t capacity;
} var_collector_t;

static void collect_variable(const string_t* name, const value_t* value, void* user_data) {
    var_collector_t* collector = (var_collector_t*)user_data;

    if (collector->count >= collector->capacity) {
        collector->capacity = collector->capacity == 0 ? 8 : collector->capacity * 2;
        collector->vars = realloc(collector->vars, collector->capacity * sizeof(var_info_t));
    }

    var_info_t* var = &collector->vars[collector->count];
    var->name = string_create_from_string(name);
    var->value = value_to_string(value);

    value_type_t type = value_type(value);
    switch (type) {
        case VALUE_INT:
            var->type = string_create_from("int");
            break;
        case VALUE_FLOAT:
            var->type = string_create_from("float");
            break;
        case VALUE_STRING:
            var->type = string_create_from("string");
            break;
        default:
            var->type = string_create_from("unknown");
            break;
    }

    collector->count++;
}

var_info_t* runtime_get_variables(runtime_t* rt, size_t* count) {
    if (!rt || !count) {
        if (count) *count = 0;
        return NULL;
    }

    var_collector_t collector = { NULL, 0, 0 };
    env_foreach(rt->env, collect_variable, &collector);

    *count = collector.count;
    return collector.vars;
}

void free_var_info_array(var_info_t* vars, size_t count) {
    if (!vars) return;

    for (size_t i = 0; i < count; i++) {
        string_destroy(vars[i].name);
        string_destroy(vars[i].value);
        string_destroy(vars[i].type);
    }
    free(vars);
}

// === JSON Serialization ===

static void escape_json_string(string_t* out, const string_t* str) {
    const char* s = string_cstr(str);
    while (*s) {
        switch (*s) {
            case '"':  string_append(out, "\\\""); break;
            case '\\': string_append(out, "\\\\"); break;
            case '\b': string_append(out, "\\b"); break;
            case '\f': string_append(out, "\\f"); break;
            case '\n': string_append(out, "\\n"); break;
            case '\r': string_append(out, "\\r"); break;
            case '\t': string_append(out, "\\t"); break;
            default:
                if ((unsigned char)*s < 32) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", (unsigned char)*s);
                    string_append(out, buf);
                } else {
                    string_append_char(out, *s);
                }
                break;
        }
        s++;
    }
}

string_t* runtime_get_variables_json(runtime_t* rt) {
    size_t count;
    var_info_t* vars = runtime_get_variables(rt, &count);

    string_t* json = string_create();
    string_append_char(json, '[');

    for (size_t i = 0; i < count; i++) {
        if (i > 0) string_append_char(json, ',');

        string_append(json, "{\"name\":\"");
        escape_json_string(json, vars[i].name);
        string_append(json, "\",\"value\":\"");
        escape_json_string(json, vars[i].value);
        string_append(json, "\",\"type\":\"");
        escape_json_string(json, vars[i].type);
        string_append(json, "\"}");
    }

    string_append_char(json, ']');

    free_var_info_array(vars, count);
    return json;
}

// === Snapshot Management ===

static void free_snapshot(runtime_snapshot_t* snap) {
    if (!snap) return;
    free_var_info_array(snap->variables, snap->var_count);
    if (snap->frames) {
        for (int i = 0; i < snap->frame_count; i++) {
            if (snap->frames[i].loop_var) {
                string_destroy(snap->frames[i].loop_var);
            }
        }
        free(snap->frames);
    }
    free(snap);
}

// Find a node by its start byte (for restoration)
static TSNode find_node_by_start(TSNode root, uint32_t start_byte) {
    if (ts_node_start_byte(root) == start_byte) {
        return root;
    }

    uint32_t count = ts_node_child_count(root);
    for (uint32_t i = 0; i < count; i++) {
        TSNode child = ts_node_child(root, i);
        TSNode found = find_node_by_start(child, start_byte);
        if (!ts_node_is_null(found) && ts_node_start_byte(found) == start_byte) {
            return found;
        }
    }

    return root; // Return root as fallback
}

int runtime_create_snapshot(runtime_t* rt) {
    if (!rt) return -1;

    runtime_snapshot_t* snap = malloc(sizeof(runtime_snapshot_t));
    if (!snap) return -1;

    // Capture variables
    snap->variables = runtime_get_variables(rt, &snap->var_count);

    // Capture execution stack
    snap->frame_count = rt->stack_top + 1;
    if (snap->frame_count > 0) {
        snap->frames = malloc(snap->frame_count * sizeof(saved_frame_t));
        for (int i = 0; i < snap->frame_count; i++) {
            exec_frame_t* f = &rt->exec_stack[i];
            snap->frames[i].type = f->type;
            snap->frames[i].phase = f->phase;
            snap->frames[i].child_idx = f->child_idx;
            snap->frames[i].loop_current = f->loop_current;
            snap->frames[i].loop_end = f->loop_end;
            snap->frames[i].loop_step = f->loop_step;
            snap->frames[i].loop_var = f->loop_var ? string_create_from_string(f->loop_var) : NULL;
            snap->frames[i].condition_result = f->condition_result;
            snap->frames[i].node_id = ts_node_start_byte(f->node);
        }
    } else {
        snap->frames = NULL;
    }

    // Capture other state
    snap->read_var_index = rt->read_var_index;
    snap->has_pending_read = rt->has_pending_read;
    snap->current_line = rt->current_line;
    snap->state = rt->state;

    // Store in circular buffer
    int id = rt->snapshot_head;

    if (rt->snapshots[id]) {
        free_snapshot(rt->snapshots[id]);
    }

    rt->snapshots[id] = snap;
    rt->snapshot_head = (rt->snapshot_head + 1) % MAX_SNAPSHOTS;

    if (rt->snapshot_count < MAX_SNAPSHOTS) {
        rt->snapshot_count++;
    }

    return id;
}

bool runtime_restore_snapshot(runtime_t* rt, int snapshot_id) {
    if (!rt || snapshot_id < 0 || snapshot_id >= MAX_SNAPSHOTS) {
        return false;
    }

    runtime_snapshot_t* snap = rt->snapshots[snapshot_id];
    if (!snap) return false;

    // Clear current stack
    while (rt->stack_top >= 0) {
        exec_frame_t* frame = &rt->exec_stack[rt->stack_top];
        if (frame->loop_var) {
            string_destroy(frame->loop_var);
            frame->loop_var = NULL;
        }
        rt->stack_top--;
    }

    // Restore execution stack
    for (int i = 0; i < snap->frame_count; i++) {
        rt->stack_top = i;
        exec_frame_t* f = &rt->exec_stack[i];
        f->type = snap->frames[i].type;
        f->phase = snap->frames[i].phase;
        f->child_idx = snap->frames[i].child_idx;
        f->loop_current = snap->frames[i].loop_current;
        f->loop_end = snap->frames[i].loop_end;
        f->loop_step = snap->frames[i].loop_step;
        f->loop_var = snap->frames[i].loop_var ? string_create_from_string(snap->frames[i].loop_var) : NULL;
        f->condition_result = snap->frames[i].condition_result;
        f->node = find_node_by_start(rt->program_root, snap->frames[i].node_id);
    }

    // Restore other state
    rt->read_var_index = snap->read_var_index;
    rt->has_pending_read = snap->has_pending_read;
    rt->current_line = snap->current_line;
    rt->state = snap->state;

    // Restore variables
    env_clear(rt->env);
    for (size_t i = 0; i < snap->var_count; i++) {
        value_t* val = NULL;
        const char* type_str = string_cstr(snap->variables[i].type);
        const char* val_str = string_cstr(snap->variables[i].value);

        if (strcmp(type_str, "int") == 0) {
            val = value_create_int(strtoll(val_str, NULL, 10));
        } else if (strcmp(type_str, "float") == 0) {
            val = value_create_float(strtod(val_str, NULL));
        } else {
            val = value_create_string_from(val_str);
        }

        if (val) {
            env_set(rt->env, snap->variables[i].name, val);
        }
    }

    // Invalidate snapshots after this one
    int next_pos = (snapshot_id + 1) % MAX_SNAPSHOTS;
    while (next_pos != rt->snapshot_head && rt->snapshots[next_pos]) {
        free_snapshot(rt->snapshots[next_pos]);
        rt->snapshots[next_pos] = NULL;
        rt->snapshot_count--;
        next_pos = (next_pos + 1) % MAX_SNAPSHOTS;
    }

    rt->snapshot_head = (snapshot_id + 1) % MAX_SNAPSHOTS;

    return true;
}

void runtime_clear_snapshots(runtime_t* rt) {
    if (!rt) return;

    for (int i = 0; i < MAX_SNAPSHOTS; i++) {
        if (rt->snapshots[i]) {
            free_snapshot(rt->snapshots[i]);
            rt->snapshots[i] = NULL;
        }
    }
    rt->snapshot_head = 0;
    rt->snapshot_count = 0;
}

uint32_t runtime_get_next_line(runtime_t* rt) {
    return rt ? rt->current_line : 0;
}

int runtime_get_snapshot_count(runtime_t* rt) {
    return rt ? rt->snapshot_count : 0;
}
