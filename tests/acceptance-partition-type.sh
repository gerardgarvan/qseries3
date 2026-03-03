#!/usr/bin/env bash
# Phase 70: Partition Type Infrastructure acceptance tests
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }
QSERIES="$BIN --no-banner"
PASS=0; FAIL=0

run_test() {
    local label="$1" input="$2" expect="$3"
    local got
    got=$(echo "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if echo "$got" | grep -qF "$expect"; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected: $expect"
        echo "  got:      $got"
        ((FAIL++))
    fi
}

run_exact() {
    local label="$1" input="$2" expect="$3"
    local got
    got=$(echo "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if [ "$got" = "$expect" ]; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected: $expect"
        echo "  got:      $got"
        ((FAIL++))
    fi
}

count_lines() {
    local label="$1" input="$2" expect="$3"
    local got
    got=$(echo "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d' | wc -l | tr -d ' ')
    if [ "$got" = "$expect" ]; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected $expect lines, got $got"
        ((FAIL++))
    fi
}

# SC1: [4,2,2,1] evaluates and displays as [4, 2, 2, 1]
run_exact "list literal display" "[4,2,2,1]" "[4, 2, 2, 1]"

# SC2: variable assignment
run_exact "partition variable assign+recall" "$(printf 'p := [3,3,1]\np')" "[3, 3, 1]
[3, 3, 1]"

# SC3: partitions(5) has 7 partitions
count_lines "partitions(5) count" "partitions(5)" "7"

# SC4: empty list
run_exact "empty list" "[]" "[]"

# SC5: conjugate partition
run_exact "conjpart([4,2,2,1])" "conjpart([4,2,2,1])" "[4, 3, 1, 1]"

# Additional: conjpart on variable
run_exact "conjpart on variable" "$(printf 'p := [4,2,2,1]\nconjpart(p)')" "[4, 2, 2, 1]
[4, 3, 1, 1]"

# Partition statistics
run_exact "lp([5,3,2,1])" "lp([5,3,2,1])" "5"
run_exact "np_parts([5,3,2,1])" "np_parts([5,3,2,1])" "4"
run_exact "nep([4,2,2,1])" "nep([4,2,2,1])" "3"
run_exact "ptnnorm([5,3,2,1])" "ptnnorm([5,3,2,1])" "11"

# Conjugate involution: conjpart(conjpart(p)) = p
run_exact "conjpart involution" "conjpart(conjpart([5,3,2,1]))" "[5, 3, 2, 1]"

# partitions(0) = one empty partition
count_lines "partitions(0)" "partitions(0)" "1"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
