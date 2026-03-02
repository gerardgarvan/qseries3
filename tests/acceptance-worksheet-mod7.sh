#!/bin/bash
# acceptance-worksheet-mod7.sh — Phase 68: Garvan's mod-7 eta dissection worksheet
PASS=0; FAIL=0
check() {
    local desc="$1" input="$2" pattern="$3"
    local out
    out=$(printf '%s\n' "$input" | ./qseries.exe 2>&1)
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

# Test 1: A0 Ramanujan sum computes correctly
check "A0 Ramanujan sum" \
    "$(printf 'set_trunc(200)\nA0 := sum((-1)^(n+1)*n*(3*n-1)*q^(n*(3*n-1)), n, -20, 20)')" \
    "2q.*4q"

# Test 2: findhommodp on 8-element sifted eta list finds X7 and X8
FINDHOM_INPUT="set_trunc(200)
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
findhommodp([FL1,FL2,FL3,FL4,FL5,FL6,FL7,FL8], 7, 1, 0)"
OUT2=$(printf '%s\n' "$FINDHOM_INPUT" | ./qseries.exe 2>&1)
# Should find both X7 and X8 as kernel vectors
if echo "$OUT2" | grep -q "X.*7" && echo "$OUT2" | grep -q "X.*8"; then
    echo "PASS: findhommodp finds X7 and X8"
    ((PASS++))
else
    echo "FAIL: findhommodp finds X7 and X8"
    echo "  got: $OUT2"
    ((FAIL++))
fi

# Test 3: nterms(sift(A0, 7, 0, 1000)) = 9
check "nterms sift(A0,7,0) = 9" \
    "$(printf 'set_trunc(1000)\nA0 := sum((-1)^(n+1)*n*(3*n-1)*q^(n*(3*n-1)), n, -20, 20):\nnterms(sift(A0, 7, 0, 1000))')" \
    "^9$"

# Test 4: nterms(modp(sift(A0,7,0,1000),7)) = 0 (all coeffs div by 7)
check "nterms modp(sift(A0,7,0),7) = 0" \
    "$(printf 'set_trunc(1000)\nA0 := sum((-1)^(n+1)*n*(3*n-1)*q^(n*(3*n-1)), n, -20, 20):\nnterms(modp(sift(A0, 7, 0, 1000), 7))')" \
    "^0$"

# Test 5: E2 sifting — nterms(modp(sift(E2,7,0,1000),7)) = 10
check "nterms modp(sift(E2,7,0),7) = 10" \
    "$(printf 'set_trunc(1000)\nE2 := etaq(2,1000):\nnterms(modp(sift(E2, 7, 0, 1000), 7))')" \
    "^10$"

# Test 6: E2 sifting — nterms(modp(sift(E2,7,2,1000),7)) = 11
check "nterms modp(sift(E2,7,2),7) = 11" \
    "$(printf 'set_trunc(1000)\nE2 := etaq(2,1000):\nnterms(modp(sift(E2, 7, 2, 1000), 7))')" \
    "^11$"

# Test 7: E2 sifting — nterms(modp(sift(E2,7,4,1000),7)) = 5
check "nterms modp(sift(E2,7,4),7) = 5" \
    "$(printf 'set_trunc(1000)\nE2 := etaq(2,1000):\nnterms(modp(sift(E2, 7, 4, 1000), 7))')" \
    "^5$"

# Test 8: jacprodmake on sift(E2,7,0) produces Jacobi product with period 14
check "jacprodmake sift(E2,7,0) gives Jacobi product" \
    "$(printf 'set_trunc(1000)\nE2 := etaq(2,1000):\njacprodmake(sift(E2, 7, 0, 1000), 100)')" \
    "q.*14.*q.*14"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
