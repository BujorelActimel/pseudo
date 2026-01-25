#ifndef PSEUDO_RUNTIME_H
#define PSEUDO_RUNTIME_H

#include "pseudo/io.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct runtime runtime_t;

typedef enum {
    EXEC_CONTINUE,    // More statements to execute
    EXEC_DONE,        // Program finished successfully
    EXEC_NEEDS_INPUT, // Waiting for input (WASM mode)
    EXEC_ERROR        // Error occurred
} exec_state_t;

// Lifecycle
runtime_t* runtime_create(io_t* io);
void runtime_destroy(runtime_t* rt);

// Loading source code
bool runtime_load(runtime_t* rt, const char* source);

// Execution
exec_state_t runtime_step(runtime_t* rt);  // Execute one statement
exec_state_t runtime_run(runtime_t* rt);   // Run until done/input/error (CLI)
void runtime_resume(runtime_t* rt);        // Resume after input provided (WASM mode)
void runtime_request_stop(runtime_t* rt);  // Request stop (checked in loops)

// Error reporting
const char* runtime_get_error(runtime_t* rt);

// Debug helpers (for future debugger support)
uint32_t runtime_get_current_line(runtime_t* rt);
uint32_t runtime_get_current_column(runtime_t* rt);

#endif // PSEUDO_RUNTIME_H
