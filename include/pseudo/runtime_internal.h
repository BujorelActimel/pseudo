#ifndef PSEUDO_RUNTIME_INTERNAL_H
#define PSEUDO_RUNTIME_INTERNAL_H

#include "pseudo/runtime.h"
#include "pseudo/debugger.h"
#include "pseudo/parser.h"
#include "pseudo/environment.h"
#include "pseudo/string.h"
#include <tree_sitter/api.h>

// Execution frame types for stack-based stepping
typedef enum {
    FRAME_PROGRAM,      // Top-level program, iterating statements
    FRAME_IF,           // If statement (phase: 0=condition, 1=then, 2=else)
    FRAME_FOR,          // For loop (phase: 0=init, 1=check, 2=body, 3=next)
    FRAME_WHILE,        // While loop (phase: 0=check, 1=body)
    FRAME_DO_WHILE,     // Do-while loop (phase: 0=body, 1=check)
    FRAME_REPEAT,       // Repeat-until loop (phase: 0=body, 1=check)
    FRAME_BLOCK,        // Block of statements (if body, loop body, etc.)
} frame_type_t;

// Execution stack frame
typedef struct exec_frame {
    frame_type_t type;
    TSNode node;
    int phase;              // State machine phase within this frame
    uint32_t child_idx;     // Current child index for iteration

    // For loop frames
    int64_t loop_current;
    int64_t loop_end;
    int64_t loop_step;
    string_t* loop_var;     // Loop variable name (owned by frame)

    // For if frames
    bool condition_result;
    bool in_else;           // Whether we're in the else branch (persists across calls)
} exec_frame_t;

#define MAX_STACK_DEPTH 256

// Internal runtime structure - shared between interpreter.c and debugger.c
struct runtime {
    parser_t* parser;
    io_t* io;
    environment_t* env;

    exec_state_t state;
    string_t* error_msg;

    TSNode program_root;

    // Execution stack for line-by-line stepping
    exec_frame_t exec_stack[MAX_STACK_DEPTH];
    int stack_top;          // Index of top frame (-1 = empty)

    // For multi-variable read statements (citeste a,b,c)
    uint32_t read_var_index;
    TSNode pending_read_node;  // Node being read from
    bool has_pending_read;

    // For stopping execution from JS
    bool stop_requested;

    // Debug mode - when true, step returns after each visible action
    // when false, continues execution without returning on internal phases
    bool debug_mode;

    // Current line being executed (for debugger)
    uint32_t current_line;

    // Last evaluated condition (for visualization)
    string_t* last_condition_text;
    bool last_condition_result;
    bool has_condition_info;

    // Debugger state - snapshots stored as circular buffer
    struct runtime_snapshot* snapshots[MAX_SNAPSHOTS];
    int snapshot_head;
    int snapshot_count;
};

#endif // PSEUDO_RUNTIME_INTERNAL_H
