#!/usr/bin/env bash
# Acceptance test: Phase 24 mprodmake
# ep = etaq(2,T)^2/etaq(4,T)/etaq(1,T); mprodmake(ep,17) -> (1+q)(1+q³)...(1+q¹⁷)
# mprodmake(1,10) -> "1"
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

# 1. mprodmake(ep,18) for ep = etaq(2,100)^2/etaq(4,100)/etaq(1,100)
# T=18 so prodmake computes a[17]; expect (1+q)(1+q³)...(1+q¹⁷) - odd exponents 1,3,5,...,17
out=$(run "ep := etaq(2,100)^2/etaq(4,100)/etaq(1,100)" "mprodmake(ep,18)")
if echo "$out" | grep -q "(1+q)(1+q³)" && echo "$out" | grep -q "(1+q¹⁷)"; then
    echo "PASS: mprodmake(ep,18) produces (1+q)(1+q³)...(1+q¹⁷)"
    PASS=$((PASS+1))
else
    echo "FAIL: mprodmake(ep,18) - expected product with (1+q)(1+q³) and (1+q¹⁷)"
    echo "  got: $out"
    FAIL=$((FAIL+1))
fi

# 2. mprodmake(1,10) -> "1" (constant 1)
out=$(run "mprodmake(1, 10)")
if echo "$out" | grep -xq "1"; then
    echo "PASS: mprodmake(1,10) displays 1"
    PASS=$((PASS+1))
else
    echo "FAIL: mprodmake(1,10) - expected 1"
    echo "  got: $out"
    FAIL=$((FAIL+1))
fi

# 3. help(mprodmake)
if run "help(mprodmake)" | grep -q "mprodmake(f,T)" && run "help(mprodmake)" | grep -q "1+q"; then
    echo "PASS: help(mprodmake)"
    PASS=$((PASS+1))
else
    echo "FAIL: help(mprodmake)"
    FAIL=$((FAIL+1))
fi

echo ""
echo "mprodmake: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
