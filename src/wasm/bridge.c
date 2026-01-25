#include "pseudo/runtime.h"
#include "pseudo/io.h"
#include <emscripten.h>
#include <stdlib.h>
#include <string.h>

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
