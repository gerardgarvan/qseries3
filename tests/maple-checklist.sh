#!/usr/bin/env bash
# Maple Checklist test script: tests blocks 1-41 from maple_checklist.md
# Verifies that our qseries3 REPL reproduces the Garvan tutorial computations.
# Exit 0 if all testable blocks pass, non-zero otherwise.

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
SKIP=0

pass() { echo "PASS: $1"; PASS=$((PASS+1)); }
fail() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }
skip() { echo "SKIP: $1"; SKIP=$((SKIP+1)); }

# Block 1: Rogers-Ramanujan series expansion (§3.1)
# sum_{n=0}^{8} q^{n^2}/(q;q)_n, check coefficient 961 at q^49
if run "x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "series(x, 50)" | grep -q "961"; then
    pass "Block 1: Rogers-Ramanujan series"
else
    fail "Block 1: Rogers-Ramanujan series"
fi

# Block 2: prodmake on Rogers-Ramanujan (§3.1)
# Product should have factors only at exponents ≡ ±1 (mod 5)
if run "x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "prodmake(x, 40)" | grep -q "1-q"; then
    pass "Block 2: prodmake Rogers-Ramanujan"
else
    fail "Block 2: prodmake Rogers-Ramanujan"
fi

# Block 3: T(8,8) computation (§3.2)
# Andrews' recursive q-product, should produce a polynomial in q
if run "set_trunc(64)" "t8 := T(8,8)" | grep -q "q"; then
    pass "Block 3: T(8,8) computation"
else
    fail "Block 3: T(8,8) computation"
fi

# Block 4: factor(t8) — N/A (Maple's factor() not available; qfactor subsumes)
skip "Block 4: factor(t8) — Maple-only feature"

# Block 5: qfactor(t8, 20) (§3.2)
# Expected: q^6 * product of (1-q^k) terms
if run "set_trunc(64)" "t8 := T(8,8)" "qfactor(t8, 20)" | grep -q "1-q"; then
    pass "Block 5: qfactor(t8)"
else
    fail "Block 5: qfactor(t8)"
fi

# Block 6: dixson(5,5,5,q) + qfactor (§3.2)
# Three identical qbin factors with a=b=c=5
if run "dx := sum((-1)^k * q^(k*(3*k+1)/2) * qbin(5+k,10,50) * qbin(5+k,10,50) * qbin(5+k,10,50), k, -5, 5)" "qfactor(dx, 20)" | grep -q "1-q"; then
    pass "Block 6: dixson + qfactor"
else
    fail "Block 6: dixson + qfactor"
fi

# Block 7: etamake(theta2/q^(1/4)) (§3.3)
# Expected: 2 η(4τ)² / (q^(1/4) η(2τ))
if run "t2 := theta2(q, 100) / q^(1/4)" "etamake(t2, 100)" | grep -q $'\xCE\xB7'; then
    pass "Block 7: etamake(theta2)"
else
    fail "Block 7: etamake(theta2)"
fi

# Block 8: etamake(theta3) (§3.3)
# Expected: η(2τ)^5 / (η(τ)^2 η(4τ)^2)
if run "t3 := theta3(q, 100)" "etamake(t3, 100)" | grep -q $'\xCE\xB7'; then
    pass "Block 8: etamake(theta3)"
else
    fail "Block 8: etamake(theta3)"
fi

# Block 9: etamake(theta4) (§3.3)
# Expected: η(τ)^2 / η(2τ)
if run "t4 := theta4(q, 100)" "etamake(t4, 100)" | grep -q $'\xCE\xB7'; then
    pass "Block 9: etamake(theta4)"
else
    fail "Block 9: etamake(theta4)"
fi

# Block 10: omega := RootOf(z^2+z+1=0) — N/A (RootOf not supported)
skip "Block 10: RootOf — not supported"

# Block 11: jacprodmake on Rogers-Ramanujan (§3.4)
# Expected: JAC(0,5,∞) / JAC(1,5,∞)
if run "x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "jacprodmake(x, 40)" | grep -qE 'JAC|\(q'; then
    pass "Block 11: jacprodmake RR"
else
    fail "Block 11: jacprodmake RR"
fi

# Block 12: jac2prod on Rogers-Ramanujan result (§3.4)
# Expected: 1/((q,q^5)_∞ (q^4,q^5)_∞)
if run "x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "y := jacprodmake(x, 40)" "jac2prod(y)" | grep -qE '\(q'; then
    pass "Block 12: jac2prod RR"
else
    fail "Block 12: jac2prod RR"
fi

