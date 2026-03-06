#!/usr/bin/env bash
# Phase 108 error diagnostics acceptance: parse/runtime/script error formatting.

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
echo "--- Phase 108: Error diagnostics ---"

# 1. Parse error: source line with caret, Error (in parser), line 1 col 5, expected
out=$(run "x: = 1" 2>&1)
if echo "$out" | grep -q "Error, (in parser)"; then
    echo "PASS: Parse error shows Error, (in parser)"
    PASS=$((PASS+1))
else
    echo "FAIL: Parse error should show Error, (in parser)"
    FAIL=$((FAIL+1))
fi
if echo "$out" | grep -qE "line 1, col [0-9]+"; then
    echo "PASS: Parse error shows line 1, col N"
    PASS=$((PASS+1))
else
    echo "FAIL: Parse error should show line 1, col N"
    FAIL=$((FAIL+1))
fi
if echo "$out" | grep -q "expected"; then
    echo "PASS: Parse error shows expected"
    PASS=$((PASS+1))
else
    echo "FAIL: Parse error should show expected"
    FAIL=$((FAIL+1))
fi
if echo "$out" | grep -qE "^\s+x: = 1" || echo "$out" | grep -q "x: = 1"; then
    echo "PASS: Parse error shows source line"
    PASS=$((PASS+1))
else
    echo "FAIL: Parse error should show source line"
    FAIL=$((FAIL+1))
fi
if echo "$out" | grep -q "\^"; then
    echo "PASS: Parse error shows caret"
    PASS=$((PASS+1))
else
    echo "FAIL: Parse error should show caret"
    FAIL=$((FAIL+1))
fi

# 2. Runtime: etaq() — Error, (in etaq), invalid input, expects/1st
out=$(run "etaq()" 2>&1)
if echo "$out" | grep -q "Error, (in etaq)"; then
    echo "PASS: Runtime error shows Error, (in etaq)"
    PASS=$((PASS+1))
else
    echo "FAIL: Runtime error should show Error, (in etaq)"
    FAIL=$((FAIL+1))
fi
if echo "$out" | grep -qE "invalid input|expects|1st"; then
    echo "PASS: Runtime error shows invalid input or expects"
    PASS=$((PASS+1))
else
    echo "FAIL: Runtime error should show invalid input or expects"
    FAIL=$((FAIL+1))
fi

# 3. Script mode: 2-line script, parse error on line 2 — line 2: prefix
tmp_script=$(mktemp)
printf '1+1\nfoo : 1\n' > "$tmp_script"
if "$BIN" < "$tmp_script" 2>&1 | grep -q "line 2:"; then
    echo "PASS: Script mode error shows line 2: prefix"
    PASS=$((PASS+1))
else
    echo "FAIL: Script mode error should show line 2: prefix"
    FAIL=$((FAIL+1))
fi
rm -f "$tmp_script"

echo ""
echo "Total: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
