#ifndef PSEUDO_VALUE_H
#define PSEUDO_VALUE_H

#include "pseudo/string.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum {
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_STRING,
} value_type_t;

typedef enum {
    VALUE_OK,
    VALUE_ERR_TYPE,         // Type mismatch (e.g., "abc" - 5)
    VALUE_ERR_DIV_ZERO,     // Division by zero
    VALUE_ERR_NEGATIVE_SQRT, // Square root of negative number
} value_error_t;

typedef struct value value_t;

// Creation
value_t* value_create_int(int64_t val);
value_t* value_create_float(double val);
value_t* value_create_string(const string_t* val);
value_t* value_create_string_from(const char* val);
value_t* value_copy(const value_t* val);
void value_destroy(value_t* val);

// Type inspection
value_type_t value_type(const value_t* val);
bool value_is_int(const value_t* val);
bool value_is_float(const value_t* val);
bool value_is_string(const value_t* val);
bool value_is_numeric(const value_t* val);

// Value access (asserts correct type)
int64_t value_as_int(const value_t* val);
double value_as_float(const value_t* val);
const string_t* value_as_string(const value_t* val);

// Type conversions (always succeed, coerce as needed)
int64_t value_to_int(const value_t* val);
double value_to_float(const value_t* val);
string_t* value_to_string(const value_t* val);  // caller frees
bool value_to_bool(const value_t* val);

// Arithmetic operations (return NULL on error, set error code)
value_t* value_add(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_sub(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_mul(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_div(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_mod(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_neg(const value_t* val, value_error_t* err);
value_t* value_sqrt(const value_t* val, value_error_t* err);
value_t* value_floor(const value_t* val, value_error_t* err);

// Comparison operations (return NULL on error for type mismatch)
value_t* value_eq(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_ne(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_lt(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_le(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_gt(const value_t* a, const value_t* b, value_error_t* err);
value_t* value_ge(const value_t* a, const value_t* b, value_error_t* err);

// Logical operations (use truthiness, always succeed)
value_t* value_and(const value_t* a, const value_t* b);
value_t* value_or(const value_t* a, const value_t* b);
value_t* value_not(const value_t* val);

// Error description
const char* value_error_string(value_error_t err);

#endif // PSEUDO_VALUE_H
