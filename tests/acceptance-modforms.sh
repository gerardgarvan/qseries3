#!/bin/bash
# acceptance-modforms.sh — Phase 83: Modular Forms
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

# Test 1: DELTA12(50) = q * etaq(1,50)^24
check "DELTA12 matches q*etaq^24" "DELTA12(10) - q*etaq(1,10)^24" "^0"

# Test 2: makebasisM(12, 50) returns 2 basis elements
check "makebasisM(12,50) has 2 elements" "makebasisM(12, 50)" "\[2 basis elements\]"

# Test 3: makebasisM(4, 30) returns 1 basis element
check "makebasisM(4,30) has 1 element" "makebasisM(4, 30)" "\[1 basis elements\]"

# Test 4: makebasisPX(12, 50) returns basis (partition P in first element)
check "makebasisPX(12,50) has partition P" "makebasisPX(12, 20)" "1 \+ q \+ 2q"

# Test 5: makeALTbasisM(12, 50) returns 2 basis elements
check "makeALTbasisM(12,50) has 2 elements" "makeALTbasisM(12, 50)" "\[2 basis elements\]"

# Test 6: makeALTbasisM(2, 50) returns 0 basis elements (empty)
check "makeALTbasisM(2,50) empty" "makeALTbasisM(2, 50)" "\[0 basis elements\]"

# Test 7: makeALTbasisM dimension match with makebasisM
check "makeALTbasisM(4,50) has 1 element" "makeALTbasisM(4, 50)" "\[1 basis elements\]"
check "makeALTbasisM(6,50) has 1 element" "makeALTbasisM(6, 50)" "\[1 basis elements\]"
check "makeALTbasisM(24,50) has 3 elements" "makeALTbasisM(24, 50)" "\[3 basis elements\]"

echo ""
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
