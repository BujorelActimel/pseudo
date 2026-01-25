#include "pseudo/environment.h"
#include "pseudo/string.h"
#include "pseudo/value.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#define INITIAL_CAPACITY 16
#define LOAD_FACTOR 0.7

typedef enum {
    SLOT_EMPTY,
    SLOT_OCCUPIED,
    SLOT_DELETED
} slot_state_t;

typedef struct {
    string_t* name;
    value_t* value;
    slot_state_t state;
} env_entry_t;

struct environment {
    env_entry_t* entries;
    size_t capacity;
    size_t size;
    size_t deleted_count;
};

static uint64_t hash_string(const char* str) {
    uint64_t hash = 14695981039346656037ULL;
    while (*str) {
        hash ^= (uint64_t)(unsigned char)(*str++);
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void env_resize(environment_t* env, size_t new_capacity);

environment_t* env_create(void) {
    environment_t* env = malloc(sizeof(environment_t));
    if (!env) return NULL;

    env->capacity = INITIAL_CAPACITY;
    env->size = 0;
    env->deleted_count = 0;
    env->entries = calloc(INITIAL_CAPACITY, sizeof(env_entry_t));

    if (!env->entries) {
        free(env);
        return NULL;
    }

    for (size_t i = 0; i < INITIAL_CAPACITY; i++) {
        env->entries[i].state = SLOT_EMPTY;
        env->entries[i].name = NULL;
        env->entries[i].value = NULL;
    }

    return env;
}

void env_destroy(environment_t* env) {
    if (!env) return;

    for (size_t i = 0; i < env->capacity; i++) {
        if (env->entries[i].state == SLOT_OCCUPIED) {
            string_destroy(env->entries[i].name);
            value_destroy(env->entries[i].value);
        }
    }

    free(env->entries);
    free(env);
}

static env_entry_t* find_entry(environment_t* env, const string_t* name) {
    uint64_t hash = hash_string(string_cstr(name));
    size_t index = hash % env->capacity;
    size_t start_index = index;

    do {
        env_entry_t* entry = &env->entries[index];

        if (entry->state == SLOT_EMPTY) {
            return NULL;
        }

        if (entry->state == SLOT_OCCUPIED && string_equals_string(entry->name, name)) {
            return entry;
        }

        index = (index + 1) % env->capacity;
    } while (index != start_index);

    return NULL;
}

static env_entry_t* find_slot(environment_t* env, const string_t* name) {
    uint64_t hash = hash_string(string_cstr(name));
    size_t index = hash % env->capacity;
    size_t start_index = index;
    env_entry_t* first_deleted = NULL;

    do {
        env_entry_t* entry = &env->entries[index];

        if (entry->state == SLOT_EMPTY) {
            return first_deleted ? first_deleted : entry;
        }

        if (entry->state == SLOT_DELETED && !first_deleted) {
            first_deleted = entry;
        }

        if (entry->state == SLOT_OCCUPIED && string_equals_string(entry->name, name)) {
            return entry;
        }

        index = (index + 1) % env->capacity;
    } while (index != start_index);

    return first_deleted;
}

void env_set(environment_t* env, const string_t* name, value_t* value) {
    assert(env != NULL);
    assert(name != NULL);
    assert(value != NULL);

    double load_factor = (double)(env->size + env->deleted_count) / env->capacity;
    if (load_factor > LOAD_FACTOR) {
        env_resize(env, env->capacity * 2);
    }

    env_entry_t* entry = find_slot(env, name);
    assert(entry != NULL);

    if (entry->state == SLOT_OCCUPIED) {
        value_destroy(entry->value);
        entry->value = value;
    } else {
        if (entry->state == SLOT_DELETED) {
            env->deleted_count--;
        }
        entry->name = string_create_from_string(name);
        entry->value = value;
        entry->state = SLOT_OCCUPIED;
        env->size++;
    }
}

value_t* env_get(environment_t* env, const string_t* name) {
    assert(env != NULL);
    assert(name != NULL);

    env_entry_t* entry = find_entry(env, name);
    return entry ? entry->value : NULL;
}

bool env_has(environment_t* env, const string_t* name) {
    assert(env != NULL);
    assert(name != NULL);

    return find_entry(env, name) != NULL;
}

void env_clear(environment_t* env) {
    assert(env != NULL);

    for (size_t i = 0; i < env->capacity; i++) {
        if (env->entries[i].state == SLOT_OCCUPIED) {
            string_destroy(env->entries[i].name);
            value_destroy(env->entries[i].value);
            env->entries[i].name = NULL;
            env->entries[i].value = NULL;
            env->entries[i].state = SLOT_EMPTY;
        } else if (env->entries[i].state == SLOT_DELETED) {
            env->entries[i].state = SLOT_EMPTY;
        }
    }

    env->size = 0;
    env->deleted_count = 0;
}

size_t env_size(environment_t* env) {
    assert(env != NULL);
    return env->size;
}

void env_foreach(environment_t* env, env_iter_fn callback, void* user_data) {
    assert(env != NULL);
    assert(callback != NULL);

    for (size_t i = 0; i < env->capacity; i++) {
        if (env->entries[i].state == SLOT_OCCUPIED) {
            callback(env->entries[i].name, env->entries[i].value, user_data);
        }
    }
}

static void env_resize(environment_t* env, size_t new_capacity) {
    assert(env != NULL);

    env_entry_t* old_entries = env->entries;
    size_t old_capacity = env->capacity;

    env->entries = calloc(new_capacity, sizeof(env_entry_t));
    assert(env->entries != NULL);

    env->capacity = new_capacity;
    env->size = 0;
    env->deleted_count = 0;

    for (size_t i = 0; i < new_capacity; i++) {
        env->entries[i].state = SLOT_EMPTY;
        env->entries[i].name = NULL;
        env->entries[i].value = NULL;
    }

    for (size_t i = 0; i < old_capacity; i++) {
        if (old_entries[i].state == SLOT_OCCUPIED) {
            // Transfer ownership directly
            env_entry_t* slot = find_slot(env, old_entries[i].name);
            slot->name = old_entries[i].name;
            slot->value = old_entries[i].value;
            slot->state = SLOT_OCCUPIED;
            env->size++;
        }
    }

    free(old_entries);
}
