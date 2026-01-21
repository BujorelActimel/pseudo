#ifndef PSEUDO_ENVIRONMENT_H
#define PSEUDO_ENVIRONMENT_H

#include "pseudo/string.h"
#include "pseudo/value.h"
#include <stdbool.h>
#include <stddef.h>

typedef struct environment environment_t;

environment_t* env_create(void);
void env_destroy(environment_t* env);

// Set a variable (takes ownership of value)
void env_set(environment_t* env, const string_t* name, value_t* value);

// Get a variable (returns NULL if undefined, does not transfer ownership)
value_t* env_get(environment_t* env, const string_t* name);

// Check if variable exists
bool env_has(environment_t* env, const string_t* name);

// Clear all variables
void env_clear(environment_t* env);

// Get number of variables
size_t env_size(environment_t* env);

// Iteration callback
typedef void (*env_iter_fn)(const string_t* name, const value_t* value, void* user_data);
void env_foreach(environment_t* env, env_iter_fn callback, void* user_data);

#endif // PSEUDO_ENVIRONMENT_H