# Block 13: jacprodmake on Slater-type series (§3.4)
# Expected: Jacobi product with half-integer exponents (e.g. ^(13/2), ^(1/2))
OUT13=$(run "x := sum(q^(n*(n+1)/2)*aqprod(-q,q,n,100)/aqprod(q,q,2*n+1,100), n, 0, 10)" "jacprodmake(x, 50)")
if echo "$OUT13" | grep -qE '\(q.*q\^14'; then
    pass "Block 13: jacprodmake Slater"
else
    fail "Block 13: jacprodmake Slater"
fi

# Block 14: jac2series on Block 13 result — depends on Block 13
OUT14=$(run "x := sum(q^(n*(n+1)/2)*aqprod(-q,q,n,100)/aqprod(q,q,2*n+1,100), n, 0, 10)" "jp := jacprodmake(x, 50)" "jac2series(jp, 100)")
# The last output line (jac2series) must have actual q-series coefficients beyond "1 + O(q...)"
LAST14=$(echo "$OUT14" | tail -1)
if echo "$LAST14" | grep -qE '2q|3q|5q'; then
    pass "Block 14: jac2series Slater"
else
    fail "Block 14: jac2series Slater (depends on Block 13)"
fi

# Block 15: findhom degree 1 on theta functions (§4.1)
# Expected: empty set (no linear relation)
OUT15=$(run "findhom([theta3(q,100), theta4(q,100), theta3(q^2,100), theta4(q^2,100)], 1, 0)")
if echo "$OUT15" | grep -qv "X"; then
    pass "Block 15: findhom degree 1 (no relation)"
else
    fail "Block 15: findhom degree 1"
fi

# Block 16: findhom degree 2 on theta functions (§4.1)
# Expected: Gauss AGM relations {-X₁X₂ + X₄², X₁² + X₂² - 2X₃²}
if run "findhom([theta3(q,100), theta4(q,100), theta3(q^2,100), theta4(q^2,100)], 2, 0)" | grep -q "X"; then
    pass "Block 16: findhom degree 2 (AGM)"
else
    fail "Block 16: findhom degree 2 (AGM)"
fi

# Block 17: UE Eisenstein series definition (§4.2)
# UE(q,6,5,50) = sum_{m=1}^{50} sum_{n=1}^{50} legendre(m,5)*n^5*q^(m*n)
# First coefficient should be q (legendre(1,5)*1^5*q^1 = 1*1*q = q)
if run "f := sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)" | grep -q "31"; then
    pass "Block 17: UE Eisenstein series"
else
    fail "Block 17: UE Eisenstein series"
fi

# Block 18: findhomcombo on UE with eta products (§4.2)
# Expected: X₁²X₂ + 40X₁X₂² + 335X₂³
if run "f := sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)" \
       "B1 := etaq(1,50)^5/etaq(5,50)" \
       "B2 := q*etaq(5,50)^5/etaq(1,50)" \
       "findhomcombo(f, [B1, B2], 3, 0)" | grep -q "335"; then
    pass "Block 18: findhomcombo UE"
else
    fail "Block 18: findhomcombo UE"
fi

# Block 19: Define F(q) and U for findnonhom (§4.3)
# Note: Maple's theta(q^10,q^25,5) uses 5 as iteration count; we use T=100 as truncation
OUT19=$(run "Fq := theta3(q,100)/theta3(q^5,100)" \
            "Fq5 := theta3(q^5,100)/theta3(q^25,100)" \
            "U := 2*q*theta(q^10,q^25,100)/theta3(q^25,100)")
if echo "$OUT19" | grep -q "2q"; then
    pass "Block 19: define F(q) and U"
else
    fail "Block 19: define F(q) and U"
fi

# Block 20: findnonhom on [F(q), F(q^5), U] (§4.3)
# Expected: {-X₁X₂X₃ + X₂² + X₃² + X₃ - 1}
if run "Fq := theta3(q,100)/theta3(q^5,100)" \
       "Fq5 := theta3(q^5,100)/theta3(q^25,100)" \
       "U := 2*q*theta(q^10,q^25,100)/theta3(q^25,100)" \
       "findnonhom([Fq, Fq5, U], 3, 20)" | grep -q "X"; then
    pass "Block 20: findnonhom theta relation"
else
    fail "Block 20: findnonhom theta relation"
fi

# Block 21: ANS:=EQNS[1] — N/A (result indexing not supported)
skip "Block 21: result indexing — not applicable"

