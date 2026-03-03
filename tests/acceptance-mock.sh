#!/usr/bin/env bash
# Phase 77: Mock Theta Functions acceptance tests
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }
QSERIES="$BIN --no-banner"
PASS=0; FAIL=0

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

echo "=== mockdesorder ==="
run_contains "mockdesorder(3) count" "mockdesorder(3)" "7 mock theta functions"
run_contains "mockdesorder(3) list" "mockdesorder(3)" "f3, phi3, psi3, chi3, omega3, nu3, rho3"
run_contains "mockdesorder(5) count" "mockdesorder(5)" "12 mock theta functions"
run_contains "mockdesorder(5) list" "mockdesorder(5)" "f05, f15, phi05, phi15"
run_contains "mockdesorder(2)" "mockdesorder(2)" "A2, B2, mu2"
run_contains "mockdesorder(10)" "mockdesorder(10)" "phi10, psi10, X10, chi10"

echo "=== mockqs f3 (Ramanujan 3rd order) ==="
run_contains "mockqs(f3,3,20) coeff 1" "mockqs(f3, 3, 20)" "1 + q - 2q"
run_contains "mockqs(f3,3,20) coeff 5" "mockqs(f3, 3, 20)" "- 5q"
run_contains "mockqs(f3,3,20) coeff 10" "mockqs(f3, 3, 20)" "- 10q"

echo "=== mockqs other functions ==="
run_contains "mockqs(phi3,3,15)" "mockqs(phi3, 3, 15)" "1 + q"
run_contains "mockqs(f05,5,15)" "mockqs(f05, 5, 15)" "1 + q - q"
run_contains "mockqs(F05,5,15)" "mockqs(F05, 5, 15)" "1 + q"
run_contains "mockqs(phi10,10,15)" "mockqs(phi10, 10, 15)" "1 + 2q"
run_contains "mockqs(A2,2,15)" "mockqs(A2, 2, 15)" "q + 2q"
run_contains "mockqs(S08,8,15)" "mockqs(S08, 8, 15)" "1 "

echo "=== help ==="
run_contains "help mockqs" "help(mockqs)" "mockqs"
run_contains "help mockdesorder" "help(mockdesorder)" "mockdesorder"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
