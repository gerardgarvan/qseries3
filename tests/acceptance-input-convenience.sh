#!/usr/bin/env bash
# Phase 110 input convenience: undefined-variable typo suggestions, etc.

set -e
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

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

echo ""
echo "--- Phase 110: Input convenience ---"

# 1. Undefined variable with close match (user var): RR vs rr
out=$(run "rr := 1" "RR")
if echo "$out" | grep -q "Did you mean: rr"; then
    echo "PASS: undefined variable RR suggests rr"
    PASS=$((PASS+1))
else
    echo "FAIL: RR should show 'Did you mean: rr'"
    FAIL=$((FAIL+1))
fi

# 2. Unknown built-in typo: etamke vs etamake
out=$(run "etamke(q,1)" 2>&1)
if echo "$out" | grep -q "Did you mean:.*etamake"; then
    echo "PASS: unknown built-in etamke suggests etamake"
    PASS=$((PASS+1))
else
    echo "FAIL: etamke should suggest etamake"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Phase 110: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
