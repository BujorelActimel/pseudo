// Generic hashmap with string keys and void* values
// Uses open addressing with linear probing

#ifndef PSEUDO_HASHMAP_H
#define PSEUDO_HASHMAP_H

#include <stddef.h>
#include <stdbool.h>

typedef struct hashmap hashmap_t;

hashmap_t* hashmap_create(size_t initial_capacity);
void hashmap_destroy(hashmap_t* map, void (*value_destructor)(void* ));

void hashmap_set(hashmap_t* map, const char* key, void* value);
void* hashmap_get(hashmap_t* map, const char* key);
bool hashmap_has(hashmap_t* map, const char* key);
bool hashmap_delete(hashmap_t* map, const char* key);

size_t hashmap_size(hashmap_t* map);
size_t hashmap_capacity(hashmap_t* map);

typedef void (*hashmap_iter_fn)(const char* key, void* value, void* user_data);
void hashmap_foreach(hashmap_t* map, hashmap_iter_fn callback, void* user_data);

void hashmap_clear(hashmap_t* map, void (*value_destructor)(void*));

#endif // PSEUDO_HASHMAP_H
