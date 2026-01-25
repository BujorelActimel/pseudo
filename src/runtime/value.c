#include "pseudo/value.h"
#include "pseudo/string.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <inttypes.h>

struct value {
    value_type_t type;
    union {
        int64_t int_val;
        double float_val;
        string_t* string_val;
    };
};

// Creation

value_t* value_create_int(int64_t val) {
    value_t* v = malloc(sizeof(value_t));
    if (!v) return NULL;
    v->type = VALUE_INT;
    v->int_val = val;
    return v;
}

value_t* value_create_float(double val) {
    value_t* v = malloc(sizeof(value_t));
    if (!v) return NULL;
    v->type = VALUE_FLOAT;
    v->float_val = val;
    return v;
}

value_t* value_create_string(const string_t* val) {
    if (!val) return NULL;
    value_t* v = malloc(sizeof(value_t));
    if (!v) return NULL;
    v->type = VALUE_STRING;
    v->string_val = string_create_from_string(val);
    if (!v->string_val) {
        free(v);
        return NULL;
    }
    return v;
}

value_t* value_create_string_from(const char* val) {
    if (!val) return NULL;
    value_t* v = malloc(sizeof(value_t));
    if (!v) return NULL;
    v->type = VALUE_STRING;
    v->string_val = string_create_from(val);
    if (!v->string_val) {
        free(v);
        return NULL;
    }
    return v;
}

value_t* value_copy(const value_t* val) {
    if (!val) return NULL;
    switch (val->type) {
        case VALUE_INT:
            return value_create_int(val->int_val);
        case VALUE_FLOAT:
            return value_create_float(val->float_val);
        case VALUE_STRING:
            return value_create_string(val->string_val);
    }
    return NULL;
}

void value_destroy(value_t* val) {
    if (!val) return;
    if (val->type == VALUE_STRING) {
        string_destroy(val->string_val);
    }
    free(val);
}

// Type inspection

value_type_t value_type(const value_t* val) {
    if (!val) return VALUE_INT; // Safe default
    return val->type;
}

bool value_is_int(const value_t* val) {
    if (!val) return false;
    return val->type == VALUE_INT;
}

bool value_is_float(const value_t* val) {
    if (!val) return false;
    return val->type == VALUE_FLOAT;
}

bool value_is_string(const value_t* val) {
    if (!val) return false;
    return val->type == VALUE_STRING;
}

bool value_is_numeric(const value_t* val) {
    if (!val) return false;
    return val->type == VALUE_INT || val->type == VALUE_FLOAT;
}

// Value access

int64_t value_as_int(const value_t* val) {
    if (!val || val->type != VALUE_INT) return 0;
    return val->int_val;
}

double value_as_float(const value_t* val) {
    if (!val || val->type != VALUE_FLOAT) return 0.0;
    return val->float_val;
}

const string_t* value_as_string(const value_t* val) {
    if (!val || val->type != VALUE_STRING) return NULL;
    return val->string_val;
}

// Type conversions

int64_t value_to_int(const value_t* val) {
    if (!val || !value_is_numeric(val)) return 0;
    switch (val->type) {
        case VALUE_INT:
            return val->int_val;
        case VALUE_FLOAT:
            return (int64_t)val->float_val;
        case VALUE_STRING:
            return 0;
    }
    return 0;
}

double value_to_float(const value_t* val) {
    if (!val || !value_is_numeric(val)) return 0.0;
    switch (val->type) {
        case VALUE_INT:
            return (double)val->int_val;
        case VALUE_FLOAT:
            return val->float_val;
        case VALUE_STRING:
            return 0.0;
    }
    return 0.0;
}

string_t* value_to_string(const value_t* val) {
    if (!val) return NULL;
    char buffer[64];
    switch (val->type) {
        case VALUE_INT:
            snprintf(buffer, sizeof(buffer), "%" PRId64, val->int_val);
            return string_create_from(buffer);
        case VALUE_FLOAT: {
            if (val->float_val == floor(val->float_val) &&
                fabs(val->float_val) < 1e15) {
                snprintf(buffer, sizeof(buffer), "%.0f", val->float_val);
            } else {
                snprintf(buffer, sizeof(buffer), "%g", val->float_val);
            }
            return string_create_from(buffer);
        }
        case VALUE_STRING:
            return string_create_from_string(val->string_val);
    }
    return NULL;
}

bool value_to_bool(const value_t* val) {
    if (!val) return false;
    switch (val->type) {
        case VALUE_INT:
            return val->int_val != 0;
        case VALUE_FLOAT:
            return val->float_val != 0.0;
        case VALUE_STRING:
            return string_length(val->string_val) > 0;
    }
    return false;
}

// Helper macros
#define SET_OK(err) do { if (err) *err = VALUE_OK; } while(0)
#define SET_ERR(err, code) do { if (err) *err = code; } while(0)

static bool needs_float_math(const value_t* a, const value_t* b) {
    return a->type == VALUE_FLOAT || b->type == VALUE_FLOAT;
}

// Arithmetic operations

value_t* value_add(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }

    // String concatenation: both must be strings
    if (a->type == VALUE_STRING && b->type == VALUE_STRING) {
        SET_OK(err);
        string_t* result = string_create_from_string(a->string_val);
        string_append_string(result, b->string_val);
        value_t* v = value_create_string(result);
        string_destroy(result);
        return v;
    }

    // Numeric addition: both must be numeric
    if (!value_is_numeric(a) || !value_is_numeric(b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }

    SET_OK(err);
    if (needs_float_math(a, b)) {
        return value_create_float(value_to_float(a) + value_to_float(b));
    }
    return value_create_int(value_to_int(a) + value_to_int(b));
}

