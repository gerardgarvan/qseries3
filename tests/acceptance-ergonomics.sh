#!/usr/bin/env bash
# Phase 111 ergonomics: bracket continuation, trailing backslash at EOF, TTY guards.

set -e
cd "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }

PASS=0
FAIL=0

echo ""
echo "--- Phase 111: Ergonomics (ERGO-01, ERGO-02, ERGO-04) ---"

# 1. ERGO-02: Trailing backslash at EOF must not hang (timeout 3s)
# Produce "x := 1" + backslash, no newline; pipe closes at EOF.
if timeout 3 bash -c "printf 'x := 1\\\\\\\\' | $BIN 2>&1" >/dev/null 2>&1; then
    echo "PASS: trailing backslash at EOF exits without hanging"
    PASS=$((PASS+1))
else
    echo "FAIL: trailing backslash at EOF should exit (no hang)"
    FAIL=$((FAIL+1))
fi

# 2. ERGO-01: Bracket continuation in script mode (sum(q^n,\nn,0,5))
out=$(printf 'sum(q^n,\nn,0,5)\n' | "$BIN" 2>&1)
if echo "$out" | grep -qE "1 \+ q|q²"; then
    echo "PASS: bracket continuation in script mode works"
    PASS=$((PASS+1))
else
    echo "FAIL: bracket continuation should combine lines and compute"
    echo "  Got: $out"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Phase 111: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
