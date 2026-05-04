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
    assert_lint_equals("hello world", "hello world\n");
}

TEST(single_replacement_arrow) {
    assert_lint_equals("a → b", "a -> b\n");
}

TEST(single_replacement_less_equal) {
    assert_lint_equals("x ≤ 5", "x <= 5\n");
}

TEST(single_replacement_not_equal) {
    assert_lint_equals("x ≠ y", "x != y\n");
}

TEST(single_replacement_greater_equal) {
    assert_lint_equals("x ≥ 10", "x >= 10\n");
}

TEST(single_replacement_left_arrow) {
    assert_lint_equals("x ← 5", "x <- 5\n");
}

TEST(single_replacement_square) {
    assert_lint_equals("■ algoritm", "sf algoritm\n");
}

TEST(multiple_replacements) {
    assert_lint_equals("x ≤ 5 ≠ y ≥ 10", "x <= 5 != y >= 10\n");
}

// After structural pass, a pipe/bar at depth-0 with no block context yields no indent
TEST(pipe_stripped) {
    assert_lint_equals("|code", "code\n");
}

TEST(pipe_space_stripped) {
    assert_lint_equals("| code", "code\n");
}

TEST(vertical_bar_stripped) {
    assert_lint_equals("│code", "code\n");
}

TEST(vertical_bar_space_stripped) {
    assert_lint_equals("│ code", "code\n");
}

TEST(smart_quotes) {
    assert_lint_equals("'hello'", "'hello'\n");
}

TEST(consecutive_replacements) {
    assert_lint_equals("→→→", "->->->\n");
}

TEST(replacement_at_start) {
    assert_lint_equals("≤start", "<=start\n");
}

TEST(replacement_at_end) {
    assert_lint_equals("end≥", "end>=\n");
}

TEST(mixed_text_and_replacements) {
    assert_lint_equals("dacă x ≤ 10 atunci\n│x ← x + 1", "daca x <= 10 atunci\n\tx <- x + 1\n");
}

TEST(empty_string) {
    assert_lint_equals("", "");
}

TEST(only_replacement) {
    assert_lint_equals("≠", "!=\n");
}

TEST(box_string) {
    assert_lint_equals("└■", "sf\n");
    assert_lint_equals("┌daca", "daca\n");
}

// Keyword normalization tests
TEST(keyword_citeste_variant1) {
    assert_lint_equals("citeşte x", "citeste x\n");
}

TEST(keyword_citeste_variant2) {
    assert_lint_equals("citește x", "citeste x\n");
}

TEST(keyword_cat_timp) {
    assert_lint_equals("cât timp x > 0", "cat timp x > 0\n");
}

TEST(keyword_pana_cand) {
    assert_lint_equals("până când done", "pana cand done\n");
}

TEST(keyword_daca) {
    assert_lint_equals("dacă x > 5", "daca x > 5\n");
}

TEST(keyword_executa) {
    assert_lint_equals("execută cod", "executa cod\n");
}

TEST(keyword_repeta) {
    assert_lint_equals("repetă 10", "repeta 10\n");
}

TEST(keyword_si_variant1) {
    assert_lint_equals("x și y", "x si y\n");
}

TEST(keyword_si_variant2) {
    assert_lint_equals("a şi b", "a si b\n");
}

// Structural indentation tests
TEST(structural_if) {
    assert_lint_equals(
        "daca x > 0 atunci\n"
        "scrie x\n"
        "sf",
        "daca x > 0 atunci\n"
        "\tscrie x\n"
        "sf\n"
    );
}

TEST(structural_if_else) {
    assert_lint_equals(
        "daca x > 0 atunci\n"
        "scrie x\n"
        "altfel\n"
        "scrie 0\n"
        "sf",
        "daca x > 0 atunci\n"
        "\tscrie x\n"
        "altfel\n"
        "\tscrie 0\n"
        "sf\n"
    );
}

TEST(structural_while) {
    assert_lint_equals(
        "cat timp x > 0 executa\n"
        "x <- x - 1\n"
        "sf",
        "cat timp x > 0 executa\n"
        "\tx <- x - 1\n"
        "sf\n"
    );
}

TEST(structural_for) {
    assert_lint_equals(
        "pentru i <- 1, 10 executa\n"
        "scrie i\n"
        "sf",
        "pentru i <- 1, 10 executa\n"
        "\tscrie i\n"
        "sf\n"
    );
}

TEST(structural_repeta) {
    assert_lint_equals(
        "repeta\n"
        "x <- x + 1\n"
        "pana cand x > 10",
        "repeta\n"
        "\tx <- x + 1\n"
        "pana cand x > 10\n"
    );
}

TEST(structural_do_while) {
    assert_lint_equals(
        "executa\n"
        "x <- x + 1\n"
        "cat timp x < 10",
        "executa\n"
        "\tx <- x + 1\n"
        "cat timp x < 10\n"
    );
}

TEST(structural_nested) {
    assert_lint_equals(
        "daca x > 0 atunci\n"
        "pentru i <- 1, x executa\n"
        "scrie i\n"
        "sf\n"
        "sf",
        "daca x > 0 atunci\n"
        "\tpentru i <- 1, x executa\n"
        "\t\tscrie i\n"
        "\tsf\n"
        "sf\n"
    );
}

TEST(structural_pipes_in_context) {
    assert_lint_equals(
        "daca x > 0 atunci\n"
        "|scrie x\n"
        "sf",
        "daca x > 0 atunci\n"
        "\tscrie x\n"
        "sf\n"
    );
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
        "\t\trepeta 3\n"
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
    RUN_TEST(pipe_stripped);
    RUN_TEST(pipe_space_stripped);
    RUN_TEST(vertical_bar_stripped);
    RUN_TEST(vertical_bar_space_stripped);
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
    RUN_TEST(structural_if);
    RUN_TEST(structural_if_else);
    RUN_TEST(structural_while);
    RUN_TEST(structural_for);
    RUN_TEST(structural_repeta);
    RUN_TEST(structural_do_while);
    RUN_TEST(structural_nested);
    RUN_TEST(structural_pipes_in_context);
    RUN_TEST(full_pseudocode_example);

    printf("\nAll tests passed\n");
    return 0;
}