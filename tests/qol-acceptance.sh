#!/usr/bin/env bash
# QoL acceptance tests: phases 19–22 (multi-line, error messages, demo packaging).
# Runs from project root. Exit 0 if all pass, non-zero otherwise.

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

# --- Phase 19: Multi-line backslash continuation ---
echo ""
echo "--- Phase 19: Multi-line input ---"
# 1 + \ + 2 should evaluate to 3 (displayed as "3 + O(q^...)")
if printf '1 + \\\n2\n' | "$BIN" 2>&1 | grep -qE "3 \+ O|^3$"; then
    echo "PASS: Multi-line continuation (1 + \\ + 2 = 3)"
    PASS=$((PASS+1))
else
    echo "FAIL: Multi-line continuation"
    FAIL=$((FAIL+1))
fi

# Multi-line assignment
if printf 'x := 1 + \\\n1\n' | "$BIN" 2>&1 | grep -qE "2 \+ O|^2$"; then
    echo "PASS: Multi-line assignment (x := 1 + \\ + 1)"
    PASS=$((PASS+1))
else
    echo "FAIL: Multi-line assignment"
    FAIL=$((FAIL+1))
fi

# Phase 20: Tab in input (pipe mode uses getline; tab as whitespace should parse)
if printf '1\t+\t1\n' | "$BIN" 2>&1 | grep -qE "2 \+ O|^2$"; then
    echo "PASS: Input with tab characters parses (1+1=2)"
    PASS=$((PASS+1))
else
    echo "FAIL: Tab in input should parse"
    FAIL=$((FAIL+1))
fi

# --- Phase 21: Error message format ---
echo ""
echo "--- Phase 21: Error messages ---"

# Parse error shows line and column
if run "foo : 1" 2>&1 | grep -qE "parser:.*line.*col|line 1, col"; then
    echo "PASS: Parse error shows line/column"
    PASS=$((PASS+1))
else
    echo "FAIL: Parse error should show line/column"
    FAIL=$((FAIL+1))
fi

# Parse error shows expected token
if run "foo : 1" 2>&1 | grep -qi "expected"; then
    echo "PASS: Parse error shows expected token"
    PASS=$((PASS+1))
else
    echo "FAIL: Parse error should show expected token"
    FAIL=$((FAIL+1))
fi

# Runtime error shows function name prefix
if run "etaq(1)" 2>&1 | grep -qE "^error:.*etaq:|etaq:.*expected"; then
    echo "PASS: Runtime error shows function name (etaq:)"
    PASS=$((PASS+1))
else
    echo "FAIL: Runtime error should show etaq: prefix"
    FAIL=$((FAIL+1))
fi

# Script mode: error includes line number for script
tmp_script=$(mktemp)
printf '1+1\nfoo : 1\n' > "$tmp_script"
if "$BIN" < "$tmp_script" 2>&1 | grep -qE "line 2:|error:.*line 2"; then
    echo "PASS: Script mode error shows line number"
    PASS=$((PASS+1))
else
    echo "FAIL: Script mode error should show line 2"
    FAIL=$((FAIL+1))
fi
rm -f "$tmp_script"

# --- Phase 22: Demo packaging ---
echo ""
echo "--- Phase 22: Demo packaging ---"

if ! make package-demo >/dev/null 2>&1; then
    echo "FAIL: make package-demo failed"
    FAIL=$((FAIL+1))
else
    echo "PASS: make package-demo succeeds"
    PASS=$((PASS+1))
fi

# qseries-demo/ contains required files
if [ -f qseries-demo/qseries.exe ] || [ -f qseries-demo/qseries ]; then
    echo "PASS: qseries-demo/ contains binary"
    PASS=$((PASS+1))
else
    echo "FAIL: qseries-demo/ missing binary"
    FAIL=$((FAIL+1))
fi

if [ -f qseries-demo/garvan-demo.sh ]; then
    echo "PASS: qseries-demo/ contains garvan-demo.sh"
    PASS=$((PASS+1))
else
    echo "FAIL: qseries-demo/ missing garvan-demo.sh"
    FAIL=$((FAIL+1))
fi

if [ -f qseries-demo/README.md ]; then
    echo "PASS: qseries-demo/ contains README.md"
    PASS=$((PASS+1))
else
    echo "FAIL: qseries-demo/ missing README.md"
    FAIL=$((FAIL+1))
fi

# garvan-demo.sh runs from qseries-demo/ (DIR-based resolution)
if (cd qseries-demo && bash garvan-demo.sh) >/dev/null 2>&1; then
    echo "PASS: cd qseries-demo && bash garvan-demo.sh runs"
    PASS=$((PASS+1))
else
    echo "FAIL: garvan-demo.sh should run from qseries-demo/"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Total: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
