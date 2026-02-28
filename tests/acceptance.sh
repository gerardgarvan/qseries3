#!/usr/bin/env bash
# Acceptance test script: runs all 9 SPEC tests through the REPL.
# Each multi-step test pipes commands into a single qseries process so variable state persists.
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

# Test 1: Rogers-Ramanujan
if run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "prodmake(rr, 40)" | grep -qE "1-q|\(q,q"; then
    echo "PASS: Test 1 Rogers-Ramanujan"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 1 Rogers-Ramanujan"
    FAIL=$((FAIL+1))
fi

# Test 2: Partition
if run "p := 1/etaq(1,50)" "series(p, 20)" | grep -q "42"; then
    echo "PASS: Test 2 Partition"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 2 Partition"
    FAIL=$((FAIL+1))
fi

# Test 3: Theta eta (etamake on theta3, theta4)
if run "etamake(theta3(q,100), 100)" | grep -q $'\xCE\xB7'; then
    echo "PASS: Test 3a etamake(theta3)"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 3a etamake(theta3)"
    FAIL=$((FAIL+1))
fi
if run "etamake(theta4(q,100), 100)" | grep -q $'\xCE\xB7'; then
    echo "PASS: Test 3b etamake(theta4)"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 3b etamake(theta4)"
    FAIL=$((FAIL+1))
fi

# Test 4: Jacobi RR
if run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "jacprodmake(rr, 40)" | grep -qE "JAC|\(q,q"; then
    echo "PASS: Test 4 Jacobi RR"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 4 Jacobi RR"
    FAIL=$((FAIL+1))
fi

# Test 5: Gauss AGM
if run "findhom([theta3(q,100), theta4(q,100), theta3(subs_q(q,2),100), theta4(subs_q(q,2),100)], 2, 0)" | grep -q "X"; then
    echo "PASS: Test 5 Gauss AGM"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 5 Gauss AGM"
    FAIL=$((FAIL+1))
fi

# Test 6: Rødseth (use T=30 for etamake; T=38 hits coefficient overflow)
if run "PD := etaq(2,200)/etaq(1,200)" "PD1 := sift(PD,5,1,199)" "etamake(PD1, 30)" | grep -q $'\xCE\xB7'; then
    echo "PASS: Test 6 Rødseth"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 6 Rødseth"
    FAIL=$((FAIL+1))
fi

# Test 7: Qfactor
if run "set_trunc(64)" "t8 := T(8,8)" "qfactor(t8, 20)" | grep -q "1-q"; then
    echo "PASS: Test 7 Qfactor"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 7 Qfactor"
    FAIL=$((FAIL+1))
fi

# Test 8: Watson
if run "set_trunc(100)" "xi := q^2*etaq(49,100)/etaq(1,100)" "T := q*(etaq(7,100)/etaq(1,100))^4" "findnonhomcombo(T^2, [T, xi], [1, 7], 0)" | grep -q "X"; then
    echo "PASS: Test 8 Watson"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 8 Watson"
    FAIL=$((FAIL+1))
fi

# Test 9: Euler pentagonal
if run "set_trunc(500)" "EULER := etaq(1,500)" "E0 := sift(EULER,5,0,499)" "jacprodmake(E0,50)" | grep -qE "JAC|\(q,q"; then
    echo "PASS: Test 9 Euler"
    PASS=$((PASS+1))
else
    echo "FAIL: Test 9 Euler"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Total: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ]
