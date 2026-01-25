#include "pseudo/io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char input_buffer[4096];
} stdio_ctx_t;

static void stdio_write(io_t* io, const char* text) {
    (void)io;
    printf("%s", text);
    fflush(stdout);
}

static const char* stdio_read(io_t* io) {
    stdio_ctx_t* ctx = (stdio_ctx_t*)io->ctx;
    
    if (fgets(ctx->input_buffer, sizeof(ctx->input_buffer), stdin)) {
        // Remove trailing newline
        size_t len = strlen(ctx->input_buffer);
        if (len > 0 && ctx->input_buffer[len-1] == '\n') {
            ctx->input_buffer[len-1] = '\0';
        }
        return ctx->input_buffer;
    }
    
    return NULL;  // EOF
}

static void stdio_destroy(io_t* io) {
    if (!io) return;
    free(io->ctx);
    free(io);
}

io_t* io_stdio_create(void) {
    io_t* io = malloc(sizeof(io_t));
    if (!io) return NULL;
    
    stdio_ctx_t* ctx = malloc(sizeof(stdio_ctx_t));
    if (!ctx) {
        free(io);
        return NULL;
    }
    
    io->ops.write = stdio_write;
    io->ops.read = stdio_read;
    io->ops.destroy = stdio_destroy;
    io->ctx = ctx;
    
    return io;
}

void io_destroy(io_t* io) {
    if (io && io->ops.destroy) {
        io->ops.destroy(io);
    }
}
