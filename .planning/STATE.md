# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2025-02-24)

**Core value:** Rogers-Ramanujan must work — prodmake recovers product form with denominators at ±1 mod 5
**Current focus:** Milestone v1.1 (Garvan demo) — Phase 14 Relations demo

## Current Position

Phase: 14 (Relations demo)
Plan: 14-01
Status: Complete
Last activity: 2026-02-25 — Plan 14-01 executed

Progress: [██████████] 100%

## Performance Metrics

**Velocity:**
- Total plans completed: 8
- Average duration: ~15min
- Total execution time: ~55min

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 01-bigint | 3 | ~45min | ~15min |
| 02-frac | 2 | ~25min | ~12min |

**Recent Trend:** Phase 10 complete — REPL with findhom/findnonhomcombo, subs_q, tests/acceptance.sh; all 9 SPEC tests pass

## Accumulated Context

### Decisions

- Base 10⁹ for BigInt; prodmake divisor sum excludes d=n; jacprodmake 80% periodicity — see PROJECT.md
- Plan 01-01: std::invalid_argument for invalid string; canonical zero neg=false,d={0}; INT64_MIN handled in int64_t ctor
- Plan 01-02: std::invalid_argument for division by zero; binary-search quotient for divmod
- Plan 01-03: main.cpp as Phase 1 BigInt test driver; CHECK macro with PASS/FAIL; remainder sign = dividend sign
- Phase 2 (Frac): throw on Frac(n,0) and a/b when b zero; both standalone and integration tests; broader suite (SPEC + arithmetic + sign); must pass before Phase 3
- Plan 02-01: std::invalid_argument for zero denominator; den > 0, gcd(|num|,den)=1, 0/1 for zero
- Plan 02-02: main.cpp Frac test suite - SPEC reduction, arithmetic, sign, long-chain (add 50x, mul 20x)
- Phase 3 (Series): throw on invalid inverse; display "1 + q + 2q² + ... + O(q^T)"; subs_q(0) ok; broader suite (SPEC + ops); must pass before Phase 4
- Plan 10-01: EnvValue = variant<Series, vector<JacFactor>>; Call stub returns DisplayOnly; prompt "qseries> "; history 100
- Plan 10-02: q-injection when 1 fewer arg; jac2prod(var) looks up env; Rogers-Ramanujan verified via REPL
- Plan 10-03: findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly, subs_q; tests/acceptance.sh; all 9 SPEC tests pass
- Plan 12-01: series(f,T) honors T; O-term uses expToUnicode; formatProdmake mapleStyle (1-q), (-q^n+1); demo series(rr,50)
- [Phase 13-product-conversion-demo]: Plan 13-01: theta2(100) without /q^(1/4); rr redefined in jacprodmake block
- [Phase 14-relations-demo]: Plan 14-01: findhomcombo theta fallback (nested sum unsupported); Relations section with findhom, findhomcombo, findnonhomcombo

### Pending Todos

None yet.

### Blockers/Concerns

None yet.

## Session Continuity

Last session: 2026-02-25
Stopped at: Completed 14-01-PLAN.md (Relations run blocks)

### Milestone v1.1 Roadmap (2026-02-25)

- Phases 11–15 derived from DEMO-01 through DEMO-05
- Phase 11: Demo artifact (script/doc)
- Phase 12: Rogers-Ramanujan demo (§3.1)
- Phase 13: Product conversion demo (§3.2–3.4)
- Phase 14: Relations demo (§4)
- Phase 15: Sifting and product identities demo (§5, §6)
