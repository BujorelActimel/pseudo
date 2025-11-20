#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SKIPPED_TESTS=0

declare -a FAILED_TEST_DETAILS
declare -a SKIPPED_TEST_DETAILS

print_color() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

print_header() {
    local message=$1
    echo
    print_color $CYAN "=================================================================================="
    print_color $CYAN "  $message"
    print_color $CYAN "=================================================================================="
    echo
}

check_required_files() {
    local test_dir=$1
    local missing_files=()
    
    [[ ! -f "$test_dir/src" ]] && missing_files+=("src")
    [[ ! -f "$test_dir/input" ]] && missing_files+=("input")
    [[ ! -f "$test_dir/expected-output" ]] && missing_files+=("expected-output")
    
    if [[ ${#missing_files[@]} -gt 0 ]]; then
        return 1
    fi
    return 0
}

run_test() {
    local test_dir=$1
    local test_name=$2
    
    echo -n "  Running $test_name... "
    
    if ! check_required_files "$test_dir"; then
        print_color $YELLOW "SKIPPED (missing files)"
        ((SKIPPED_TESTS++))
        SKIPPED_TEST_DETAILS+=("$test_name: Missing required files")
        return
    fi
    
    if [[ -f "$test_dir/src" ]]; then
        if ! scripts/linter.bash "$test_dir/src" > "$test_dir/cleaned-src" 2>/dev/null; then
            print_color $RED "FAILED (linting error)"
            ((FAILED_TESTS++))
            FAILED_TEST_DETAILS+=("$test_name: Linting failed")
            return
        fi
    fi
    
    if ! scripts/pseudo.bash "$test_dir/cleaned-src" --quiet < "$test_dir/input" > "$test_dir/output" 2>/dev/null; then
        print_color $RED "FAILED (runtime error)"
        ((FAILED_TESTS++))
        FAILED_TEST_DETAILS+=("$test_name: Runtime error during execution")
        return
    fi
    
    if diff -Z "$test_dir/expected-output" "$test_dir/output" > /dev/null 2>&1; then
        print_color $GREEN "PASSED"
        ((PASSED_TESTS++))
    else
        print_color $RED "FAILED (output mismatch)"
        ((FAILED_TESTS++))
        
        local diff_output=$(diff -Z "$test_dir/expected-output" "$test_dir/output" 2>/dev/null)
        FAILED_TEST_DETAILS+=("$test_name|$diff_output")
    fi
}

process_year() {
    local year_dir=$1
    local year=$(basename "$year_dir")
    
    print_header "Testing Year $year"
    
    local year_tests=0
    local year_passed=0
    local start_passed=$PASSED_TESTS
    
    while IFS= read -r -d '' session_dir; do
        local session=$(basename "$session_dir")
        echo
        print_color $BLUE "Session: $session"
        echo "----------------------------------------"
        
        local test_name="$year/$session"
        run_test "$session_dir" "$test_name"
        ((TOTAL_TESTS++))
        ((year_tests++))
        
    done < <(find "$year_dir" -mindepth 1 -maxdepth 1 -type d -print0 | sort -z)
    
    year_passed=$(($PASSED_TESTS - $start_passed))
    
    echo
    if [[ $year_tests -eq 0 ]]; then
        print_color $YELLOW "No tests found in year $year"
    else
        local year_failed=$((year_tests - year_passed))
        if [[ $year_failed -eq 0 ]]; then
            print_color $GREEN "Year $year: All $year_tests tests passed"
        else
            print_color $YELLOW "Year $year: $year_passed/$year_tests tests passed"
        fi
    fi
}

print_final_report() {
    print_header "FINAL TEST REPORT"
    
    echo "Test Summary:"
    echo "   Total tests: $TOTAL_TESTS"
    print_color $GREEN "   Passed: $PASSED_TESTS"
    print_color $RED "   Failed: $FAILED_TESTS"
    print_color $YELLOW "   Skipped: $SKIPPED_TESTS"
    echo
    
    if [[ $TOTAL_TESTS -eq 0 ]]; then
        print_color $YELLOW "No tests were found to run."
        return
    fi
    
    local success_rate=$((PASSED_TESTS * 100 / TOTAL_TESTS))
    echo "Success Rate: $success_rate%"
    echo
    
    if [[ $SKIPPED_TESTS -gt 0 ]]; then
        print_color $YELLOW "SKIPPED TESTS:"
        for detail in "${SKIPPED_TEST_DETAILS[@]}"; do
            echo "   • $detail"
        done
        echo
    fi
    
    if [[ $FAILED_TESTS -gt 0 ]]; then
        print_color $RED "FAILED TESTS:"
        echo
        for detail in "${FAILED_TEST_DETAILS[@]}"; do
            IFS='|' read -r test_name diff_output <<< "$detail"
            
            print_color $RED "   Test: $test_name"
            if [[ -n "$diff_output" ]]; then
                echo "      Diff output:"
                echo "$diff_output" | sed 's/^/        /'
            fi
            echo
        done
    else
        print_color $GREEN "All tests passed!"
    fi
    
    echo
    if [[ $FAILED_TESTS -eq 0 && $SKIPPED_TESTS -eq 0 ]]; then
        print_color $GREEN "RESULT: ALL TESTS SUCCESSFUL"
    elif [[ $FAILED_TESTS -eq 0 ]]; then
        print_color $YELLOW "RESULT: ALL RUNNABLE TESTS PASSED (some skipped)"
    else
        print_color $RED "RESULT: SOME TESTS FAILED - Review and fix the issues above"
    fi
}

main() {
    local test_root_dir=${1:-"testing/bac"}
    
    if [[ ! -d "$test_root_dir" ]]; then
        print_color $RED "Error: Test directory '$test_root_dir' does not exist."
        echo "Usage: $0 [test_directory]"
        echo "Example: $0 testing/bac/test"
        exit 1
    fi
    
    if [[ ! -f "scripts/linter.bash" ]]; then
        print_color $RED "Error: Linter script 'scripts/linter.bash' not found."
        echo "Make sure you're running this from the project root directory."
        exit 1
    fi
    
    if [[ ! -f "scripts/pseudo.bash" ]]; then
        print_color $RED "Error: Pseudocode script 'scripts/pseudo.bash' not found."
        echo "Make sure you're running this from the project root directory."
        exit 1
    fi
    
    print_header "PSEUDOCODE TEST SUITE RUNNER"
    print_color $BLUE "Test Directory: $test_root_dir"
    print_color $BLUE "Linter: scripts/linter.bash"
    print_color $BLUE "Interpreter: scripts/pseudo.bash"
    
    local found_years=false
    while IFS= read -r -d '' year_dir; do
        found_years=true
        process_year "$year_dir"
    done < <(find "$test_root_dir" -mindepth 1 -maxdepth 1 -type d -print0 | sort -z)
    
    if [[ "$found_years" == "false" ]]; then
        print_color $YELLOW "No year directories found in $test_root_dir"
        exit 1
    fi
    
    print_final_report
    
    if [[ $FAILED_TESTS -gt 0 ]]; then
        exit 1
    else
        exit 0
    fi
}

main "$@"
