#include "pseudo/runtime.h"
#include "pseudo/debugger.h"
#include "pseudo/string.h"
#include "pseudo/io.h"
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

static runtime_t* g_runtime = NULL;
static io_t* g_io = NULL;
static const char* g_init_error = NULL;

EMSCRIPTEN_KEEPALIVE
int pseudo_init(void) {
    g_init_error = NULL;

    if (g_runtime) {
        runtime_destroy(g_runtime);
        g_runtime = NULL;
    }
    if (g_io) {
        g_io->ops.destroy(g_io);
        g_io = NULL;
    }

    g_io = io_buffered_create();
    if (!g_io) {
        g_init_error = "Failed to create I/O buffer";
        return 0;
    }

    g_runtime = runtime_create(g_io);
    if (!g_runtime) {
        g_init_error = "Failed to create runtime (parser initialization failed)";
        g_io->ops.destroy(g_io);
        g_io = NULL;
        return 0;
    }

    return 1;
}

EMSCRIPTEN_KEEPALIVE
const char* pseudo_get_init_error(void) {
    return g_init_error;
}

EMSCRIPTEN_KEEPALIVE
int pseudo_load(const char* source) {
    if (!g_runtime || !source) return 0;
    return runtime_load(g_runtime, source) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
int pseudo_step(void) {
    if (!g_runtime) return 1; // EXEC_DONE
    return (int)runtime_step(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
int pseudo_run(void) {
    if (!g_runtime) return 1; // EXEC_DONE
    return (int)runtime_run(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
int pseudo_step_over(void) {
    if (!g_runtime) return 1; // EXEC_DONE
    return (int)runtime_step_over(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
int pseudo_get_stack_depth(void) {
    if (!g_runtime) return -1;
    return runtime_get_stack_depth(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
void pseudo_push_input(const char* value) {
    if (!g_io || !value) return;
    io_buffered_push_input(g_io, value);
    // Resume execution now that input is available
    if (g_runtime) {
        runtime_resume(g_runtime);
    }
}

EMSCRIPTEN_KEEPALIVE
int pseudo_has_output(void) {
    if (!g_io) return 0;
    return io_buffered_has_output(g_io) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
char* pseudo_pop_output(void) {
    if (!g_io) return NULL;
    return io_buffered_pop_output(g_io);
}

EMSCRIPTEN_KEEPALIVE
void pseudo_free_output(char* ptr) {
    free(ptr);
}

EMSCRIPTEN_KEEPALIVE
const char* pseudo_get_error(void) {
    if (!g_runtime) return NULL;
    return runtime_get_error(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
int pseudo_get_line(void) {
    if (!g_runtime) return 0;
    return (int)runtime_get_current_line(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
void pseudo_reset(void) {
    if (g_io) {
        io_buffered_clear(g_io);
    }
    // Runtime will be reset on next load
}

EMSCRIPTEN_KEEPALIVE
int pseudo_needs_input(void) {
    if (!g_io) return 0;
    return io_buffered_needs_input(g_io) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
void pseudo_request_stop(void) {
    if (g_runtime) {
        runtime_request_stop(g_runtime);
    }
}

// === Debugger exports ===

EMSCRIPTEN_KEEPALIVE
char* pseudo_get_variables_json(void) {
    if (!g_runtime) return NULL;
    string_t* json = runtime_get_variables_json(g_runtime);
    if (!json) return NULL;

    // Copy to malloc'd buffer for JS to free
    const char* str = string_cstr(json);
    size_t len = string_length(json);
    char* result = malloc(len + 1);
    if (result) {
        memcpy(result, str, len + 1);
    }
    string_destroy(json);
    return result;
}

EMSCRIPTEN_KEEPALIVE
int pseudo_create_snapshot(void) {
    if (!g_runtime) return -1;
    return runtime_create_snapshot(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
int pseudo_restore_snapshot(int snapshot_id) {
    if (!g_runtime) return 0;
    return runtime_restore_snapshot(g_runtime, snapshot_id) ? 1 : 0;
}

EMSCRIPTEN_KEEPALIVE
void pseudo_clear_snapshots(void) {
    if (g_runtime) {
        runtime_clear_snapshots(g_runtime);
    }
}

EMSCRIPTEN_KEEPALIVE
int pseudo_get_next_line(void) {
    if (!g_runtime) return 0;
    return (int)runtime_get_next_line(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
int pseudo_get_snapshot_count(void) {
    if (!g_runtime) return 0;
    return runtime_get_snapshot_count(g_runtime);
}

EMSCRIPTEN_KEEPALIVE
void pseudo_set_debug_mode(int enabled) {
    if (g_runtime) {
        runtime_set_debug_mode(g_runtime, enabled != 0);
    }
}

// Returns JSON: {"text": "condition", "result": true/false, "valid": true/false}
EMSCRIPTEN_KEEPALIVE
char* pseudo_get_condition_info(void) {
    if (!g_runtime) {
        char* result = malloc(32);
        if (result) strcpy(result, "{\"valid\":false}");
        return result;
    }

    condition_info_t info = runtime_get_last_condition(g_runtime);

    if (!info.valid) {
        char* result = malloc(32);
        if (result) strcpy(result, "{\"valid\":false}");
        return result;
    }

    // Build JSON with proper escaping
    size_t text_len = info.condition_text ? strlen(info.condition_text) : 0;
    size_t buf_size = text_len * 2 + 64;  // Extra space for escaping and JSON structure
    char* result = malloc(buf_size);
    if (!result) return NULL;

    // Simple JSON construction (condition text should be simple enough)
    snprintf(result, buf_size, "{\"text\":\"%s\",\"result\":%s,\"valid\":true}",
        info.condition_text ? info.condition_text : "",
        info.result ? "true" : "false");

    return result;
}
