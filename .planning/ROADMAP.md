# Roadmap: Q-Series REPL

## Overview

Build a zero-dependency C++20 REPL for q-series computation bottom-up: BigInt → Frac → Series → qfuncs → prodmake (canary) → convert-extended → linalg → relations → parser → repl. Rogers-Ramanujan (prodmake on sum q^(n²)/(q;q)_n yields denominators at ±1 mod 5) is the central acceptance gate. All 9 SPEC acceptance tests must pass when the REPL is complete.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

- [x] **Phase 1: BigInt** - Arbitrary precision integers, base 10⁹, signed (completed 2026-02-25)
- [x] **Phase 2: Frac** - Exact rationals with auto-reduce via GCD (completed 2026-02-25)
- [x] **Phase 3: Series** - Truncated power series with inverse, truncation propagation (completed 2026-02-25)
- [x] **Phase 4: qfuncs** - aqprod, etaq, theta, qbin, products; number theory helpers (completed 2025-02-24)
- [x] **Phase 5: prodmake** - Andrews' algorithm; Rogers-Ramanujan canary (completed 2025-02-24)
- [x] **Phase 6: convert-extended** - etamake, jacprodmake, qfactor, sift (completed 2025-02-24)
- [x] **Phase 7: linalg** - Gaussian elimination over Q, kernel computation (completed 2025-02-24)
- [x] **Phase 8: relations** - findhom, findnonhom, findpoly family (completed 2025-02-25)
- [x] **Phase 9: parser** - Tokenizer + recursive-descent expression parser (completed 2025-02-25)
- [x] **Phase 10: repl** - REPL loop, variables, dispatch, display; all 9 SPEC tests pass (completed 2026-02-25)

**Milestone v1.1 (Garvan Demo) — phases 11–15:**

- [x] **Phase 11: Demo artifact** - Script or doc that runs qseries commands (completed 2026-02-26)
- [x] **Phase 12: Rogers-Ramanujan demo** - sum + prodmake, qseriesdoc §3.1 style (completed 2026-02-25)
- [x] **Phase 13: Product conversion demo** - qfactor, etamake, jacprodmake (§3.2–3.4) (completed 2026-02-25)
- [x] **Phase 14: Relations demo** - findhom, findhomcombo, findnonhomcombo (§4) (completed 2026-02-25)
- [x] **Phase 15: Sifting and product identities demo** - sift, triple/quin/winquist (§5, §6) (completed 2026-02-25)

**Milestone v1.2 (More QoL) — phases 16–22:**

- [x] **Phase 16: Script mode** - Run qseries < script.qs non-interactively (completed 2026-02-26)
- [x] **Phase 17: help + help(func)** - General help and per-function documentation (completed 2026-02-26)
- [x] **Phase 18: Timing** - Elapsed time per command (ms or s) (completed 2026-02-26)
- [x] **Phase 19: Multi-line input** - Backslash continuation for long expressions (completed 2026-02-24)
- [x] **Phase 20: Tab completion** - Autocomplete identifiers and built-in names (completed 2026-02-26)
- [x] **Phase 21: Error messages** - Clearer parse/runtime diagnostics (completed 2026-02-26)
- [x] **Phase 22: Demo packaging** - Single folder (binary + demo + README) for distribution (completed 2026-02-26)

**Milestone v1.3 (Quick wins) — phase 23:**

- [x] **Phase 23: Quick wins** - version, qdegree, lqdegree, jac2series, findlincombo, Makefile CXX default (completed 2026-02-25)

**Milestone v1.4 (Close more gaps) — phases 24–26:**

- [x] **Phase 24: mprodmake** - Convert q-series to product (1+q^n1)*(1+q^n2)*... (completed 2026-02-26)
- [x] **Phase 25: checkprod and checkmult** - Validation utilities (completed 2026-02-26)
- [x] **Phase 26: findmaxind** - Maximal independent subset of q-series (completed 2026-02-26)

**Milestone v1.5 (REPL improvements) — phases 27–29:**

- [x] **Phase 27: Suppress output (semicolon)** - Trailing semicolon prevents printing result (completed 2026-02-26)
- [x] **Phase 28: Arrow-key line navigation** - Left/right keys move cursor within input (TTY) (completed 2026-02-26)
- [x] **Phase 29: Optional-arg audit** - 2/3-arg variants work; omitted args use defaults (completed 2026-02-26)

