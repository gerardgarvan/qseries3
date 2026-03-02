#!/usr/bin/env bash
BIN="./qseries.exe"
PASS=0; FAIL=0

check() {
    local desc="$1" input="$2" expect="$3"
    local got
    got=$(echo "$input" | $BIN 2>&1 | tail -1)
    if echo "$got" | grep -qF "$expect"; then
        echo "PASS: $desc"
        ((PASS++))
    else
        echo "FAIL: $desc"
        echo "  expected fragment: $expect"
        echo "  got: $got"
        ((FAIL++))
    fi
}

check "Test 1: (1-q)^(1/2) first coeffs" \
    "(1-q)^(1/2)" \
    "1 - 1/2q - 1/8q² - 1/16q³ - 5/128q⁴"

check "Test 2: roundtrip ((1-q)^(1/2))^2 = 1-q" \
    "$(printf 'f := (1-q)^(1/2)\nf^2')" \
    "1 - q + O(q"

check "Test 3: (1-q)^(-1/2) coeffs" \
    "(1-q)^(-1/2)" \
    "1 + 1/2q + 3/8q² + 5/16q³"

check "Test 4: (4-4*q)^(1/2) scaling" \
    "(4-4*q)^(1/2)" \
    "2 - q - 1/4q²"

check "Test 5: (q-q^2)^(1/2) q_shift" \
    "(q-q^2)^(1/2)" \
    "q^(1/2) - 1/2q^(3/2)"

check "Test 6: (1+q+q^2)^(1/2) multi-term h[2]=3/8" \
    "(1+q+q^2)^(1/2)" \
    "1 + 1/2q + 3/8q²"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
