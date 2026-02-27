# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2025-02-24)

**Core value:** Rogers-Ramanujan must work — prodmake recovers product form with denominators at ±1 mod 5
**Current focus:** Milestone v2.0 (Robustness & Tutorial Coverage)

## Current Position

Phase: 41-robustness-edge-cases
Plan: 02 -- READY
Status: Plan 01 complete; Plan 02 ready for execution
Last activity: 2026-02-27 -- 41-01 executed (2 tasks, 4 files)

Progress: [█████░░░░░] 50%

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
| Phase 28-arrow-key-line-navigation P01 | 20 | 3 tasks | 3 files |
| Phase 29-optional-arg-audit P01 | ~12 | 4 tasks | 3 files |

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
- [Phase 16-script-mode]: Plan 16-01: Banner suppressed when stdin not TTY; wrap banner in if(stdin_is_tty()) in runRepl()
- [Phase 17-help-help-func]: Plan 17-01: help and help(func) built-in with getHelpTable(), 28 built-ins
- [Phase 18-timing]: Plan 18-01: Elapsed time after output on next line (X.XXXs), interactive only
- [Phase 19-multi-line-input]: Plan 19-01: Backslash continuation in runRepl(); prompt "  > "; max 100 lines; script mode echo
- [Phase 20-tab-completion]: Plan 20-01: Raw terminal (termios/SetConsoleMode); readLineRaw; Tab completion (getHelpTable+env); unique replace, multiple list; script mode getline unchanged
- [Phase 21-error-messages]: Plan 21-01: Token.offset, offsetToLineCol, kindToExpected; Tokenizer and Parser throw sites include "parser: line L, col C: message"
- [Phase 21-error-messages]: Plan 21-02: runtimeErr + ev/evi wrappers; inputLineNum in runRepl; script mode "error: line N: message"
- [Phase 22-demo-packaging]: Plan 22-01: package-demo copies binary, garvan-demo.sh, README-dist.md; DIR-based BIN resolution for flat and nested layouts; QOL-08 satisfied
- [Phase 23-quick-wins]: Plan 23-01: acceptance-wins.sh for version, qdegree, lqdegree, jac2series, findlincombo; Makefile acceptance-wins target; MANUAL.md updates; implementation was pre-existing
- [Phase 27-suppress-output-semicolon]: Plan 27-01: Trailing colon suppresses output; REPL-layer only; strip before parse; display and timing skipped; acceptance-suppress-output.sh
- [Phase 28-arrow-key-line-navigation]: Plan 28-01: readLineRaw cursor pos, redrawLineRaw, ESC left/right, handleTabCompletion(line,pos,env); acceptance-arrow-keys.sh via script/PTY
- [Phase 29-optional-arg-audit]: Plan 29-01: etaq(k), checkprod(f), checkmult(f), findmaxind(L) 1-arg forms; acceptance-optional-args.sh; getHelpTable updates
- [Phase 41-robustness-edge-cases]: Plan 41-01: etaq k>0, sift n>0, pow ±10000 limit, parser depth 256 limit, INT64_MAX overflow check; all 10 acceptance tests pass

### Roadmap Evolution

- Phase 30 added: Output on next line after input
- Phase 31 added: Up-down arrows for history
- Milestone v1.6 started: etaq Performance
- Phase 32 added: etaq memoization (PERF-01)
- Phase 33 added: Euler pentagonal optimization (PERF-02)
- Milestone v1.7 started: Polish & Documentation
- Phase 34 added: Fix compiler warnings (POLISH-01, POLISH-02)
- Phase 35 added: User manual (DOC-01, DOC-02)
- Milestone v1.8 started: More Math Functions
- Phase 36 added: Expose NT helpers (MATH-01, MATH-02, MATH-03)
- Phase 37 added: Convenience functions (MATH-04 through MATH-07)
- Phase 38 added: Math enrichment (MATH-08, MATH-09, MATH-10)
- Milestone v1.9 started: Manual Update & Testing
- Phase 39 added: Update MANUAL.md (DOC-03, DOC-04)
- Phase 40 added: Acceptance tests for v1.8 (TEST-01, TEST-02)
- v1.9 committed and tagged
- Milestone v2.0 started: Robustness & Tutorial Coverage
- Phase 41 added: Robustness & edge cases (ROBUST-01 through ROBUST-06)
- Phase 42 added: Garvan tutorial coverage (TUTORIAL-01, TUTORIAL-02)

### Pending Todos

None yet.

### Blockers/Concerns

None yet.

## Session Continuity

Last session: 2026-02-27
Stopped at: Completed 41-01-PLAN.md (robustness input validation guards)

### Milestone v1.1 Roadmap (2026-02-25)

- Phases 11–15 derived from DEMO-01 through DEMO-05

### Milestone v1.2 Roadmap (2026-02-25)

- Phases 16–22 derived from QOL-01 through QOL-08

### Milestone v1.3 Roadmap (2026-02-25)

- Phase 23: Quick wins — version, qdegree, lqdegree, jac2series, findlincombo

### Milestone v1.4 Roadmap (2026-02-25)

- Phases 24–26 derived from GAP-01 through GAP-04 (mprodmake, checkprod, checkmult, findmaxind)

### Milestone v1.5 Roadmap (2026-02-26)

- Phases 27–29 derived from REPL-OPTS-01 through REPL-OPTS-03
- Phase 27: Suppress output (semicolon)
- Phase 28: Arrow-key line navigation
- Phase 29: Optional-arg audit (series, etaq, qfactor, jac2series, checkprod, checkmult, findmaxind)
- Phase 16: Script mode — qseries < script.qs
- Phase 17: help + help(func)
- Phase 18: Timing
- Phase 19: Multi-line input
- Phase 20: Tab completion
- Phase 21: Error messages
- Phase 22: Demo packaging
