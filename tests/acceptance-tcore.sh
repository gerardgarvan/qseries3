#!/usr/bin/env bash
# Phase 71: Core t-core Algorithms acceptance tests
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }
QSERIES="$BIN --no-banner"
PASS=0; FAIL=0

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

# --- rvec ---
run_exact "rvec([4,2,2,1],3,0)" "rvec([4,2,2,1],3,0)" "4"
run_exact "rvec([4,2,2,1],3,1)" "rvec([4,2,2,1],3,1)" "2"
run_exact "rvec([4,2,2,1],3,2)" "rvec([4,2,2,1],3,2)" "3"
run_exact "rvec([],3,0)" "rvec([],3,0)" "0"

# --- istcore ---
run_exact "istcore([3,1],3) = true" "istcore([3,1],3)" "1"
run_exact "istcore([4,2,2,1],3) = false" "istcore([4,2,2,1],3)" "0"
run_exact "istcore([2,1,1],3) = true" "istcore([2,1,1],3)" "1"
run_exact "istcore([2,1],2) = true" "istcore([2,1],2)" "1"
run_exact "istcore([],5) = true" "istcore([],5)" "1"

# --- tcoreofptn ---
run_exact "tcoreofptn([4,2,2,1],3)" "tcoreofptn([4,2,2,1],3)" "[4, 2]"
run_exact "tcoreofptn([3,1],3) = self" "tcoreofptn([3,1],3)" "[3, 1]"
run_exact "tcoreofptn([],3)" "tcoreofptn([],3)" "[]"

# Verify 3-core of [4,2,2,1] IS a 3-core
run_exact "istcore(tcoreofptn result)" "istcore([4,2],3)" "1"

# --- tcores ---
count_lines "tcores(3,4) = 2 cores" "tcores(3,4)" "2"
run_exact "tcores(3,0) = empty ptn" "tcores(3,0)" "[]"
count_lines "tcores(2,3) = 1 (staircase)" "tcores(2,3)" "1"
run_exact "tcores(2,3) = [2,1]" "tcores(2,3)" "[2, 1]"

# No 2-cores of 5 (not triangular)
count_lines "tcores(2,5) = 0" "tcores(2,5)" "0"

# --- ptn2nvec / ptn2rvec ---
run_exact "ptn2nvec([4,2,2,1],3)" "ptn2nvec([4,2,2,1],3)" "[2, -1, -1]"
run_exact "ptn2rvec([4,2,2,1],3)" "ptn2rvec([4,2,2,1],3)" "[4, 2, 3]"

# --- Variable integration ---
run_exact "tcoreofptn on variable" "$(printf 'p := [4,2,2,1]\ntcoreofptn(p,3)')" "[4, 2, 2, 1]
[4, 2]"

# --- Size identity: |core| <= |ptn| ---
run_exact "ptnnorm of 3-core" "ptnnorm([4,2])" "6"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
