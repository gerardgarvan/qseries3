# Q-Series REPL

## What This Is

A standalone single-binary REPL for q-series computation in C++20. Reimplements the core functionality of Frank Garvan's `qseries` Maple package — the standard tool for researchers in number theory, partition theory, and modular forms. Lets mathematicians build q-series from standard constructions, convert series to infinite product form (Andrews' algorithm), identify eta/theta/Jacobi products, find algebraic relations between series, and sift coefficients.

## Core Value

**Rogers-Ramanujan must work.** Given `Σ q^(n²) / (q;q)_n`, prodmake must recover the product form with denominators only at exponents ≡ ±1 (mod 5). If this test passes, the core math is correct. Everything else builds on it.

## Requirements

### Validated

(None yet — ship to validate)

### Active

- [ ] BigInt: arbitrary precision integer (base 10⁹, signed)
- [ ] Frac: exact rational (auto-reduced via GCD on every operation)
- [ ] Series: truncated power series (std::map<int,Frac>, truncation propagation)
- [ ] qfuncs: aqprod, qbin, etaq, theta2/3/4, tripleprod, quinprod, winquist
- [ ] convert: prodmake (Andrews' algorithm), etamake, jacprodmake, qfactor
- [ ] sift: coefficient extraction by residue mod n
- [ ] linalg: Gaussian elimination over Q, kernel computation
- [ ] relations: findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly
- [ ] number theory helpers: divisors, mobius, legendre, sigma, euler_phi
- [ ] parser: recursive-descent expression parser (Maple-like syntax)
- [ ] repl: REPL loop, variable environment, help system, history
- [ ] All SPEC acceptance tests pass (Rogers-Ramanujan, partition function, theta/eta identities, Jacobi product, Gauss AGM, sift+eta, qfactor, Watson's equation, Euler pentagonal)

### Out of Scope

- Memoization of etaq(k,T) — defer to post-v1 optimization
- Euler pentagonal optimization for etaq(1,T) — O(T·√T) acceptable for v1
- LaTeX output — plain text sufficient
- Script mode (qseries < file.qs) — REPL only for v1
- Tab completion — simple getline + history sufficient
- External libraries (GMP, Boost) — zero dependencies is non-negotiable

## Context

- **Reference**: qseriesdoc.md contains Frank Garvan's tutorial with worked examples. Every computation there is an acceptance test.
- **Target users**: Mathematicians doing q-series work who want a portable, dependency-free alternative to Maple.
- **Build order**: Bottom-up layers — BigInt → Frac → Series → qfuncs → convert → linalg → relations → parser → repl. Test each layer before the next.
- **Critical test**: Rogers-Ramanujan is the canary. If prodmake fails on it, the recurrence or divisor extraction is wrong.

## Constraints

- **Tech stack**: C++20, g++ with `-std=c++20 -O2 -static`. Single binary output.
- **Dependencies**: ZERO external libraries. No GMP, no Boost, no package managers. All bignum arithmetic built from scratch.
- **Arithmetic**: Exact rational throughout. No floating point anywhere in the mathematical pipeline.
- **Structure**: Header-only (.h files) + main.cpp. Everything inline for single-binary simplicity.
- **Performance**: etaq(1,200) < 0.5s; prodmake(f,100) < 2s; findhom(4 series, degree 2) < 5s. Binary < 2MB.

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Base 10⁹ for BigInt | Two digits fit in uint64_t for multiplication; decimal I/O trivial | — Pending |
| std::map for Series coefficients | Ordered iteration needed for display, minExp, prodmake | — Pending |
| Header-only architecture | Single translation unit, no linker complexity | — Pending |
| prodmake divisor sum excludes d=n | Andrews' algorithm: Σ_{d\|n,d<n} d·a[d] in step 3 | — Pending |
| jacprodmake 80% periodicity threshold | Don't demand exact match; verify reconstruction | — Pending |

---
*Last updated: 2025-02-24 after initialization*
