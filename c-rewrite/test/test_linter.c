#include "pseudo/linter.h"
#include "pseudo/string.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define TEST(name) static void test_##name(void)
#define RUN_TEST(name) do { \
    printf("Running test_%s...", #name); \
    test_##name(); \
    printf(" PASSED\n"); \
} while(0)

static void assert_lint_equals(const char* input, const char* expected) {
    string_t* input_str = string_create_from(input);
    string_t* result = lint(input_str);
    const char* result_cstr = string_cstr(result);

    if (strcmp(result_cstr, expected) != 0) {
        printf("\n  Expected: \"%s\"\n", expected);
        printf("  Got:      \"%s\"\n", result_cstr);
        assert(0 && "Linter output mismatch");
    }

    string_destroy(input_str);
    string_destroy(result);
}

TEST(no_replacements) {
    assert_lint_equals("hello world", "hello world");
}

TEST(single_replacement_arrow) {
    assert_lint_equals("a → b", "a -> b");
}

TEST(single_replacement_less_equal) {
    assert_lint_equals("x ≤ 5", "x <= 5");
}

TEST(single_replacement_not_equal) {
    assert_lint_equals("x ≠ y", "x != y");
}

TEST(single_replacement_greater_equal) {
    assert_lint_equals("x ≥ 10", "x >= 10");
}

TEST(single_replacement_left_arrow) {
    assert_lint_equals("x ← 5", "x <- 5");
}

TEST(single_replacement_square) {
    assert_lint_equals("■ algoritm", "sf algoritm");
}

TEST(multiple_replacements) {
    assert_lint_equals("x ≤ 5 ≠ y ≥ 10", "x <= 5 != y >= 10");
}

TEST(pipe_to_tab) {
    assert_lint_equals("|code", "\tcode");
}

TEST(pipe_space_to_tab) {
    assert_lint_equals("| code", "\tcode");
}

TEST(vertical_bar_to_tab) {
    assert_lint_equals("│code", "\tcode");
}

TEST(vertical_bar_space_to_tab) {
    assert_lint_equals("│ code", "\tcode");
}

TEST(smart_quotes) {
    assert_lint_equals("'hello'", "'hello'");
}

TEST(consecutive_replacements) {
    assert_lint_equals("→→→", "->->->");
}

TEST(replacement_at_start) {
    assert_lint_equals("≤start", "<=start");
}

TEST(replacement_at_end) {
    assert_lint_equals("end≥", "end>=");
}

TEST(mixed_text_and_replacements) {
    assert_lint_equals("dacă x ≤ 10 atunci\n│x ← x + 1", "daca x <= 10 atunci\n\tx <- x + 1");
}

TEST(empty_string) {
    assert_lint_equals("", "");
}

TEST(only_replacement) {
    assert_lint_equals("≠", "!=");
}

TEST(box_string) {
    assert_lint_equals("└■", "sf");
    assert_lint_equals("┌daca", "daca");
}

// Keyword normalization tests
TEST(keyword_citeste_variant1) {
    assert_lint_equals("citeşte x", "citeste x");
}

TEST(keyword_citeste_variant2) {
    assert_lint_equals("citește x", "citeste x");
}

TEST(keyword_cat_timp) {
    assert_lint_equals("cât timp x > 0", "cat timp x > 0");
}

TEST(keyword_pana_cand) {
    assert_lint_equals("până când done", "pana cand done");
}

TEST(keyword_daca) {
    assert_lint_equals("dacă x > 5", "daca x > 5");
}

TEST(keyword_executa) {
    assert_lint_equals("execută cod", "executa cod");
}

TEST(keyword_repeta) {
    assert_lint_equals("repetă 10", "repeta 10");
}

TEST(keyword_si_variant1) {
    assert_lint_equals("x și y", "x si y");
}

TEST(keyword_si_variant2) {
    assert_lint_equals("a şi b", "a si b");
}

TEST(full_pseudocode_example) {
    assert_lint_equals(
        "■ algoritm\n"
        "│ citeşte x\n"
        "│ dacă x ≥ 5 atunci\n"
        "│ │ execută cod\n"
        "│ altfel\n"
        "│ │ repetă 3",
        "sf algoritm\n"
        "\tciteste x\n"
        "\tdaca x >= 5 atunci\n"
        "\t\texecuta cod\n"
        "\taltfel\n"
        "\t\trepeta 3"
    );
}

int main(void) {
    printf("Running linter tests...\n\n");

    RUN_TEST(no_replacements);
    RUN_TEST(single_replacement_arrow);
    RUN_TEST(single_replacement_less_equal);
    RUN_TEST(single_replacement_not_equal);
    RUN_TEST(single_replacement_greater_equal);
    RUN_TEST(single_replacement_left_arrow);
    RUN_TEST(single_replacement_square);
    RUN_TEST(multiple_replacements);
    RUN_TEST(pipe_to_tab);
    RUN_TEST(pipe_space_to_tab);
    RUN_TEST(vertical_bar_to_tab);
    RUN_TEST(vertical_bar_space_to_tab);
    RUN_TEST(smart_quotes);
    RUN_TEST(consecutive_replacements);
    RUN_TEST(replacement_at_start);
    RUN_TEST(replacement_at_end);
    RUN_TEST(mixed_text_and_replacements);
    RUN_TEST(empty_string);
    RUN_TEST(only_replacement);
    RUN_TEST(box_string);
    RUN_TEST(keyword_citeste_variant1);
    RUN_TEST(keyword_citeste_variant2);
    RUN_TEST(keyword_cat_timp);
    RUN_TEST(keyword_pana_cand);
    RUN_TEST(keyword_daca);
    RUN_TEST(keyword_executa);
    RUN_TEST(keyword_repeta);
    RUN_TEST(keyword_si_variant1);
    RUN_TEST(keyword_si_variant2);
    RUN_TEST(full_pseudocode_example);

    printf("\nAll tests passed\n");
    return 0;
}
