# 10-03 Plan Summary

**Plan:** 10-03  
**Phase:** 10-repl  
**Status:** Complete

## What Was Built

1. **List eval and relation built-ins** — Expr::List evals each element to Series, collects into vector<Series>. findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly dispatch to relations.h. legendre/sigma return int64_t.

2. **subs_q built-in** — subs_q(f, k) evals f as Series, returns f.subs_q(k). Enables theta3(subs_q(q,2),100) for findhom Test 5 (Gauss AGM).

3. **Acceptance test script** — tests/acceptance.sh pipes REPL commands into single qseries sessions per multi-step test. All 9 SPEC tests pass.

## Key Files Created/Modified

- src/repl.h — findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly, subs_q, legendre, sigma; list eval
- tests/acceptance.sh — 9 SPEC acceptance tests with printf '%s\n' ... | qseries

## Verification

- All 9 SPEC acceptance tests pass via bash tests/acceptance.sh
- Test 5 (Gauss AGM): findhom with theta3(subs_q(q,2),100)
- Test 8 (Watson): findnonhomcombo(T^2, [T, xi], [1, 7], 0)
