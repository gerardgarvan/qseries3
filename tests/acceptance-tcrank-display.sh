#!/usr/bin/env bash
# Phase 74: t-core Crank and Display acceptance tests
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

run_range() {
    local label="$1" input="$2" lo="$3" hi="$4"
    local got
    got=$(echo "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if [ "$got" -ge "$lo" ] 2>/dev/null && [ "$got" -le "$hi" ] 2>/dev/null; then
        echo "PASS: $label (=$got)"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected: $lo <= val <= $hi"
        echo "  got: $got"
        ((FAIL++))
    fi
}

# SC1: tcrank returns value in {0..p-1}
run_range "tcrank([4,2,2,1],5) in 0..4" "tcrank([4,2,2,1],5)" 0 4
run_range "tcrank([3,1],5) in 0..4" "tcrank([3,1],5)" 0 4
run_range "tcrank([6,6,6,5,4,4,2,1,1],5) in 0..4" "tcrank([6,6,6,5,4,4,2,1,1],5)" 0 4

# SC1: tcrank with t=7
run_range "tcrank([4,2,2,1],7) in 0..6" "tcrank([4,2,2,1],7)" 0 6

# SC2: tcrank empty partition
run_exact "tcrank([],5) = 0" "tcrank([],5)" "0"

# SC2: tcrank error for even t
run_contains "tcrank even t error" "tcrank([3,1],4)" "odd prime"

# SC3: tresdiag 3-residue diagram
run_contains "tresdiag([4,2,2,1],3) first row" "tresdiag([4,2,2,1],3)" "0 1 2 0"
run_contains "tresdiag([4,2,2,1],3) last row" "tresdiag([4,2,2,1],3)" "0"

# SC4: makebiw matches Maple
run_contains "makebiw header" "makebiw([6,6,6,5,4,4,2,1,1],5,3)" "-3-2-1 0 1 2 3"
run_contains "makebiw W0 line" "makebiw([6,6,6,5,4,4,2,1,1],5,3)" "W0  E E E E N E N"
run_contains "makebiw W4 line" "makebiw([6,6,6,5,4,4,2,1,1],5,3)" "W4  E E N E E N N"

# SC5: tresdiag with t=5
run_contains "tresdiag([5,3,2,1],5) first row" "tresdiag([5,3,2,1],5)" "0 1 2 3 4"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
