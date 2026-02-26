#!/usr/bin/env bash
# Acceptance test: checkprod and checkmult (Phase 25)
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

# 1. checkprod nice: Rogers-Ramanujan sum
OUT=$(run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "checkprod(rr, 20)")
if echo "$OUT" | grep -q "nice product" && echo "$OUT" | grep -q "minExp=0"; then
    echo "PASS: checkprod Rogers-Ramanujan nice"
    PASS=$((PASS+1))
else
    echo "FAIL: checkprod Rogers-Ramanujan nice"
    FAIL=$((FAIL+1))
fi

# 2. checkmult multiplicative: etaq(1,50)
OUT=$(run "checkmult(etaq(1,50), 50)")
if echo "$OUT" | grep -q "MULTIPLICATIVE"; then
    echo "PASS: checkmult etaq multiplicative"
    PASS=$((PASS+1))
else
    echo "FAIL: checkmult etaq multiplicative"
    FAIL=$((FAIL+1))
fi

# 3. checkmult non-multiplicative: Rogers-Ramanujan
OUT=$(run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "checkmult(rr, 30)")
if echo "$OUT" | grep -q "NOT MULTIPLICATIVE"; then
    echo "PASS: checkmult Rogers-Ramanujan not multiplicative"
    PASS=$((PASS+1))
else
    echo "FAIL: checkmult Rogers-Ramanujan not multiplicative"
    FAIL=$((FAIL+1))
fi

echo ""
echo "checkprod/checkmult: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
