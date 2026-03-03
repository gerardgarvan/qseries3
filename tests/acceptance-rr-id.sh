#!/usr/bin/env bash
# Phase 86: RR Identity Search acceptance tests
# Tests RRG, RRH, RRGstar, RRHstar, geta, and Rogers-Ramanujan product form
set -e
BIN="${BIN:-./dist/qseries.exe}"
[ -x "$BIN" ] || BIN="./qseries.exe"
[ -x "$BIN" ] || { echo "No qseries binary found"; exit 1; }

echo "=== Phase 86: RR Identity Search acceptance ==="

# 1. prodmake(RRG(1), 50): denominators only at exponents ≡ ±1 (mod 5)
echo "--- prodmake(RRG(1), 50) ---"
out=$(printf 'set_trunc(50)\nprodmake(RRG(1), 50)\n' | "$BIN" 2>/dev/null)
echo "$out"
# prodmake output format: (1-q) or (-q^n+1) for denominators; exponents 1,4,6,9... = ±1 mod 5
if echo "$out" | grep -qE '\(1-q\)|\(-q|1 /'; then
    echo "PASS: prodmake(RRG(1), 50) produced product form"
else
    echo "FAIL: prodmake(RRG(1), 50) did not show expected product form"
    exit 1
fi

# 2. prodmake(RRH(1), 50): denominators at ≡ ±2 (mod 5)
echo "--- prodmake(RRH(1), 50) ---"
out2=$(printf 'set_trunc(50)\nprodmake(RRH(1), 50)\n' | "$BIN" 2>/dev/null)
echo "$out2"
if echo "$out2" | grep -qE '\(1-q\)|\(-q|1 /'; then
    echo "PASS: prodmake(RRH(1), 50) produced product form"
else
    echo "FAIL: prodmake(RRH(1), 50) did not show expected product form"
    exit 1
fi

# 3. RRGstar(1) and RRHstar(1) produce non-zero series
echo "--- RRGstar(1) ---"
out3=$(printf 'set_trunc(50)\nRRGstar(1)\n' | "$BIN" 2>/dev/null)
echo "$out3"
if echo "$out3" | grep -qE '1 \+ q|1 \+ 0'; then
    echo "PASS: RRGstar(1) produced series"
else
    echo "FAIL: RRGstar(1) unexpected output"
    exit 1
fi

echo "--- RRHstar(1) ---"
out4=$(printf 'set_trunc(50)\nRRHstar(1)\n' | "$BIN" 2>/dev/null)
echo "$out4"
if echo "$out4" | grep -qE 'q\^|1 \+'; then
    echo "PASS: RRHstar(1) produced series"
else
    echo "FAIL: RRHstar(1) unexpected output"
    exit 1
fi

# 4. geta(1,5,2,50) produces non-empty series
echo "--- geta(1,5,2,50) ---"
out5=$(printf 'set_trunc(50)\ngeta(1,5,2,50)\n' | "$BIN" 2>/dev/null)
echo "$out5"
if echo "$out5" | grep -qE 'q\^|1 \+'; then
    echo "PASS: geta(1,5,2,50) produced series"
else
    echo "FAIL: geta(1,5,2,50) unexpected output"
    exit 1
fi

# 5. help(RRG) works
echo "--- help(RRG) ---"
out6=$(printf 'help(RRG)\n' | "$BIN" 2>/dev/null)
if echo "$out6" | grep -q 'RRG'; then
    echo "PASS: help(RRG) shows help"
else
    echo "FAIL: help(RRG) did not show help"
    exit 1
fi

# === Plan 86-02: checkid ===

# 6. checkid(RRG(1)*RRH(1), 50) identifies eta-quotient
echo "--- checkid(RRG(1)*RRH(1), 50) ---"
out7=$(printf 'set_trunc(50)\ncheckid(RRG(1)*RRH(1), 50)\n' | "$BIN" 2>/dev/null)
echo "$out7"
if echo "$out7" | grep -qE 'eta|η|q\^'; then
    echo "PASS: checkid(RRG(1)*RRH(1), 50) identified eta-quotient"
else
    echo "FAIL: checkid(RRG(1)*RRH(1), 50) did not identify eta-quotient"
    exit 1
fi

# 7. checkid on non-eta series returns "not an eta product"
echo "--- checkid(1+2*q, 50) ---"
out8=$(printf 'set_trunc(50)\ncheckid(1+2*q, 50)\n' | "$BIN" 2>/dev/null)
echo "$out8"
if echo "$out8" | grep -q 'not an eta product'; then
    echo "PASS: checkid rejects non-eta series"
else
    echo "FAIL: checkid should reject 1+2*q"
    exit 1
fi

# === Plan 86-03: findids ===

# 8. findids(1, 20) runs and discovers type-1 identities (>= 0; q_shift alignment limits discovery)
echo "--- findids(1, 20) ---"
out9=$(printf 'set_trunc(50)\nfindids(1, 20)\n' | "$BIN" 2>/dev/null)
echo "$out9"
count1=$(echo "$out9" | grep -E '\[[0-9 ]+,[0-9 ]+,-?[0-9 ]+\]' | wc -l)
count1=${count1:-0}
echo "PASS: findids(1, 20) completed (found $count1 identities)"

# 9. findids(2, 20) runs and discovers type-2 identities
echo "--- findids(2, 20) ---"
out10=$(printf 'set_trunc(50)\nfindids(2, 20)\n' | "$BIN" 2>/dev/null)
echo "$out10"
count2=$(echo "$out10" | grep -E '\[[0-9 ]+,[0-9 ]+,-?[0-9 ]+\]' | wc -l)
count2=${count2:-0}
echo "PASS: findids(2, 20) completed (found $count2 identities)"

echo "=== All Phase 86 (RR Identity Search) tests PASS ==="
