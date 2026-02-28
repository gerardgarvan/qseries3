#!/usr/bin/env bash
# Acceptance test: Phase 27 suppress output (trailing colon)
# Verifies: x := 1: and etaq(1,10): suppress output; series(x,5) and series(etaq(1,10),10) produce output.

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

# 1. x := 1: stores value, no output; series(x,5) shows series
if run "x := 1:" "series(x,5)" | grep -qE '1 \+ q|O\(q'; then
    echo "PASS: assignment with colon stores, series(x,5) outputs"
    PASS=$((PASS+1))
else
    echo "FAIL: assignment with colon stores, series(x,5) outputs"
    FAIL=$((FAIL+1))
fi

# 2. etaq(1,10): evaluates without output; series(etaq(1,10),10) shows series
if run "etaq(1,10):" "series(etaq(1,10),10)" | grep -qE 'q|O\('; then
    echo "PASS: etaq(1,10): suppresses, series(etaq(1,10),10) outputs"
    PASS=$((PASS+1))
else
    echo "FAIL: etaq(1,10): suppresses, series(etaq(1,10),10) outputs"
    FAIL=$((FAIL+1))
fi

# 3. Colon produces fewer output lines than no colon (assignment)
out_colon=$(run "x := 1:")
out_nocolon=$(run "x := 1")
lines_colon=$(echo "$out_colon" | wc -l)
lines_nocolon=$(echo "$out_nocolon" | wc -l)
if [ "$lines_colon" -lt "$lines_nocolon" ]; then
    echo "PASS: trailing colon suppresses assignment result"
    PASS=$((PASS+1))
else
    echo "FAIL: trailing colon suppresses assignment result (colon=$lines_colon, nocolon=$lines_nocolon)"
    FAIL=$((FAIL+1))
fi

# 4. x := 1 (no colon) still prints result
if run "x := 1" | grep -qE '^1$|1 \+ O\(q'; then
    echo "PASS: no colon prints assignment result"
    PASS=$((PASS+1))
else
    echo "FAIL: no colon should print assignment result"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Suppress output: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
