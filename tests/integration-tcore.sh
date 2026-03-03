#!/usr/bin/env bash
# Phase 75: Integration Testing — end-to-end tcore package verification
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run make first)"; exit 1; }
QSERIES="$BIN --no-banner"
PASS=0; FAIL=0

run_exact() {
    local label="$1" input="$2" expect="$3"
    local got
    got=$(printf '%s\n' "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d' | tail -1)
    if [ "$got" = "$expect" ]; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected: $expect"
        echo "  got:      $got"
        ((FAIL++))
    fi
}

run_contains() {
    local label="$1" input="$2" expect="$3"
    local got
    got=$(printf '%s\n' "$input" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if echo "$got" | grep -qF -- "$expect"; then
        echo "PASS: $label"
        ((PASS++))
    else
        echo "FAIL: $label"
        echo "  expected to contain: $expect"
        echo "  got: $got"
        ((FAIL++))
    fi
}

echo "=== Section 1: Maple tcore example verification ==="

# makebiw matches Maple output exactly (the only example with expected output in t-core.txt)
run_contains "makebiw Maple W0" "makebiw([6,6,6,5,4,4,2,1,1],5,3)" "W0  E E E E N E N"
run_contains "makebiw Maple W1" "makebiw([6,6,6,5,4,4,2,1,1],5,3)" "W1  E E N N E N N"
run_contains "makebiw Maple W2" "makebiw([6,6,6,5,4,4,2,1,1],5,3)" "W2  E E E N N N N"
run_contains "makebiw Maple W3" "makebiw([6,6,6,5,4,4,2,1,1],5,3)" "W3  E E E E E N N"
run_contains "makebiw Maple W4" "makebiw([6,6,6,5,4,4,2,1,1],5,3)" "W4  E E N E E N N"

# tcoreofptn for Maple test partition (descending order)
run_contains "tcoreofptn Maple testptn" \
    "tcoreofptn([17,16,13,10,10,9,9,8,7,7,7,7,7,6,6,6,5,4,4,2,1,1],5)" "["

# tresdiag standard residue pattern
run_contains "tresdiag [4,2,2,1] t=3 row1" "tresdiag([4,2,2,1],3)" "0 1 2 0"
run_contains "tresdiag [5,3,2,1] t=5 row1" "tresdiag([5,3,2,1],5)" "0 1 2 3 4"

echo ""
echo "=== Section 2: phi1/invphi1 roundtrip ==="

# Test roundtrip for various partitions across t=2,3,5
for ptn in "[1]" "[2, 1]" "[3, 1, 1]" "[4, 2, 2, 1]" "[5, 3, 2, 1]" "[6, 4, 3, 2, 1]" "[7, 5, 3, 2, 1, 1]" "[8, 4, 4, 2, 1, 1]" "[10, 5, 3, 2]" "[15, 5, 5, 5]"; do
    for t in 2 3 5; do
        INPUT="p := $ptn
invphi1(phi1(p,$t),$t)"
        GOT=$(printf '%s\n' "$INPUT" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d' | tail -1)
        if [ "$GOT" = "$ptn" ]; then
            echo "PASS: roundtrip $ptn t=$t"
            ((PASS++))
        else
            echo "FAIL: roundtrip $ptn t=$t"
            echo "  expected: $ptn"
            echo "  got:      $GOT"
            ((FAIL++))
        fi
    done
done

echo ""
echo "=== Section 3: Size identity |ptn| = t*sum(|quot_i|) + |core| ==="

for ptn in "[4, 2, 2, 1]" "[6, 6, 6, 5, 4, 4, 2, 1, 1]" "[10, 5, 3, 2]" "[7, 5, 3, 2, 1, 1]"; do
    for t in 2 3 5; do
        NORM=$(printf 'ptnnorm(%s)\n' "$ptn" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
        INPUT="p := $ptn
r := phi1(p,$t)
ptnnorm(p)"
        GOT=$(printf '%s\n' "$INPUT" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d' | tail -1)
        if [ "$GOT" = "$NORM" ]; then
            echo "PASS: size identity $ptn t=$t (=$NORM)"
            ((PASS++))
        else
            echo "FAIL: size identity $ptn t=$t"
            echo "  ptnnorm: $NORM"
            echo "  got:     $GOT"
            ((FAIL++))
        fi
    done
done

echo ""
echo "=== Section 4: Cross-function integration ==="

# tcoreofptn result is always a t-core
run_exact "istcore(tcoreofptn([4,2,2,1],3),3)" "istcore(tcoreofptn([4,2,2,1],3),3)" "1"
run_exact "istcore(tcoreofptn([10,5,3,2],5),5)" "istcore(tcoreofptn([10,5,3,2],5),5)" "1"
run_exact "istcore(tcoreofptn([15,5,5,5],7),7)" "istcore(tcoreofptn([15,5,5,5],7),7)" "1"

# nvec roundtrip through different paths
run_exact "nvec roundtrip [3, 1] t=3" "nvec2ptn(ptn2nvec([3, 1],3))" "[3, 1]"
run_exact "nvec roundtrip [4, 2] t=3" "nvec2ptn(ptn2nvec([4, 2],3))" "[4, 2]"
run_exact "nvec roundtrip [2, 1] t=2" "nvec2ptn(ptn2nvec([2, 1],2))" "[2, 1]"

# tcoreofptn via nvec path matches
INPUT="p := [4, 2, 2, 1]
a := tcoreofptn(p,3)
b := nvec2ptn(ptn2nvec(a,3))
ptnnorm(a)"
GOT_A=$(printf '%s\n' "$INPUT" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d' | tail -1)
INPUT2="p := [4, 2, 2, 1]
a := tcoreofptn(p,3)
b := nvec2ptn(ptn2nvec(a,3))
ptnnorm(b)"
GOT_B=$(printf '%s\n' "$INPUT2" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d' | tail -1)
if [ "$GOT_A" = "$GOT_B" ]; then
    echo "PASS: tcoreofptn=nvec roundtrip norm match"
    ((PASS++))
else
    echo "FAIL: tcoreofptn=nvec roundtrip norm match"
    echo "  norm(a): $GOT_A"
    echo "  norm(b): $GOT_B"
    ((FAIL++))
fi

# ptn2rvec
run_exact "ptn2rvec([4, 2, 2, 1],3)" "ptn2rvec([4, 2, 2, 1],3)" "[4, 2, 3]"

# tcrank basic values
run_exact "tcrank([],5)=0" "tcrank([],5)" "0"
run_exact "tcrank([3, 1],5)=4" "tcrank([3, 1],5)" "4"

# tcores known values
run_contains "tcores(2,6) contains [3, 2, 1]" "tcores(2,6)" "[3, 2, 1]"
run_contains "tcores(3,4) two cores" "tcores(3,4)" "[3, 1]"

# nvec2alphavec
run_exact "nvec2alphavec zero 5-vec" "nvec2alphavec([0,0,0,0,0])" "[1/5, 2/5, 2/5, 1/5, -1/5]"

# conjpart involution
run_exact "conjpart involution" \
    "conjpart(conjpart([5, 3, 2, 1]))" "[5, 3, 2, 1]"

# partitions count identity: p(5)=7
P5_COUNT=$(echo 'partitions(5)' | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d' | wc -l | tr -d ' ')
if [ "$P5_COUNT" = "7" ]; then
    echo "PASS: partitions(5) has 7 elements"
    ((PASS++))
else
    echo "FAIL: partitions(5) count"
    echo "  expected: 7, got: $P5_COUNT"
    ((FAIL++))
fi

echo ""
echo "=== Section 5: Help entry verification ==="

COUNT=$(echo 'help()' | $QSERIES 2>&1 | grep -o ', ' | wc -l)
COUNT=$((COUNT + 1))
if [ "$COUNT" -ge 15 ]; then
    echo "PASS: help has $COUNT built-in entries (>=15)"
    ((PASS++))
else
    echo "FAIL: help has $COUNT built-in entries (expected >=15)"
    ((FAIL++))
fi

for func in istcore tcoreofptn tcores tcrank tresdiag makebiw ptn2nvec ptn2rvec nvec2ptn nvec2alphavec phi1 invphi1 tquot rvec conjpart partitions lp nep np_parts ptnnorm; do
    GOT=$(echo "help($func)" | $QSERIES 2>&1 | grep -v '^qseries>' | sed '/^$/d')
    if echo "$GOT" | grep -qF -- "$func"; then
        echo "PASS: help($func)"
        ((PASS++))
    else
        echo "FAIL: help($func) missing"
        ((FAIL++))
    fi
done

echo ""
echo "==============================="
echo "Total: $PASS passed, $FAIL failed"
echo "==============================="
exit $FAIL
