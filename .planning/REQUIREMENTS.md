# Requirements: Q-Series REPL

**Defined:** 2025-02-24  
**Core Value:** Rogers-Ramanujan must work — prodmake recovers product form with denominators at exponents ≡ ±1 (mod 5)

## v1 Requirements

Requirements for initial release. Each maps to roadmap phases.

### Core (Foundation)

- [ ] **CORE-01**: BigInt: arbitrary precision integer, base 10⁹, signed, vector<uint32_t>
- [ ] **CORE-02**: BigInt: +, -, *, divmod, gcd; long division with binary-search quotient
- [ ] **CORE-03**: Frac: exact rational (BigInt num, den); reduce() after every construction and operation
- [ ] **CORE-04**: Series: truncated power series (std::map<int,Frac>, int trunc)
- [ ] **CORE-05**: Series: +, -, *, /, pow, inverse (recurrence j=1..n), subs_q; truncation propagation
- [ ] **CORE-06**: Number theory helpers: divisors, mobius, legendre, sigma, euler_phi

### Q-Functions

- [ ] **QFUNCS-01**: aqprod — rising q-factorial (a;q)_n
- [ ] **QFUNCS-02**: qbin — Gaussian polynomial [n;m]_q
- [ ] **QFUNCS-03**: etaq — eta product Π(1-q^{kn})
- [ ] **QFUNCS-04**: theta2, theta3, theta4 — theta functions
- [ ] **QFUNCS-05**: tripleprod, quinprod — triple and quintuple products
- [ ] **QFUNCS-06**: winquist — Winquist identity

### Conversion

- [ ] **CONVERT-01**: prodmake — Andrews' algorithm, series → infinite product
- [ ] **CONVERT-02**: etamake — eta product identification
- [ ] **CONVERT-03**: jacprodmake — Jacobi product identification (80% periodicity, verify)
- [ ] **CONVERT-04**: qfactor — finite q-product factorization
- [ ] **CONVERT-05**: sift — coefficient extraction a_{ni+k}

### Linear Algebra

- [ ] **LINALG-01**: Gaussian elimination over Q
- [ ] **LINALG-02**: kernel computation for rational matrices

### Relations

- [ ] **REL-01**: findhom — homogeneous polynomial relations
- [ ] **REL-02**: findnonhom — nonhomogeneous polynomial relations
- [ ] **REL-03**: findhomcombo, findnonhomcombo — express f as polynomial in list
- [ ] **REL-04**: findpoly — polynomial relation between two series

### Parser and REPL

- [ ] **REPL-01**: Parser: tokenizer + recursive-descent expression parser (Maple-like syntax)
- [ ] **REPL-02**: Variable environment, assignment (:=)
- [ ] **REPL-03**: sum/add — summation over index
- [ ] **REPL-04**: Built-in function dispatch (aqprod, etaq, prodmake, etc.)
- [ ] **REPL-05**: series/coeffs display, set_trunc
- [ ] **REPL-06**: REPL loop with basic history

### Acceptance Tests

- [ ] **TEST-01**: Rogers-Ramanujan — prodmake yields denominators at ±1 mod 5
- [ ] **TEST-02**: Partition function — 1/etaq(1,50) coefficients correct
- [ ] **TEST-03**: Theta as eta — etamake(theta3), etamake(theta4) match SPEC
- [ ] **TEST-04**: Jacobi product — jacprodmake on Rogers-Ramanujan → JAC(0,5,∞)/JAC(1,5,∞)
- [ ] **TEST-05**: Gauss AGM — findhom yields X₁²+X₂²-2X₃², -X₁X₂+X₄²
- [ ] **TEST-06**: Sift + eta — Rødseth identity
- [ ] **TEST-07**: qfactor — T(8,8) factorization
- [ ] **TEST-08**: Watson's modular equation — findnonhomcombo
- [ ] **TEST-09**: Euler pentagonal dissection — jacprodmake on sifted etaq(1)

## v1.1 Requirements (Garvan Demo)

