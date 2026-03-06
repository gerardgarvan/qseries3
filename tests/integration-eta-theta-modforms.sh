#!/bin/bash
# integration-eta-theta-modforms.sh — Phase 87: Cross-package ETA + theta IDs + modforms
# Verifies end-to-end flows chaining eta_cusp, modforms, theta_ids.
export PATH="/usr/bin:$PATH"
PASS=0; FAIL=0
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }

check() {
    local desc="$1" input="$2" pattern="$3"
    local out
    out=$(echo "$input" | "$BIN" 2>&1)
    if echo "$out" | grep -qE "$pattern"; then
        echo "PASS: $desc"; ((PASS++))
    else
        echo "FAIL: $desc"; echo "  got: $out"; ((FAIL++))
    fi
}

run() { printf '%s\n' "$@" | "$BIN" 2>&1; }

# --- Test 1: ETA + modforms chain ---
# DELTA12 and q*etaq^24 produce identical Delta (modforms + qfuncs)
check "DELTA12(50) - q*etaq^24 = 0" "DELTA12(50) - q*etaq(1,50)^24" "^0"

# provemodfuncGAMMA0id proves eta-quotient identity on Gamma_0(2)
check "provemodfuncGAMMA0id eta identity" "provemodfuncGAMMA0id([[1, 2,24, 1,-24], [-1, 2,24, 1,-24]], 2)" "proven=1"

# --- Test 2: Theta IDs chain (rr -> jacprodmake -> jac2eprod -> provemodfuncid) ---
# jac2eprod produces GETA or EETA form
if run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "j := jacprodmake(rr, 50)" "jac2eprod(j)" | grep -qE "GETA|EETA"; then
    echo "PASS: rr->jacprodmake->jac2eprod yields GETA/EETA"; ((PASS++))
else
    echo "FAIL: rr->jacprodmake->jac2eprod yields GETA/EETA"; ((FAIL++))
fi

# provemodfuncid trivial identity on Gamma_1(25)
check "provemodfuncid Rogers-Ramanujan form" "provemodfuncid([[1, [5,25,-1], [10,25,1]], [-1, [5,25,-1], [10,25,1]]], 25)" "proven=1"

# --- Test 3: ETA roundtrip (optional) ---
# etamake(theta3(100), 100) yields valid eta product (η or [[...]])
check "etamake(theta3(100),100) yields eta product" "etamake(theta3(100), 100)" "(η|\[\[[0-9])"

# --- Test 4: Single proof chain (eta + theta + modular form basis) ---
# Chains makeALTbasisM + provemodfuncGAMMA0id + etamake in one flow
chain_out=$(run "makeALTbasisM(12,50)" "provemodfuncGAMMA0id([[1, 2,24, 1,-24], [-1, 2,24, 1,-24]], 2)" "etamake(theta3(50), 50)")
if echo "$chain_out" | grep -qE "basis elements|makeALTbasisM|makebasisM" && \
   echo "$chain_out" | grep -q "proven=1" && \
   echo "$chain_out" | grep -qE "η|GETA|EETA|\[\[[0-9]"; then
    echo "PASS: single proof chain (makeALTbasisM + provemodfuncGAMMA0id + etamake)"; ((PASS++))
else
    echo "FAIL: single proof chain (makeALTbasisM + provemodfuncGAMMA0id + etamake)"; ((FAIL++))
fi

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
