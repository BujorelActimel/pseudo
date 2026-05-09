#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$SCRIPT_DIR/.."
PSEUDO="$ROOT/build/debug/pseudo"
TESTS_DIR="$ROOT/int-test/equivalence"

if [ ! -x "$PSEUDO" ]; then
    echo "Binary not found: $PSEUDO"
    echo "Run 'make' first."
    exit 1
fi

pass=0
fail=0

for dir in "$TESTS_DIR"/*/; do
    name=$(basename "$dir")
    src="$dir/src.pseudo"
    input_file="$dir/input.txt"
    expected="$dir/expected-output.txt"
    loop_line=$(cat "$dir/loop_line.txt")
    target=$(cat "$dir/target.txt")

    converted=$("$PSEUDO" equivalence "$src" "$loop_line" "$target" 2>&1)
    if echo "$converted" | grep -q '"error"'; then
        echo "FAIL [$name]: conversion error: $converted"
        fail=$((fail + 1))
        continue
    fi

    tmp=$(mktemp /tmp/equiv_XXXXXX.pseudo)
    echo "$converted" > "$tmp"

    actual=$("$PSEUDO" run "$tmp" < "$input_file" 2>&1)
    rm -f "$tmp"

    expected_val=$(cat "$expected")
    if [ "$actual" = "$expected_val" ]; then
        echo "PASS [$name]"
        pass=$((pass + 1))
    else
        echo "FAIL [$name]: expected='$expected_val' got='$actual'"
        fail=$((fail + 1))
    fi
done

echo ""
echo "Results: $pass passed, $fail failed"
[ "$fail" -eq 0 ]