Requirements for milestone v1.1 — demo using Frank Garvan's qseriesdoc examples.

### Demo

- [ ] **DEMO-01**: Demo artifact exists — script or doc that runs qseries commands to reproduce qseriesdoc examples
- [ ] **DEMO-02**: Rogers-Ramanujan example — sum + prodmake, matches Output (1) and (2) style from qseriesdoc §3.1
- [ ] **DEMO-03**: Product conversion examples — qfactor T(8,8), etamake(theta2/3/4), jacprodmake on Rogers-Ramanujan (qseriesdoc §3.2, §3.3, §3.4)
- [ ] **DEMO-04**: Relations examples — findhom (Gauss AGM), findhomcombo, findnonhomcombo (qseriesdoc §4)
- [ ] **DEMO-05**: Sifting and product identities — sift, triple product, quintuple product, Winquist (qseriesdoc §5, §6)

## v1.2 Requirements (More QoL)

Requirements for milestone v1.2 — quality-of-life improvements.

### Script Mode

- [ ] **QOL-01**: User can run `qseries < script.qs` to execute a file of commands non-interactively

### Help

- [ ] **QOL-02**: User can type `help` to see general help and list of built-ins
- [ ] **QOL-03**: User can type `help(func)` to see documentation for a specific built-in function

### Timing

- [ ] **QOL-04**: User sees elapsed time for each command (ms or s)

### Multi-line Input

- [ ] **QOL-05**: User can use backslash continuation to split long expressions across lines

### Tab Completion

- [ ] **QOL-06**: User can press Tab to autocomplete identifiers and built-in function names

### Error Messages

- [ ] **QOL-07**: Parse and runtime errors show clearer diagnostics (e.g., line/column, expected token, function name)

### Demo Packaging

- [ ] **QOL-08**: User can obtain a single folder (binary + demo script + README) for sharing or distribution

## v1.4 Requirements (Close more gaps)

Requirements for milestone v1.4 — additional Maple qseries gap-closure from FEATURE-GAPS.md.

### Product Conversion

- [ ] **GAP-01**: User can call `mprodmake(f,T)` to convert q-series f to product (1+q^n1)*(1+q^n2)*...

### Validation Utilities

- [ ] **GAP-02**: User can call `checkprod(f,T)` to check whether q-series f is a "nice" product
- [ ] **GAP-03**: User can call `checkmult(f,T)` to check whether coefficients of f are multiplicative

### Relations

- [ ] **GAP-04**: User can call `findmaxind(L,n,topshift)` to find a maximal independent subset of q-series in list L

---

## v1.5 Requirements (REPL improvements)

Requirements for milestone v1.5 — REPL UX: suppress output, arrow-key navigation, optional-arg variants.

### Output Suppression

- [ ] **REPL-OPTS-01**: User can suppress output by ending a statement with semicolon (e.g. `rr := sum(...);` stores without printing)

### Line Navigation

- [ ] **REPL-OPTS-02**: User can move cursor left/right within the input line using arrow keys (when stdin is TTY)

### Optional Arguments

- [ ] **REPL-OPTS-03**: All 2-arg and 3-arg function variants work; omitted optional args use defaults (env.T, etc.) — audit series, etaq, qfactor, jac2series, checkprod, checkmult, findmaxind

---

## v2 Requirements

Deferred to future release.

### Deferred

- **OPT-01**: Memoization of etaq(k,T)
- **OPT-02**: Euler pentagonal optimization for etaq(1,T)
- **OPT-03**: LaTeX output

## Out of Scope

