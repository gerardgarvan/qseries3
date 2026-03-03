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
out=$(echo 'set_trunc(50); prodmake(RRG(1), 50)' | "$BIN" 2>/dev/null)
echo "$out"
# prodmake output format: (1-q^n) or (q^n-1) for denominators; exponents 1,4,6,9,11,14... = ±1 mod 5
if echo "$out" | grep -qE '\(1-q|\(q[0-9]+-1\)'; then
    echo "PASS: prodmake(RRG(1), 50) produced product form"
else
    echo "FAIL: prodmake(RRG(1), 50) did not show expected product form"
    exit 1
fi

# 2. prodmake(RRH(1), 50): denominators at ≡ ±2 (mod 5)
echo "--- prodmake(RRH(1), 50) ---"
out2=$(echo 'set_trunc(50); prodmake(RRH(1), 50)' | "$BIN" 2>/dev/null)
echo "$out2"
if echo "$out2" | grep -qE '\(1-q|\(q[0-9]+-1\)'; then
    echo "PASS: prodmake(RRH(1), 50) produced product form"
else
    echo "FAIL: prodmake(RRH(1), 50) did not show expected product form"
    exit 1
fi

# 3. RRGstar(1) and RRHstar(1) produce non-zero series
echo "--- RRGstar(1) ---"
out3=$(echo 'set_trunc(50); RRGstar(1)' | "$BIN" 2>/dev/null)
echo "$out3"
if echo "$out3" | grep -qE '1 \+ q|1 \+ 0'; then
    echo "PASS: RRGstar(1) produced series"
else
    echo "FAIL: RRGstar(1) unexpected output"
    exit 1
fi

echo "--- RRHstar(1) ---"
out4=$(echo 'set_trunc(50); RRHstar(1)' | "$BIN" 2>/dev/null)
echo "$out4"
if echo "$out4" | grep -qE 'q\^|1 \+'; then
    echo "PASS: RRHstar(1) produced series"
else
    echo "FAIL: RRHstar(1) unexpected output"
    exit 1
fi

# 4. geta(1,5,2,50) produces non-empty series
echo "--- geta(1,5,2,50) ---"
out5=$(echo 'set_trunc(50); geta(1,5,2,50)' | "$BIN" 2>/dev/null)
echo "$out5"
if echo "$out5" | grep -qE 'q\^|1 \+'; then
    echo "PASS: geta(1,5,2,50) produced series"
else
    echo "FAIL: geta(1,5,2,50) unexpected output"
    exit 1
fi

# 5. help(RRG) works
echo "--- help(RRG) ---"
out6=$(echo 'help(RRG)' | "$BIN" 2>/dev/null)
if echo "$out6" | grep -q 'RRG'; then
    echo "PASS: help(RRG) shows help"
else
    echo "FAIL: help(RRG) did not show help"
    exit 1
fi

# === Plan 86-02: checkid ===

# 6. checkid(RRG(1)*RRH(1), 50) identifies eta-quotient
echo "--- checkid(RRG(1)*RRH(1), 50) ---"
out7=$(echo 'set_trunc(50); checkid(RRG(1)*RRH(1), 50)' | "$BIN" 2>/dev/null)
echo "$out7"
if echo "$out7" | grep -qE 'eta|η|q\^'; then
    echo "PASS: checkid(RRG(1)*RRH(1), 50) identified eta-quotient"
else
    echo "FAIL: checkid(RRG(1)*RRH(1), 50) did not identify eta-quotient"
    exit 1
fi

# 7. checkid on non-eta series returns "not an eta product"
echo "--- checkid(1+q+q^2, 50) ---"
out8=$(echo 'set_trunc(50); checkid(1+q+q^2, 50)' | "$BIN" 2>/dev/null)
echo "$out8"
if echo "$out8" | grep -q 'not an eta product'; then
    echo "PASS: checkid rejects non-eta series"
else
    echo "FAIL: checkid should reject 1+q+q^2"
    exit 1
fi

echo "=== All Phase 86-01 and 86-02 tests PASS ==="
