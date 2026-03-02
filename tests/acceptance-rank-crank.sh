#!/bin/bash
# acceptance-rank-crank.sh — Phase 69: Rank and Crank generating functions
PASS=0; FAIL=0
check() {
    local desc="$1" input="$2" pattern="$3"
    local out
    out=$(echo "$input" | ./qseries.exe 2>&1)
    if echo "$out" | grep -qE "$pattern"; then
        echo "PASS: $desc"; ((PASS++))
    else
        echo "FAIL: $desc"; echo "  got: $out"; ((FAIL++))
    fi
}

# Test 1: Rank symmetry — rankgf(2,20) = rankgf(-2,20)
check "rank symmetry" \
    "rankgf(2,20) - rankgf(-2,20)" \
    "^0"

# Test 2: Crank symmetry — crankgf(3,20) = crankgf(-3,20)
check "crank symmetry" \
    "crankgf(3,20) - crankgf(-3,20)" \
    "^0"

# Test 3: N(0,5) = 1
check "N(0,5) = 1" \
    "coeff(rankgf(0,20), 5)" \
    "^1$"

# Test 4: N(0,7) = 3
check "N(0,7) = 3" \
    "coeff(rankgf(0,20), 7)" \
    "^3$"

# Test 5: N(1,10) = 5
check "N(1,10) = 5" \
    "coeff(rankgf(1,20), 10)" \
    "^5$"

# Test 6: M(0,0) = 1
check "M(0,0) = 1" \
    "coeff(crankgf(0,20), 0)" \
    "^1$"

# Test 7: M(0,1) = -1 (Andrews-Garvan convention)
check "M(0,1) = -1" \
    "coeff(crankgf(0,20), 1)" \
    "^-1$"

# Test 8: M(1,7) = 2
check "M(1,7) = 2" \
    "coeff(crankgf(1,20), 7)" \
    "^2$"

# Test 9: M(2,10) = 4
check "M(2,10) = 4" \
    "coeff(crankgf(2,20), 10)" \
    "^4$"

# Test 10: Partition count — Σ_m N(m,10) = p(10) = 42
check "rank sum = p(10) = 42" \
    "coeff(rankgf(0,15) + 2*sum(rankgf(m,15), m, 1, 14), 10)" \
    "^42$"

# Test 11: Partition count — Σ_m M(m,10) = p(10) = 42
check "crank sum = p(10) = 42" \
    "coeff(crankgf(0,15) + 2*sum(crankgf(m,15), m, 1, 14), 10)" \
    "^42$"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