## Phase Details

### Phase 1: BigInt
**Goal**: Arbitrary-precision integers available for exact arithmetic
**Depends on**: Nothing (first phase)
**Requirements**: CORE-01, CORE-02
**Success Criteria** (what must be TRUE):
  1. BigInt supports +, -, *, divmod with correct results for large operands
  2. Long division uses binary-search quotient; remainder invariant holds
  3. GCD computes correctly; edge cases (0, negatives, base boundaries) handled
**Plans**: 3 plans

Plans:
- [ ] 01-01-PLAN.md — BigInt core: struct, constructors, +, -, *, comparisons, str
- [ ] 01-02-PLAN.md — divmod (binary-search quotient), /, %, bigGcd
- [ ] 01-03-PLAN.md — main.cpp test driver with SPEC edge cases

### Phase 2: Frac
**Goal**: Exact rational arithmetic with no coefficient growth
**Depends on**: Phase 1
**Requirements**: CORE-03
**Success Criteria** (what must be TRUE):
  1. 6/4 reduces to 3/2; 0/5 reduces to 0/1
  2. reduce() runs after every construction and operation
  3. Long chains of Frac ops produce no exponential BigInt growth
**Plans**: 2 plans

Plans:
- [ ] 02-01-PLAN.md — frac.h: Frac struct, reduce, arithmetic, comparisons, str
- [ ] 02-02-PLAN.md — main.cpp Frac test suite (SPEC + add/sub/mul/div + sign + long-chain)

### Phase 3: Series
**Goal**: Truncated power series with correct arithmetic and inversion
**Depends on**: Phase 2
**Requirements**: CORE-04, CORE-05
**Success Criteria** (what must be TRUE):
  1. (1-q) * (1/(1-q)) = 1 to truncation
  2. Inverse recurrence uses j=1..n (not j=0); invert q*(1-q) correctly
  3. Truncation propagates in +, -, *, /, pow, subs_q; multiplication skips pairs where e1+e2 >= trunc
**Plans**: 2 plans

Plans:
- [x] 03-01-PLAN.md — series.h: struct, constructors, arithmetic, inverse, subs_q, str
- [x] 03-02-PLAN.md — main.cpp Series test suite (SPEC inverse + arithmetic + subs_q + pow)

### Phase 4: qfuncs
**Goal**: q-series building blocks and number theory helpers
**Depends on**: Phase 3
**Requirements**: CORE-06, QFUNCS-01, QFUNCS-02, QFUNCS-03, QFUNCS-04, QFUNCS-05, QFUNCS-06, TEST-02
**Success Criteria** (what must be TRUE):
  1. aqprod, qbin, etaq produce correct coefficients (etaq(1,T) matches Euler pentagonal)
  2. theta2, theta3, theta4, tripleprod, quinprod, winquist work per SPEC
  3. divisors, mobius, legendre, sigma, euler_phi return correct values
  4. Partition function 1/etaq(1,50) coefficients match SPEC (TEST-02)
**Plans**: 4 plans

Plans:
- [ ] 04-01-PLAN.md — qfuncs.h: nthelpers, etaq, aqprod, qbin
- [ ] 04-02-PLAN.md — theta2, theta3, theta4, theta
- [x] 04-03-PLAN.md — tripleprod, quinprod, winquist
- [x] 04-04-PLAN.md — main.cpp qfuncs test suite (pentagonal, partition TEST-02)

### Phase 5: prodmake
**Goal**: Andrews' algorithm recovers infinite product from series; Rogers-Ramanujan works
**Depends on**: Phase 4
**Requirements**: CONVERT-01, TEST-01
**Success Criteria** (what must be TRUE):
  1. prodmake on Σ q^(n²)/(q;q)_n yields product with denominators only at exponents ≡ ±1 (mod 5)
  2. Divisor sum in step 3 excludes d=n; recurrence step 2 correct
  3. Rogers-Ramanujan is the canary — if this passes, core math is correct
**Plans**: 1 plan

Plans:
- [x] 05-01-PLAN.md — convert.h prodmake + Rogers-Ramanujan TEST-01

