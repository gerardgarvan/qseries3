#!/usr/bin/env bash
# Garvan demo — runs qseries commands from qseriesdoc (human demonstration).
# Phases 12–15 append sections below.
# Run from dist/ (cd dist && ./demo/garvan-demo.sh) or qseries-demo/ (cd qseries-demo && bash garvan-demo.sh).
set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$DIR/qseries.exe"
[ -f "$BIN" ] || BIN="$DIR/qseries"
[ -f "$BIN" ] || BIN="$DIR/../qseries.exe"
[ -f "$BIN" ] || BIN="$DIR/../qseries"
[ -f "$BIN" ] || { echo "error: qseries not found"; exit 1; }

run() {
  for cmd in "$@"; do echo "qseries> $cmd"; done
  printf '%s\n' "$@" | "$BIN" 2>&1
}

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
echo ""
echo "--- Sifting: Rødseth (qseriesdoc §5) ---"
run "set_trunc(200)" "PD := etaq(2,200)/etaq(1,200)" "PD1 := sift(PD,5,1,199)" "etamake(PD1, 30)"

echo ""
echo "--- Triple product: Euler pentagonal (qseriesdoc §6.1) ---"
run "set_trunc(60)" "series(tripleprod(q,q^3,10), 60)"

echo ""
echo "--- Quintuple product and Euler dissection (qseriesdoc §6.2) ---"
run "set_trunc(500)" "EULER := etaq(1,500)" "E0 := sift(EULER,5,0,499)" "jp := jacprodmake(E0,50)" "jac2prod(jp)" "series(quinprod(q,q^5,20), 100)"

echo ""
echo "--- Winquist identity (qseriesdoc §6.3) ---"
run "set_trunc(200)" "series(winquist(q^5,q^3,q^11,20), 60)"
