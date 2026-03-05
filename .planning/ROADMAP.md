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
| 33. Euler pentagonal optimization | 1/1 | Complete    | 2026-02-26 |
| 34. Fix compiler warnings | 1/1 | Complete    | 2026-02-26 |
| 35. User manual | 1/1 | Complete    | 2026-02-26 |
| 36. Expose NT helpers | 0/? | Not started | - |
| 37. Convenience functions | 0/? | Not started | - |
| 38. Math enrichment | 0/? | Not started | - |
| 39. Update MANUAL.md | 1/1 | Complete | 2026-02-26 |
| 40. Acceptance tests v1.8 | 1/1 | Complete | 2026-02-26 |
| 41. Robustness & edge cases | 2/2 | Complete | 2026-02-27 |
| 42. Garvan tutorial coverage | 2/2 | Complete | 2026-02-27 |
| 43. Wasm compile | 0/? | Not started | - |
| 44. Astro Starlight scaffold | 0/? | Complete    | 2026-02-27 |
| 45. Documentation content | 0/? | Complete    | 2026-02-28 |
| 46. Playground | 0/? | Complete    | 2026-02-28 |
| 47. CI/CD & deployment | 0/? | Complete    | 2026-02-28 |
| 48. Cleanup | 0/? | Complete    | 2026-02-28 |
| 49. Single-page website | 0/? | Complete    | 2026-02-28 |
| 50. Example audit | 0/? | Complete    | 2026-02-28 |
| 51. ANSI Color + Clear Screen | 0/? | Complete    | 2026-02-28 |
| 52. Karatsuba Multiplication | 0/? | Complete    | 2026-02-28 |
| 53. Series Optimization | 0/? | Complete    | 2026-02-28 |
| 54. Benchmarking Suite | 0/? | Complete    | 2026-02-28 |
| 55. Smart Tab Completion | 0/? | Complete    | 2026-02-28 |
| 56. Session Save/Load + History | 0/? | Not started | - |
| 63. Q-Shift Arithmetic Fix | 0/? | Complete    | 2026-03-01 |
| 64. Fractional Power Infrastructure | 0/? | Complete    | 2026-03-02 |
| 65. Jacobi Half-Integer Exponents | 0/? | Complete    | 2026-03-02 |
| 66. Exercise Solutions & Regression | 0/? | Complete    | 2026-03-02 |
| 67. Modular Series Arithmetic | 1/1 | Complete    | 2026-03-02 |
| 68. Modular Worksheet Verification | 1/1 | Complete    | 2026-03-02 |
| 69. Rank and Crank Functions | 1/1 | Complete    | 2026-03-02 |
| 70. Partition Type Infrastructure | 0/? | Not started | —          |
| 71. Core t-core Algorithms | 0/? | Not started | —          |
| 72. t-Quotient and GSK Bijection | 0/? | Not started | —          |
| 73. Vector Representations | 0/? | Not started | —          |
| 74. t-core Crank and Display | 0/? | Not started | —          |
| 75. Integration Testing | 0/? | Not started | —          |

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
- [x] **Phase 33: Euler pentagonal optimization** - etaq(1,T) via pentagonal number theorem recurrence O(√T) (completed 2026-02-26)

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
- [x] 33-01-PLAN.md — Pentagonal fast path in etaq() + acceptance verification

**Milestone v1.7 (Polish & Documentation) — phases 34–35:**

- [x] **Phase 34: Fix compiler warnings** - Eliminate all -Wall -Wextra -Wpedantic warnings for clean build (completed 2026-02-26)
- [x] **Phase 35: User manual** - MANUAL.md with getting started, all 37 built-ins, workflow tutorials (completed 2026-02-26)

### Phase 34: Fix compiler warnings
**Goal**: Clean build with zero warnings under -Wall -Wextra -Wpedantic
**Depends on**: Phase 33
**Requirements**: POLISH-01, POLISH-02
**Success Criteria** (what must be TRUE):
  1. `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic` produces zero warnings
  2. All existing acceptance tests still pass
  3. No behavioral changes — warning fixes only
**Plans**: 1 plan

Plans:
- [x] 34-01-PLAN.md — Fix all 8 warnings across qfuncs.h, convert.h, repl.h

### Phase 35: User manual
**Goal**: Comprehensive MANUAL.md documenting all built-in functions with examples and workflow tutorials
**Depends on**: Phase 34
**Requirements**: DOC-01, DOC-02
**Success Criteria** (what must be TRUE):
  1. MANUAL.md exists with getting-started section (build, launch, basic usage)
  2. All 37 built-in functions documented with signature, description, and example
  3. Workflow tutorials: Rogers-Ramanujan, product identification, relation finding
  4. Document covers REPL features: tab completion, arrow keys, history, semicolon suppression, backslash continuation
**Plans**: 1 plan

Plans:
- [x] 35-01-PLAN.md — Write MANUAL.md with all 37 built-ins, tutorials, REPL features

**Milestone v1.8 (More Math Functions) — phases 36–38:**

- [x] **Phase 36: Expose NT helpers** - REPL bindings for divisors, mobius, euler_phi (completed 2026-02-25)
- [ ] **Phase 37: Convenience functions** - coeff, dissect, jacobi, kronecker
- [ ] **Phase 38: Math enrichment** - eisenstein, partition, qdiff

### Phase 36: Expose NT helpers
**Goal**: Expose the 3 internal number theory helpers as REPL built-ins
**Depends on**: Phase 35
**Requirements**: MATH-01, MATH-02, MATH-03, MATH-11
**Success Criteria** (what must be TRUE):
  1. User can call `divisors(12)` and get `[1, 2, 3, 4, 6, 12]`
  2. User can call `mobius(6)` and get `1`, `mobius(4)` and get `0`
  3. User can call `euler_phi(12)` and get `4`
  4. help(divisors), help(mobius), help(euler_phi) work
  5. All existing acceptance tests still pass
**Plans**: 1 plan

Plans:
- [x] 36-01-PLAN.md — Add REPL bindings + help for divisors, mobius, euler_phi

### Phase 37: Convenience functions
**Goal**: Add coeff, dissect, jacobi, kronecker built-ins
**Depends on**: Phase 36
**Requirements**: MATH-04, MATH-05, MATH-06, MATH-07, MATH-11
**Success Criteria** (what must be TRUE):
  1. `coeff(etaq(1,30), 5)` returns `1`
  2. `dissect(f, 5, T)` returns list of 5 sifted series
  3. `jacobi(2, 15)` returns correct Jacobi symbol
  4. `kronecker(a, n)` works for all integer n
  5. All existing acceptance tests still pass
**Plans**: 1 plan

Plans:
- [x] 37-01-PLAN.md — Add coeff, dissect, jacobi, kronecker built-ins

### Phase 38: Math enrichment
**Goal**: Add eisenstein, partition, qdiff built-ins
**Depends on**: Phase 37
**Requirements**: MATH-08, MATH-09, MATH-10, MATH-11
**Success Criteria** (what must be TRUE):
  1. `eisenstein(2, 50)` produces E_4(q) = 1 + 240q + 2160q² + ...
  2. `partition(100)` returns 190569292 (exact)
  3. `qdiff(f)` computes the formal q-derivative correctly
  4. All existing acceptance tests still pass
**Plans**: 1 plan

Plans:
- [x] 38-01-PLAN.md — Add eisenstein, partition, qdiff built-ins

---

## Milestone v1.9 (Manual Update & Testing)

**Objective:** Update documentation and add acceptance tests for all v1.8 functions.
**Requirements:** DOC-03, DOC-04, TEST-01, TEST-02

### Progress

- [ ] **Phase 39: Update MANUAL.md** - Document 10 new built-ins, update version references
- [ ] **Phase 40: Acceptance tests** - Test script for all v1.8 functions

### Phase 39: Update MANUAL.md
**Goal**: Document all 10 v1.8 built-ins with signatures, descriptions, and examples; update version references
**Depends on**: Phase 38
**Requirements**: DOC-03, DOC-04
**Success Criteria** (what must be TRUE):
  1. MANUAL.md contains entries for coeff, dissect, divisors, eisenstein, euler_phi, jacobi, kronecker, mobius, partition, qdiff
  2. Each entry has signature, description, and example with actual REPL output
  3. Version references updated to 1.9
