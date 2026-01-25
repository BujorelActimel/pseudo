#include "pseudo/linter.h"
#include "pseudo/hashmap.h"
#include <string.h>

static hashmap_t* replacement_map = NULL;

static hashmap_t* get_replacement_map(void) {
    if (replacement_map) return replacement_map;

    replacement_map = hashmap_create(33);

    #define ADD_MAPPING(k, v) do { \
        string_t* key = string_create_from(k); \
        string_t* val = string_create_from(v); \
        hashmap_set(replacement_map, key, val); \
        string_destroy(key); \
        string_destroy(val); \
    } while(0)

    // Symbols
    ADD_MAPPING("≤", "<=");
    ADD_MAPPING("", "<=");
    ADD_MAPPING("≠", "!=");
    ADD_MAPPING("", "!=");
    ADD_MAPPING("≥", ">=");
    ADD_MAPPING("→", "->");
    ADD_MAPPING("←", "<-");
    ADD_MAPPING("", "<-");
    ADD_MAPPING("", "<-");
    ADD_MAPPING("", "<->");
    ADD_MAPPING("■", "sf");
    // might remove these
    ADD_MAPPING("<-->", "<->");
    ADD_MAPPING("<--->", "<->");

    // Indentation
    ADD_MAPPING("│ ", "    ");
    ADD_MAPPING("│", "    ");
    ADD_MAPPING("| ", "    ");
    ADD_MAPPING("|", "    ");

    // Quotes
    ADD_MAPPING("’", "'");
    ADD_MAPPING("‘", "'");
    ADD_MAPPING("”", "\"");
    ADD_MAPPING("„", "\"");

    // Box drawing
    ADD_MAPPING("┌", "");
    ADD_MAPPING("└", "");

    // Romanian diacritics
    ADD_MAPPING("ă", "a");
    ADD_MAPPING("â", "a");
    ADD_MAPPING("î", "i");
    ADD_MAPPING("ș", "s");
    ADD_MAPPING("ş", "s");
    ADD_MAPPING("ț", "t");
    ADD_MAPPING("ţ", "t");

    #undef ADD_MAPPING

    return replacement_map;
}

typedef struct {
    const string_t* source;
    size_t pos;
    const string_t* match_key;
    const string_t* match_value;
    size_t match_len;
} match_ctx_t;

static void find_match(const string_t* key, const string_t* value, void* user_data) {
    match_ctx_t* ctx = (match_ctx_t*)user_data;

    const char* source_str = string_cstr(ctx->source);
    const char* key_str = string_cstr(key);
    size_t key_len = string_length(key);
    size_t source_len = string_length(ctx->source);

    if (ctx->pos + key_len > source_len) return;

    if (strncmp(source_str + ctx->pos, key_str, key_len) == 0) {
        if (key_len > ctx->match_len) {
            ctx->match_key = key;
            ctx->match_value = value;
            ctx->match_len = key_len;
        }
    }
}

string_t* lint(const string_t* source) {
    hashmap_t* map = get_replacement_map();
    string_t* result = string_create();

    size_t length = string_length(source);

    for (size_t i = 0; i < length; ) {
        match_ctx_t ctx = {
            .source = source,
            .pos = i,
            .match_key = NULL,
            .match_value = NULL,
            .match_len = 0
        };

        hashmap_foreach(map, find_match, &ctx);

        if (ctx.match_value) {
            string_append_string(result, ctx.match_value);
            i += ctx.match_len;
        } else {
            string_append_char(result, string_at(source, i));
            i++;
        }
    }

    // Ensure output ends with newline
    size_t result_len = string_length(result);
    if (result_len > 0 && string_at(result, result_len - 1) != '\n') {
        string_append_char(result, '\n');
    }

    return result;
}