| Feature | Reason |
|---------|--------|
| Floating point in math pipeline | Exact rationals only; corrupts coefficients |
| External libraries (GMP, Boost) | Zero-dependency constraint |
| General-purpose CAS | Scope: q-series, partition theory, modular forms only |
| Full readline/libedit | External dependency; simple history sufficient |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| CORE-01 | Phase 1 | Pending |
| CORE-02 | Phase 1 | Pending |
| CORE-03 | Phase 2 | Pending |
| CORE-04 | Phase 3 | Pending |
| CORE-05 | Phase 3 | Pending |
| CORE-06 | Phase 4 | Pending |
| QFUNCS-01 | Phase 4 | Pending |
| QFUNCS-02 | Phase 4 | Pending |
| QFUNCS-03 | Phase 4 | Pending |
| QFUNCS-04 | Phase 4 | Pending |
| QFUNCS-05 | Phase 4 | Pending |
| QFUNCS-06 | Phase 4 | Pending |
| CONVERT-01 | Phase 5 | Pending |
| CONVERT-02 | Phase 6 | Pending |
| CONVERT-03 | Phase 6 | Pending |
| CONVERT-04 | Phase 6 | Pending |
| CONVERT-05 | Phase 6 | Pending |
| LINALG-01 | Phase 7 | Pending |
| LINALG-02 | Phase 7 | Pending |
| REL-01 | Phase 8 | Pending |
| REL-02 | Phase 8 | Pending |
| REL-03 | Phase 8 | Pending |
| REL-04 | Phase 8 | Pending |
| REPL-01 | Phase 9 | Pending |
| REPL-02 | Phase 10 | Pending |
| REPL-03 | Phase 10 | Pending |
| REPL-04 | Phase 10 | Pending |
| REPL-05 | Phase 10 | Pending |
| REPL-06 | Phase 10 | Pending |
| TEST-01 | Phase 5 | Pending |
| TEST-02 | Phase 4 | Pending |
| TEST-03 | Phase 6 | Pending |
| TEST-04 | Phase 6 | Pending |
| TEST-05 | Phase 8 | Pending |
| TEST-06 | Phase 6 | Pending |
| TEST-07 | Phase 6 | Pending |
| TEST-08 | Phase 8 | Pending |
| TEST-09 | Phase 6 | Pending |
| DEMO-01 | Phase 11 | Pending |
| DEMO-02 | Phase 12 | Pending |
| DEMO-03 | Phase 13 | Pending |
| DEMO-04 | Phase 14 | Pending |
| DEMO-05 | Phase 15 | Pending |
| QOL-01 | Phase 16 | Pending |
| QOL-02 | Phase 17 | Pending |
| QOL-03 | Phase 17 | Pending |
| QOL-04 | Phase 18 | Pending |
| QOL-05 | Phase 19 | Pending |
| QOL-06 | Phase 20 | Pending |
| QOL-07 | Phase 21 | Pending |
| QOL-08 | Phase 22 | Pending |
| GAP-01 | Phase 24 | Pending |
| GAP-02 | Phase 25 | Pending |
| GAP-03 | Phase 25 | Pending |
| GAP-04 | Phase 26 | Pending |
| REPL-OPTS-01 | Phase 27 | Pending |
| REPL-OPTS-02 | Phase 28 | Pending |
| REPL-OPTS-03 | Phase 29 | Pending |
| PERF-01 | Phase 32 | Pending |
| PERF-02 | Phase 33 | Pending |
| PERF-03 | Phase 32, 33 | Pending |

**Coverage:**
- v1 requirements: 38 total
- v1.1 (DEMO) requirements: 5 total
- v1.2 (QOL) requirements: 8 total
- v1.4 (GAP) requirements: 4 total
- v1.5 (REPL-OPTS) requirements: 3 total
- v1.6 (PERF) requirements: 3 total
- Mapped to phases: 3/3 v1.6 PERF requirements ✓

---

## v1.6 Requirements (etaq Performance)

Requirements for milestone v1.6 — etaq memoization and pentagonal optimization.

### Performance

- [ ] **PERF-01**: `etaq(k,T)` results are memoized — repeated calls with the same `(k,T)` return cached Series without recomputation
- [ ] **PERF-02**: `etaq(1,T)` uses Euler's pentagonal number theorem recurrence, reducing complexity from O(T^2) to O(T*sqrt(T))
- [ ] **PERF-03**: All existing acceptance tests still pass after optimizations (no regressions)