**Plans**: 1 plan

Plans:
- [x] 39-01-PLAN.md — Document 10 v1.8 built-ins + fix version refs

### Phase 40: Acceptance tests for v1.8 functions
**Goal**: Add test script covering all 10 new v1.8 functions
**Depends on**: Phase 39
**Requirements**: TEST-01, TEST-02
**Success Criteria** (what must be TRUE):
  1. Test script tests each of the 10 new functions with correct expected output
  2. All new tests pass
  3. All existing acceptance tests still pass
**Plans**: 1 plan

Plans:
- [x] 40-01-PLAN.md — Create acceptance-v18.sh test script + Makefile target

---

## Milestone v2.0 (Robustness & Tutorial Coverage)

**Objective:** Fix critical crash/hang bugs, harden input validation, and improve Garvan tutorial coverage.
**Requirements:** ROBUST-01 through ROBUST-06, TUTORIAL-01, TUTORIAL-02

### Progress

- [x] **Phase 41: Robustness & edge cases** - Fix 6 critical bugs + medium-priority hardening (completed 2026-02-27)
- [x] **Phase 42: Garvan tutorial coverage** - Character Eisenstein UE(k,p,T), T_rn memoization, min/max builtins (completed 2026-02-27)

### Phase 41: Robustness & edge cases
**Goal**: Fix all critical crash/hang bugs and key medium-severity issues
**Depends on**: Phase 40
**Requirements**: ROBUST-01 through ROBUST-09
**Success Criteria** (what must be TRUE):
  1. `etaq(0, 50)` throws error instead of hanging
  2. `sift(f, 0, 0, 50)` throws error instead of hanging
  3. Integer division by zero in expressions throws "division by zero" error
  4. Deeply nested input `(((((...))))` (256+ levels) throws error, no crash
  5. Integer literals exceeding int64_t range throw error
  6. `pow(INT_MIN)` handled safely (no UB)
  7. `std::bad_variant_access` never leaks to user
  8. Unknown built-in suggests closest match
  9. Integer-valued builtins can be assigned to variables
  10. All existing acceptance tests still pass
**Plans**: 2 plans

Plans:
- [ ] 41-01-PLAN.md — Validation guards: etaq k>0, sift n>0, pow limit, parser depth/overflow
- [ ] 41-02-PLAN.md — REPL fixes: div-by-zero, safe variant access, Levenshtein suggestions, int assignment + acceptance tests

### Phase 42: Garvan tutorial coverage
**Goal**: Implement features needed to reproduce more qseriesdoc examples
**Depends on**: Phase 41
**Requirements**: TUTORIAL-01, TUTORIAL-02
**Success Criteria** (what must be TRUE):
  1. `T_rn(r, n, T)` uses memoization; T(0,30,50) completes in <2s
  2. `min(a,b,...)` and `max(a,b,...)` built-ins work
  3. Tutorial §4.2 Eisenstein example can be partially reproduced
  4. Version updated to 2.0
  5. MANUAL.md updated with new features
  6. All existing acceptance tests still pass
**Plans**: 2 plans

Plans:
- [ ] 42-01-PLAN.md — T_rn memoization + min/max integer builtins
- [ ] 42-02-PLAN.md — Version bump 1.9→2.0, MANUAL.md updates, acceptance tests

---

## Milestone v2.1 (Website) — phases 43–47:

- [ ] **Phase 43: Wasm compile** - C++ → WebAssembly via Emscripten with evaluate() API and exception safety
- [x] **Phase 44: Astro Starlight scaffold** - Documentation site skeleton with responsive layout, sidebar, search (completed 2026-02-27)
- [x] **Phase 45: Documentation content** - Landing page, MANUAL.md conversion, Garvan tutorial with KaTeX (completed 2026-02-28)
- [x] **Phase 46: Playground** - xterm.js terminal UI, Web Worker Wasm execution, example dropdown (completed 2026-02-28)
- [x] **Phase 47: CI/CD & deployment** - GitHub Actions pipeline, Cloudflare Pages with correct .wasm MIME (completed 2026-02-28)

### Phase 43: Wasm compile
**Goal**: C++ codebase compiles to WebAssembly with a working evaluate(expr) → string API
**Depends on**: Phase 42 (current codebase must be stable)
**Requirements**: WASM-01, WASM-02, WASM-03
**Success Criteria** (what must be TRUE):
  1. `emcc` compiles a Wasm entry point (main_wasm.cpp with `#ifdef __EMSCRIPTEN__`) to .wasm/.js; `evaluate("1+1")` returns `"2"` in Node.js
  2. `evaluate("etaq(0,50)")` returns an error message string, not a module abort or Wasm trap
  3. Compressed .wasm file (gzip) is under 1.5 MB
  4. All REPL-level expressions work via evaluate() — series arithmetic, prodmake, theta functions, relations
**Plans**: 2 plans

Plans:
- [ ] 43-01-PLAN.md — Guard repl.h terminal I/O, create main_wasm.cpp Embind entry point, add Makefile wasm target
- [ ] 43-02-PLAN.md — Node.js test script + HTML test page for comprehensive Wasm API verification

### Phase 44: Astro Starlight scaffold
**Goal**: Documentation site skeleton exists with responsive layout and navigation
**Depends on**: Nothing (can scaffold independently of Wasm)
**Requirements**: SITE-01, SITE-05
**Success Criteria** (what must be TRUE):
  1. `npm run dev` in `website/` serves a Starlight site with sidebar navigation, dark mode toggle, and responsive layout
  2. Placeholder pages exist for each documentation section (manual, tutorial, playground)
  3. Pagefind full-text search is integrated and functional (Starlight built-in)
  4. `npm run build` produces static output in `website/dist/`
**Plans**: 1 plan

Plans:
- [x] 44-01-PLAN.md — Scaffold Astro Starlight site with config, sidebar, custom theme, and 16 placeholder content pages ✅ 2026-02-27

### Phase 45: Documentation content
**Goal**: Landing page, manual reference, and tutorial content with math rendering
**Depends on**: Phase 44 (site scaffold must exist)
**Requirements**: SITE-02, SITE-03, SITE-04
**Success Criteria** (what must be TRUE):
  1. Landing page displays project overview, feature highlights, and download link
  2. MANUAL.md content is converted to navigable HTML documentation sections (all 50+ built-ins reachable via sidebar)
  3. Garvan tutorial examples render with KaTeX mathematics and explanatory commentary
  4. Navigation between landing page, manual sections, and tutorials works seamlessly
**Plans**: 3 plans

Plans:
- [ ] 45-01-PLAN.md — KaTeX math integration, landing page with feature cards + RR demo, getting-started pages
- [ ] 45-02-PLAN.md — All 7 reference manual pages (38+ built-in functions with typed signatures and REPL examples)
- [ ] 45-03-PLAN.md — All 5 tutorial pages (Rogers-Ramanujan, theta functions, partition identities, modular equations)

### Phase 46: Playground
**Goal**: Users can run q-series computations live in the browser
**Depends on**: Phase 43 (Wasm binary), Phase 44 (site scaffold)
**Requirements**: PG-01, PG-02, PG-03, WASM-04
**Success Criteria** (what must be TRUE):
  1. Playground page has terminal-style UI using xterm.js with monospace input/output
  2. Wasm runs in a Web Worker — UI never freezes during long computations (e.g. prodmake with T=200)
  3. Example dropdown preloads Rogers-Ramanujan, prodmake, theta function, and relation-finding examples
  4. Loading spinner shown while Wasm downloads; computing indicator shown while evaluating
  5. User can type `prodmake(sum(q^(n^2)/aqprod(q,q,n),n,0,8),50)` and get correct output in the browser
**Plans**: 1 plan

Plans:
- [ ] 46-01-PLAN.md — Complete playground page with xterm.js terminal, Web Worker Wasm execution, example dropdown, loading states