### Phase 6: convert-extended
**Goal**: Product identification and coefficient extraction
**Depends on**: Phase 5
**Requirements**: CONVERT-02, CONVERT-03, CONVERT-04, CONVERT-05, TEST-03, TEST-04, TEST-06, TEST-07, TEST-09
**Success Criteria** (what must be TRUE):
  1. etamake identifies theta3, theta4 as eta products (TEST-03)
  2. jacprodmake on Rogers-Ramanujan yields JAC(0,5,∞)/JAC(1,5,∞) (TEST-04)
  3. sift extracts coefficients a_{ni+k}; Rødseth identity holds (TEST-06)
  4. qfactor factorizes T(8,8) correctly (TEST-07)
  5. jacprodmake on sifted etaq(1) yields Euler pentagonal dissection (TEST-09)
**Plans**: 3 plans (all complete)

Plans:
- [x] 06-01-PLAN.md — sift + etamake (TEST-03, TEST-06)
- [x] 06-02-PLAN.md — jacprodmake + jac2prod + jac2series (TEST-04, TEST-09)
- [x] 06-03-PLAN.md — qfactor + T(r,n) (TEST-07)

### Phase 7: linalg
**Goal**: Linear algebra over Q for relation finding
**Depends on**: Phase 2 (Frac)
**Requirements**: LINALG-01, LINALG-02
**Success Criteria** (what must be TRUE):
  1. Gaussian elimination over vector<vector<Frac>> returns correct reduced form
  2. Kernel computation returns basis for null space of rational matrices
**Plans**: 1 plan

Plans:
- [ ] 07-01-PLAN.md — linalg.h kernel(M) + main.cpp tests (2×3, full rank, all-zero, single row)

### Phase 8: relations
**Goal**: Polynomial relation discovery between series
**Depends on**: Phase 7, Phase 6 (needs Series and convert for theta)
**Requirements**: REL-01, REL-02, REL-03, REL-04, TEST-05, TEST-08
**Success Criteria** (what must be TRUE):
  1. findhom yields X₁²+X₂²-2X₃², -X₁X₂+X₄² for Gauss AGM (TEST-05)
  2. findnonhom, findhomcombo, findnonhomcombo, findpoly work per SPEC
  3. Watson's modular equation recovered via findnonhomcombo (TEST-08)
**Plans**: 2 plans

Plans:
- [x] 08-01-PLAN.md — findhom, findnonhom, findpoly + TEST-05 (Gauss AGM)
- [x] 08-02-PLAN.md — solve in linalg; findhomcombo, findnonhomcombo + TEST-08 (Watson)

### Phase 9: parser
**Goal**: Tokenizer + recursive-descent expression parser
**Depends on**: Nothing (standalone; produces AST)
**Requirements**: REPL-01
**Success Criteria** (what must be TRUE):
  1. Tokenizer handles identifiers, numbers, operators, := assignment, # comments
  2. Recursive-descent parser produces AST for expressions, assignments, function calls
  3. sum/add summation constructs and list literals parse correctly
**Plans**: 1 plan

Plans:
- [x] 09-01-PLAN.md — Tokenizer + parser + AST; sum/add, list literals; main.cpp tests

### Phase 10: repl
**Goal**: Full REPL with variable environment; all 9 SPEC acceptance tests pass
**Depends on**: Phases 4, 5, 6, 7, 8, 9
**Requirements**: REPL-02, REPL-03, REPL-04, REPL-05, REPL-06
**Success Criteria** (what must be TRUE):
  1. User can assign variables, evaluate expressions, invoke built-ins (aqprod, etaq, prodmake, etc.)
  2. sum/add dispatches correctly; set_trunc and series/coeffs display work
  3. REPL loop runs with basic history
  4. All 9 SPEC acceptance tests pass when run through the REPL
**Plans**: 3 plans

Plans:
- [x] 10-01-PLAN.md — repl.h foundation (env, eval, display, runRepl loop)
- [x] 10-02-PLAN.md — Built-in dispatch (qfuncs, convert, sum/add)
- [x] 10-03-PLAN.md — Relation built-ins, subs_q, 9 SPEC acceptance tests

