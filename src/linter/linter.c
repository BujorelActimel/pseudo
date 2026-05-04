#include "pseudo/linter.h"
#include "pseudo/hashmap.h"
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

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

// Structural indentation helpers

static bool word_starts(const char* str, const char* word) {
    size_t wlen = strlen(word);
    return strncmp(str, word, wlen) == 0 &&
           (str[wlen] == '\0' || str[wlen] == ' ' || str[wlen] == '\t');
}

static bool word_ends(const char* str, size_t len, const char* word) {
    size_t wlen = strlen(word);
    if (len < wlen) return false;
    if (memcmp(str + len - wlen, word, wlen) != 0) return false;
    if (len == wlen) return true;
    char prev = str[len - wlen - 1];
    return prev == ' ' || prev == '\t';
}

// Lines that decrease indent BEFORE they are printed
static bool is_dedent_line(const char* line, size_t len) {
    // "sf" alone closes a block
    if (len == 2 && memcmp(line, "sf", 2) == 0) return true;
    // altfel [daca ... atunci] — else / else-if
    if (word_starts(line, "altfel")) return true;
    // pana cand <condition> — end of repeat-until
    if (len >= 9 && memcmp(line, "pana cand", 9) == 0 &&
        (len == 9 || line[9] == ' ')) return true;
    // cat timp <condition> WITHOUT trailing executa — end of do-while
    if (len >= 8 && memcmp(line, "cat timp", 8) == 0 &&
        (len == 8 || line[8] == ' ') && !word_ends(line, len, "executa")) return true;
    return false;
}

// Lines that increase indent AFTER they are printed
static bool is_indent_line(const char* line, size_t len) {
    // daca ... atunci / altfel daca ... atunci
    if (word_ends(line, len, "atunci")) return true;
    // para .../cat timp .../executa (standalone do-while opener)
    if (word_ends(line, len, "executa")) return true;
    // repeta (standalone — start of repeat-until)
    if (len == 6 && memcmp(line, "repeta", 6) == 0) return true;
    // altfel opens its own block
    if (word_starts(line, "altfel")) return true;
    // "sf <name>" — algorithm header, opens the algorithm body
    if (len > 3 && memcmp(line, "sf ", 3) == 0) return true;
    return false;
}

// Second pass: strip all leading whitespace and reindent based on block structure
static string_t* structural_indent(const string_t* source) {
    const char* src = string_cstr(source);
    size_t src_len = string_length(source);
    string_t* result = string_create();

    int depth = 0;
    size_t pos = 0;
    bool need_newline = false;

    while (pos < src_len) {
        size_t line_start = pos;
        while (pos < src_len && src[pos] != '\n') pos++;

        const char* raw = src + line_start;
        size_t raw_len = pos - line_start;

        // Strip leading whitespace (tabs from │/| substitution and any spaces)
        size_t l = 0;
        while (l < raw_len && (raw[l] == ' ' || raw[l] == '\t')) l++;

        // Strip trailing whitespace and \r
        size_t r = raw_len;
        while (r > l && (raw[r-1] == ' ' || raw[r-1] == '\t' || raw[r-1] == '\r')) r--;

        size_t line_len = r - l;

        if (need_newline) string_append_char(result, '\n');
        need_newline = true;

        if (line_len > 0) {
            char* line_cstr = malloc(line_len + 1);
            memcpy(line_cstr, raw + l, line_len);
            line_cstr[line_len] = '\0';

            if (is_dedent_line(line_cstr, line_len) && depth > 0) depth--;

            for (int i = 0; i < depth; i++) string_append_char(result, '\t');
            string_append_buf(result, raw + l, line_len);

            if (is_indent_line(line_cstr, line_len)) depth++;

            free(line_cstr);
        }

        if (pos < src_len) pos++;  // skip '\n'
    }

    if (string_length(result) > 0) string_append_char(result, '\n');

    return result;
}

string_t* lint(const string_t* source) {
    hashmap_t* map = get_replacement_map();
    string_t* substituted = string_create();

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
            string_append_string(substituted, ctx.match_value);
            i += ctx.match_len;
        } else {
            string_append_char(substituted, string_at(source, i));
            i++;
        }
    }

    string_t* result = structural_indent(substituted);
    string_destroy(substituted);
    return result;
}