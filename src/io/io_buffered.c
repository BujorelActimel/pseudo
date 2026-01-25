#include "pseudo/io.h"
#include <stdlib.h>
#include <string.h>

typedef struct string_node {
    char* str;
    struct string_node* next;
} string_node_t;

typedef struct {
    string_node_t* output_head;
    string_node_t* output_tail;
    string_node_t* input_head;
    string_node_t* input_tail;
    bool waiting_for_input;
} buffered_ctx_t;

static void buffered_write(io_t* io, const char* text) {
    if (!io || !text) return;
    buffered_ctx_t* ctx = (buffered_ctx_t*)io->ctx;

    string_node_t* node = malloc(sizeof(string_node_t));
    if (!node) return;

    node->str = strdup(text);
    if (!node->str) {
        free(node);
        return;
    }
    node->next = NULL;

    if (ctx->output_tail) {
        ctx->output_tail->next = node;
    } else {
        ctx->output_head = node;
    }
    ctx->output_tail = node;
}

// Maximum input length (inputs longer than this will be truncated)
#define MAX_INPUT_LENGTH 65536

static const char* buffered_read(io_t* io) {
    if (!io) return NULL;
    buffered_ctx_t* ctx = (buffered_ctx_t*)io->ctx;

    if (!ctx->input_head) {
        ctx->waiting_for_input = true;
        return NULL;  // No input available
    }

    // Dequeue input
    string_node_t* node = ctx->input_head;
    ctx->input_head = node->next;
    if (!ctx->input_head) {
        ctx->input_tail = NULL;
    }

    ctx->waiting_for_input = false;

    // Use static buffer to avoid requiring caller to free
    static char input_buffer[MAX_INPUT_LENGTH];
    strncpy(input_buffer, node->str, sizeof(input_buffer) - 1);
    input_buffer[sizeof(input_buffer) - 1] = '\0';

    free(node->str);
    free(node);

    return input_buffer;
}

static void buffered_destroy(io_t* io) {
    if (!io) return;
    
    buffered_ctx_t* ctx = (buffered_ctx_t*)io->ctx;
    
    // Free output queue
    string_node_t* node = ctx->output_head;
    while (node) {
        string_node_t* next = node->next;
        free(node->str);
        free(node);
        node = next;
    }
    
    // Free input queue
    node = ctx->input_head;
    while (node) {
        string_node_t* next = node->next;
        free(node->str);
        free(node);
        node = next;
    }
    
    free(ctx);
    free(io);
}

io_t* io_buffered_create(void) {
    io_t* io = malloc(sizeof(io_t));
    if (!io) return NULL;
    
    buffered_ctx_t* ctx = calloc(1, sizeof(buffered_ctx_t));
    if (!ctx) {
        free(io);
        return NULL;
    }
    
    io->ops.write = buffered_write;
    io->ops.read = buffered_read;
    io->ops.destroy = buffered_destroy;
    io->ctx = ctx;
    
    return io;
}

void io_buffered_push_input(io_t* io, const char* value) {
    if (!io || !value) return;
    buffered_ctx_t* ctx = (buffered_ctx_t*)io->ctx;

    string_node_t* node = malloc(sizeof(string_node_t));
    if (!node) return;

    node->str = strdup(value);
    if (!node->str) {
        free(node);
        return;
    }
    node->next = NULL;

    if (ctx->input_tail) {
        ctx->input_tail->next = node;
    } else {
        ctx->input_head = node;
    }
    ctx->input_tail = node;
}

char* io_buffered_pop_output(io_t* io) {
    if (!io) return NULL;
    buffered_ctx_t* ctx = (buffered_ctx_t*)io->ctx;

    if (!ctx->output_head) return NULL;
    
    string_node_t* node = ctx->output_head;
    ctx->output_head = node->next;
    if (!ctx->output_head) {
        ctx->output_tail = NULL;
    }
    
    char* str = node->str;
    free(node);
    return str;  // Caller owns
}

bool io_buffered_has_output(io_t* io) {
    if (!io) return false;
    buffered_ctx_t* ctx = (buffered_ctx_t*)io->ctx;
    return ctx->output_head != NULL;
}

bool io_buffered_needs_input(io_t* io) {
    if (!io) return false;
    buffered_ctx_t* ctx = (buffered_ctx_t*)io->ctx;
    return ctx->waiting_for_input;
}

void io_buffered_clear(io_t* io) {
    if (!io) return;
    buffered_ctx_t* ctx = (buffered_ctx_t*)io->ctx;
    
    // Clear output
    while (ctx->output_head) {
        char* str = io_buffered_pop_output(io);
        free(str);
    }
    
    // Clear input
    string_node_t* node = ctx->input_head;
    while (node) {
        string_node_t* next = node->next;
        free(node->str);
        free(node);
        node = next;
    }
    ctx->input_head = NULL;
    ctx->input_tail = NULL;
    ctx->waiting_for_input = false;
}