### Phase 11: Demo artifact
**Goal**: Runnable demo artifact exists that can execute qseries commands
**Depends on**: Phase 10 (REPL)
**Requirements**: DEMO-01
**Success Criteria** (what must be TRUE):
  1. A script or doc file exists (e.g., demo.qs or DEMO.md with runnable commands)
  2. User can pipe commands to qseries binary (or run interactively) and get output
  3. Artifact structure supports appending subsequent Garvan examples
**Plans**: 1 plan

Plans:
- [ ] 11-01-PLAN.md — demo/garvan-demo.sh + Makefile demo target + README

### Phase 12: Rogers-Ramanujan demo
**Goal**: Rogers-Ramanujan example reproduces qseriesdoc §3.1 Output (1) and (2)
**Depends on**: Phase 11
**Requirements**: DEMO-02
**Success Criteria** (what must be TRUE):
  1. Sum Σ q^(n²)/(q;q)_n is computed and displayed in demo artifact
  2. prodmake on that sum yields infinite product form
  3. Output format matches Output (1) and (2) style from qseriesdoc §3.1
**Plans**: 1 plan

Plans:
- [ ] 12-01-PLAN.md — REPL format fixes (series honors T, O-term Unicode, Maple factor) + demo script update

### Phase 13: Product conversion demo
**Goal**: Product conversion examples reproduce qseriesdoc §3.2, §3.3, §3.4
**Depends on**: Phase 12
**Requirements**: DEMO-03
**Success Criteria** (what must be TRUE):
  1. qfactor T(8,8) produces expected factorization in demo
  2. etamake on theta2, theta3, theta4 identifies eta products
  3. jacprodmake on Rogers-Ramanujan yields Jacobi product form (JAC notation)
**Plans**: 1 plan

Plans:
- [ ] 13-01-PLAN.md — Replace Product conversion placeholder with three run blocks (qfactor, etamake, jacprodmake)

### Phase 14: Relations demo
**Goal**: Relation-finding examples reproduce qseriesdoc §4
**Depends on**: Phase 13
**Requirements**: DEMO-04
**Success Criteria** (what must be TRUE):
  1. findhom yields Gauss AGM relations (X₁²+X₂²-2X₃², -X₁X₂+X₄²) in demo
  2. findhomcombo example runs and produces expected polynomial
  3. findnonhomcombo example runs and produces expected output
**Plans**: 1 plan

Plans:
- [ ] 14-01-PLAN.md — Replace Relations placeholder with three run blocks (findhom, findhomcombo, findnonhomcombo)

### Phase 15: Sifting and product identities demo
**Goal**: Sifting and product identity examples reproduce qseriesdoc §5, §6
**Depends on**: Phase 14
**Requirements**: DEMO-05
**Success Criteria** (what must be TRUE):
  1. sift example runs and extracts coefficients as in qseriesdoc §5
  2. triple product identity demonstrated
  3. quintuple product and Winquist identity demonstrated
**Plans**: 1 plan

Plans:
- [x] 15-01-PLAN.md — Add four run blocks (sift+etamake, tripleprod, quinprod+Euler, winquist)

### Phase 16: Script mode
**Goal**: User can run qseries non-interactively from a file
**Depends on**: Phase 10 (REPL)
**Requirements**: QOL-01
**Success Criteria** (what must be TRUE):
  1. User can run `qseries < script.qs` and see output from each command
  2. Commands are executed sequentially; process exits when file ends
  3. Script mode is detected when stdin is not a TTY (no interactive prompt)
**Plans**: 1 plan

Plans:
- [x] 16-01-PLAN.md — Guard banner with stdin_is_tty(); suppress in script mode

### Phase 17: help + help(func)
**Goal**: User can access built-in documentation
**Depends on**: Phase 10 (REPL)
**Requirements**: QOL-02, QOL-03
**Success Criteria** (what must be TRUE):
  1. User can type `help` and see general usage and list of built-ins
  2. User can type `help(etaq)` or `help(prodmake)` and see per-function documentation
  3. `help(unknown)` gives helpful feedback (e.g., "unknown function")
**Plans**: 1 plan

Plans:
- [x] 17-01-PLAN.md — Add help table and help/help(func) built-in in dispatchBuiltin

