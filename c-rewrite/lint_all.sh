#!/usr/bin/env bash
#
# Lint script for all BAC exam programs
# Processes src.pseudo -> cleaned-src.pseudo for all tests
#

set -uo pipefail

# Configuration
PSEUDO_BIN="./build/debug/pseudo"
TEST_DIR="../testing/bac"
PROCESSED=0
FAILED=0

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
echo "  Linting All BAC Exam Programs"
echo "  Processing src.pseudo -> cleaned-src.pseudo"
echo "========================================"
echo ""

# Array to store failed tests for summary
declare -a FAILED_TESTS

# Run lint for each year
for year_dir in "$TEST_DIR"/*; do
    if [ ! -d "$year_dir" ]; then
        continue
    fi
    
    year=$(basename "$year_dir")
    echo -e "Processing year: ${YELLOW}$year${NC}"
    
    # Run lint for each exam
    for test_dir in "$year_dir"/*; do
        if [ ! -d "$test_dir" ]; then
            continue
        fi
        
        test_name=$(basename "$test_dir")
        test_path="$year/$test_name"
        
        # Check if src.pseudo exists
        if [ ! -f "$test_dir/src.pseudo" ]; then
            echo -e "  ${YELLOW}⊘${NC} $test_path - SKIP (no src.pseudo)"
            continue
        fi
        
        # Run lint command on src.pseudo -> cleaned-src.pseudo
        if "$PSEUDO_BIN" lint "$test_dir/src.pseudo" > "$test_dir/cleaned-src.pseudo" 2>/dev/null; then
            echo -e "  ${GREEN}✓${NC} $test_path"
            ((PROCESSED++))
        else
            echo -e "  ${RED}✗${NC} $test_path - FAIL (lint error)"
            ((FAILED++))
            FAILED_TESTS+=("$test_path")
        fi
    done
done

echo ""
echo "========================================"
echo "  Lint Results"
echo "========================================"
echo -e "  ${GREEN}Processed:${NC}  $PROCESSED"
echo -e "  ${RED}Failed:${NC}  $FAILED"
echo "  Total:   $((PROCESSED + FAILED))"
echo ""

# Show failed tests if any
if [ $FAILED -gt 0 ]; then
    echo "Failed tests:"
    for test in "${FAILED_TESTS[@]}"; do
        echo "  - $test"
    done
    echo ""
fi

echo ""

# Exit with error if any failed
if [ $FAILED -gt 0 ]; then
    exit 1
fi

exit 0