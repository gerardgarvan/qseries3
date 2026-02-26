#!/usr/bin/env bash
# Acceptance test: optional-arg variants (Phase 29)
# etaq(k), checkprod(f), checkmult(f), findmaxind(L) 1-arg forms
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

# 1. etaq(k) 1-arg uses env.T
OUT=$(run "etaq(1)")
if echo "$OUT" | grep -q "1 - q - q²" && echo "$OUT" | grep -q "O(q⁵⁰)"; then
    echo "PASS: etaq(1) uses env.T"
    PASS=$((PASS+1))
else
    echo "FAIL: etaq(1) 1-arg"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

# 2. checkprod(f) 1-arg uses env.T
OUT=$(run "set_trunc(30)" "checkprod(etaq(1,30))")
if echo "$OUT" | grep -q "nice product"; then
    echo "PASS: checkprod(etaq(1,30)) 1-arg"
    PASS=$((PASS+1))
else
    echo "FAIL: checkprod 1-arg"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

# 3. checkmult(f) 1-arg uses env.T
OUT=$(run "set_trunc(30)" "checkmult(etaq(1,30))")
if echo "$OUT" | grep -q "NOT MULTIPLICATIVE" || echo "$OUT" | grep -q "MULTIPLICATIVE"; then
    echo "PASS: checkmult(etaq(1,30)) 1-arg"
    PASS=$((PASS+1))
else
    echo "FAIL: checkmult 1-arg"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

# 4. findmaxind(L) 1-arg uses topshift=0
OUT=$(run "findmaxind([q,q^2,q^3])")
if echo "$OUT" | grep -q "indices:"; then
    echo "PASS: findmaxind([q,q^2,q^3]) 1-arg"
    PASS=$((PASS+1))
else
    echo "FAIL: findmaxind 1-arg"
    echo "$OUT" | tail -5
    FAIL=$((FAIL+1))
fi

# 5. help shows 1-arg variants
OUT=$(run "help(etaq)" "help(checkprod)" "help(findmaxind)")
if echo "$OUT" | grep -q "etaq(k)" && echo "$OUT" | grep -q "checkprod(f)" && echo "$OUT" | grep -q "findmaxind(L)"; then
    echo "PASS: help reflects optional args"
    PASS=$((PASS+1))
else
    echo "FAIL: help table"
    echo "$OUT" | tail -10
    FAIL=$((FAIL+1))
fi

echo ""
echo "optional-args: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