### Phase 18: Timing
**Goal**: User sees elapsed time for each command
**Depends on**: Phase 10 (REPL)
**Requirements**: QOL-04
**Success Criteria** (what must be TRUE):
  1. After each command evaluation, elapsed time is shown (ms or s)
  2. Time includes parse + eval (full evaluation)
  3. Format is readable (e.g., "0.123s" or "42ms")
**Plans**: 1 plan

Plans:
- [x] 18-01-PLAN.md — Add timing to runRepl (steady_clock, next line, interactive only)

### Phase 19: Multi-line input
**Goal**: User can split long expressions across lines
**Depends on**: Phase 10 (REPL)
**Requirements**: QOL-05
**Success Criteria** (what must be TRUE):
  1. User can end a line with backslash to continue on next line
  2. REPL prompts for continuation until expression is complete
  3. Backslash-newline treated as whitespace; expression parses as single unit
**Plans**: 1 plan

Plans:
- [x] 19-01-PLAN.md — Backslash continuation loop in runRepl; prompt "  > "; max 100 lines

### Phase 20: Tab completion
**Goal**: User can autocomplete identifiers and function names
**Depends on**: Phase 10 (REPL)
**Requirements**: QOL-06
**Success Criteria** (what must be TRUE):
  1. User can press Tab to complete partial identifier (variable or function)
  2. Unique match: completes; multiple matches: shows options; no match: no change
  3. Completion works for built-ins and user-defined variables
**Plans**: 1 plan

Plans:
- [x] 20-01-PLAN.md — Raw readline + Tab completion (TTY); script mode getline unchanged

### Phase 21: Error messages
**Goal**: Parse and runtime errors show clearer diagnostics
**Depends on**: Phase 9 (parser), Phase 10 (REPL)
**Requirements**: QOL-07
**Success Criteria** (what must be TRUE):
  1. Parse errors show line/column (or offset) and expected token
  2. Runtime errors show function name and brief context
  3. Error messages are actionable (user can locate and fix the problem)
**Plans**: 2 plans

Plans:
- [ ] 21-01-PLAN.md — Parse error diagnostics: Token offset, offsetToLineCol, position in all parser throws
- [ ] 21-02-PLAN.md — Runtime errors: runtimeErr helper, ev/evi wrap, REPL format with script line number

### Phase 22: Demo packaging
**Goal**: Single distributable folder for sharing
**Depends on**: Phase 15 (demo content)
**Requirements**: QOL-08
**Success Criteria** (what must be TRUE):
  1. User can obtain a folder containing: qseries binary, demo script, README
  2. Folder is self-contained (runnable without external dependencies)
  3. README explains how to run the demo
**Plans**: 1 plan

Plans:
- [ ] 22-01-PLAN.md — Extend package-demo: DIR-based script, README-dist.md, Makefile copies all three into qseries-demo/

### Phase 23: Quick wins
**Goal**: Add Maple qseries gap-closure features (version, qdegree, lqdegree, jac2series, findlincombo) and improve build default
**Depends on**: Phase 22
**Requirements**: (from FEATURE-GAPS.md quick wins)
**Success Criteria** (what must be TRUE):
  1. User can run `qseries --version` or type `version` in REPL to get version
  2. User can call `qdegree(f)` and `lqdegree(f)` for series degree inspection
  3. User can call `jac2series(var)` or `jac2series(var,T)` to convert Jacobi product to series
  4. User can call `findlincombo(f,L,topshift)` to express f as linear combination of L
  5. `make` uses g++ by default (CXX ?= g++); MinGW override via CXX=...
**Plans**: 1 plan

Plans:
- [x] 23-01-PLAN.md — version, qdegree, lqdegree, jac2series, findlincombo, Makefile CXX; acceptance-wins.sh

### Phase 24: mprodmake
**Goal**: Convert q-series to product of form (1+q^n1)*(1+q^n2)*...
**Depends on**: Phase 23
**Requirements**: GAP-01
**Success Criteria** (what must be TRUE):
  1. User can call `mprodmake(f,T)` to attempt conversion of f to m-product form
  2. Output is product of factors (1+q^j) or indicates failure
**Plans**: 1 plan

Plans:
- [x] 24-01-PLAN.md — mprodmake in convert.h; formatMprodmake; REPL dispatch; help; acceptance test

