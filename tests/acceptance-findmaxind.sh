#!/usr/bin/env bash
# Acceptance test: findmaxind (Phase 26)
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

# 1. Independent list: etaq(1), etaq(2), etaq(3) are typically independent
OUT=$(run "findmaxind([etaq(1,50), etaq(2,50), etaq(3,50)], 0)")
if echo "$OUT" | grep -q "indices" && echo "$OUT" | grep -qE "\[1, 2, 3\]|\[1, 3, 2\]|\[2, 1, 3\]|\[2, 3, 1\]|\[3, 1, 2\]|\[3, 2, 1\]"; then
    echo "PASS: findmaxind independent list (3 eta products)"
    PASS=$((PASS+1))
else
    echo "FAIL: findmaxind independent list"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

# 2. Dependent list: duplicate etaq(1,50) -> 1 independent
OUT=$(run "findmaxind([etaq(1,50), etaq(1,50)], 0)")
if echo "$OUT" | grep -q "indices" && echo "$OUT" | grep -qE "\[1\]|\[2\]"; then
    echo "PASS: findmaxind dependent list (duplicate -> 1 index)"
    PASS=$((PASS+1))
else
    echo "FAIL: findmaxind dependent list"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

# 3. Rogers-Ramanujan G1, G2: two independent series
OUT=$(run "g1 := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "g2 := sum(q^(n*(n+1))/aqprod(q,q,n,50), n, 0, 8)" "findmaxind([g1, g2], 0)")
if echo "$OUT" | grep -q "indices" && echo "$OUT" | grep -qE "\[1, 2\]|\[2, 1\]"; then
    echo "PASS: findmaxind Rogers-Ramanujan G1,G2 (2 independent)"
    PASS=$((PASS+1))
else
    echo "FAIL: findmaxind Rogers-Ramanujan G1,G2"
    echo "$OUT" | tail -8
    FAIL=$((FAIL+1))
fi

echo ""
echo "findmaxind: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
