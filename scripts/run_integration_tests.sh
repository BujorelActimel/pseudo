#!/usr/bin/env bash
#
# Integration test runner for C pseudocode interpreter
# Tests against all BAC exam programs (2020-2025)
#
# Process:
# 1. Run interpreter on cleaned-src.pseudo with input.txt -> output.txt
# 2. Compare output.txt with expected-output.txt
# 3. (Optional) Transpile to C/C++/Pascal, compile, run, and compare output
#

set -uo pipefail

# Configuration
PSEUDO_BIN="./build/release/pseudo"
TEST_DIR="./int-test/bac"
PASSED=0
FAILED=0
SKIPPED=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

# Check if pseudo binary exists
if [ ! -f "$PSEUDO_BIN" ]; then
    echo -e "${RED}Error: $PSEUDO_BIN not found. Run 'make' first.${NC}"
    exit 1
fi

# Check if test directory exists
if [ ! -d "$TEST_DIR" ]; then
    echo -e "${RED}Error: Test directory $TEST_DIR not found.${NC}"
    exit 1
fi

# Detect available transpile compilers
HAVE_GCC=0; HAVE_GPP=0; HAVE_FPC=0
command -v gcc &>/dev/null && HAVE_GCC=1
command -v g++ &>/dev/null && HAVE_GPP=1
command -v fpc &>/dev/null && HAVE_FPC=1

echo "========================================"
echo "  Romanian Pseudocode Interpreter Test"
echo "  C Implementation Integration Tests"
echo "========================================"
echo ""

# Array to store failed tests for summary
declare -a FAILED_TESTS

# ─── Helper: run one transpile test ──────────────────────────────────────────
run_transpile_test() {
    local test_dir="$1"
    local test_path="$2"
    local lang="$3"       # c | cpp | pascal
    local ext="$4"        # c | cpp | pas
    local compiler="$5"   # gcc | g++ | fpc
    local have_compiler="$6"

    if [ "$have_compiler" = "0" ]; then
        echo -e "    ${YELLOW}⊘${NC} transpile/$lang - SKIP ($compiler not found)"
        ((SKIPPED++))
        return
    fi

    local src_file="/tmp/pseudo_transpile_test.$ext"
    local bin_file="/tmp/pseudo_transpile_bin"
    local out_file="/tmp/pseudo_transpile_out.txt"

    # Transpile
    "$PSEUDO_BIN" transpile "$lang" "$test_dir/cleaned-src.pseudo" > "$src_file" 2>/dev/null || {
        echo -e "    ${RED}✗${NC} transpile/$lang - FAIL (transpile error)"
        ((FAILED++))
        FAILED_TESTS+=("$test_path (transpile/$lang transpile error)")
        return
    }

    # Compile
    local compile_ok=0
    case "$lang" in
        c)      gcc "$src_file" -lm -o "$bin_file" 2>/dev/null && compile_ok=1 ;;
        cpp)    g++ "$src_file" -o "$bin_file" 2>/dev/null && compile_ok=1 ;;
        pascal) fpc "$src_file" -o"$bin_file" 2>/dev/null && compile_ok=1 ;;
    esac

    if [ "$compile_ok" = "0" ]; then
        echo -e "    ${RED}✗${NC} transpile/$lang - FAIL (compile error)"
        ((FAILED++))
        FAILED_TESTS+=("$test_path (transpile/$lang compile error)")
        return
    fi

    # Run and compare
    "$bin_file" < "$test_dir/input.txt" > "$out_file" 2>/dev/null || true
    if diff -q "$out_file" "$test_dir/expected-output.txt" > /dev/null 2>&1; then
        echo -e "    ${GREEN}✓${NC} transpile/$lang"
        ((PASSED++))
    else
        echo -e "    ${RED}✗${NC} transpile/$lang - FAIL (output mismatch)"
        ((FAILED++))
        FAILED_TESTS+=("$test_path (transpile/$lang output)")
        if [ "${VERBOSE:-0}" = "1" ]; then
            echo "      Diff:"
            diff "$test_dir/expected-output.txt" "$out_file" | head -10
        fi
    fi
}