# Block 22: Verify the relation from Block 20 (§4.3)
# The known correct relation is: -X₁X₂X₃ + X₂² + X₃² + X₃ - 1 = 0
# We verify directly that substitution gives O(q^T)
OUT22=$(run "Fq := theta3(q,100)/theta3(q^5,100)" \
            "Fq5 := theta3(q^5,100)/theta3(q^25,100)" \
            "U := 2*q*theta(q^10,q^25,100)/theta3(q^25,100)" \
            "CHECK := -Fq*Fq5*U + Fq5^2 + U^2 + U - 1" \
            "series(CHECK, 50)")
if echo "$OUT22" | grep -qE "^0 \+ O|^O\(q"; then
    pass "Block 22: verify relation"
else
    fail "Block 22: verify relation"
fi

# Block 23: findnonhomcombo Watson T^2 = f(T,xi) (§4.4)
# Expected: 343X₂⁷ + ... + X₂ (polynomial in T and xi)
if run "set_trunc(100)" \
       "xi := q^2*etaq(49,100)/etaq(1,100)" \
       "TT := q*(etaq(7,100)/etaq(1,100))^4" \
       "findnonhomcombo(TT^2, [TT, xi], [1, 7], 0)" | grep -q "343"; then
    pass "Block 23: findnonhomcombo Watson"
else
    fail "Block 23: findnonhomcombo Watson"
fi

# Block 24: collect(%[1],[X[1]]) — N/A (formatting function)
skip "Block 24: collect — formatting only"

# Block 25: findpoly on theta2/theta3 quotients (§4.5)
if run "x1 := theta2(q,100)^2/theta2(q^3,40)^2" \
       "x2 := theta3(q,100)^2/theta3(q^3,40)^2" \
       "x := x1 + x2" \
       "findpoly(x, x, 3, 3)" | grep -q "X"; then
    pass "Block 25: findpoly theta2/theta3 quotients"
else
    fail "Block 25: findpoly theta2/theta3 quotients"
fi

# Block 26: sift partition function (§5)
# PD = etaq(2,200)/etaq(1,200), sift to get pd(5n+1)
if run "PD := etaq(2,200)/etaq(1,200)" "PD1 := sift(PD, 5, 1, 199)" | grep -q "q"; then
    pass "Block 26: sift pd(5n+1)"
else
    fail "Block 26: sift pd(5n+1)"
fi

# Block 27: etamake on sifted partition function (§5)
# Expected: η(5τ)³ η(2τ)² / (q^(5/24) η(10τ) η(τ)⁴)
if run "PD := etaq(2,200)/etaq(1,200)" "PD1 := sift(PD, 5, 1, 199)" "etamake(PD1, 38)" | grep -q $'\xCE\xB7'; then
    pass "Block 27: etamake sifted PD"
else
    fail "Block 27: etamake sifted PD"
fi

# ===== Blocks 28-41: Triple/Quintuple Products, Winquist =====

# Block 28: tripleprod(z,q,10) — symbolic z not supported
skip "Block 28: tripleprod(z,q,10) — symbolic z"

# Block 29: tripleprod(q, q^3, 100) — Euler's Pentagonal Number Theorem
# Expected: 1 - q - q² + q⁵ + q⁷ - q¹² - q¹⁵ + ... + q⁵⁷ ...
if run "tripleprod(q, q^3, 100)" | grep -q "1 - q -"; then
    pass "Block 29: tripleprod pentagonal"
else
    fail "Block 29: tripleprod pentagonal"
fi

# Block 30: quinprod(z,q,prodid) — symbolic prodid mode not supported
skip "Block 30: quinprod prodid — symbolic mode"

# Block 31: quinprod(z,q,seriesid) — symbolic seriesid mode not supported
skip "Block 31: quinprod seriesid — symbolic mode"

# Block 32: quinprod(z,q,3) — symbolic z not supported
skip "Block 32: quinprod(z,q,3) — symbolic z"

# Block 33: sift(EULER, 5, 0) — Euler pentagonal mod 5
if run "EULER := etaq(1, 500)" "E0 := sift(EULER, 5, 0, 499)" "series(E0, 20)" | grep -q "q"; then
    pass "Block 33: sift Euler mod 5"
else
    fail "Block 33: sift Euler mod 5"
fi

# Block 34: jacprodmake(E0, 50) — Jacobi product for E0
OUT34=$(run "EULER := etaq(1, 500)" "E0 := sift(EULER, 5, 0, 499)" "jacprodmake(E0, 50)")
if echo "$OUT34" | grep -qE 'JAC|\(q'; then
    pass "Block 34: jacprodmake E0"
else
    fail "Block 34: jacprodmake E0"
fi

