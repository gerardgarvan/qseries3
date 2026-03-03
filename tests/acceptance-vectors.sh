#!/usr/bin/env bash
# Phase 73: Vector Representations acceptance tests
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }
QSERIES="$BIN --no-banner"
PASS=0; FAIL=0

run_exact() {
    local label="$1" input="$2" expect="$3"
    local got
    got=$(echo "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if [ "$got" = "$expect" ]; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected: $expect"
        echo "  got:      $got"
        ((FAIL++))
    fi
}

run_contains() {
    local label="$1" input="$2" expect="$3"
    local got
    got=$(echo "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if echo "$got" | grep -qF "$expect"; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected to contain: $expect"
        echo "  got: $got"
        ((FAIL++))
    fi
}

# SC1: ptn2nvec
run_exact "ptn2nvec([3,1],3)" "ptn2nvec([3,1],3)" "[0, -1, 1]"

# SC2: nvec2ptn roundtrip
run_exact "nvec2ptn roundtrip [3,1]" "nvec2ptn(ptn2nvec([3,1],3))" "[3, 1]"
run_exact "nvec2ptn roundtrip [4,2]" "nvec2ptn(ptn2nvec([4,2],3))" "[4, 2]"
run_exact "nvec2ptn roundtrip [2,1]" "nvec2ptn(ptn2nvec([2,1],2))" "[2, 1]"
run_exact "nvec2ptn roundtrip [3,2,1]" "nvec2ptn(ptn2nvec([3,2,1],2))" "[3, 2, 1]"
run_exact "nvec2ptn empty" "nvec2ptn([0,0,0])" "[]"

# SC3: ptn2rvec
run_exact "ptn2rvec([4,2,2,1],3)" "ptn2rvec([4,2,2,1],3)" "[4, 2, 3]"

# SC4: nvec2alphavec t=5
run_exact "nvec2alphavec t=5 simple" "nvec2alphavec(ptn2nvec([3,1],5))" "[0, 0, 1, 0, 0]"

# SC4: nvec2alphavec t=7
run_contains "nvec2alphavec t=7" "nvec2alphavec([1,0,0,0,-1,0,0])" "5/7"

# SC4: nvec2alphavec rejects other t
run_contains "nvec2alphavec t=3 error" "nvec2alphavec([1,0,0])" "only implemented for t=5 and t=7"

# SC5: Frobenius coordinate reconstruction (verified by roundtrips above, plus direct)
run_exact "nvec2ptn [1,-1,0]" "nvec2ptn([1,-1,0])" "[1, 1]"
run_exact "nvec2ptn [0,0,-1,0,1]" "nvec2ptn([0,0,-1,0,1])" "[5, 1, 1]"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