### Phase 25: checkprod and checkmult
**Goal**: Validation utilities for q-series
**Depends on**: Phase 24
**Requirements**: GAP-02, GAP-03
**Success Criteria** (what must be TRUE):
  1. User can call `checkprod(f,T)` to check if f is a "nice" product
  2. User can call `checkmult(f,T)` to check if coefficients are multiplicative
**Plans**: 1 plan

Plans:
- [x] 25-01-PLAN.md — checkprod, checkmult in convert.h or relations.h; REPL dispatch; help table

### Phase 26: findmaxind
**Goal**: Find maximal independent subset of q-series
**Depends on**: Phase 8 (relations), Phase 7 (linalg)
**Requirements**: GAP-04
**Success Criteria** (what must be TRUE):
  1. User can call `findmaxind(L, topshift)` to get maximal independent subset and 1-based indices
**Plans**: 1 plan

Plans:
- [x] 26-01-PLAN.md — findmaxind in relations.h (RREF on M^T, pivot cols = indices); REPL dispatch; help; acceptance test

### Phase 27: Suppress output (semicolon)
**Goal**: User can suppress result printing by ending a statement with semicolon
**Depends on**: Phase 10 (REPL)
**Requirements**: REPL-OPTS-01
**Success Criteria** (what must be TRUE):
  1. User can type `rr := sum(...);` and the result is stored but not printed
  2. Without semicolon, result is printed as before
  3. Semicolon works for assignments and expression statements
  4. Semicolon does not cause parse errors; parser treats it as optional statement terminator
**Plans**: 1 plan

Plans:
- [ ] 27-01-PLAN.md — REPL: trailing colon detection; skip display and timing when suppress_output

### Phase 28: Arrow-key line navigation
**Goal**: User can move cursor left/right within input line using arrow keys
**Depends on**: Phase 10 (REPL)
**Requirements**: REPL-OPTS-02
**Success Criteria** (what must be TRUE):
  1. When stdin is TTY, left/right arrow keys move cursor within the current input line
  2. Cursor position is correct; editing (typing, backspace) works at cursor
  3. When stdin is not TTY (script mode), input behavior unchanged (no arrow handling needed)
  4. History navigation (up/down) and line editing remain functional together
**Plans**: 1 plan

