#!/usr/bin/env bash
# Exercise verification tests from qseriesdoc.md (Exercises 4, 9, 10).
# These depend on q-shift fix (Phase 63), fractional power (Phase 64),
# and Jacobi half-integer exponents (Phase 65).

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

pass() { echo "PASS: $1"; PASS=$((PASS+1)); }
fail() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

# --- Exercise 4: b(q) = eta(tau)^3 / eta(3*tau) ---

# EX-04a: series coefficients start with 1 - 3q
if run "b := etaq(1,50)^3 / etaq(3,50)" "series(b, 20)" | grep -q "1 - 3q"; then
    pass "EX-04a b(q) series starts 1 - 3q"
else
    fail "EX-04a b(q) series starts 1 - 3q"
fi

# EX-04b: etamake identifies the eta quotient
if run "b := etaq(1,50)^3 / etaq(3,50)" "etamake(b, 50)" | grep -q $'\xCE\xB7'; then
    pass "EX-04b etamake identifies eta quotient"
else
    fail "EX-04b etamake identifies eta quotient"
fi

# --- Exercise 9: N(q) = a^6(1 - 20x - 8x^2) via findnonhomcombo ---

OUTPUT9=$(run \
    "set_trunc(100)" \
    "a := theta3(q,100)*theta3(q^3,100) + theta2(q,100)*theta2(q^3,100)" \
    "c3 := 27*q*etaq(3,100)^9/etaq(1,100)^3" \
    "x := c3/a^3" \
    "N := 1 - 504*sum(sigma(n,5)*q^n, n, 1, 99)" \
    "findnonhomcombo(N, [a, x], [6, 2], 0)")

# EX-09a: output contains coefficient 20
if echo "$OUTPUT9" | grep -q "20"; then
    pass "EX-09a findnonhomcombo contains coefficient 20"
else
    fail "EX-09a findnonhomcombo contains coefficient 20"
fi

# EX-09b: output contains coefficient 8
if echo "$OUTPUT9" | grep -q "8"; then
    pass "EX-09b findnonhomcombo contains coefficient 8"
else
    fail "EX-09b findnonhomcombo contains coefficient 8"
fi

# EX-09c: output uses X subscript variables
if echo "$OUTPUT9" | grep -q "X"; then
    pass "EX-09c findnonhomcombo uses X variables"
else
    fail "EX-09c findnonhomcombo uses X variables"
fi

# --- Exercise 10: findpoly cubic relation ---

OUTPUT10=$(run \
    "set_trunc(100)" \
    "m := theta3(q,100)^2 / theta3(q^3,100)^2" \
    "a := theta3(q,100)*theta3(q^3,100) + theta2(q,100)*theta2(q^3,100)" \
    "c3 := 27*q*etaq(3,100)^9/etaq(1,100)^3" \
    "y := c3/a^3" \
    "findpoly(m, y, 6, 1)")

# EX-10a: output contains coefficient 27
if echo "$OUTPUT10" | grep -q "27"; then
    pass "EX-10a findpoly contains coefficient 27"
else
    fail "EX-10a findpoly contains coefficient 27"
fi

# EX-10b: output uses X subscript variables
if echo "$OUTPUT10" | grep -q "X"; then
    pass "EX-10b findpoly uses X variables"
else
    fail "EX-10b findpoly uses X variables"
fi

echo ""
echo "Exercise tests: $PASS pass, $FAIL fail"
[ "$FAIL" -eq 0 ]