# ─── Main test loop ───────────────────────────────────────────────────────────

for year_dir in "$TEST_DIR"/*; do
    if [ ! -d "$year_dir" ]; then
        continue
    fi

    year=$(basename "$year_dir")
    echo -e "Testing year: ${YELLOW}$year${NC}"

    for test_dir in "$year_dir"/*; do
        if [ ! -d "$test_dir" ]; then
            continue
        fi

        test_name=$(basename "$test_dir")
        test_path="$year/$test_name"

        # Check if required files exist
        if [ ! -f "$test_dir/cleaned-src.pseudo" ]; then
            echo -e "  ${YELLOW}⊘${NC} $test_path - SKIP (no cleaned-src.pseudo)"
            ((SKIPPED++))
            continue
        fi

        if [ ! -f "$test_dir/input.txt" ]; then
            echo -e "  ${YELLOW}⊘${NC} $test_path - SKIP (no input.txt)"
            ((SKIPPED++))
            continue
        fi

        if [ ! -f "$test_dir/expected-output.txt" ]; then
            echo -e "  ${YELLOW}⊘${NC} $test_path - SKIP (no expected-output.txt)"
            ((SKIPPED++))
            continue
        fi

        # ── Interpreter test ──────────────────────────────────────────────
        "$PSEUDO_BIN" run "$test_dir/cleaned-src.pseudo" < "$test_dir/input.txt" > "$test_dir/output.txt" 2>/dev/null || {
            echo -e "  ${RED}✗${NC} $test_path - FAIL (execution error)"
            ((FAILED++))
            FAILED_TESTS+=("$test_path (execution)")
            continue
        }

        if diff -q "$test_dir/output.txt" "$test_dir/expected-output.txt" > /dev/null 2>&1; then
            echo -e "  ${GREEN}✓${NC} $test_path"
            ((PASSED++))
        else
            echo -e "  ${RED}✗${NC} $test_path - FAIL (output mismatch)"
            ((FAILED++))
            FAILED_TESTS+=("$test_path (output)")
            if [ "${VERBOSE:-0}" = "1" ]; then
                echo "    Diff:"
                diff "$test_dir/expected-output.txt" "$test_dir/output.txt" | head -20
            fi
        fi

        # ── Transpile tests (only if TRANSPILE=1 or TRANSPILE_ALL=1) ─────
        if [ "${TRANSPILE:-0}" = "1" ] || [ "${TRANSPILE_ALL:-0}" = "1" ]; then
            echo -e "  ${CYAN}↪${NC} $test_path (transpile)"
            run_transpile_test "$test_dir" "$test_path" "c"      "c"   "gcc" "$HAVE_GCC"
            run_transpile_test "$test_dir" "$test_path" "cpp"    "cpp" "g++" "$HAVE_GPP"
            run_transpile_test "$test_dir" "$test_path" "pascal" "pas" "fpc" "$HAVE_FPC"
        fi
    done
done

echo ""
echo "========================================"
echo "  Test Results"
echo "========================================"
echo -e "  ${GREEN}Passed:${NC}  $PASSED"
echo -e "  ${RED}Failed:${NC}  $FAILED"
echo -e "  ${YELLOW}Skipped:${NC} $SKIPPED"
echo "  Total:   $((PASSED + FAILED + SKIPPED))"
echo ""

if [ $FAILED -gt 0 ]; then
    echo "Failed tests:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
    echo "Run with VERBOSE=1 to see detailed output differences."
    echo "Run with TRANSPILE=1 to also test transpiled C and C++ output."
    echo ""
fi

if [ $((PASSED + FAILED)) -gt 0 ]; then
    success_rate=$((PASSED * 100 / (PASSED + FAILED)))
    echo "Success rate: $success_rate%"
fi

echo ""

if [ $FAILED -gt 0 ]; then
    exit 1
fi

exit 0
