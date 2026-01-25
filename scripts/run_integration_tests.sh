#!/usr/bin/env bash
#
# Integration test runner for C pseudocode interpreter
# Tests against all BAC exam programs (2020-2025)
#
# Process:
# 1. Run interpreter on cleaned-src.pseudo with input.txt -> output.txt
# 2. Compare output.txt with expected-output.txt
#

set -uo pipefail

# Configuration
PSEUDO_BIN="./build/debug/pseudo"
TEST_DIR="./int-test/bac"
PASSED=0
FAILED=0
SKIPPED=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
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

echo "========================================"
echo "  Romanian Pseudocode Interpreter Test"
echo "  C Implementation Integration Tests"
echo "========================================"
echo ""

# Array to store failed tests for summary
declare -a FAILED_TESTS

# Run tests for each year
for year_dir in "$TEST_DIR"/*; do
    if [ ! -d "$year_dir" ]; then
        continue
    fi
    
    year=$(basename "$year_dir")
    echo -e "Testing year: ${YELLOW}$year${NC}"
    
    # Run tests for each exam
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
        
        # Step 1: Run interpreter with input.txt -> output.txt
        "$PSEUDO_BIN" run "$test_dir/cleaned-src.pseudo" < "$test_dir/input.txt" > "$test_dir/output.txt" 2>/dev/null || {
            echo -e "  ${RED}✗${NC} $test_path - FAIL (execution error)"
            ((FAILED++))
            FAILED_TESTS+=("$test_path (execution)")
            continue
        }
        
        # Step 2: Compare output.txt with expected-output.txt
        if diff -q "$test_dir/output.txt" "$test_dir/expected-output.txt" > /dev/null 2>&1; then
            echo -e "  ${GREEN}✓${NC} $test_path"
            ((PASSED++))
        else
            echo -e "  ${RED}✗${NC} $test_path - FAIL (output mismatch)"
            ((FAILED++))
            FAILED_TESTS+=("$test_path (output)")
            
            # Show diff if verbose mode
            if [ "${VERBOSE:-0}" = "1" ]; then
                echo "    Diff:"
                diff "$test_dir/expected-output.txt" "$test_dir/output.txt" | head -20
            fi
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

# Show failed tests if any
if [ $FAILED -gt 0 ]; then
    echo "Failed tests:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
    echo "Run with VERBOSE=1 to see detailed output differences."
    echo ""
fi

# Calculate success rate
if [ $((PASSED + FAILED)) -gt 0 ]; then
    success_rate=$((PASSED * 100 / (PASSED + FAILED)))
    echo "Success rate: $success_rate%"
fi

echo ""

# Exit with error if tests failed
if [ $FAILED -gt 0 ]; then
    exit 1
fi

exit 0