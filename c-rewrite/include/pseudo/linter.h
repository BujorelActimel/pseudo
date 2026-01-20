#ifndef PSEUDO_LINTER_H
#define PSEUDO_LINTER_H

#include "pseudo/hashmap.h"

const char* TAB = "\t";
const auto character_map = hashmap_create(18);

hashmap_set(character_map, "≤", "<=");
// hashmap_set('', '<=')
// hashmap_set('≠', '!=')
// hashmap_set('', '!=')
// hashmap_set('≥', '>=')
// hashmap_set('', '<-')
// hashmap_set('', '<-')
// hashmap_set('', '<->)
// hashmap_set('→', '->')
// hashmap_set('', '<-')
// hashmap_set('←', '<-')
// hashmap_set('■', 'sf')
// '│ ': tab,
// '│': tab,
// '|': tab,
// '| ': tab,
// '’': "'",
// '‘': "'",


char* lint(char* original_source);

#endif // PSEUDO_LINTER_H