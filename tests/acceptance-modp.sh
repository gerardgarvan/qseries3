#!/bin/bash
# acceptance-modp.sh — Phase 67: Modular series arithmetic tests
PASS=0; FAIL=0
check() {
    local desc="$1" input="$2" pattern="$3"
    local out
    out=$(printf '%s\n' "$input" | ./qseries.exe 2>&1)
    if echo "$out" | grep -qE "$pattern"; then
        echo "PASS: $desc"
        ((PASS++))
    else
        echo "FAIL: $desc"
        echo "  input:    $input"
        echo "  expected: $pattern"
        echo "  got:      $out"
        ((FAIL++))
    fi
}

check_no_error() {
    local desc="$1" input="$2"
    local out
    out=$(printf '%s\n' "$input" | ./qseries.exe 2>&1)
    if echo "$out" | grep -qi "error"; then
        echo "FAIL: $desc"
        echo "  got: $out"
        ((FAIL++))
    else
        echo "PASS: $desc"
        ((PASS++))
    fi
}

# Test 1: modp basic — coefficients in {0..6}
check "modp(etaq(1,20), 7) has small coeffs" \
    "modp(etaq(1,20), 7)" \
    "^1 \+ 6q \+ 6q"

# Test 2: modp mod 2 — coefficients are 0 or 1
check "modp(etaq(1,20), 2) binary coeffs" \
    "modp(etaq(1,20), 2)" \
    "^1 \+ q \+ q"

# Test 3: nterms returns correct count
check "nterms(etaq(1,20)) = 7" \
    "nterms(etaq(1,20))" \
    "^7$"

# Test 4: findhommodp runs without error
check_no_error "findhommodp runs" \
    "findhommodp([etaq(1,30), etaq(2,30)], 7, 1, 0)"

# Test 5: findhommodp finds relation for dependent series
check "findhommodp finds dependency" \
    "findhommodp([etaq(1,30), etaq(1,30)], 7, 1, 0)" \
    "X.*X"

# Test 6: modp on negative coefficients
check "modp handles negative coefficients" \
    "modp(etaq(1,20) - 2*etaq(2,20), 3)" \
    "q"

# Test 7: findlincombomodp f=L1+L2
check "findlincombomodp f=L1+L2" \
    "findlincombomodp(etaq(1,30)+etaq(2,30), [etaq(1,30), etaq(2,30)], 7)" \
    "1 L1.*1 L2.*mod 7"

# Test 8: findlincombomodp runs
check_no_error "findlincombomodp runs" \
    "findlincombomodp(etaq(1,20), [etaq(1,20)], 5)"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
