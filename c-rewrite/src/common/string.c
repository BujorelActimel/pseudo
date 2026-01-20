#include "pseudo/string.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define INITIAL_CAPACITY 32
#define GROWTH_FACTOR 2

struct string {
    char* buffer;
    size_t length;
    size_t capacity;
};

static void ensure_capacity(string_t* str, size_t min_capacity) {
    if (str->capacity >= min_capacity + 1) return;

    size_t new_capacity = str->capacity > 0 ? str->capacity : INITIAL_CAPACITY;
    while (new_capacity < min_capacity + 1) {
        new_capacity *= GROWTH_FACTOR;
    }

    char* new_buffer = realloc(str->buffer, new_capacity);
    assert(new_buffer != NULL);

    str->buffer = new_buffer;
    str->capacity = new_capacity;
}

string_t* string_create(void) {
    return string_create_with_capacity(INITIAL_CAPACITY);
}

string_t* string_create_from(const char* cstr) {
    assert(cstr != NULL);
    return string_create_from_buf(cstr, strlen(cstr));
}

string_t* string_create_from_buf(const char* buf, size_t len) {
    assert(buf != NULL);

    string_t* str = string_create_with_capacity(len + 1);
    if (!str) return NULL;

    memcpy(str->buffer, buf, len);
    str->buffer[len] = '\0';
    str->length = len;

    return str;
}

string_t* string_create_from_string(const string_t* str) {
    assert(str != NULL);
    return string_create_from_buf(str->buffer, str->length);
}

string_t* string_create_with_capacity(size_t capacity) {
    string_t* str = malloc(sizeof(string_t));
    if (!str) return NULL;

    if (capacity < INITIAL_CAPACITY) {
        capacity = INITIAL_CAPACITY;
    }

    str->buffer = malloc(capacity);
    if (!str->buffer) {
        free(str);
        return NULL;
    }

    str->buffer[0] = '\0';
    str->length = 0;
    str->capacity = capacity;

    return str;
}

void string_destroy(string_t* str) {
    if (!str) return;
    free(str->buffer);
    free(str);
}

size_t string_length(const string_t* str) {
    assert(str != NULL);
    return str->length;
}

size_t string_capacity(const string_t* str) {
    assert(str != NULL);
    return str->capacity > 0 ? str->capacity - 1 : 0;
}

void string_reserve(string_t* str, size_t new_capacity) {
    assert(str != NULL);
    if (new_capacity <= string_capacity(str)) return;

    char* new_buffer = realloc(str->buffer, new_capacity + 1);
    assert(new_buffer != NULL);

    str->buffer = new_buffer;
    str->capacity = new_capacity + 1;
}

void string_shrink_to_fit(string_t* str) {
    assert(str != NULL);

    if (str->capacity == str->length + 1) return;

    size_t new_capacity = str->length + 1;
    char* new_buffer = realloc(str->buffer, new_capacity);
    assert(new_buffer != NULL);

    str->buffer = new_buffer;
    str->capacity = new_capacity;
}

const char* string_cstr(const string_t* str) {
    assert(str != NULL);
    return str->buffer;
}

char string_at(const string_t* str, size_t index) {
    assert(str != NULL);
    assert(index < str->length);
    return str->buffer[index];
}

void string_append(string_t* str, const char* cstr) {
    assert(str != NULL);
    assert(cstr != NULL);
    string_append_buf(str, cstr, strlen(cstr));
}

void string_append_buf(string_t* str, const char* buf, size_t len) {
    assert(str != NULL);
    assert(buf != NULL);

    if (len == 0) return;

    ensure_capacity(str, str->length + len);
    memcpy(str->buffer + str->length, buf, len);
    str->length += len;
    str->buffer[str->length] = '\0';
}

void string_append_char(string_t* str, char c) {
    assert(str != NULL);

    ensure_capacity(str, str->length + 1);
    str->buffer[str->length] = c;
    str->length++;
    str->buffer[str->length] = '\0';
}

void string_append_string(string_t* str, const string_t* other) {
    assert(str != NULL);
    assert(other != NULL);
    string_append_buf(str, other->buffer, other->length);
}

void string_clear(string_t* str) {
    assert(str != NULL);
    str->length = 0;
    if (str->buffer) {
        str->buffer[0] = '\0';
    }
}

bool string_equals(const string_t* str, const char* cstr) {
    assert(str != NULL);
    assert(cstr != NULL);
    return strcmp(str->buffer, cstr) == 0;
}

bool string_equals_string(const string_t* a, const string_t* b) {
    assert(a != NULL);
    assert(b != NULL);

    if (a->length != b->length) return false;
    return memcmp(a->buffer, b->buffer, a->length) == 0;
}
