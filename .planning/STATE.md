# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2025-02-24)

**Core value:** Rogers-Ramanujan must work — prodmake recovers product form with denominators at ±1 mod 5
**Current focus:** Milestone v10.0 (Close Remaining Gaps)

## Current Position

Phase: Not started (defining requirements)
Plan: —
Status: Defining requirements for milestone v10.0
Last activity: 2026-03-03 — v10.0 started: Block 25, Block 24, mod-p

Progress: [░░░░░░░░░░] 0% — MILESTONE v10.0 IN PROGRESS

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
| Phase 42-garvan-tutorial-coverage P01 | ~14 | 2 tasks | 2 files |
| Phase 42-garvan-tutorial-coverage P02 | ~10 | 2 tasks | 4 files |
| Phase 43-wasm-compile P01 | ~7 | 2 tasks | 3 files |
| Phase 43-wasm-compile P02 | ~1 | 1 task | 2 files |
| Phase 44-starlight-scaffold P01 | ~7 | 2 tasks | 24 files |
| Phase 45-doc-content P01 | ~5 | 3 tasks | 6 files |
| Phase 45-doc-content P02 | ~5 | 3 tasks | 7 files |
| Phase 45-doc-content P03 | ~6 | 2 tasks | 5 files |
| Phase 46-playground P01 | ~3 | 2 tasks | 8 files |
| Phase 47-cicd-deploy P01 | ~3 | 2 tasks | 2 files |
| Phase 48-cleanup P01 | ~3 | 2 tasks | 28 files |
| Phase 49-single-page-website P01 | ~10 | 2 tasks | 1 file |
| Phase 50-example-audit P01 | ~10 | 3 tasks | 2 files |
| Phase 51-ansi-color-clear P01 | ~7 | 2 tasks | 1 file |
| Phase 52-karatsuba P01 | ~4 | 1 task | 1 file |
| Phase 63-q-shift-arithmetic-fix P01 | ~7 | 2 tasks | 2 files |
| Phase 64-fractional-power-infrastructure P01 | ~10 | 5 tasks | 5 files |
| Phase 65-jacobi-half-integer-exponents P01 | ~12 | 2 tasks | 2 files |
| Phase 66-exercise-solutions-regression P01 | ~8 | 2 tasks | 1 file |
| Phase 88 P01 | 12 | 3 tasks | 2 files |
| Phase 90 P01 | 15 | 3 tasks | 2 files |
| Phase 90-symbolic-z-tripleprod-quinprod P02 | 10 | 2 tasks | 2 files |
| Phase 91-omega3-type P01 | ~15 | 2 tasks | 2 files |
| Phase 92-series-omega P01 | ~10 | 2 tasks | 2 files |
| Phase 93-parser-repl-omega P01 | 15 | 3 tasks | 4 files |

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
- [Phase 41-robustness-edge-cases]: Plan 41-02: div-by-zero guard in evalToInt and evalExpr, toSeries() safe variant access, levenshteinDistance() typo suggestions, int64_t assignment support; pow limit moved to REPL layer; acceptance-robustness.sh 9 tests pass
- [Phase 42-garvan-tutorial-coverage]: Plan 42-01: T_rn memoized via wrapper+helper with qbin_fast addition recurrence (no division); T(0,30,50) from intractable to ~2s; min/max integer builtins with variadic args
- [Phase 42-garvan-tutorial-coverage]: Plan 42-02: Version bump 1.9→2.0 across 7 locations in 4 files; MANUAL.md gains Integer Utilities section (min/max) and T_rn memoization note; Milestone v2.0 complete
- [Phase 43-wasm-compile]: Plan 43-01: Emscripten guards in repl.h (4 #ifndef blocks); main_wasm.cpp with Embind evaluate()/get_banner(); Makefile wasm target; emsdk not installed so Wasm build untested
- [Phase 43-wasm-compile]: Plan 43-02: test_wasm.mjs (16 assertions, 10 categories) and test_wasm.html (browser test page); checkpoint deferred until emsdk available
- [Phase 44-starlight-scaffold]: Plan 44-01: Astro Starlight site in website/ with manual sidebar (4 groups, 16 slugs), teal accent theme, 16 placeholder pages, Pagefind search; npm run build produces dist/ with 17 HTML files
- [Phase 45-doc-content]: Plan 45-01: KaTeX math pipeline (remark-math + rehype-katex + CSS fix); landing page with hero, 4 feature cards, RR KaTeX demo, Garvan attribution; installation page with build one-liner and platform notes; quick-start page with 6 REPL examples
- [Phase 45-doc-content]: Plan 45-02: 7 reference manual pages with typed signatures, KaTeX formulas, REPL examples for all 38+ built-ins; bigint-frac conceptual overview; q-functions grouped into 3 sections; standard function entry format (heading, math, syntax, params, example)
- [Phase 45-doc-content]: Plan 45-03: 5 tutorial pages (overview, Rogers-Ramanujan, theta/eta, partition identities, modular equations); real REPL output for all examples; KaTeX math-then-REPL layout; sift T parameter is input truncation
- [Phase 46-playground]: Plan 46-01: xterm.js playground at /playground/ with Worker-based Wasm evaluation; @xterm/xterm 5.5.0 + @xterm/addon-fit; Worker in public/wasm/ with importScripts; sequential example execution via queue; wasm-website Makefile target
- [Phase 47-cicd-deploy]: Plan 47-01: GitHub Actions single-job pipeline (checkout → emsdk 3.1.64 → wasm → wasm-website → Node 22 → npm ci → Astro → verify wasm → Cloudflare Pages); _headers for application/wasm + immutable caching
- [Phase 48-cleanup]: Plan 48-01: Deleted website/ (27 files, 9818 lines), .github/ (CI/CD), wasm-website Makefile target; core build and wasm target preserved
- [Phase 49-single-page-website]: Plan 49-01: Self-contained index.html (864 lines) with kangaroo-banner hero, 4 real REPL examples, 53-function reference; Eisenstein E4^2=E8 for relation-finding demo
- [Phase 50-example-audit]: Plan 50-01: Audited all examples across demo/manual/website; fixed version 1.3→2.0, version→version(), help→help(); all REPL examples verified against binary output
- [Phase 51-ansi-color-clear]: Plan 51-01: ansi namespace (gold/red/dim/bold/reset) gated on stdin_is_tty() && !NO_COLOR; prompt gold, errors red, timing dim; Ctrl+L and clear command; clear command also gated on TTY; zero ANSI in piped output
- [Phase 52-karatsuba]: Plan 52-01: Hybrid Karatsuba/schoolbook multiply; KARATSUBA_THRESHOLD=32 limbs; mulSchoolbook/addVec/subVec vector helpers; karatsubaMultiply recursive with 3-multiply decomposition; operator* dispatches automatically
- [Phase 62-maple-checklist]: Plan 62-02: jacprodmake fixed for general moduli (mod-11, mod-20); jac2prod display fix (num+den); Watson T=ξ+5ξ²+15ξ³+25ξ⁴+25ξ⁵; Slater(46)=parts≢0,±4,±6(mod20); 30/41 blocks pass; 13 exercises documented
- [Phase 63-q-shift-arithmetic-fix]: Plan 63-01: normalize_q_shift() absorbs integer part into coeff indices; q_shift in [0,1) after operator*/inverse/subs_q; expanded fractional display q^(n/d); Block 25 findpoly theta2/theta3 passes; 31/41 blocks pass
- [Phase 64-fractional-power-infrastructure]: Plan 64-01: bigpow/iroot in BigInt; Frac::rational_pow exact rational exponentiation; Series::powFrac via generalized binomial recurrence O(T²); REPL ^ fallback to powFrac; acceptance-powfrac.sh 6 tests; all 10 acceptance + 31/41 maple-checklist pass
- [Phase 65-jacobi-half-integer-exponents]: Plan 65-01: jacprodmake b/2 exponent halving for even-period products; jac2series_impl powFrac dispatch for fractional exponents; jac2prod ^(n/d) display; Blocks 13+14 pass; 33/41 maple-checklist pass
- [Phase 66-exercise-solutions-regression]: Plan 66-01: acceptance-exercises.sh for Exercises 4/9/10; full regression 56 tests pass, 0 fail; Milestone v4.2 complete
- [Phase 70-partition-type-infrastructure]: struct Partition{vector<int64_t>} in EvalResult+EnvValue; list literals preserve user order (no auto-sort); partitions(n) non-increasing; conjpart non-increasing; partitions(n) plural avoids collision with partition(n); session P-tag; empty list [] via parser fix; lp/np_parts/nep/ptnnorm statistics; 12 acceptance tests pass; 33/41 maple-checklist + 11 rank-crank pass
- [Phase 71-core-tcore-algorithms]: tcore.h with safe_mod, tcore_rvec, tcore_istcore, tcore_ptn2nvec, tcore_ptn2rvec, tcore_nvec2ptn (Frobenius coordinates), tcore_tcoreofptn, tcore_tcores; REPL dispatch for rvec/istcore/tcoreofptn/tcores/ptn2nvec/ptn2rvec; success criteria correction: tcores(3,4) has 2 cores not 1; 22 acceptance tests pass; all regressions pass
- [Phase 72-gsk-bijection]: tcore_tquot (beta-set decomposition), tcore_findhookinpos (bi-infinite word scan), tcore_addrimthook (rim hook tracing), tcore_invphi1 (inverse bijection via rim hook insertion); floor_div for negative numbers; Phi1Result struct in EvalResult+EnvValue; phi1 display (Core/Quotient); roundtrip passes for t=2,3,5,7; 12 acceptance tests pass; all regressions pass
- [Phase 73-vector-representations]: nvec2ptn REPL dispatch (was in tcore.h but not exposed); nvec2alphavec implemented in REPL dispatch using Frac (rational linear formulas from Garvan's tcore package); t=5 and t=7 only, error for other t; 12 acceptance tests pass; VEC-01..04 satisfied; ptn2nvec/ptn2rvec were already dispatched in Phase 71
- [Phase 74-tcrank-display]: mod_pow (modular exponentiation) + tcore_tcrank (GKS crank: h(t)=(t-(p-1)/2)^(p-3) mod p) in tcore.h; tresdiag (t-residue diagram: cell residue=(col-row) mod t) and makebiw (bi-infinite words via beta-number runner encoding) in REPL dispatch; makebiw output matches Maple exactly for [6,6,6,5,4,4,2,1,1] t=5 m=3; 12 acceptance tests pass; STAT-08/DISP-01/DISP-02 satisfied
- [Phase 87-integration-testing] Plan 87-01: acceptance-all Makefile target (maple-checklist, run-all, modforms, theta-ids); BIN fallback in integration-tcore.sh and acceptance-bailey.sh for dist/qseries.exe
- [Phase 75-integration-testing]: tests/integration-tcore.sh with 86 tests covering: Maple makebiw example match (5 words), phi1/invphi1 roundtrip for 10 partitions × 3 values of t (=30 roundtrips), size identity for 4 partitions × 3 t values (=12), cross-function integration (istcore∘tcoreofptn, nvec roundtrip, tcoreofptn∘nvec path), tcrank values, tcores, nvec2alphavec, conjpart involution, partitions count, 83 help entries verified, 20 individual help(func) checks; total 200 tests across 8 suites, 0 failures; MILESTONE v5.0 COMPLETE
- [Phase 87]: Plan 87-02: Cross-package integration script (ETA+theta+modforms); theta3(100) not theta3 in etamake
- [Phase 87-integration-testing]: Plan 87-03: BIN fallback chain in 9 run-all scripts (dist/qseries.exe first)
- [Phase 88]: prodid/seriesid use Expr::Tag::Var branch before evi(2) to avoid evaluating z,q
- [Phase 90]: BivariateSeries in series.h; formatBivariate, tripleprod_symbolic, quinprod_symbolic in qfuncs.h; identity formulas for symbolic z path
- [Phase 90]: Plan 90-02: DisplayOnly for symbolic z path; assignment to BivariateSeries deferred
- [Phase 94-bq-block10]: bq/aq/cq builtins deferred; Phase 94 success criteria do not require them
- [Phase 95-factor-builtin]: factor(expr) built-in via qfactor→cyclotomic expansion; FactorResult with Φ_n notation; Block 4 parity ready
- [Phase 97]: addAligned in series.h centralizes q-shift alignment; operator+ uses it when q_shifts differ; rr_ids.h delegates to it

### Roadmap Evolution

- Milestone v8.0 started: RootOf Function — Phase 91 (Omega3 type), 92 (SeriesOmega), 93 (Parser/REPL), 94 (b(q) and Block 10)
- [Phase 91-omega3-type] Plan 91-01: Omega3 struct { Frac a, b } for a+bω (ω²=-ω-1); omega(), omega2(), pow, str; 11 unit tests; str() special-case for omega, omega^2
- [Phase 92-series-omega] Plan 92-01: SeriesOmega (map<int,Omega3>, trunc); add, mul, truncTo, Omega3*Series; 5 Phase 92 unit tests
- [Phase 93-parser-repl-omega] Plan 93-01: omega, RootOf(3), RootOf([1,1,1]), sum(omega^n)=0, assignment, omega^2; acceptance-omega.sh 6 tests
- [Phase 94-bq-block10] Plan 94-01: Sum(SeriesOmega), toRationalSeries, b(q) double-sum, Block 10 real test, EX-04c/d/e; MILESTONE v8.0 COMPLETE
- Phases 91–94 derived from ROOT-01..ROOT-14; 14/14 requirements mapped ✓
- Milestone v9.0 started: factor(t8) + Close Block 4 — Phase 95 (factor builtin), 96 (Block 4 parity); FACTOR-01..03, BLOCK4-01; research SUMMARY-factor.md
- Milestone v4.2 started: Fix Block Failures
- Phase 63 added: Q-Shift Arithmetic Fix (QSHIFT-01, QSHIFT-02)
- Phase 64 added: Fractional Power Infrastructure (JAC-02)
- Phase 65 added: Jacobi Half-Integer Exponents (JAC-01, JAC-03, JAC-04)
- Phase 66 added: Exercise Solutions & Regression (EX-01, EX-02, EX-03, REG-04)
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
- v2.0 committed and tagged
- Milestone v2.1 started: Website (docs site + Wasm playground)
- Phase 43 added: Wasm compile (WASM-01, WASM-02, WASM-03)
- Phase 44 added: Astro Starlight scaffold (SITE-01, SITE-05)
- Phase 45 added: Documentation content (SITE-02, SITE-03, SITE-04)
- Phase 46 added: Playground (PG-01, PG-02, PG-03, WASM-04)
- Phase 47 added: CI/CD & deployment (DEPLOY-01, DEPLOY-02)

- Milestone v3.0 started: Simple Website & Example Audit
- Phase 48 added: Cleanup (CLEAN-01, CLEAN-02, CLEAN-03)
- Phase 49 added: Single-page website (PAGE-01 through PAGE-05)
- Phase 50 added: Example audit (AUDIT-01 through AUDIT-04)
- Milestone v4.0 started: Core Improvements
- Phase 51 added: ANSI Color + Clear Screen (UX-01, UX-02, REG-02)
- Phase 52 added: Karatsuba Multiplication (PERF-05)
- Phase 53 added: Series Optimization (PERF-04)
- Phase 54 added: Benchmarking Suite (PERF-06)
- Phase 55 added: Smart Tab Completion (UX-04, UX-05)
- Phase 56 added: Session Save/Load + History Persistence (UX-03, UX-06, REG-01)
- Milestone v4.1 started: Distribution
- Phase 57 added: CI/CD Release Pipeline (DIST-07, DIST-08)
- Phase 58 added: Docker (DIST-01, DIST-02, DIST-03, DIST-09, REG-03)
- Phase 59 added: Install Script (DIST-04, DIST-05, DIST-06)
- Phase 60 added: etamake scalar factor and non-integer q exponents
- Phase 61 added: fix integer exponents only gap
- Phase 62 added: maple checklist satisfied

### Pending Todos

None yet.

### Blockers/Concerns

None yet.

## Session Continuity

Last session: [now]
Stopped at: Completed 94-01-PLAN.md; omega/RootOf REPL integration

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
