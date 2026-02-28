#!/usr/bin/env bash
# Acceptance test: Phase 28 arrow-key line navigation (TTY only)
# Verifies: left/right arrows move cursor; typing and backspace operate at cursor.
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

# Prefer timeout if available (avoid hanging)
run_script() {
    if command -v timeout >/dev/null 2>&1; then
        timeout 5 "$@" || true
    else
        "$@" || true
    fi
}

# Left = ESC [ D, Right = ESC [ C, Backspace = 127 (DEL) or 8
LEFT=$'\033[D'
BS=$'\177'   # 127 (DEL) - common for backspace in terminals

PASS=0
FAIL=0

# Test 1: "ab" + left + "cd" -> "acdb" (cursor after 'b', left once to after 'a', type "cd")
# Feed input into script's stdin so qseries receives it via PTY (TTY)
out=$(printf 'ab%scd\n\n' "$LEFT" | run_script script -q -c "$BIN" /dev/null 2>/dev/null)
if echo "$out" | grep -q "acdb"; then
    echo "PASS: arrow-left + insert at cursor"
    PASS=$((PASS+1))
else
    echo "FAIL: arrow-left + insert (expected acdb in output)"
    echo "  output snippet: $(echo "$out" | head -5)"
    FAIL=$((FAIL+1))
fi

# Test 2: "hello" + left + left + backspace + newline -> "helo"
# Try DEL (127) first; some terminals use BS (8)
t2_ok=0
for BS in $'\177' $'\010'; do
  out2=$(printf 'hello%s%s%s\n\n' "$LEFT" "$LEFT" "$BS" | run_script script -q -c "$BIN" /dev/null 2>/dev/null)
  if echo "$out2" | grep -q "helo"; then
    echo "PASS: arrow-left + backspace deletes before cursor"
    PASS=$((PASS+1))
    t2_ok=1
    break
  fi
done
if [ "$t2_ok" -eq 0 ]; then
    echo "FAIL: arrow-left + backspace (expected helo)"
    echo "  output snippet: $(echo "$out2" | head -5)"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Arrow keys: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