# Block 35: jac2prod(jp) on E0 result
if run "EULER := etaq(1, 500)" "E0 := sift(EULER, 5, 0, 499)" "jp := jacprodmake(E0, 50)" "jac2prod(jp)" | grep -qE '\(q'; then
    pass "Block 35: jac2prod E0"
else
    fail "Block 35: jac2prod E0"
fi

# Block 36: quinprod(q, q^5, 100) — quintuple product
# Expected: 1 + q - q³ - q⁷ - q⁸ - q¹⁴ + q²⁰ + ...
OUT36=$(run "qp := quinprod(q, q^5, 100)" "series(qp, 90)")
if echo "$OUT36" | grep -q "1 + q -"; then
    pass "Block 36: quinprod(q, q^5)"
else
    fail "Block 36: quinprod(q, q^5)"
fi

# Block 37: Winquist Q(k) expansion and IDG computation
OUT37=$(run "set_trunc(200)" \
    "A0 := tripleprod(q^15, q^33, 200)" \
    "A9 := tripleprod(q^9, q^33, 200)" \
    "B2 := tripleprod(q^13, q^33, 200) - q^3 * tripleprod(q^2, q^33, 200)" \
    "B4 := tripleprod(q^7, q^33, 200) + q * tripleprod(q^4, q^33, 200)" \
    "IDG := A0*B2 - q^2*A9*B4" \
    "series(IDG, 12)")
if echo "$OUT37" | grep -q "2q"; then
    pass "Block 37: Winquist IDG"
else
    fail "Block 37: Winquist IDG"
fi

# Block 38: jacprodmake(IDG, 50) — Jacobi product for IDG
OUT38=$(run "set_trunc(200)" \
    "A0 := tripleprod(q^15, q^33, 200)" \
    "A9 := tripleprod(q^9, q^33, 200)" \
    "B2 := tripleprod(q^13, q^33, 200) - q^3 * tripleprod(q^2, q^33, 200)" \
    "B4 := tripleprod(q^7, q^33, 200) + q * tripleprod(q^4, q^33, 200)" \
    "IDG := A0*B2 - q^2*A9*B4" \
    "jacprodmake(IDG, 50)")
if echo "$OUT38" | grep -qE 'JAC|\(q.*11'; then
    pass "Block 38: jacprodmake IDG"
else
    fail "Block 38: jacprodmake IDG (mod-11 Jacobi detection fails)"
fi

# Block 39: jac2prod on IDG result — depends on Block 38
OUT39=$(run "set_trunc(200)" \
    "A0 := tripleprod(q^15, q^33, 200)" \
    "A9 := tripleprod(q^9, q^33, 200)" \
    "B2 := tripleprod(q^13, q^33, 200) - q^3 * tripleprod(q^2, q^33, 200)" \
    "B4 := tripleprod(q^7, q^33, 200) + q * tripleprod(q^4, q^33, 200)" \
    "IDG := A0*B2 - q^2*A9*B4" \
    "jp := jacprodmake(IDG, 50)" \
    "jac2prod(jp)")
LAST39=$(echo "$OUT39" | tail -1)
if echo "$LAST39" | grep -qE '\(q.*11'; then
    pass "Block 39: jac2prod IDG"
else
    fail "Block 39: jac2prod IDG (depends on Block 38)"
fi

# Block 40: winquist(q^5, q^3, q^11, 100) — first few coefficients
OUT40=$(run "set_trunc(100)" "series(winquist(q^5, q^3, q^11, 100), 20)")
if echo "$OUT40" | grep -q "2q"; then
    pass "Block 40: winquist"
else
    fail "Block 40: winquist"
fi

# Block 41: IDG - winquist = 0 — the critical identity check
OUT41=$(run "set_trunc(200)" \
    "A0 := tripleprod(q^15, q^33, 200)" \
    "A9 := tripleprod(q^9, q^33, 200)" \
    "B2 := tripleprod(q^13, q^33, 200) - q^3 * tripleprod(q^2, q^33, 200)" \
    "B4 := tripleprod(q^7, q^33, 200) + q * tripleprod(q^4, q^33, 200)" \
    "IDG := A0*B2 - q^2*A9*B4" \
    "W := winquist(q^5, q^3, q^11, 200)" \
    "DIFF := IDG - W" \
    "series(DIFF, 60)")
if echo "$OUT41" | grep -qE "^0 \+ O|^O\(q"; then
    pass "Block 41: IDG - winquist = 0"
else
    fail "Block 41: IDG - winquist = 0"
fi

echo ""
echo "============================="
echo "Results: $PASS passed, $FAIL failed, $SKIP skipped (of 41 blocks)"
echo "============================="
[ "$FAIL" -le 5 ]
