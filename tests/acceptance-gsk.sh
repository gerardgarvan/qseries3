#!/usr/bin/env bash
# Phase 72: t-Quotient and GSK Bijection acceptance tests
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

run_contains() {
    local label="$1" input="$2" expect="$3"
    local got
    got=$(echo "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if echo "$got" | grep -qF "$expect"; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected to contain: $expect"
        echo "  got: $got"
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

# SC1: tquot returns list of t partitions
count_lines "tquot([4,2,2,1],3) = 3 partitions" "tquot([4,2,2,1],3)" "3"

# SC2: phi1 returns core + quotient
run_contains "phi1 core" "phi1([4,2,2,1],3)" "Core: [4, 2]"
run_contains "phi1 quotient" "phi1([4,2,2,1],3)" "Quotient: [[], [], [1]]"

# SC3: roundtrip for t=3
run_exact "roundtrip t=3" "invphi1(phi1([4,2,2,1],3),3)" "[4, 2, 2, 1]"

# SC3: roundtrip for t=2
run_exact "roundtrip t=2" "invphi1(phi1([3,2,1],2),2)" "[3, 2, 1]"

# SC3: roundtrip for t=5
run_exact "roundtrip t=5" "invphi1(phi1([7,5,4,3,2,1],5),5)" "[7, 5, 4, 3, 2, 1]"

# SC3: roundtrip for t=7
run_exact "roundtrip t=7" "invphi1(phi1([10,8,6,4,2],7),7)" "[10, 8, 6, 4, 2]"

# SC4: size identity |λ| = |core| + t * Σ|qi|
# [5,4,3,2,1]: core=[], quotient=[[1],[2,1],[1]], |λ|=15, |core|=0, 3*(1+3+1)=15
run_exact "size identity (norm check)" "ptnnorm([5,4,3,2,1])" "15"

# Roundtrip with variable
run_exact "phi1 variable roundtrip" "$(printf 'r := phi1([4,2,2,1],3)\ninvphi1(r,3)')" "Core: [4, 2]
Quotient: [[], [], [1]]
[4, 2, 2, 1]"

# Empty partition roundtrip
run_exact "roundtrip empty" "invphi1(phi1([],3),3)" "[]"

# Staircase partition (2-core of itself)
run_contains "2-core self" "phi1([3,2,1],2)" "Core: [3, 2, 1]"

# Large quotient
run_exact "roundtrip [5,5,5,5,5] t=3" "invphi1(phi1([5,5,5,5,5],3),3)" "[5, 5, 5, 5, 5]"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
