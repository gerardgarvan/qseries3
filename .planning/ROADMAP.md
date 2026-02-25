# Roadmap: Q-Series REPL

## Overview

Build a zero-dependency C++20 REPL for q-series computation bottom-up: BigInt → Frac → Series → qfuncs → prodmake (canary) → convert-extended → linalg → relations → parser → repl. Rogers-Ramanujan (prodmake on sum q^(n²)/(q;q)_n yields denominators at ±1 mod 5) is the central acceptance gate. All 9 SPEC acceptance tests must pass when the REPL is complete.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

- [x] **Phase 1: BigInt** - Arbitrary precision integers, base 10⁹, signed (completed 2026-02-25)
- [x] **Phase 2: Frac** - Exact rationals with auto-reduce via GCD (completed 2026-02-25)
- [ ] **Phase 3: Series** - Truncated power series with inverse, truncation propagation
- [ ] **Phase 4: qfuncs** - aqprod, etaq, theta, qbin, products; number theory helpers
- [ ] **Phase 5: prodmake** - Andrews' algorithm; Rogers-Ramanujan canary test
- [ ] **Phase 6: convert-extended** - etamake, jacprodmake, qfactor, sift
- [ ] **Phase 7: linalg** - Gaussian elimination over Q, kernel computation
- [ ] **Phase 8: relations** - findhom, findnonhom, findpoly family
- [ ] **Phase 9: parser** - Tokenizer + recursive-descent expression parser
- [ ] **Phase 10: repl** - REPL loop, variables, dispatch, display; all 9 SPEC tests pass

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
**Plans**: TBD

Plans:
- [ ] 03-01: TBD

### Phase 4: qfuncs
**Goal**: q-series building blocks and number theory helpers
**Depends on**: Phase 3
**Requirements**: CORE-06, QFUNCS-01, QFUNCS-02, QFUNCS-03, QFUNCS-04, QFUNCS-05, QFUNCS-06, TEST-02
**Success Criteria** (what must be TRUE):
  1. aqprod, qbin, etaq produce correct coefficients (etaq(1,T) matches Euler pentagonal)
  2. theta2, theta3, theta4, tripleprod, quinprod, winquist work per SPEC
  3. divisors, mobius, legendre, sigma, euler_phi return correct values
  4. Partition function 1/etaq(1,50) coefficients match SPEC (TEST-02)
**Plans**: TBD

Plans:
- [ ] 04-01: TBD

### Phase 5: prodmake
**Goal**: Andrews' algorithm recovers infinite product from series; Rogers-Ramanujan works
**Depends on**: Phase 4
**Requirements**: CONVERT-01, TEST-01
**Success Criteria** (what must be TRUE):
  1. prodmake on Σ q^(n²)/(q;q)_n yields product with denominators only at exponents ≡ ±1 (mod 5)
  2. Divisor sum in step 3 excludes d=n; recurrence step 2 correct
  3. Rogers-Ramanujan is the canary — if this passes, core math is correct
**Plans**: TBD

Plans:
- [ ] 05-01: TBD

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
**Plans**: TBD

Plans:
- [ ] 06-01: TBD

### Phase 7: linalg
**Goal**: Linear algebra over Q for relation finding
**Depends on**: Phase 2 (Frac)
**Requirements**: LINALG-01, LINALG-02
**Success Criteria** (what must be TRUE):
  1. Gaussian elimination over vector<vector<Frac>> returns correct reduced form
  2. Kernel computation returns basis for null space of rational matrices
**Plans**: TBD

Plans:
- [ ] 07-01: TBD

### Phase 8: relations
**Goal**: Polynomial relation discovery between series
**Depends on**: Phase 7, Phase 6 (needs Series and convert for theta)
**Requirements**: REL-01, REL-02, REL-03, REL-04, TEST-05, TEST-08
**Success Criteria** (what must be TRUE):
  1. findhom yields X₁²+X₂²-2X₃², -X₁X₂+X₄² for Gauss AGM (TEST-05)
  2. findnonhom, findhomcombo, findnonhomcombo, findpoly work per SPEC
  3. Watson's modular equation recovered via findnonhomcombo (TEST-08)
**Plans**: TBD

Plans:
- [ ] 08-01: TBD

### Phase 9: parser
**Goal**: Parse Maple-like expressions into evaluable form
**Depends on**: Nothing (can develop in parallel with math layers)
**Requirements**: REPL-01
**Success Criteria** (what must be TRUE):
  1. Tokenizer handles identifiers, numbers, operators, := assignment
  2. Recursive-descent parser produces AST for expressions, assignments, function calls
  3. sum/add summation constructs parse correctly
**Plans**: TBD

Plans:
- [ ] 09-01: TBD

### Phase 10: repl
**Goal**: Full REPL with variable environment; all 9 SPEC acceptance tests pass
**Depends on**: Phases 4, 5, 6, 7, 8, 9
**Requirements**: REPL-02, REPL-03, REPL-04, REPL-05, REPL-06
**Success Criteria** (what must be TRUE):
  1. User can assign variables, evaluate expressions, invoke built-ins (aqprod, etaq, prodmake, etc.)
  2. sum/add dispatches correctly; set_trunc and series/coeffs display work
  3. REPL loop runs with basic history
  4. All 9 SPEC acceptance tests pass when run through the REPL
**Plans**: TBD

Plans:
- [ ] 10-01: TBD

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. BigInt | 0/3 | Complete    | 2026-02-25 |
| 2. Frac | 0/? | Complete    | 2026-02-25 |
| 3. Series | 0/? | Not started | - |
| 4. qfuncs | 0/? | Not started | - |
| 5. prodmake | 0/? | Not started | - |
| 6. convert-extended | 0/? | Not started | - |
| 7. linalg | 0/? | Not started | - |
| 8. relations | 0/? | Not started | - |
| 9. parser | 0/? | Not started | - |
| 10. repl | 0/? | Not started | - |