---

## v1.7 Requirements (Polish & Documentation)

Requirements for milestone v1.7 — compiler warning cleanup and user-facing documentation.

### Polish

- [ ] **POLISH-01**: Build with `-Wall -Wextra -Wpedantic` produces zero warnings
- [ ] **POLISH-02**: All existing acceptance tests still pass after warning fixes (no regressions)

### Documentation

- [ ] **DOC-01**: `MANUAL.md` exists with getting-started guide, all built-in functions documented with signatures, descriptions, and examples
- [ ] **DOC-02**: `MANUAL.md` includes workflow tutorials (Rogers-Ramanujan walkthrough, product identification, relation finding)

---

## Traceability (v1.7)

| Requirement | Phase | Status |
|-------------|-------|--------|
| POLISH-01 | Phase 34 | Pending |
| POLISH-02 | Phase 34 | Pending |
| DOC-01 | Phase 35 | Pending |
| DOC-02 | Phase 35 | Pending |

---

## v1.8 Requirements (More Math Functions)

Requirements for milestone v1.8 — expose internal helpers, add convenience functions, and math enrichment.

### Expose Internal Helpers

- [ ] **MATH-01**: `divisors(n)` exposed as REPL built-in — returns sorted list of positive divisors
- [ ] **MATH-02**: `mobius(n)` exposed as REPL built-in — returns Mobius function μ(n)
- [ ] **MATH-03**: `euler_phi(n)` exposed as REPL built-in — returns Euler's totient φ(n)

### Convenience Functions

- [ ] **MATH-04**: `coeff(f, n)` — extract single coefficient of q^n from series f
- [ ] **MATH-05**: `dissect(f, m, T)` — return list of all m siftings [sift(f,m,0,T), ..., sift(f,m,m-1,T)]
- [ ] **MATH-06**: `jacobi(a, n)` — Jacobi symbol (a/n) generalizing Legendre to composite odd n
- [ ] **MATH-07**: `kronecker(a, n)` — Kronecker symbol generalizing Jacobi to all integers n

### Math Enrichment

- [ ] **MATH-08**: `eisenstein(k, T)` — normalized Eisenstein series E_{2k}(q) = 1 - (4k/B_{2k}) Σ σ_{2k-1}(n) q^n
- [ ] **MATH-09**: `partition(n)` — direct computation of partition number p(n)
- [ ] **MATH-10**: `qdiff(f)` — formal q-derivative of series f

### Regression

- [ ] **MATH-11**: All existing acceptance tests still pass after additions

---

## Traceability (v1.8)

| Requirement | Phase | Status |
|-------------|-------|--------|
| MATH-01, MATH-02, MATH-03 | Phase 36 | Pending |
| MATH-04, MATH-05, MATH-06, MATH-07 | Phase 37 | Pending |
| MATH-08, MATH-09, MATH-10 | Phase 38 | Pending |
| MATH-11 | Phase 36, 37, 38 | Pending |

---

## v1.9 Requirements (Manual Update & Testing)

Requirements for milestone v1.9 — update documentation and add acceptance tests for v1.8 functions.

### Documentation

- [ ] **DOC-03**: MANUAL.md updated with all 10 new v1.8 built-ins (coeff, dissect, divisors, eisenstein, euler_phi, jacobi, kronecker, mobius, partition, qdiff) — each with signature, description, and example
- [ ] **DOC-04**: MANUAL.md version references updated to 1.9

### Testing

- [ ] **TEST-01**: Acceptance test script covering all 10 new v1.8 functions with correct expected output
- [ ] **TEST-02**: All existing acceptance tests still pass after additions

---

## Traceability (v1.9)

| Requirement | Phase | Status |
|-------------|-------|--------|
| DOC-03, DOC-04 | Phase 39 | Pending |
| TEST-01, TEST-02 | Phase 40 | Pending |

---
*Requirements defined: 2025-02-24*
*Last updated: 2026-02-27 after v1.9 requirements*
