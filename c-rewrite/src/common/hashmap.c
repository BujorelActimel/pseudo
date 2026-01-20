#include "pseudo/hashmap.h"
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
    string_t* key;
    string_t* value;
    slot_state_t state;
} hashmap_entry_t;

struct hashmap {
    hashmap_entry_t* entries;
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

static void hashmap_resize(hashmap_t* map, size_t new_capacity);

hashmap_t* hashmap_create(size_t initial_capacity) {
    hashmap_t* map = malloc(sizeof(hashmap_t));
    if (!map) return NULL;

    if (initial_capacity < INITIAL_CAPACITY) {
        initial_capacity = INITIAL_CAPACITY;
    }

    map->capacity = initial_capacity;
    map->size = 0;
    map->deleted_count = 0;
    map->entries = calloc(initial_capacity, sizeof(hashmap_entry_t));

    if (!map->entries) {
        free(map);
        return NULL;
    }

    for (size_t i = 0; i < initial_capacity; i++) {
        map->entries[i].state = SLOT_EMPTY;
        map->entries[i].key = NULL;
        map->entries[i].value = NULL;
    }

    return map;
}

void hashmap_destroy(hashmap_t* map) {
    if (!map) return;

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].state == SLOT_OCCUPIED) {
            string_destroy(map->entries[i].key);
            string_destroy(map->entries[i].value);
        }
    }

    free(map->entries);
    free(map);
}

static hashmap_entry_t* find_entry(hashmap_t* map, const string_t* key) {
    uint64_t hash = hash_string(string_cstr(key));
    size_t index = hash % map->capacity;
    size_t start_index = index;

    do {
        hashmap_entry_t* entry = &map->entries[index];

        if (entry->state == SLOT_EMPTY) {
            return NULL; // Not found
        }

        if (entry->state == SLOT_OCCUPIED && string_equals_string(entry->key, key)) {
            return entry; // Found
        }

        index = (index + 1) % map->capacity;
    } while (index != start_index);

    return NULL; // Table is full (shouldn't happen with proper resizing)
}

static hashmap_entry_t* find_slot(hashmap_t* map, const string_t* key) {
    uint64_t hash = hash_string(string_cstr(key));
    size_t index = hash % map->capacity;
    size_t start_index = index;
    hashmap_entry_t* first_deleted = NULL;

    do {
        hashmap_entry_t* entry = &map->entries[index];

        if (entry->state == SLOT_EMPTY) {
            return first_deleted ? first_deleted : entry;
        }

        if (entry->state == SLOT_DELETED && !first_deleted) {
            first_deleted = entry;
        }

        if (entry->state == SLOT_OCCUPIED && string_equals_string(entry->key, key)) {
            return entry;
        }

        index = (index + 1) % map->capacity;
    } while (index != start_index);

    return first_deleted;
}

void hashmap_set(hashmap_t* map, const string_t* key, const string_t* value) {
    assert(map != NULL);
    assert(key != NULL);
    assert(value != NULL);

    double load_factor = (double)(map->size + map->deleted_count) / map->capacity;
    if (load_factor > LOAD_FACTOR) {
        hashmap_resize(map, map->capacity * 2);
    }

    hashmap_entry_t* entry = find_slot(map, key);
    assert(entry != NULL);

    if (entry->state == SLOT_OCCUPIED) {
        string_destroy(entry->value);
        entry->value = string_create_from_string(value);
    } else {
        if (entry->state == SLOT_DELETED) {
            map->deleted_count--;
        }
        entry->key = string_create_from_string(key);
        entry->value = string_create_from_string(value);
        entry->state = SLOT_OCCUPIED;
        map->size++;
    }
}

string_t* hashmap_get(hashmap_t* map, const string_t* key) {
    assert(map != NULL);
    assert(key != NULL);

    hashmap_entry_t* entry = find_entry(map, key);
    return entry ? entry->value : NULL;
}

bool hashmap_has(hashmap_t* map, const string_t* key) {
    assert(map != NULL);
    assert(key != NULL);

    return find_entry(map, key) != NULL;
}

bool hashmap_delete(hashmap_t* map, const string_t* key) {
    assert(map != NULL);
    assert(key != NULL);

    hashmap_entry_t* entry = find_entry(map, key);
    if (!entry) return false;

    // Mark as deleted (tombstone)
    string_destroy(entry->key);
    string_destroy(entry->value);
    entry->key = NULL;
    entry->value = NULL;
    entry->state = SLOT_DELETED;
    map->size--;
    map->deleted_count++;

    return true;
}

size_t hashmap_size(hashmap_t* map) {
    assert(map != NULL);
    return map->size;
}

size_t hashmap_capacity(hashmap_t* map) {
    assert(map != NULL);
    return map->capacity;
}

void hashmap_foreach(hashmap_t* map, hashmap_iter_fn callback, void* user_data) {
    assert(map != NULL);
    assert(callback != NULL);

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].state == SLOT_OCCUPIED) {
            callback(map->entries[i].key, map->entries[i].value, user_data);
        }
    }
}

void hashmap_clear(hashmap_t* map) {
    assert(map != NULL);

    for (size_t i = 0; i < map->capacity; i++) {
        if (map->entries[i].state == SLOT_OCCUPIED) {
            string_destroy(map->entries[i].key);
            string_destroy(map->entries[i].value);
            map->entries[i].key = NULL;
            map->entries[i].value = NULL;
            map->entries[i].state = SLOT_EMPTY;
        } else if (map->entries[i].state == SLOT_DELETED) {
            map->entries[i].state = SLOT_EMPTY;
        }
    }

    map->size = 0;
    map->deleted_count = 0;
}

static void hashmap_resize(hashmap_t* map, size_t new_capacity) {
    assert(map != NULL);

    // Create new entries array
    hashmap_entry_t* old_entries = map->entries;
    size_t old_capacity = map->capacity;

    map->entries = calloc(new_capacity, sizeof(hashmap_entry_t));
    assert(map->entries != NULL);

    map->capacity = new_capacity;
    map->size = 0;
    map->deleted_count = 0;

    for (size_t i = 0; i < new_capacity; i++) {
        map->entries[i].state = SLOT_EMPTY;
        map->entries[i].key = NULL;
        map->entries[i].value = NULL;
    }

    for (size_t i = 0; i < old_capacity; i++) {
        if (old_entries[i].state == SLOT_OCCUPIED) {
            hashmap_set(map, old_entries[i].key, old_entries[i].value);
            string_destroy(old_entries[i].key);
            string_destroy(old_entries[i].value);
        }
    }

    free(old_entries);
}