### Phase 47: CI/CD & deployment
**Goal**: Push-to-deploy pipeline with correct Wasm serving on Cloudflare Pages
**Depends on**: Phase 43, 44, 45, 46 (everything must be buildable)
**Requirements**: DEPLOY-01, DEPLOY-02
**Success Criteria** (what must be TRUE):
  1. GitHub Actions workflow: push to main → build Wasm (Emscripten) → build site (Astro) → deploy to Cloudflare Pages
  2. Deployed site serves `.wasm` files with correct `application/wasm` MIME type
  3. Playground works end-to-end on the deployed URL (Wasm loads, evaluates, returns results)
**Plans:** 1/1 plans complete

Plans:
- [ ] 47-01-PLAN.md — GitHub Actions workflow (Wasm → Astro → Cloudflare Pages deploy) + _headers for Wasm MIME type

---

## Milestone v3.0 (Simple Website & Example Audit) — phases 48–50:

- [x] **Phase 48: Cleanup** - Delete website/, remove CI/CD workflow, clean Makefile of wasm-website target (completed 2026-02-28)
- [x] **Phase 49: Single-page website** - Self-contained HTML page with kangaroo-banner aesthetic, examples, full function reference (completed 2026-02-28)
- [x] **Phase 50: Example audit** - Run every example across all docs, fix any failures (completed 2026-02-28)

### Phase 48: Cleanup
**Goal**: The codebase is free of all complex website infrastructure
**Depends on**: Phase 47
**Requirements**: CLEAN-01, CLEAN-02, CLEAN-03
**Success Criteria** (what must be TRUE):
  1. The `website/` directory no longer exists in the repository
  2. `.github/workflows/deploy.yml` and `website/public/_headers` no longer exist
  3. `make wasm-website` is no longer a valid Makefile target
  4. The core build still works (`make` produces a working qseries binary)
**Plans**: TBD

