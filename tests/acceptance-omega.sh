#!/usr/bin/env bash
# Phase 93 acceptance tests: omega, RootOf(3), sum(omega^n), assignment, omega^2
# Exit 0 if all pass, non-zero otherwise.

set -e
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }

run() {
    printf '%s\n' "$@" | "$BIN" 2>&1
}

PASS=0
FAIL=0

# Test 1: omega -> "omega"
if run "omega" | grep -q "omega"; then
    echo "PASS: Test 1 omega"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 1 omega"
    FAIL=$((FAIL+1))
fi

# Test 2: RootOf(3) -> "omega"; RootOf([1,1,1]) -> "omega"
if run "RootOf(3)" | grep -q "omega"; then
    echo "PASS: Test 2a RootOf(3)"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 2a RootOf(3)"
    FAIL=$((FAIL+1))
fi
if run "RootOf([1,1,1])" | grep -q "omega"; then
    echo "PASS: Test 2b RootOf([1,1,1])"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 2b RootOf([1,1,1])"
    FAIL=$((FAIL+1))
fi

# Test 3: sum(omega^n, n, 0, 2) -> "0"
if run "sum(omega^n, n, 0, 2)" | grep -q "0"; then
    echo "PASS: Test 3 sum(omega^n, n, 0, 2) = 0"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 3 sum(omega^n, n, 0, 2)"
    FAIL=$((FAIL+1))
fi

# Test 4: omega := RootOf(3); omega -> "omega"
if run "omega := RootOf(3)" "omega" | grep -q "omega"; then
    echo "PASS: Test 4 omega := RootOf(3); omega"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 4 omega assignment"
    FAIL=$((FAIL+1))
fi

# Test 5: omega^2 -> "omega^2"
if run "omega^2" | grep -q "omega^2"; then
    echo "PASS: Test 5 omega^2"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 5 omega^2"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Phase 93 acceptance-omega: $PASS pass, $FAIL fail"
[ "$FAIL" -eq 0 ]
