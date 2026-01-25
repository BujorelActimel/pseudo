#ifndef PSEUDO_IO_H
#define PSEUDO_IO_H

#include <stdbool.h>

typedef struct io io_t;

// Virtual function table
typedef struct {
    void (*write)(io_t* io, const char* text);
    const char* (*read)(io_t* io);  // Returns NULL if input not ready
    void (*destroy)(io_t* io);
} io_ops_t;

// Base I/O interface
struct io {
    io_ops_t ops;
    void* ctx;  // Backend-specific context
};

// CLI backend - blocking stdio
io_t* io_stdio_create(void);

// WASM backend - non-blocking buffered I/O
io_t* io_buffered_create(void);
void io_buffered_push_input(io_t* io, const char* value);
char* io_buffered_pop_output(io_t* io);  // Returns NULL if empty, caller owns
bool io_buffered_has_output(io_t* io);
bool io_buffered_needs_input(io_t* io);
void io_buffered_clear(io_t* io);

// Generic destroy (calls ops.destroy)
void io_destroy(io_t* io);

#endif // PSEUDO_IO_H
