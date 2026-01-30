#ifndef PSEUDO_DEBUGGER_H
#define PSEUDO_DEBUGGER_H

#include "pseudo/runtime.h"
#include "pseudo/string.h"
#include "pseudo/value.h"
#include <stddef.h>
#include <stdint.h>

// Variable information for debugging
typedef struct {
    string_t* name;
    string_t* value;  // String representation
    string_t* type;   // "int", "float", "string"
} var_info_t;

// Get all variables from the runtime environment
// Returns an array of var_info_t, caller must free with free_var_info_array()
var_info_t* runtime_get_variables(runtime_t* rt, size_t* count);

// Free variable info array
void free_var_info_array(var_info_t* vars, size_t count);

// State snapshot for step-back functionality
typedef struct runtime_snapshot runtime_snapshot_t;

// Snapshot management
#define MAX_SNAPSHOTS 100

// Create a snapshot of the current runtime state
// Returns snapshot ID (0 to MAX_SNAPSHOTS-1), or -1 on error
int runtime_create_snapshot(runtime_t* rt);

// Restore runtime to a previously saved snapshot
// Returns true on success, false if snapshot ID is invalid
bool runtime_restore_snapshot(runtime_t* rt, int snapshot_id);

// Clear all snapshots (free memory)
void runtime_clear_snapshots(runtime_t* rt);

// Get the current line number being executed (before the statement runs)
uint32_t runtime_get_next_line(runtime_t* rt);

// Get total number of snapshots currently stored
int runtime_get_snapshot_count(runtime_t* rt);

// Get variables JSON string (caller must free)
string_t* runtime_get_variables_json(runtime_t* rt);

#endif // PSEUDO_DEBUGGER_H
