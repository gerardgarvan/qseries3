#!/usr/bin/env bash
# Phase 76: Partition Statistics acceptance tests
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
    got=$(printf '%s\n' "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d' | tail -1)
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
    got=$(printf '%s\n' "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if echo "$got" | grep -qF -- "$expect"; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected to contain: $expect"
        echo "  got: $got"
        ((FAIL++))
    fi
}

echo "=== Dyson rank ==="
run_exact "drank([5,3,2,1])=1" "drank([5,3,2,1])" "1"
run_exact "drank([3])=2" "drank([3])" "2"
run_exact "drank([1,1,1])=-2" "drank([1,1,1])" "-2"
run_exact "drank([])=0" "drank([])" "0"

echo "=== Andrews-Garvan crank ==="
run_exact "agcrank([5,3,2])=5" "agcrank([5,3,2])" "5"
run_exact "agcrank([4,3,1,1])=0" "agcrank([4,3,1,1])" "0"
run_exact "agcrank([3,2,1,1])=-1" "agcrank([3,2,1,1])" "-1"
run_exact "agcrank([1,1,1])=-3" "agcrank([1,1,1])" "-3"
run_exact "agcrank([])=0" "agcrank([])" "0"

echo "=== Predicates ==="
run_exact "ptnDP([5,3,1])=1" "ptnDP([5,3,1])" "1"
run_exact "ptnDP([3,3,1])=0" "ptnDP([3,3,1])" "0"
run_exact "ptnOP([5,3,1])=1" "ptnOP([5,3,1])" "1"
run_exact "ptnOP([4,3,1])=0" "ptnOP([4,3,1])" "0"
run_exact "ptnRR([5,3,1])=1" "ptnRR([5,3,1])" "1"
run_exact "ptnRR([3,2,1])=0" "ptnRR([3,2,1])" "0"
run_exact "ptnCC([7,4,1])=1" "ptnCC([7,4,1])" "1"
run_exact "ptnCC([5,3,1])=0" "ptnCC([5,3,1])" "0"
run_exact "ptnSCHUR([5,3,1])=1" "ptnSCHUR([5,3,1])" "1"
run_exact "ptnSCHUR([3,2,1])=0" "ptnSCHUR([3,2,1])" "0"
run_exact "ptnOE([5,3,2])=1" "ptnOE([5,3,2])" "1"
run_exact "ptnOE([6,3,1])=0" "ptnOE([6,3,1])" "0"

echo "=== Counting functions ==="
run_exact "PDP(5)=3" "PDP(5)" "3"
run_exact "PDP(0)=1" "PDP(0)" "1"
run_exact "POE(5)=4" "POE(5)" "4"
run_exact "PRR(10)=6" "PRR(10)" "6"
run_exact "PSCHUR(10)=6" "PSCHUR(10)" "6"

echo "=== Overpartitions ==="
run_contains "overptns(3) count" "overptns(3)" "Total: 8 overpartitions"
run_contains "overptns(0) count" "overptns(0)" "Total: 1 overpartitions"
run_exact "overptnrank([3],[2,1])=0" "overptnrank([3],[2,1])" "0"
run_exact "overptncrank([5,3],[2,1])=0" "overptncrank([5,3],[2,1])" "0"

echo "=== Helpers ==="
run_exact "lamPD([5,3,1])=4" "lamPD([5,3,1])" "4"
run_exact "kapPD([6,5,3,1])=0" "kapPD([6,5,3,1])" "0"
run_exact "numLE([6,4,3,1])=1" "numLE([6,4,3,1])" "1"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