value_t* value_sub(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !value_is_numeric(a) || !value_is_numeric(b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    if (needs_float_math(a, b)) {
        return value_create_float(value_to_float(a) - value_to_float(b));
    }
    return value_create_int(value_to_int(a) - value_to_int(b));
}

value_t* value_mul(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !value_is_numeric(a) || !value_is_numeric(b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    if (needs_float_math(a, b)) {
        return value_create_float(value_to_float(a) * value_to_float(b));
    }
    return value_create_int(value_to_int(a) * value_to_int(b));
}

value_t* value_div(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !value_is_numeric(a) || !value_is_numeric(b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    double divisor = value_to_float(b);
    if (divisor == 0.0) {
        SET_ERR(err, VALUE_ERR_DIV_ZERO);
        return NULL;
    }
    SET_OK(err);
    // Division always returns float (use [x/y] for integer division)
    return value_create_float(value_to_float(a) / divisor);
}

value_t* value_mod(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !value_is_numeric(a) || !value_is_numeric(b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    int64_t divisor = value_to_int(b);
    if (divisor == 0) {
        SET_ERR(err, VALUE_ERR_DIV_ZERO);
        return NULL;
    }
    SET_OK(err);
    return value_create_int(value_to_int(a) % divisor);
}

value_t* value_neg(const value_t* val, value_error_t* err) {
    if (!val || !value_is_numeric(val)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    if (val->type == VALUE_FLOAT) {
        return value_create_float(-val->float_val);
    }
    return value_create_int(-val->int_val);
}

value_t* value_sqrt(const value_t* val, value_error_t* err) {
    if (!val || !value_is_numeric(val)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    double d = value_to_float(val);
    if (d < 0) {
        SET_ERR(err, VALUE_ERR_NEGATIVE_SQRT);
        return NULL;
    }
    SET_OK(err);
    double result = sqrt(d);
    if (result == floor(result)) {
        return value_create_int((int64_t)result);
    }
    return value_create_float(result);
}

value_t* value_floor(const value_t* val, value_error_t* err) {
    if (!val || !value_is_numeric(val)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    return value_create_int((int64_t)floor(value_to_float(val)));
}

// Comparison operations

static bool types_comparable(const value_t* a, const value_t* b) {
    if (value_is_numeric(a) && value_is_numeric(b)) return true;
    if (a->type == VALUE_STRING && b->type == VALUE_STRING) return true;
    return false;
}

static int compare_values(const value_t* a, const value_t* b) {
    if (a->type == VALUE_STRING && b->type == VALUE_STRING) {
        return strcmp(string_cstr(a->string_val), string_cstr(b->string_val));
    }
    double fa = value_to_float(a);
    double fb = value_to_float(b);
    if (fa < fb) return -1;
    if (fa > fb) return 1;
    return 0;
}

value_t* value_eq(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !types_comparable(a, b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    return value_create_int(compare_values(a, b) == 0 ? 1 : 0);
}

value_t* value_ne(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !types_comparable(a, b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    return value_create_int(compare_values(a, b) != 0 ? 1 : 0);
}

value_t* value_lt(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !types_comparable(a, b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    return value_create_int(compare_values(a, b) < 0 ? 1 : 0);
}

value_t* value_le(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !types_comparable(a, b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    return value_create_int(compare_values(a, b) <= 0 ? 1 : 0);
}

value_t* value_gt(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !types_comparable(a, b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    return value_create_int(compare_values(a, b) > 0 ? 1 : 0);
}

value_t* value_ge(const value_t* a, const value_t* b, value_error_t* err) {
    if (!a || !b || !types_comparable(a, b)) {
        SET_ERR(err, VALUE_ERR_TYPE);
        return NULL;
    }
    SET_OK(err);
    return value_create_int(compare_values(a, b) >= 0 ? 1 : 0);
}

// Logical operations (always succeed, use truthiness)

value_t* value_and(const value_t* a, const value_t* b) {
    if (!a || !b) return value_create_int(0);
    return value_create_int(value_to_bool(a) && value_to_bool(b) ? 1 : 0);
}

value_t* value_or(const value_t* a, const value_t* b) {
    if (!a || !b) return value_create_int(0);
    return value_create_int(value_to_bool(a) || value_to_bool(b) ? 1 : 0);
}

value_t* value_not(const value_t* val) {
    if (!val) return value_create_int(1);
    return value_create_int(value_to_bool(val) ? 0 : 1);
}

// Error description

const char* value_error_string(value_error_t err) {
    switch (err) {
        case VALUE_OK:
            return "Fara eroare";
        case VALUE_ERR_TYPE:
            return "Tipuri incompatibile";
        case VALUE_ERR_DIV_ZERO:
            return "Impartire la zero";
        case VALUE_ERR_NEGATIVE_SQRT:
            return "Nu se poate calcula radicalul unui numar negativ";
    }
    return "Eroare necunoscuta";
}

// Additional helper functions

value_t* value_create_string_buf(const char* val, size_t len) {
    if (!val) return NULL;
    value_t* v = malloc(sizeof(value_t));
    if (!v) return NULL;
    
    v->type = VALUE_STRING;
    v->string_val = string_create_from_buf(val, len);
    if (!v->string_val) {
        free(v);
        return NULL;
    }
    
    return v;
}

value_t* value_clone(const value_t* val) {
    return value_copy(val);
}