Plans:
- [ ] 28-01-PLAN.md — readLineRaw: pos, ESC [ D/C, redrawLine, cursor-aware backspace/typing/Tab; acceptance-arrow-keys

### Phase 29: Optional-arg audit
**Goal**: All 2-arg and 3-arg function variants work with omitted optional args
**Depends on**: Phase 10 (REPL)
**Requirements**: REPL-OPTS-03
**Success Criteria** (what must be TRUE):
  1. series(f) and series(f,T) work; omitted T uses env.T
  2. etaq(k) and etaq(k,T) work; omitted T uses env.T
  3. qfactor(f), qfactor(f,T), jac2series(var), jac2series(var,T) work with optional T
  4. checkprod(f), checkprod(f,T), checkmult(f), checkmult(f,T) work; omitted T uses env.T
  5. findmaxind(L), findmaxind(L,n), findmaxind(L,n,topshift) work; omitted args use defaults
**Plans**: 1 plan

Plans:
- [ ] 29-01-PLAN.md — Add etaq(k), checkprod(f), checkmult(f), findmaxind(L) 1-arg; acceptance-optional-args

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. BigInt | 0/3 | Complete    | 2026-02-25 |
| 2. Frac | 0/? | Complete    | 2026-02-25 |
| 3. Series | 2/2 | Complete | 2026-02-25 |
| 4. qfuncs | 4/4 | Complete | 2025-02-24 |
| 5. prodmake | 1/1 | Complete | 2025-02-24 |
| 6. convert-extended | 3/3 | Complete | 2025-02-24 |
| 7. linalg | 1/1 | Complete | 2025-02-24 |
| 8. relations | 0/? | Not started | - |
| 9. parser | 0/? | Not started | - |
| 10. repl | 3/3 | Complete | 2026-02-25 |
| 11. Demo artifact | 0/? | Complete    | 2026-02-26 |
| 12. Rogers-Ramanujan demo | 0/? | Complete    | 2026-02-25 |
| 13. Product conversion demo | 0/? | Complete    | 2026-02-25 |
| 14. Relations demo | 0/? | Complete    | 2026-02-25 |
| 15. Sifting and product identities demo | 1/1 | Complete | 2026-02-25 |
| 16. Script mode | 1/1 | Complete    | 2026-02-26 |
| 17. help + help(func) | 1/1 | Complete    | 2026-02-26 |
| 18. Timing | 1/1 | Complete    | 2026-02-26 |
| 19. Multi-line input | 1/1 | Complete | 2026-02-24 |
| 20. Tab completion | 0/? | Complete    | 2026-02-26 |
| 21. Error messages | 0/? | Complete    | 2026-02-26 |
| 22. Demo packaging | 0/? | Complete    | 2026-02-26 |
| 23. Quick wins | 1/1 | Complete | 2026-02-25 |
| 24. mprodmake | 1/1 | Complete | 2026-02-26 |
| 25. checkprod/checkmult | 1/1 | Complete | 2026-02-26 |
| 26. findmaxind | 1/1 | Complete | 2026-02-26 |
| 27. Suppress output (semicolon) | 0/? | Complete    | 2026-02-26 |
| 28. Arrow-key line navigation | 0/? | Complete    | 2026-02-26 |
| 29. Optional-arg audit | 0/? | Complete    | 2026-02-26 |
| 30. Output on next line | 1/1 | Complete    | 2026-02-26 |
| 31. Up-down arrows for history | 1/1 | Complete | 2026-02-26 |
| 32. etaq memoization | 0/? | Complete    | 2026-02-26 |
| 33. Euler pentagonal optimization | 0/? | Not started | - |

### Phase 30: Output on next line after input

**Goal:** REPL output appears on the line after input, not same line
**Depends on:** Phase 29
**Plans:** 1/1 plans complete

Plans:
- [x] 30-01-PLAN.md — Add newline after readLineRaw returns in REPL loop

### Phase 31: up-down arrows for history

**Goal:** Up/down arrow keys navigate command history in the REPL
**Depends on:** Phase 30
**Plans:** 1/1 plans complete

Plans:
- [x] 31-01-PLAN.md — Add history parameter to readLineRaw, up/down ESC handlers, acceptance test

**Milestone v1.6 (etaq Performance) — phases 32–33:**

- [x] **Phase 32: etaq memoization** - Cache etaq(k,T) results so repeated calls return instantly (completed -) (completed 2026-02-26)
- [ ] **Phase 33: Euler pentagonal optimization** - etaq(1,T) via pentagonal number theorem recurrence O(T*sqrt(T)) (completed -)

### Phase 32: etaq memoization
**Goal**: Repeated etaq(k,T) calls return cached results without recomputation
**Depends on**: Phase 31
**Requirements**: PERF-01, PERF-03
**Success Criteria** (what must be TRUE):
  1. Second call to etaq(k,T) with same arguments returns immediately (no recomputation)
  2. Cache is keyed on (k, T) pair; different T values for same k produce correct results
  3. All existing acceptance tests still pass
  4. Timing shows measurable speedup on repeated etaq calls (e.g. demo script)
**Plans**: 1 plan

Plans:
- [ ] 32-01-PLAN.md — Add etaq memoization cache + REPL clear_cache command + set_trunc cache clearing

### Phase 33: Euler pentagonal optimization
**Goal**: etaq(1,T) uses pentagonal number recurrence instead of naive product, achieving O(T*sqrt(T))
**Depends on**: Phase 32
**Requirements**: PERF-02, PERF-03
**Success Criteria** (what must be TRUE):
  1. etaq(1,T) for large T (e.g. T=500) is measurably faster than before
  2. Pentagonal recurrence: p(n) = sum_{k!=0} (-1)^{k+1} * p(n - k(3k-1)/2), used for 1/eta = partition function
  3. Coefficients match naive computation exactly (no numerical drift — exact rational)
  4. All 10 existing acceptance tests still pass
  5. etaq(1,200) completes well under 0.5s
**Plans**: 1 plan

Plans:
- [ ] 33-01-PLAN.md — Pentagonal fast path in etaq() + acceptance test verification
