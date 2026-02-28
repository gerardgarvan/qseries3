#!/usr/bin/env bash
# Acceptance test: Phase 31 up/down arrow history navigation (TTY only)
# Verifies: up arrow recalls previous commands; down arrow restores current line.
# Requires: script (Unix) or similar TTY simulation. Skips gracefully if unavailable.

set -e
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }

if ! command -v script >/dev/null 2>&1; then
    echo "skipping (script not available)"
    exit 0
fi

run_script() {
    if command -v timeout >/dev/null 2>&1; then
        timeout 10 "$@" || true
    else
        "$@" || true
    fi
}

UP=$'\033[A'
DOWN=$'\033[B'

PASS=0
FAIL=0

# Test 1: Enter "1+1", press Enter, then Up + Enter → should see "2" twice
out=$(printf '1+1\n%s\n\n' "$UP" | run_script script -q -c "$BIN" /dev/null 2>/dev/null)
count=$(echo "$out" | grep -c "^2 " || true)
if [ "$count" -ge 2 ]; then
    echo "PASS: up arrow recalls previous command"
    PASS=$((PASS+1))
else
    echo "FAIL: up arrow recall (expected '2' at least twice)"
    echo "  output snippet: $(echo "$out" | head -10)"
    FAIL=$((FAIL+1))
fi

# Test 2: Enter "2+3", Enter, "4+5", Enter, Up, Up, Enter → should see "5" (from 2+3)
out2=$(printf '2+3\n4+5\n%s%s\n\n' "$UP" "$UP" | run_script script -q -c "$BIN" /dev/null 2>/dev/null)
if echo "$out2" | grep -q "^5 "; then
    echo "PASS: repeated up arrows walk back through history"
    PASS=$((PASS+1))
else
    echo "FAIL: repeated up arrows (expected '5' from 2+3)"
    echo "  output snippet: $(echo "$out2" | head -10)"
    FAIL=$((FAIL+1))
fi

# Test 3: Enter "2+3", Enter, "4+5", Enter, Up, Up, Down, Enter → should see "9" (from 4+5)
out3=$(printf '2+3\n4+5\n%s%s%s\n\n' "$UP" "$UP" "$DOWN" | run_script script -q -c "$BIN" /dev/null 2>/dev/null)
if echo "$out3" | grep -q "^9 "; then
    echo "PASS: down arrow walks forward in history"
    PASS=$((PASS+1))
else
    echo "FAIL: down arrow (expected '9' from 4+5)"
    echo "  output snippet: $(echo "$out3" | head -10)"
    FAIL=$((FAIL+1))
fi

echo ""
echo "History: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