### Phase 49: Single-page website
**Goal**: A single self-contained HTML file serves as the project's web presence with kangaroo-banner aesthetic
**Depends on**: Phase 48
**Requirements**: PAGE-01, PAGE-02, PAGE-03, PAGE-04, PAGE-05
**Success Criteria** (what must be TRUE):
  1. A single HTML file exists with kangaroo-banner hero section (sunset gradient, hopping kangaroo SVG, stars, boomerang, dust particles)
  2. Build instructions section covers prerequisites, one-liner compile command, and platform notes
  3. Key examples section shows Rogers-Ramanujan, partition function, theta functions, and relation finding with REPL input/output
  4. Full function reference documents all 40+ built-in functions with signature, description, and example
  5. Design follows kangaroo-banner aesthetic: dark background (#0d0617), sunset accents (#d4451a, #f7a634), gold headings (#ffe066), Bebas Neue + Outfit fonts
**Plans**: TBD

### Phase 50: Example audit
**Goal**: Every example across all project documentation runs correctly and produces expected output
**Depends on**: Phase 49
**Requirements**: AUDIT-01, AUDIT-02, AUDIT-03, AUDIT-04
**Success Criteria** (what must be TRUE):
  1. Every command in `demo/garvan-demo.sh` runs without error and produces correct output
  2. Every example in `MANUAL.md` runs without error and produces correct output
  3. Every code snippet in the single-page website runs without error and produces correct output
  4. Any failing or incorrect examples have been fixed (in the example text or in the REPL code)

---

## Milestone v4.0 (Core Improvements) — phases 51–56:

- [x] **Phase 51: ANSI Color + Clear Screen** - Colored prompt/errors/timing, Ctrl+L and clear command, Windows VT enablement, ansi.h utility (completed -) (completed 2026-02-28)
- [x] **Phase 52: Karatsuba Multiplication** - O(n^1.585) BigInt multiply for large operands, hybrid threshold at ~32 limbs (completed -) (completed 2026-02-28)
- [x] **Phase 53: Series Optimization** - Inner-loop early break for 2-4x speedup on series multiplication (completed -) (completed 2026-02-28)
- [x] **Phase 54: Benchmarking Suite** - bench_main.cpp with micro-benchmarks for BigInt, Series, etaq, prodmake; median reporting (completed -) (completed 2026-02-28)
- [x] **Phase 55: Smart Tab Completion** - Longest-common-prefix fill and auto-parentheses on function completion (completed -) (completed 2026-02-28)
- [x] **Phase 56: Session Save/Load + History Persistence** - ~/.qseries_history persistence, save/load commands, final regression gate (completed 2026-02-28)

### Phase 51: ANSI Color + Clear Screen
**Goal**: REPL output uses color for visual clarity; user can clear the screen
**Depends on**: Phase 50
**Requirements**: UX-01, UX-02, REG-02
**Success Criteria** (what must be TRUE):
  1. Prompt displays in gold, errors in red, timing in dim gray — visually distinct at a glance
  2. `Ctrl+L` clears the screen in raw terminal mode; `clear` command clears via `\033[2J\033[H`
  3. Piping output (`qseries < script.qs | cat`) produces zero ANSI escape codes in the output stream
  4. `NO_COLOR` environment variable disables all color when set
  5. Windows terminal displays colors correctly (VT processing enabled via `SetConsoleMode`)
**Plans**: TBD

### Phase 52: Karatsuba Multiplication
**Goal**: BigInt multiplication is asymptotically faster for large operands
**Depends on**: Phase 51
**Requirements**: PERF-05
**Success Criteria** (what must be TRUE):
  1. Multiplying two 100-limb (~900-digit) BigInts uses Karatsuba and is measurably faster than schoolbook
  2. Small operands (< 32 limbs) still use schoolbook — no regression on typical REPL workloads
  3. `karatsuba(a,b) == schoolbook(a,b)` for random inputs at all size boundaries (correctness invariant)
  4. All existing acceptance tests still pass
**Plans**: TBD

### Phase 53: Series Optimization
**Goal**: Series multiplication is 2-4x faster through inner-loop early termination
**Depends on**: Phase 52
**Requirements**: PERF-04
**Success Criteria** (what must be TRUE):
  1. Series multiplication skips inner-loop iterations where exponent sum exceeds truncation
  2. `prodmake(rr, 200)` completes measurably faster than before optimization
  3. Rogers-Ramanujan prodmake still produces correct output (truncation invariant preserved)
  4. All existing acceptance tests still pass
**Plans**: TBD

### Phase 54: Benchmarking Suite
**Goal**: Developers can measure and compare performance of core operations
**Depends on**: Phase 53
**Requirements**: PERF-06
**Success Criteria** (what must be TRUE):
  1. `bench_main.cpp` exists as a separate binary with micro-benchmarks for BigInt multiply, Series multiply, etaq, and prodmake
  2. Benchmark uses `std::chrono::steady_clock` with `DoNotOptimize` compiler barriers
  3. Results report median of multiple runs (not mean) for noise resistance
  4. `make bench` builds and runs the benchmark suite
**Plans**: TBD

### Phase 55: Smart Tab Completion
**Goal**: Tab completion fills longest common prefix and appends parentheses for functions
**Depends on**: Phase 54
**Requirements**: UX-04, UX-05
**Success Criteria** (what must be TRUE):
  1. Pressing Tab with multiple matches fills to the longest common prefix before listing candidates
  2. Completing a unique function name appends `(` automatically (e.g., `prod` → `prodmake(`)
  3. Completing a variable name does NOT append `(` — only known functions get auto-parens
  4. Existing Tab behavior for unique match and no-match cases is preserved
**Plans**: TBD

### Phase 56: Session Save/Load + History Persistence
**Goal**: User can persist command history across sessions and save/load variable state
**Depends on**: Phase 55
**Requirements**: UX-03, UX-06, REG-01
**Success Criteria** (what must be TRUE):
  1. Command history is saved to `~/.qseries_history` on exit and loaded on startup; max 1000 lines
  2. `save("name")` serializes the current environment to a `.qsession` file in human-readable text format with version header
  3. `load("name")` restores variables from a `.qsession` file; user sees confirmation message
  4. All existing acceptance tests pass after all v4.0 changes (final regression gate)
  5. History persistence works on both Unix and Windows (home directory resolution)
**Plans**: TBD

### Phase 60: etamake scalar factor and non-integer q exponents

**Goal:** [To be planned]
**Depends on:** Phase 59
**Plans:** 0 plans

Plans:
- [ ] TBD (run /gsd:plan-phase 60 to break down)

### Phase 61: fix integer exponents only gap

**Goal:** [To be planned]
**Depends on:** Phase 60
**Plans:** 0 plans

Plans:
- [ ] TBD (run /gsd:plan-phase 61 to break down)

### Phase 62: maple checklist satisfied

**Goal:** Verify and ensure all Maple qseries tutorial examples (qseriesdoc.md) produce correct output in qseries3, including double sums, fractional q-powers, mod(), and Jacobi product identification
**Depends on:** Phase 61
**Plans:** 2/2 plans complete

Plans:
- [ ] 62-01-PLAN.md — Fix theta2/3/4 q-argument and tripleprod/quinprod T semantics, test blocks 1-27
- [ ] 62-02-PLAN.md — Test blocks 28-41, create exercise solutions, finalize checklist

---

## Milestone v4.1 (Distribution) — phases 57–59:

- [x] **Phase 57: CI/CD Release Pipeline** — Tag-triggered GitHub Actions workflow builds cross-platform binaries and creates GitHub Release with assets (completed 2026-02-28)
- [x] **Phase 58: Docker** — Multi-stage Dockerfile, ghcr.io publish, Makefile targets, acceptance tests in container (completed 2026-02-28)
- [x] **Phase 59: Install Script** — One-liner curl install with platform detection, SHA256 verification, version selection (completed 2026-02-28)

### Phase 57: CI/CD Release Pipeline
**Goal**: Pushing a version tag automatically builds cross-platform binaries and creates a GitHub Release
**Depends on**: Phase 56
**Requirements**: DIST-07, DIST-08
**Success Criteria** (what must be TRUE):
  1. Pushing a `v*` tag triggers a GitHub Actions workflow that builds and completes successfully
  2. The workflow produces both a Linux x86_64 binary (static) and a macOS arm64 binary (dynamic, no `-static`)
  3. A GitHub Release is automatically created with both binaries and a SHA256 checksums file attached as assets
  4. The release page shows the tag name as title and includes a changelog or auto-generated notes
**Plans**: TBD

### Phase 58: Docker
**Goal**: Users can run qseries in a Docker container with a single command
**Depends on**: Phase 57
**Requirements**: DIST-01, DIST-02, DIST-03, DIST-09, REG-03
**Success Criteria** (what must be TRUE):
  1. `make docker-build` builds a Docker image using a multi-stage Dockerfile (gcc build stage, scratch runtime) in under 2 minutes
  2. `docker run -it ghcr.io/gerardgarvan/qseries3` launches an interactive REPL that responds to commands (e.g., `etaq(1,10)`)
  3. Final Docker image size is under 5MB (`docker images` confirms)
  4. CI workflow automatically builds and pushes the Docker image to ghcr.io on each release tag
  5. All existing acceptance tests pass when run inside the Docker container
**Plans**: TBD

### Phase 59: Install Script
**Goal**: Users can install qseries with a single curl command
**Depends on**: Phase 57
**Requirements**: DIST-04, DIST-05, DIST-06
**Success Criteria** (what must be TRUE):
  1. `curl -fsSL <url> | sh` downloads and installs the qseries binary to `~/.local/bin` on both Linux and macOS
  2. The script detects the platform via `uname` and downloads the correct binary (Linux x86_64 or macOS arm64)
  3. Downloaded binary is verified against published SHA256 checksums before being placed in PATH
  4. `install.sh --version v2.1` installs a specific version; omitting `--version` installs latest

---

## Milestone v4.2 (Fix Block Failures) — phases 63–66:

- [x] **Phase 63: Q-Shift Arithmetic Fix** - Absorb integer q_shift differences in Series::operator+, unblocking Block 25 and theta quotient arithmetic (completed 2026-03-01)
- [x] **Phase 64: Fractional Power Infrastructure** - Series::powFrac(Frac) via generalized binomial series for exact rational fractional powers (completed 2026-03-02)
- [x] **Phase 65: Jacobi Half-Integer Exponents** - Fix jac2series_impl powFrac dispatch and jac2prod fractional display for Blocks 13-14 (completed 2026-03-02)
- [x] **Phase 66: Exercise Solutions & Regression** - Verify exercises 4/9/10 with fixed infrastructure, full regression gate (completed 2026-03-02)

## Milestone v4.3 (Modular Arithmetic) — phases 67–68:

- [x] **Phase 67: Modular Series Arithmetic** - modp(f,p), nterms(f), Gaussian elimination over F_p, findhommodp for modular relation finding (completed 2026-03-02)
- [x] **Phase 68: Modular Worksheet Verification** - Reproduce Garvan's mod-7 eta dissection worksheet end-to-end (completed 2026-03-02)

## Milestone v4.4 (Partition Statistics) — phase 69:

- [x] **Phase 69: Rank and Crank Functions** - rankgf(m,T) and crankgf(m,T) generating functions for partition rank and crank statistics (completed 2026-03-02)

## Milestone v5.0 (t-core Package) — phases 70–75 ✓ COMPLETE (2026-03-02):

- [x] **Phase 70: Partition Type Infrastructure** — Add Partition type to REPL: list literals `[1,2,3]` → Partition, display, variable assignment, `partitions(n)` enumeration, conjugate utility
- [x] **Phase 71: Core t-core Algorithms** — `tcore.h` with `rvec`, `istcore`, `tcoreofptn`, `tcores`; REPL dispatch and help entries
- [x] **Phase 72: t-Quotient and GSK Bijection** — `tquot`, `phi1`, `invphi1` with rim hook insertion; size identity verification
- [x] **Phase 73: Vector Representations** — `ptn2nvec`, `nvec2ptn`, `ptn2rvec`, `nvec2alphavec`; roundtrip tests
- [x] **Phase 74: t-core Crank and Display** — `tcrank` with modular exponentiation, `tresdiag`, `makebiw`
- [x] **Phase 75: Integration Testing** — End-to-end tests with Maple tcore examples, regression suite

### Phase 67: Modular Series Arithmetic [x] (completed 2026-03-02)
**Goal**: Series coefficients can be reduced mod p, and linear algebra over F_p finds modular polynomial relations
**Depends on**: Phase 66
**Requirements**: MOD-01, MOD-02, MOD-03, MOD-04
**Success Criteria** (what must be TRUE):
  1. `modp(etaq(1,100), 7)` reduces all coefficients mod 7 and returns a valid Series
  2. `nterms(f)` returns the count of non-zero coefficients in series f
  3. `findhommodp([f1,f2,...,f8], 7, 1, 0)` finds homogeneous relations over F_7 — kernel may differ from `findhom` over Q
  4. Gaussian elimination over F_p correctly handles modular inverse and produces correct kernels (e.g., matrix that is full-rank over Q but has kernel over F_7)
  5. No regressions in existing test suites

Plans:
- [x] 67-01-PLAN.md — modp, nterms, F_p linear algebra, findhommodp (completed 2026-03-02)

### Phase 68: Modular Worksheet Verification [x] (completed 2026-03-02)
**Goal**: Garvan's mod-7 eta dissection worksheet runs end-to-end in qseries3
**Depends on**: Phase 67
**Requirements**: MOD-05
**Success Criteria** (what must be TRUE):
  1. `A0 := sum((-1)^(n+1)*n*(3*n-1)*q^(n*(3*n-1)), n, -20, 20)` computes the Ramanujan-type sum
  2. `findhommodp` on the 8-element sifted eta list finds {X₇, X₈} (Output 1 from worksheet)
  3. `modp(sift(A0, 7, k, 1000), 7)` for k=0..6 matches expected nterms (Output 3-4)
  4. `jacprodmake` on sifted `etaq(2,1000)` residues produces correct Jacobi products (Output 7+)
  5. All results match the Maple worksheet outputs

Plans:
- [x] 68-01-PLAN.md — Worksheet verification test suite (completed 2026-03-02)

### Phase 70: Partition Type Infrastructure [x] (completed 2026-03-01)
**Goal**: Add partition (integer list) data type to the REPL so partitions can be created, stored, and displayed
**Depends on**: None (foundational)
**Requirements**: INFRA-01, INFRA-02, INFRA-03, INFRA-04
**Success Criteria** (what must be TRUE):
  1. `[4,2,2,1]` evaluates to a Partition and displays as `[4, 2, 2, 1]` ✓
  2. `p := [3,3,1]` stores a Partition in variable p; `p` displays it ✓
  3. `partitions(5)` enumerates all 7 partitions of 5 ✓
  4. `[]` evaluates to the empty partition ✓
  5. `conjpart([4,2,2,1])` returns the conjugate partition `[4, 3, 1, 1]` ✓

### Phase 71: Core t-core Algorithms [x] (completed 2026-03-02)
**Goal**: Implement the fundamental t-core building blocks: rvec, istcore, tcoreofptn, tcores
**Depends on**: Phase 70
**Requirements**: TCORE-01, TCORE-02, TCORE-03, TCORE-05
**Success Criteria** (what must be TRUE):
  1. `rvec([4,2,2,1], 3, 0)` returns 4 ✓
  2. `istcore([3,1], 3)` returns 1; `istcore([4,2,2,1], 3)` returns 0 ✓
  3. `tcoreofptn([4,2,2,1], 3)` returns `[4, 2]` ✓
  4. `tcores(3, 4)` lists 2 three-cores: `[3,1]` and `[2,1,1]` ✓ (note: original criteria said 1 core; both are valid 3-cores)
  5. All modular arithmetic uses safe `((x % p) + p) % p` pattern ✓

### Phase 72: t-Quotient and GSK Bijection [x] (completed 2026-03-02)
**Goal**: Implement tquot, phi1, invphi1 — the Garvan-Kim-Stanton bijection between partitions and [core, quotient] pairs
**Depends on**: Phase 71
**Requirements**: TCORE-04, GSK-01, GSK-02, GSK-03
**Success Criteria** (what must be TRUE):
  1. `tquot([4,2,2,1], 3)` returns 3 partitions: `[], [], [1]` ✓
  2. `phi1([4,2,2,1], 3)` returns Core: `[4, 2]`, Quotient: `[[], [], [1]]` ✓
  3. `invphi1(phi1([4,2,2,1], 3), 3)` returns `[4, 2, 2, 1]` ✓
  4. Size identity verified: `[5,4,3,2,1]` → core=`[]`, quotient norms 1+3+1=5, 3×5=15=|λ| ✓
  5. Roundtrips pass for t=2,3,5,7 ✓

### Phase 73: Vector Representations [x] (2026-03-02)
**Goal**: Implement n-vector, r-vector, and alpha-vector representations of t-cores
**Depends on**: Phase 71
**Requirements**: VEC-01, VEC-02, VEC-03, VEC-04
**Success Criteria** (what must be TRUE):
  1. `ptn2nvec([3,1], 3)` returns the correct 3-element n-vector
  2. `nvec2ptn(ptn2nvec(ptn, t))` roundtrips for any t-core ptn
  3. `ptn2rvec([4,2,2,1], 3)` returns the 3-element r-vector
  4. `nvec2alphavec(nvec)` works for t=5 and t=7, errors for other t
  5. Frobenius coordinate reconstruction in nvec2ptn produces correct partitions

### Phase 74: t-core Crank and Display [x] (2026-03-02)
**Goal**: Implement tcrank statistic and visual display functions (tresdiag, makebiw)
**Depends on**: Phase 71
**Requirements**: STAT-08, DISP-01, DISP-02
**Success Criteria** (what must be TRUE):
  1. `tcrank([4,2,2,1], 5)` returns a value in {0,1,2,3,4}
  2. `tcrank` uses modular exponentiation for `h(t)=(t-(p-1)/2)^(p-3)` to avoid overflow
  3. `tresdiag([4,2,2,1], 3)` prints the 3-residue diagram
  4. `makebiw([1,1,2,4,4,5,6,6,6], 5, 3)` prints bi-infinite words matching Maple output
  5. Display functions produce output matching the Maple tcore package

### Phase 75: Integration Testing [x] (2026-03-02)
**Goal**: End-to-end verification with Maple tcore examples, full regression
**Depends on**: Phases 72, 73, 74
**Requirements**: All (integration)
**Success Criteria** (what must be TRUE):
  1. Maple tcore examples from t-core.txt all reproduce correctly
  2. `phi1`/`invphi1` roundtrip passes for partitions of n=1..20 with t=2,3,5
  3. Size identity holds for all test cases
  4. No regressions in maple-checklist.sh or other acceptance tests
  5. Help entries for all 15+ new functions

---

## Milestone v6.0 (Gaps Packages) — phases 76–87:

- [x] **Phase 76: Partition Statistics** — `drank`, `agcrank`, predicates (`ptnDP`/`ptnOP`/`ptnRR`/`ptnCC`/`ptnSCHUR`/`ptnOE`), `overptns`, `overptnrank`/`overptncrank`, counting functions `PDP`/`POE`/`PRR`/`PSCHUR` ✓ (2026-03-02)
- [x] **Phase 77: Mock Theta Functions** — 44-function registry, `mockqs(name, order, T)`, `mockdesorder(m)` ✓ (2026-03-02)
- [x] **Phase 78: Misc Utilities** — `newprodmake`, `EISENq`/`Phiq`, `dilly`, `sieveqcheck`, `siftfindrange`, `polyfind` ✓ (2026-03-02)
- [x] **Phase 79: Crank/Rank Tables** — SPT-crank (`NS`), overpartition crank (`MBAR`), M2 rank/crank/orank tables, overpartition numbers, residue GFs (completed 2026-03-02)
- [x] **Phase 80: Bailey Chains** — betafind, alphaup, alphadown, bailey_sum, catalog (unit, RR, GG) (completed 2026-03-02)
- [x] **Phase 81: ETA Cusp Theory** — cuspmake, cuspord, gammacheck, etaprodtoqseries, vp (completed 2026-03-02)
- [x] **Phase 82: ETA Identity Prover** — `provemodfuncGAMMA0id`, provemodfuncGAMMA0idBATCH, U_p stub ✓ (2026-03-02)
- [x] **Phase 83: Modular Forms** — DELTA12, makebasisM, makebasisPX ✓ (2026-03-02)
- [x] **Phase 84: Theta IDs Cusp Theory** — QP2, getacuspord, cuspmake1, Gamma1ModFunc, eta2jac, jac2eprod ✓ (2026-03-02)
- [ ] **Phase 85: Theta IDs Identity Prover** — `provemodfuncid(jacid, N)` for Gamma_1(N)
- [ ] **Phase 86: RR Identity Search** — `RRG(n)`/`RRH(n)` functions, `checkid`, `findids(type, T)` search engine
- [x] **Phase 87: Integration Testing** — Cross-package tests, full regression (completed 2026-03-03)

**Milestone v7.0 (Maple Checklist Gaps) — phases 88–90:**

- [x] **Phase 88: quinprod prodid/seriesid** — quinprod(z,q,prodid) and quinprod(z,q,seriesid) return identity formulas (Blocks 30–31) (completed 2026-03-03)
- [x] **Phase 89: List indexing** — x[1] returns first element of findhom/findnonhom output (Block 21) (completed 2026-03-03)
- [x] **Phase 90: Symbolic z tripleprod/quinprod** — tripleprod(z,q,T) and quinprod(z,q,T) with symbolic z return bivariate series (Blocks 28, 32) (completed 2026-03-03)

### Phase 76: Partition Statistics [ ]
**Goal**: Implement Dyson rank, Andrews-Garvan crank, partition predicates, overpartitions, counting functions
**Depends on**: Phase 70 (Partition type)
**Requirements**: PSTAT-01, PSTAT-02, PSTAT-03, PSTAT-04, PSTAT-05, PSTAT-06, PSTAT-07
**Success Criteria** (what must be TRUE):
  1. `drank([5,3,2,1])` = 5 - 4 = 1 (largest part minus number of parts)
  2. `agcrank([5,3,2])` = 5 (no 1s: return largest part)
  3. `agcrank([3,2,1,1])` = 2 - 2 = 0 (µ=2 ones, parts > µ: {3,2} → 2 - 2)
  4. `ptnDP([5,3,1])` = true, `ptnDP([3,3,1])` = false
  5. `overptns(3)` returns all overpartitions of 3 (4 overpartitions)
  6. `PDP(5)` = 3 (partitions of 5 into distinct parts: [5], [4,1], [3,2])

### Phase 77: Mock Theta Functions [ ]
**Goal**: Implement all 44 Ramanujan mock theta functions as a registry with q-series expansion
**Depends on**: None (uses existing aqprod)
**Requirements**: MOCK-01, MOCK-02, MOCK-03
**Success Criteria** (what must be TRUE):
  1. `mockqs(f3, 3, 20)` returns the correct q-series for Ramanujan's 3rd-order f function
  2. `mockdesorder(3)` lists all 7 third-order mock theta functions
  3. `mockdesorder(5)` lists all 12 fifth-order functions
  4. Each mock theta function uses only aqprod — no new math primitives needed
  5. At least 10 mock theta functions verified against known coefficients

### Phase 78: Misc Utilities [ ]
**Goal**: Enhanced prodmake, Eisenstein series, q-dilation, sieve checks, sift diagnostics
**Depends on**: None (builds on existing sift, prodmake, sigma)
**Requirements**: MISC-01, MISC-02, MISC-03, MISC-04, MISC-05, MISC-06, MISC-07
**Success Criteria** (what must be TRUE):
  1. `newprodmake` handles series with arbitrary leading terms (not just 1 + ...)
  2. `EISENq(4, 30)` matches `eisenstein(4, 30)` (consistency check)
  3. `dilly(f, 2)` correctly substitutes q → q^2 in a series
  4. `siftfindrange(f, 5, 100)` identifies the sparsest residue class
  5. `sieveqcheck(sift(f,5,0,100), 5)` returns true when all exponents are ≡ 0 mod 5

### Phase 79: Crank/Rank Tables [x]
**Goal**: Build SPT-crank, overpartition crank, and M2-statistic tables from existing rank/crank infrastructure
**Depends on**: Phase 76 (partition predicates), existing rankgf/crankgf
**Requirements**: CRANK-01, CRANK-02, CRANK-03, CRANK-04, CRANK-05
**Success Criteria** (what must be TRUE):
  1. `NS(0, 5)` returns the correct SPT-crank count for n=5, m=0
  2. `MBAR(1, 5)` returns the correct overpartition crank count
  3. `M2N(0, 10)` returns M2-rank zero count for partitions of 10 without repeated odd parts
  4. Overpartition number computation matches OEIS A015128: pbar(5) = 24
  5. Residue GF `GFDM2N(0,1,5,30)` produces a valid series

### Phase 80: Bailey Chains [x]
**Goal**: Implement Bailey pair machinery with a catalog of known pairs
**Depends on**: None (uses existing aqprod)
**Requirements**: BAILEY-01, BAILEY-02, BAILEY-03
**Success Criteria** (what must be TRUE):
  1. `betafind(unit_alpha, n)` correctly recovers beta for the unit Bailey pair
  2. `alphaup`/`alphadown` are inverses: `alphadown(alphaup(alpha, n), n)` = alpha(n)
  3. At least 3 known Bailey pairs in the catalog (unit, Rogers-Ramanujan, Göllnitz-Gordon)
  4. Bailey chain operations produce correct series when summed

### Phase 81: ETA Cusp Theory [x]
**Goal**: Cusp computation for Gamma_0(N), Newman's theorem check, order computation at cusps
**Depends on**: None (pure number theory)
**Requirements**: ETA-01, ETA-02, ETA-03, ETA-04, ETA-05, ETA-06
**Success Criteria** (what must be TRUE):
  1. `cuspmake(12)` returns the correct set of inequivalent cusps for Gamma_0(12)
  2. `gammacheck([1,-1,2,1], 2)` correctly verifies Newman's conditions for η(τ)^{-1}η(2τ)
  3. `cuspord([1,24], [1,0])` = 1 for η(τ)^24 at cusp 0 (infinity)
  4. `etaprodtoqseries([1,24], 30)` matches `q * etaq(1,30)^24` (Delta function)
  5. `vp(72, 2)` = 3 (p-adic valuation)

### Phase 82: ETA Identity Prover [x]
**Goal**: Automated proof of eta-quotient identities on Gamma_0(N) via Sturm bound
**Depends on**: Phase 81 (cusp theory)
**Requirements**: ETA-07, ETA-08
**Success Criteria** (what must be TRUE):
  1. ✓ `provemodfuncGAMMA0id` proves trivial and eta-quotient identities
  2. ✓ Sturm bound from mintotGAMMA0ORDS; verification to that depth
  3. ✓ Batch mode: `provemodfuncGAMMA0idBATCH(etaids, N)`
  4. U_p operator: `provemodfuncGAMMA0UpETAid` stub in place; full proof deferred

### Phase 83: Modular Forms [x]
**Goal**: Construct bases for spaces of modular forms M_k(SL_2(Z))
**Depends on**: Phase 78 (EISENq)
**Requirements**: MF-01, MF-02, MF-03, MF-04
**Success Criteria** (what must be TRUE):
  1. ✓ `makebasisM(12, 50)` returns 2 basis elements (E4^3 and E6^2)
  2. ✓ `DELTA12(T)` = `q * etaq(1,T)^24`
  3. ✓ `makebasisM(4, 30)` returns 1 basis element (E4)
  4. ✓ `makebasisPX(k, T)` returns basis using partition P and Phiq(1,3,5)

### Phase 84: Theta IDs Cusp Theory [x]
**Goal**: Generalized eta-products, Gamma_1(N) cusps, Biagioli cusp order formula
**Depends on**: Phase 81 (ETA cusp theory as foundation)
**Requirements**: THETA-01, THETA-02, THETA-03, THETA-04, THETA-05
**Success Criteria** (what must be TRUE):
  1. ✓ `cuspmake1(5)` returns cusps for Gamma_1(5)
  2. ✓ `getacuspord(n, r, a, c)` Biagioli formula
  3. ✓ `Gamma1ModFunc(L, N)` validates generalized eta-products
  4. ✓ `QP2(1, 3)` = -1/18
  5. ✓ `eta2jac` and `jac2eprod` conversions

### Phase 85: Theta IDs Identity Prover [ ]
**Goal**: Prove Jacobi theta/generalized eta identities on Gamma_1(N)
**Depends on**: Phase 84 (Gamma_1 cusp theory)
**Requirements**: THETA-06
**Success Criteria** (what must be TRUE):
  1. `provemodfuncid` proves a known Jacobi theta identity
  2. The Gamma_1(N) Sturm bound is correctly computed
  3. At least 2 identities from theta_aids examples verified

### Phase 86: RR Identity Search [ ]
**Goal**: Rogers-Ramanujan/Göllnitz-Gordon functions and systematic identity search
**Depends on**: Phases 78 (newprodmake), 81 (ETA)
**Requirements**: RRID-01, RRID-02, RRID-03
**Success Criteria** (what must be TRUE):
  1. `RRG(1)` and `RRH(1)` match the standard Rogers-Ramanujan products
  2. `checkid(RRG(1)*RRH(1), 50)` identifies the product as an eta-quotient
  3. `findids(1, 20)` discovers at least 3 known type-1 identities
  4. `findids(2, 20)` discovers at least 2 known type-2 identities

**Plans:** 3 plans

Plans:
- [ ] 86-01-PLAN.md — RRG, RRH (n=1 first), RRGstar, RRHstar (n=1), geta helper, REPL commands
- [ ] 86-02-PLAN.md — checkid implementation, REPL integration
- [ ] 86-03-PLAN.md — findids for types 1 and 2 (GE/HE, ABCOND, SYMF), REPL integration

### Phase 87: Integration Testing [ ]
**Goal**: End-to-end verification of all v6.0 packages, full regression
**Depends on**: All v6.0 phases
**Requirements**: INT-01, INT-02
**Success Criteria** (what must be TRUE):
  1. All existing tests pass (maple-checklist, acceptance-*, integration-tcore)
  2. Cross-package: ETA identity prover + modular forms basis used together
  3. Mock theta + Bailey chain examples verified
  4. Partition statistics consistent with rank/crank GFs from Phase 69

**Plans:** 3/3 plans complete

Plans:
- [ ] 87-01-PLAN.md — Full regression: acceptance-all target, BIN fallbacks in integration-tcore.sh and acceptance-bailey.sh
- [ ] 87-02-PLAN.md — Cross-package: integration-eta-theta-modforms.sh, ETA + theta IDs + modforms chain
- [ ] 87-03-PLAN.md — Gap closure: BIN fallback in 9 run-all scripts (rank-crank, crank, mock, ptnstats, tcore, gsk, vectors, tcrank-display, partition-type)

---

## Milestone v7.0 (Maple Checklist Gaps) — phases 88–90

### Phase 88: quinprod prodid/seriesid
**Goal**: quinprod returns identity formulas when called with prodid or seriesid mode
**Depends on**: Phase 87 (Integration Testing)
**Requirements**: GAP-QP-01, GAP-QP-02
**Success Criteria** (what must be TRUE):
  1. `quinprod(z,q,prodid)` returns the quintuple product identity in product form (string formula)
  2. `quinprod(z,q,seriesid)` returns the quintuple product identity in series form (string formula)
  3. Maple checklist Blocks 30 and 31 pass
  4. help(quinprod) documents the prodid and seriesid modes
**Plans**: 1 plan

Plans:
- [ ] 88-01-PLAN.md — Add prodid/seriesid handling in quinprod, update Blocks 30–31, help(quinprod)

### Phase 89: List indexing
**Goal**: User can index into list results from findhom/findnonhom
**Depends on**: Phase 88
**Requirements**: GAP-IDX-01
**Success Criteria** (what must be TRUE):
  1. `x[1]` returns the first element when x is the result of findhom or findnonhom
  2. `x[n]` returns the nth element (1-based indexing); `x[2]`, `x[3]`, etc. work
  3. Out-of-range index throws a clear error (not crash)
  4. Maple checklist Block 21 passes
**Plans**: 1 plan

Plans:
- [ ] 89-01-PLAN.md — RelationKernelResult in EnvValue; parser Subscript; eval subscript; Block 21 real test

### Phase 90: Symbolic z tripleprod/quinprod
**Goal**: tripleprod and quinprod accept symbolic z and return bivariate series in z and q
**Depends on**: Phase 89
**Requirements**: GAP-SYM-01, GAP-SYM-02
**Success Criteria** (what must be TRUE):
  1. `tripleprod(z,q,T)` with symbolic z returns bivariate series (Laurent in z, power series in q)
  2. `quinprod(z,q,T)` with symbolic z returns bivariate series (Laurent in z, power series in q)
  3. Display format supports both z and q exponents (e.g., coefficients as polynomials in z^±1)
  4. Maple checklist Blocks 28 and 32 pass
**Plans**: 2 plans

Plans:
- [ ] 90-01-PLAN.md — BivariateSeries type + tripleprod_symbolic + quinprod_symbolic
- [ ] 90-02-PLAN.md — REPL symbolic z dispatch, display, Blocks 28/32 pass

### v7.0 Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 88. quinprod prodid/seriesid | 0/? | Complete    | 2026-03-03 |
| 89. List indexing | 0/1 | Complete    | 2026-03-03 |
| 90. Symbolic z tripleprod/quinprod | 0/? | Complete    | 2026-03-03 |

---

## Milestone v8.0 (RootOf Function) — phases 91–94

- [x] **Phase 91: Omega3 type** — omega3.h: struct { Frac a, b }, arithmetic, inverse, str(), ω^k reduction (ROOT-01..05) (completed 2026-03-03)
- [x] **Phase 92: SeriesOmega** — std::map<int, Omega3>, add, mul, truncation; Omega3 * Series → SeriesOmega (ROOT-06..08) (completed 2026-03-03)
- [x] **Phase 93: Parser/REPL integration** — omega, RootOf(3), sum(omega^expr), EnvValue for omega (ROOT-09..12) (completed 2026-03-03)
- [x] **Phase 94: b(q) and Block 10** — b(q) via sum or eta identity; maple-checklist Block 10 parity (ROOT-13..14) (completed 2026-03-03)

### Phase 91: Omega3 type
**Goal**: Q(ω) scalar type with ω² = -ω-1 — arithmetic, division, power reduction, display
**Depends on**: Phase 90
**Requirements**: ROOT-01, ROOT-02, ROOT-03, ROOT-04, ROOT-05
**Success Criteria** (what must be TRUE):
  1. Omega3(1,0) + Omega3(0,1) yields correct a+bω; omega*omega reduces via ω² = -ω-1
  2. Division of nonzero Omega3 by itself yields 1; division by zero throws
  3. omega^3 reduces to 1; omega^k reduces to ω^(k mod 3)
  4. str(Omega3(0,1)) displays "omega"; str(Omega3(-1,-1)) displays "omega^2"; str(Omega3(1,2)) displays "1 + 2*omega"
  5. omega + omega^2 = -1 (cyclotomic identity)
**Plans**: 1 plan

Plans:
- [ ] 91-01-PLAN.md — omega3.h: struct, arithmetic (+, -, *, /), inverse, pow reduction, str(); unit tests in main.cpp

### Phase 92: SeriesOmega
**Goal**: Series with Q(ω) coefficients — add, mul, truncation; mixed Omega3 * Series
**Depends on**: Phase 91
**Requirements**: ROOT-06, ROOT-07, ROOT-08
**Success Criteria** (what must be TRUE):
  1. SeriesOmega can be constructed with Omega3 coefficients at various exponents
  2. Adding two SeriesOmega with same truncation produces correct result; truncation propagates in add/mul
  3. omega * (1 + q) produces SeriesOmega with omega at exponent 0 and omega at exponent 1
  4. mul truncation: product of two SeriesOmega truncated at T has no terms beyond T-1
  5. Omega3 * Series(f) produces SeriesOmega where each coeff is Omega3(scalar) * Frac
**Plans**: 1 plan

Plans:
- [ ] 92-01-PLAN.md — series_omega.h: SeriesOmega (map&lt;int,Omega3&gt;, trunc), add, mul, scalar multiply, truncTo, Omega3*Series; unit tests for 5 success criteria

### Phase 93: Parser/REPL integration
**Goal**: omega symbol, RootOf dispatch, sum(omega^expr), variable assignment for omega
**Depends on**: Phase 92
**Requirements**: ROOT-09, ROOT-10, ROOT-11, ROOT-12
**Success Criteria** (what must be TRUE):
  1. Typing `omega` in REPL returns Omega3(0,1) and displays as omega
  2. RootOf(3) and RootOf([1,1,1]) both return Omega3(0,1) ([1,1,1] = coeffs of z²+z+1)
  3. sum(omega^n, n, 0, 2) evaluates to 1 + omega + omega^2 = 0
  4. omega := RootOf(z^2+z+1=0) stores; subsequent `omega` or `omega*2` uses stored value
  5. omega^2 displays correctly when evaluated
**Plans**: 1 plan

Plans:
- [ ] 93-01-PLAN.md — EnvValue/EvalResult Omega3+SeriesOmega, Var lookup, RootOf(3), BinOp omega^int and Omega3*Series, Sum Omega3 body, Assign, display, acceptance-omega.sh

### Phase 94: b(q) and Block 10
**Goal**: b(q) computation and maple-checklist Block 10 parity
**Depends on**: Phase 93
**Requirements**: ROOT-13, ROOT-14
**Success Criteria** (what must be TRUE):
  1. b(q) via sum(omega^(n-m)*q^(n²+nm+m²)) or eta identity η³/η(3τ) produces correct q-series
  2. b(q) matches eta identity: etaq(1,T)^3 / etaq(3,T) to truncation
  3. a(q), b(q), c(q) from qseriesdoc Exercise 4 can be computed and are consistent
  4. maple-checklist Block 10 (RootOf) no longer skips
  5. Acceptance test: b(q) coefficients for small exponents match known values
**Plans**: 1 plan

Plans:
- [ ] 94-01-PLAN.md — Sum(SeriesOmega) branch, toRationalSeries, b(q) double-sum match, Block 10 real test, a(q)/c(q) acceptance

### v8.0 Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 91. Omega3 type | 0/1 | Complete    | 2026-03-03 |
| 92. SeriesOmega | 0/1 | Complete    | 2026-03-03 |
| 93. Parser/REPL integration | 0/? | Complete    | 2026-03-03 |
| 94. b(q) and Block 10 | 0/? | Complete    | 2026-03-03 |

---

## Milestone v9.0 (factor(t8) + Close Block 4) — phases 95–96

- [x] **Phase 95: factor builtin** — polyfactor logic (FactorResult, cyclotomic factorization), REPL dispatch factor(expr) (completed)
- [x] **Phase 96: Block 4 parity** — replace skip with real test; factor(t8) must produce cyclotomic output (completed)

---

## Milestone v10.0 (Close Remaining Gaps) — phases 97–99

- [ ] **Phase 97: Block 25 fix** — q-shift normalization for series addition before findpoly; Block 25 passes
- [ ] **Phase 98: Block 24** — collect-style formatter or N/A documentation; Block 24 test/rationale
- [ ] **Phase 99: findlincombomodp** — linear combo mod p; REPL dispatch; help entry

### Phase 95: factor builtin
**Goal**: User can call factor(expr) when expr→Series; output is cyclotomic factorization
**Depends on**: Phase 94
**Requirements**: FACTOR-01, FACTOR-02, FACTOR-03
**Success Criteria** (what must be TRUE):
  1. factor(expr) evaluates expr to Series, then factors the univariate polynomial in q into cyclotomic form
  2. FactorResult struct: map<int,int> cyclotomic (n → exponent of Φ_n), Frac content; display uses Φ_n notation
  3. Cyclotomic factorization via qfactor→cyclotomic expansion (Approach A) or direct Φ_n extraction — sufficient for T(8,8)
  4. REPL dispatch: factor(expr) is a built-in; help(factor) documents usage
  5. qseriesdoc Output (4) style: factor(t8) produces cyclotomic product
**Plans**: 1 plan

Plans:
- [ ] 95-01-PLAN.md — FactorResult + factor impl, formatFactor, REPL dispatch + help, acceptance-factor.sh

### Phase 96: Block 4 parity
**Goal**: maple-checklist Block 4 no longer skips; factor(t8) produces cyclotomic output matching qseriesdoc
**Depends on**: Phase 95
**Requirements**: BLOCK4-01
**Success Criteria** (what must be TRUE):
  1. Block 4 in tests/maple-checklist.sh replaces skip with real test (t8 := T(8,8); factor(t8))
  2. factor(t8) returns cyclotomic factorization (Φ_n product form)
  3. Output matches qseriesdoc Output (4) style
  4. Block 5 (qfactor(t8,20)) continues to pass — no regression
**Plans**: 1 plan

Plans:
- [ ] 96-01-PLAN.md — Replace Block 4 skip with factor(t8) cyclotomic test

### v9.0 Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 95. factor builtin | 1/1 | Complete | 2026-03-03 |
| 96. Block 4 parity | 1/1 | Complete | 2026-03-03 |

### Phase 97: Block 25 fix
**Goal**: findpoly works on theta2/theta3 quotients; series with different q-shifts are normalized before polynomial relation finding
**Depends on**: Phase 96
**Requirements**: BLOCK25-01, BLOCK25-02
**Success Criteria** (what must be TRUE):
  1. Series addition normalizes q_shift differences (using normalize_q_shift or equivalent) so findpoly receives aligned series
  2. findpoly(x, y, q, 3, 1, 60) on theta quotients produces polynomial relation
  3. maple-checklist Block 25 passes (no skip)
  4. No regressions in existing findpoly or Block tests
**Plans**: 1 plan

Plans:
- [ ] 97-01-PLAN.md — Modify Series operator+ for q-shift alignment; Block 25 passes; no regressions

### Phase 98: Block 24
**Goal**: Block 24 either implements collect-style grouping for relation terms OR documents N/A and keeps explicit skip rationale
**Depends on**: Phase 97
**Requirements**: BLOCK24-01, BLOCK24-02
**Success Criteria** (what must be TRUE):
  1. User can group relation terms by variable (collect-style formatter) — OR — Block 24 is documented as N/A with clear rationale
  2. maple-checklist Block 24 has either a real test or an explicit N/A rationale in the test file
  3. No regressions in related relation-finding or Block tests
**Plans**: 1 plan

Plans:
- [ ] 98-01-PLAN.md — Document Block 24 as N/A with explicit rationale; verify no regressions

### Phase 99: findlincombomodp
**Goal**: User can express a series as linear combination of others mod p; complements findhommodp for degree-1 relations
**Depends on**: Phase 98
**Requirements**: MODP-01, MODP-02
**Success Criteria** (what must be TRUE):
  1. findlincombomodp(f, L, p, T) returns coefficients in F_p such that f ≡ Σ c_i * L_i (mod p)
  2. Uses existing modp, F_p linalg (Gaussian elimination over F_p) and findhommodp infrastructure
  3. REPL dispatch: findlincombomodp is a built-in; help(findlincombomodp) documents usage
  4. Acceptance test or worksheet example demonstrates correct linear combo mod p
**Plans**: 1 plan

Plans:
- [ ] 99-01-PLAN.md — solve_modp, findlincombomodp, REPL dispatch, formatFindlincombomodp, acceptance-modp.sh

### Phase 100: v10.0 Phase Verification Documentation (Gap Closure)
**Goal:** Add VERIFICATION.md to phases 97, 98, 99 to close audit blocker (unverified phases)
**Depends on:** Phases 97, 98, 99 (implemented)
**Gap Closure:** Phases 97, 98, 99 lack VERIFICATION.md — blocks milestone audit pass
**Success Criteria** (what must be TRUE):
  1. `.planning/phases/97-block-25-fix/97-VERIFICATION.md` exists; status passed; Block 25 and addAligned verified
  2. `.planning/phases/98-block-24/98-VERIFICATION.md` exists; status passed; Block 24 N/A rationale verified
  3. `.planning/phases/99-findlincombomodp/99-VERIFICATION.md` exists; status passed; findlincombomodp verified
  4. Re-running `/gsd:audit-milestone` yields status `passed` or `tech_debt` (no phases gap)
**Plans:** 0/1 plans complete

Plans:
- [ ] 100-01-PLAN.md — Add 97-VERIFICATION.md, 98-VERIFICATION.md, 99-VERIFICATION.md from summaries + integration check

### v10.0 Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 97. Block 25 fix | 0/? | Not started | - |
| 98. Block 24 | 0/? | Not started | - |
| 99. findlincombomodp | 0/? | Not started | - |
| 100. v10.0 Phase Verification (gap closure) | 0/? | Complete    | 2026-03-05 |

---

## Milestone v11.0 (Modular Forms) — phases 101–102

- [ ] **Phase 101: makeALTbasisM** — Alternative basis of M_k(SL_2(Z)) using Delta_12
- [ ] **Phase 102: Cross-Package Integration** — ETA + theta IDs + modforms identity proofs; integration-eta-theta-modforms.sh

### Phase 101: makeALTbasisM
**Goal:** User can call makeALTbasisM(k, T) — returns basis of M_k(SL_2(Z)) using Delta_12 instead of E4/E6
**Depends on:** Phase 83 (existing makebasisM, DELTA12)
**Requirements:** MF-02
**Success Criteria** (what must be TRUE):
  1. makeALTbasisM(k, T) returns a basis of M_k(SL_2(Z)) for k ≥ 0 even (k=2 → empty)
  2. Basis uses Delta_12 and complementary Eisenstein/monomials (Es * E6^(2r-2i) * DELTA12^i)
  3. Output spans same space as makebasisM(k, T) (verified by dimension match)
  4. REPL dispatch and help(makeALTbasisM) document usage

**Plans:** 1 plan

Plans:
- [ ] 101-01-PLAN.md — Implement makeALTbasisM in modforms.h, REPL dispatch + help, acceptance tests

### Phase 102: Cross-Package Integration
**Goal:** ETA identity prover + theta IDs + modforms work together; integration test passes
**Depends on:** Phase 101 (optional), Phases 81–84 (ETA, theta IDs, modforms)
**Requirements:** INT-02
**Success Criteria** (what must be TRUE):
  1. integration-eta-theta-modforms.sh exists and passes
  2. At least one end-to-end flow: eta identity + theta ID + modular form basis used in a single proof chain
  3. No regressions in maple-checklist or existing acceptance tests

### v11.0 Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 101. makeALTbasisM | 0/1 | Not started | - |
| 102. Cross-Package Integration | 0/? | Not started | - |

