#!/bin/bash
# acceptance-bailey.sh — Phase 80: Bailey chains
PASS=0; FAIL=0
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }
check() {
    local desc="$1" input="$2" pattern="$3"
    local out
    out=$(echo "$input" | "$BIN" 2>&1)
    if echo "$out" | grep -qE "$pattern"; then
        echo "PASS: $desc"; ((PASS++))
    else
        echo "FAIL: $desc"; echo "  got: $out"; ((FAIL++))
    fi
}

# Test 1: betafind(unit_alpha, n) recovers beta — unit pair n=0 gives 1
check "betafind(0,20) unit beta n=0" "coeff(betafind(0,20), 0)" "^1$"

# Test 2: betafind n=1 gives 1/(1-q)^2, coeff 5 = 6
check "betafind(1,20) unit beta n=1 coeff 5" "coeff(betafind(1,20), 5)" "^6$"

# Test 3: alphaup/alphadown are inverses
check "alphadown(alphaup(1,20),20) = 1" "coeff(alphadown(alphaup(1,20),20), 0)" "^1$"

# Test 4: Catalog — unit (0), RR (1), GG (2) produce valid series
check "bailey_sum(0,5,25) valid" "bailey_sum(0,5,25)" "^[0-9]"
check "bailey_sum(1,5,25) valid" "bailey_sum(1,5,25)" "^[0-9]"
check "bailey_sum(2,5,25) valid" "bailey_sum(2,5,25)" "^[0-9]"

# Test 5: Bailey chain sum produces non-trivial series (unit sum coeff 3)
check "bailey_sum(0,8,30) coeff 3" "coeff(bailey_sum(0,8,30), 3)" "^[0-9]+$"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
