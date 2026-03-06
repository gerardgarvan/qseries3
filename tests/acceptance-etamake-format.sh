#!/usr/bin/env bash
# Phase 60: etamake scalar factor and format verification
# Must-haves: theta2→"2 η(4τ)² / η(2τ)", theta3→"η(2τ)⁵ / (η(τ)² η(4τ)²)", theta4→"η(τ)² / η(2τ)"

set -e
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found"; exit 1; }

run() { printf '%s\n' "$@" | "$BIN" 2>&1; }

PASS=0; FAIL=0

# theta2/q^(1/4) → "2 η(4τ)² / η(2τ)" (scalar 2, denominator uses abs exponents)
out=$(run "set_trunc(100)" "t2 := theta2(100) / q^(1/4)" "etamake(t2, 100)")
if echo "$out" | grep -q '2 ' && echo "$out" | grep -q $'\xCE\xB7(4\xCF\x84)' && echo "$out" | grep -q $'\xCE\xB7(2\xCF\x84)'; then
    echo "PASS: etamake(theta2/q^(1/4)) scalar and format"
    PASS=$((PASS+1))
else
    echo "FAIL: etamake(theta2/q^(1/4)) (expected 2 η(4τ)² / η(2τ))"
    FAIL=$((FAIL+1))
fi

# theta3 → "η(2τ)⁵ / (η(τ)² η(4τ)²)"
out=$(run "etamake(theta3(100), 100)")
if echo "$out" | grep -q $'\xCE\xB7(2\xCF\x84)' && echo "$out" | grep -q $'\xCE\xB7(\xCF\x84)' && echo "$out" | grep -q '/'; then
    echo "PASS: etamake(theta3) format"
    PASS=$((PASS+1))
else
    echo "FAIL: etamake(theta3) format"
    FAIL=$((FAIL+1))
fi

# theta4 → "η(τ)² / η(2τ)"
out=$(run "etamake(theta4(100), 100)")
if echo "$out" | grep -q $'\xCE\xB7(\xCF\x84)' && echo "$out" | grep -q $'\xCE\xB7(2\xCF\x84)'; then
    echo "PASS: etamake(theta4) format"
    PASS=$((PASS+1))
else
    echo "FAIL: etamake(theta4) format"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Phase 60 etamake format: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
