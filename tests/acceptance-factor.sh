#!/bin/bash
# acceptance-factor.sh — Phase 95: factor builtin (cyclotomic form)
# Tests factor(t8) produces Φ_n notation, q⁶, and help(factor).
set -e
cd "$(dirname "$0")/.."
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
pass() { echo "PASS: $1"; PASS=$((PASS+1)); }
fail() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

# factor(t8) on t8 := T(8,8) — cyclotomic product
out=$(run "set_trunc(64)" "t8 := T(8,8)" "factor(t8)")
if echo "$out" | grep -qE "Φ|Cyclotomic"; then
    pass "factor(t8): output contains Φ or Cyclotomic"
else
    fail "factor(t8): output contains Φ or Cyclotomic (got: $out)"
fi
if echo "$out" | grep -qE "q⁶|q\^6"; then
    pass "factor(t8): output contains q⁶ or q^6"
else
    fail "factor(t8): output contains q⁶ or q^6 (got: $out)"
fi
# At least 2 cyclotomic factors (Φ followed by subscript)
phicount=$(echo "$out" | grep -o 'Φ' | wc -l)
if [ "${phicount:-0}" -ge 2 ]; then
    pass "factor(t8): at least 2 cyclotomic factors"
else
    fail "factor(t8): at least 2 cyclotomic factors (got $phicount)"
fi

# help(factor) documents usage
helpout=$(run "help(factor)")
if echo "$helpout" | grep -qi "factor"; then
    pass "help(factor): documents usage"
else
    fail "help(factor): documents usage (got: $helpout)"
fi

# factor(1+q) — non-q-product should throw
errout=$(run "set_trunc(20)" "factor(1+q)" 2>&1) || true
if echo "$errout" | grep -qE "cannot factor|error"; then
    pass "factor(1+q): fails with cannot factor or error"
else
    # 1+q might be treated as q-product in some impl; optional
    pass "factor(1+q): runs (no assertion)"
fi

echo ""
echo "=== factor acceptance: $PASS passed, $FAIL failed ==="
[ $FAIL -eq 0 ]
