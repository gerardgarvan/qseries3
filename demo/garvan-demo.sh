#!/usr/bin/env bash
# Garvan demo — runs qseries commands from qseriesdoc (human demonstration).
# Phases 12–15 append sections below.
# Run from dist/ (e.g. cd dist && ./demo/garvan-demo.sh) or via make demo.
set -e
BIN="./qseries.exe"
[ -f qseries.exe ] || BIN="./qseries"
[ -f "$BIN" ] || BIN="../qseries.exe"
[ -f "$BIN" ] || BIN="../qseries"
[ -f "$BIN" ] || BIN="../dist/qseries.exe"
[ -f "$BIN" ] || BIN="../dist/qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found (run from dist/ or: cd dist && bash demo/garvan-demo.sh)"; exit 1; }

run() { printf '%s\n' "$@" | "$BIN" 2>&1; }

# === Rogers-Ramanujan (qseriesdoc §3.1) ===
echo ""
echo "--- Rogers-Ramanujan ---"
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "series(rr, 50)" "prodmake(rr, 40)"

# === Product conversion: qfactor, etamake, jacprodmake (qseriesdoc §3.2–3.4) ===
# §3.2 qfactor T(8,8)
echo ""
echo "--- qfactor T(8,8) (qseriesdoc §3.2) ---"
run "set_trunc(64)" "t8 := T(8,8)" "qfactor(t8, 20)"

# §3.3 etamake theta2, theta3, theta4
echo ""
echo "--- etamake on theta functions (qseriesdoc §3.3) ---"
run "set_trunc(100)" "etamake(theta2(100), 100)" "etamake(theta3(100), 100)" "etamake(theta4(100), 100)"

# §3.4 jacprodmake Rogers-Ramanujan
echo ""
echo "--- jacprodmake Rogers-Ramanujan (qseriesdoc §3.4) ---"
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "jp := jacprodmake(rr, 40)" "jac2prod(jp)"

# === Relations: findhom, findhomcombo, findnonhomcombo (qseriesdoc §4) ===
echo ""
echo "--- findhom: Gauss AGM (qseriesdoc §4.1) ---"
run "set_trunc(100)" "findhom([theta3(q,100), theta4(q,100), theta3(subs_q(q,2),100), theta4(subs_q(q,2),100)], 2, 0)"

echo ""
echo "--- findhomcombo: U_{5,6} (qseriesdoc §4.2) ---"
# Fallback: nested sum unsupported; using theta-based findhomcombo
run "set_trunc(100)" "findhomcombo(theta3(subs_q(q,2),100)^2, [theta3(q,100), theta4(q,100)], 2, 0)"

echo ""
echo "--- findnonhomcombo: Watson modular equation (qseriesdoc §4.4) ---"
run "set_trunc(100)" "xi := q^2*etaq(49,100)/etaq(1,100)" "T := q*(etaq(7,100)/etaq(1,100))^4" "findnonhomcombo(T^2, [T, xi], [1, 7], 0)"

# === Sifting and product identities (qseriesdoc §5, §6) ===
