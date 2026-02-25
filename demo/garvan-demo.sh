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
# (Phases 12–15 append content here)

# === Relations: findhom, findhomcombo, findnonhomcombo (qseriesdoc §4) ===

# === Sifting and product identities (qseriesdoc §5, §6) ===
