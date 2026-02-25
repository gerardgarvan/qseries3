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

## v2 Requirements

Deferred to future release.

### Deferred

- **REPL-07**: Script mode — `qseries < script.qs`
- **REPL-08**: help / help(func) — built-in documentation
- **REPL-09**: Timing per command
- **REPL-10**: Multi-line input (backslash continuation)
- **OPT-01**: Memoization of etaq(k,T)
- **OPT-02**: Euler pentagonal optimization for etaq(1,T)
- **OPT-03**: LaTeX output
- **OPT-04**: Tab completion

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
| (To be filled by roadmapper) | | |

**Coverage:**
- v1 requirements: 35 total
- Mapped to phases: 0
- Unmapped: 35 ⚠️

---
*Requirements defined: 2025-02-24*
*Last updated: 2025-02-24 after initial definition*
