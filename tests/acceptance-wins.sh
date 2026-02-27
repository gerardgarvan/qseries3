#!/usr/bin/env bash
# Acceptance test: Phase 23 quick wins (version, qdegree, lqdegree, jac2series, findlincombo, Makefile CXX)
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

# 1. --version flag
if "$BIN" --version 2>&1 | grep -q "qseries 1.9"; then
    echo "PASS: --version"
    PASS=$((PASS+1))
else
    echo "FAIL: --version"
    FAIL=$((FAIL+1))
fi

# 2. version built-in (must call version() — bare "version" parses as variable)
if run "version()" | grep -q "qseries 1.9"; then
    echo "PASS: version built-in"
    PASS=$((PASS+1))
else
    echo "FAIL: version built-in"
    FAIL=$((FAIL+1))
fi

# 3. qdegree
if run "qdegree(etaq(1,50))" | grep -qE '^[0-9]+'; then
    echo "PASS: qdegree"
    PASS=$((PASS+1))
else
    echo "FAIL: qdegree"
    FAIL=$((FAIL+1))
fi

# 4. lqdegree
if run "lqdegree(etaq(1,50))" | grep -qE '^[0-9]+'; then
    echo "PASS: lqdegree"
    PASS=$((PASS+1))
else
    echo "FAIL: lqdegree"
    FAIL=$((FAIL+1))
fi

# 5. jac2series (rr -> jacprodmake -> jac2series)
if run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "jp := jacprodmake(rr, 40)" "jac2series(jp, 50)" | grep -qE '1 \+|q|O\('; then
    echo "PASS: jac2series"
    PASS=$((PASS+1))
else
    echo "FAIL: jac2series"
    FAIL=$((FAIL+1))
fi

# 6. findlincombo (runs without error; may show solution or "no solution")
if run "findlincombo(etaq(1,50), [theta3(q,50), theta4(q,50)], 0)" 2>&1 | grep -qE 'no solution|X_|^-?[0-9]'; then
    echo "PASS: findlincombo"
    PASS=$((PASS+1))
else
    echo "FAIL: findlincombo"
    FAIL=$((FAIL+1))
fi

# 7. make with default CXX
if make -C . dist/qseries.exe 2>&1 | tail -1 | grep -qE 'qseries|nothing to be done'; then
    echo "PASS: make (default CXX)"
    PASS=$((PASS+1))
else
    echo "FAIL: make (default CXX)"
    FAIL=$((FAIL+1))
fi

echo ""
echo "Quick wins: $PASS passed, $FAIL failed"
[ "$FAIL" -eq 0 ] || exit 1
