#!/usr/bin/env bash
# Acceptance test: Phase 41 robustness edge cases (all 9 fixes)
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

# 1. etaq k<=0 guard
if run "etaq(0, 50)" | grep -q "k must be positive"; then
    echo "PASS: etaq k<=0 guard"
    PASS=$((PASS+1))
else
    echo "FAIL: etaq k<=0 guard"
    FAIL=$((FAIL+1))
fi

# 2. sift n<=0 guard
if run "sift(etaq(1,30), 0, 0, 30)" | grep -q "modulus n must be positive"; then
    echo "PASS: sift n<=0 guard"
    PASS=$((PASS+1))
else
    echo "FAIL: sift n<=0 guard"
    FAIL=$((FAIL+1))
fi

# 3. Division by zero
if run "1/0" | grep -q "division by zero"; then
    echo "PASS: division by zero"
    PASS=$((PASS+1))
else
    echo "FAIL: division by zero"
    FAIL=$((FAIL+1))
fi

# 4. Deep nesting (300 levels)
DEEP=$(printf '(%.0s' $(seq 1 300))1$(printf ')%.0s' $(seq 1 300))
if run "$DEEP" | grep -q "too deeply nested"; then
    echo "PASS: deep nesting guard"
    PASS=$((PASS+1))
else
    echo "FAIL: deep nesting guard"
    FAIL=$((FAIL+1))
fi

# 5. Large integer literal
if run "99999999999999999999999" | grep -q "too large"; then
    echo "PASS: large integer literal"
    PASS=$((PASS+1))
else
    echo "FAIL: large integer literal"
    FAIL=$((FAIL+1))
fi

# 6. pow magnitude limit
if run "etaq(1,20)^20000" | grep -q "magnitude too large"; then
    echo "PASS: pow magnitude limit"
    PASS=$((PASS+1))
else
    echo "FAIL: pow magnitude limit"
    FAIL=$((FAIL+1))
fi

# 7. Unknown builtin suggestion (Levenshtein)
if run "etamke(1,50)" | grep -q "Did you mean"; then
    echo "PASS: unknown builtin suggestion"
    PASS=$((PASS+1))
else
    echo "FAIL: unknown builtin suggestion"
    FAIL=$((FAIL+1))
fi

# 8. Integer assignment (mobius(6) = 1)
if run "x := mobius(6)" "series(x, 10)" | grep -q "^1"; then
    echo "PASS: integer assignment"
    PASS=$((PASS+1))
else
    echo "FAIL: integer assignment"
    FAIL=$((FAIL+1))
fi

# 9. Existing acceptance tests still pass
if bash tests/acceptance.sh > /dev/null 2>&1; then
    echo "PASS: existing acceptance tests"
    PASS=$((PASS+1))
else
    echo "FAIL: existing acceptance tests"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Robustness: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
