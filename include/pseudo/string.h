#ifndef PSEUDO_STRING_H
#define PSEUDO_STRING_H

#include <stddef.h>
#include <stdbool.h>

typedef struct string string_t;

// Creation and destruction
string_t* string_create(void);
string_t* string_create_from(const char* cstr);
string_t* string_create_from_buf(const char* buf, size_t len);
string_t* string_create_from_string(const string_t* str);
string_t* string_create_with_capacity(size_t capacity);
void string_destroy(string_t* str);

// Capacity management
size_t string_length(const string_t* str);
size_t string_capacity(const string_t* str);
void string_reserve(string_t* str, size_t new_capacity);
void string_shrink_to_fit(string_t* str);

// Content access
const char* string_cstr(const string_t* str);
char string_at(const string_t* str, size_t index);

// Modification
void string_append(string_t* str, const char* cstr);
void string_append_buf(string_t* str, const char* buf, size_t len);
void string_append_char(string_t* str, char c);
void string_append_string(string_t* str, const string_t* other);
void string_clear(string_t* str);

// Utility
bool string_equals(const string_t* str, const char* cstr);
bool string_equals_string(const string_t* a, const string_t* b);

#endif // PSEUDO_STRING_H
