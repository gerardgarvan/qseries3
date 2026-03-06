#!/bin/bash
# acceptance-worksheet-mod7.sh — Phase 68: Garvan's mod-7 eta dissection worksheet
cd "$(dirname "$0")/.."
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }

PASS=0; FAIL=0
check() {
    local desc="$1" input="$2" pattern="$3"
    local out
    out=$(printf '%s\n' "$input" | "$BIN" 2>&1)
    if echo "$out" | grep -qE "$pattern"; then
        echo "PASS: $desc"
        ((PASS++))
    else
        echo "FAIL: $desc"
        echo "  expected: $pattern"
        echo "  got:      $out"
        ((FAIL++))
    fi
}

# Test 1: A0 Ramanujan sum computes correctly (2q² + 4q⁴ - 10q¹⁰ + ...)
check "A0 Ramanujan sum" \
    "set_trunc(200)
A0 := sum((-1)^(n+1)*n*(3*n-1)*q^(n*(3*n-1)), n, -20, 20)" \
    "2q.*4q.*10q"

# Test 2: findhommodp on 8-element sifted eta list finds X₇ and X₈
check "findhommodp finds X7 X8" \
    "set_trunc(200)
E1 := etaq(1,200):
E2 := etaq(2,200):
A0 := sum((-1)^(n+1)*n*(3*n-1)*q^(n*(3*n-1)), n, -20, 20):
FL1 := sift(E2^6, 7, 5, 200):
FL2 := sift(E1*E2^5, 7, 5, 200):
FL3 := sift(E1^2*E2^4, 7, 5, 200):
FL4 := sift(E1^3*E2^3, 7, 5, 200):
FL5 := sift(E1^4*E2^2, 7, 5, 200):
FL6 := sift(E1^5*E2, 7, 5, 200):
FL7 := sift(E1^6, 7, 5, 200):
FL8 := sift(A0*E1^5, 7, 5, 200):
findhommodp([FL1,FL2,FL3,FL4,FL5,FL6,FL7,FL8], 7, 1, 0)" \
    "X.*[78]"

# Test 3: nterms of sifted A0 by 7 for k=0 = 9
check "nterms sift(A0,7,0) = 9" \
    "set_trunc(1000)
A0 := sum((-1)^(n+1)*n*(3*n-1)*q^(n*(3*n-1)), n, -20, 20):
nterms(sift(A0, 7, 0, 1000))" \
    "^9$"

# Test 4: nterms(modp(sift(A0,7,0,1000),7)) = 0 (all coeffs divisible by 7)
check "nterms modp(sift(A0,7,0),7) = 0" \
    "set_trunc(1000)
A0 := sum((-1)^(n+1)*n*(3*n-1)*q^(n*(3*n-1)), n, -20, 20):
nterms(modp(sift(A0, 7, 0, 1000), 7))" \
    "^0$"

# Test 5: E2 sifting — nterms(modp(sift(E2,7,0,1000),7)) = 10
check "nterms modp(sift(E2,7,0),7) = 10" \
    "set_trunc(1000)
E2 := etaq(2,1000):
nterms(modp(sift(E2, 7, 0, 1000), 7))" \
    "^10$"

# Test 6: E2 sifting — nterms(modp(sift(E2,7,2,1000),7)) = 11
check "nterms modp(sift(E2,7,2),7) = 11" \
    "set_trunc(1000)
E2 := etaq(2,1000):
nterms(modp(sift(E2, 7, 2, 1000), 7))" \
    "^11$"

# Test 7: E2 sifting — nterms(modp(sift(E2,7,4,1000),7)) = 5
check "nterms modp(sift(E2,7,4),7) = 5" \
    "set_trunc(1000)
E2 := etaq(2,1000):
nterms(modp(sift(E2, 7, 4, 1000), 7))" \
    "^5$"

# Test 8: jacprodmake on sift(E2,7,0) produces Jacobi product
check "jacprodmake sift(E2,7,0) gives Jacobi product" \
    "set_trunc(1000)
E2 := etaq(2,1000):
jacprodmake(sift(E2, 7, 0, 1000), 100)" \
    "q.*14.*q.*14"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
