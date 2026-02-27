#!/usr/bin/env bash
# Acceptance test: checkprod and checkmult (Phase 25)
# Exit 0 if all pass, non-zero otherwise.

set -e
cd "$(dirname "$0")/.." || exit 1
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

# 1. checkprod nice: Rogers-Ramanujan sum (prodmake exponents in {-1,0,1})
OUT=$(run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "checkprod(rr, 20)")
if echo "$OUT" | grep -q "nice product" && echo "$OUT" | grep -q "minExp=0"; then
    echo "PASS: checkprod Rogers-Ramanujan nice"
    PASS=$((PASS+1))
else
    echo "FAIL: checkprod Rogers-Ramanujan nice"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

# 2. checkmult multiplicative: 1+q+q^2+... (geom) has all coefficients 1
OUT=$(run "geom := add(q^n, n, 0, 50)" "checkmult(geom, 50)")
if echo "$OUT" | grep -q "MULTIPLICATIVE"; then
    echo "PASS: checkmult multiplicative"
    PASS=$((PASS+1))
else
    echo "FAIL: checkmult multiplicative"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

# 3. checkmult non-multiplicative: Rogers-Ramanujan G
OUT=$(run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "checkmult(rr, 30)")
if echo "$OUT" | grep -q "NOT MULTIPLICATIVE"; then
    echo "PASS: checkmult Rogers-Ramanujan not multiplicative"
    PASS=$((PASS+1))
else
    echo "FAIL: checkmult Rogers-Ramanujan not multiplicative"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

echo ""
echo "checkprod/checkmult: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
