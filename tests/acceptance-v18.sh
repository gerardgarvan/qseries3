#!/usr/bin/env bash
# Acceptance test: v1.8 functions (divisors, mobius, euler_phi, jacobi, kronecker,
#                                   coeff, dissect, eisenstein, partition, qdiff)
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

# 1. divisors
if run "divisors(12)" | grep -q '\[1, 2, 3, 4, 6, 12\]'; then
    echo "PASS: divisors"
    PASS=$((PASS+1))
else
    echo "FAIL: divisors"
    FAIL=$((FAIL+1))
fi

# 2. mobius (squarefree, even number of factors)
if run "mobius(6)" | grep -q '^1'; then
    echo "PASS: mobius squarefree"
    PASS=$((PASS+1))
else
    echo "FAIL: mobius squarefree"
    FAIL=$((FAIL+1))
fi

# 3. mobius (squared factor)
if run "mobius(12)" | grep -q '^0'; then
    echo "PASS: mobius squared"
    PASS=$((PASS+1))
else
    echo "FAIL: mobius squared"
    FAIL=$((FAIL+1))
fi

# 4. euler_phi
if run "euler_phi(12)" | grep -q '^4'; then
    echo "PASS: euler_phi"
    PASS=$((PASS+1))
else
    echo "FAIL: euler_phi"
    FAIL=$((FAIL+1))
fi

# 5. jacobi
if run "jacobi(2, 15)" | grep -q '^1'; then
    echo "PASS: jacobi"
    PASS=$((PASS+1))
else
    echo "FAIL: jacobi"
    FAIL=$((FAIL+1))
fi

# 6. kronecker
if run "kronecker(-1, -1)" | grep -q '\-1'; then
    echo "PASS: kronecker"
    PASS=$((PASS+1))
else
    echo "FAIL: kronecker"
    FAIL=$((FAIL+1))
fi

# 7. coeff
if run "coeff(etaq(1,30), 5)" | grep -q '^1'; then
    echo "PASS: coeff"
    PASS=$((PASS+1))
else
    echo "FAIL: coeff"
    FAIL=$((FAIL+1))
fi

# 8. dissect (check that k=0 label appears)
if run "dissect(etaq(1,30), 3, 10)" | grep -q 'k=0'; then
    echo "PASS: dissect"
    PASS=$((PASS+1))
else
    echo "FAIL: dissect"
    FAIL=$((FAIL+1))
fi

# 9. eisenstein (E_4 has 240q term)
if run "eisenstein(2, 10)" | grep -q '240q'; then
    echo "PASS: eisenstein"
    PASS=$((PASS+1))
else
    echo "FAIL: eisenstein"
    FAIL=$((FAIL+1))
fi

# 10. partition
if run "partition(100)" | grep -q '190569292'; then
    echo "PASS: partition"
    PASS=$((PASS+1))
else
    echo "FAIL: partition"
    FAIL=$((FAIL+1))
fi

# 11. qdiff
if run "set_trunc(10)" "f := 1 + q + q^2 + q^3" "qdiff(f)" | grep -qE '2q'; then
    echo "PASS: qdiff"
    PASS=$((PASS+1))
else
    echo "FAIL: qdiff"
    FAIL=$((FAIL+1))
fi

echo ""
echo "v1.8 functions: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
