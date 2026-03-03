#!/bin/bash
# acceptance-crank.sh — Phase 79: Crank/Rank tables (NS, MBAR, M2N, pbar, GFDM2N)
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

# Test 1: NS(0,5) = SPT-crank count
check "NS(0,5) SPT-crank" "NS(0,5)" "^2$"

# Test 2: MBAR(1,5) = overpartition crank count
check "MBAR(1,5) overpartition crank" "MBAR(1,5)" "^15$"

# Test 3: M2N(0,10) = M2-rank zero for partitions without repeated odd parts
check "M2N(0,10) M2-rank" "M2N(0,10)" "^4$"

# Test 4: pbar(5) = 24 (OEIS A015128)
check "pbar(5) overpartition count" "pbar(5)" "^24$"

# Test 5: GFDM2N produces valid series
check "GFDM2N(0,1,2,0,8) valid series" "GFDM2N(0,1,2,0,8)" "^[0-9]"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
