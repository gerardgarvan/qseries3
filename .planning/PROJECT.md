# Q-Series REPL

## What This Is

A standalone single-binary REPL for q-series computation in C++20. Reimplements the core functionality of Frank Garvan's `qseries` Maple package — the standard tool for researchers in number theory, partition theory, and modular forms. Lets mathematicians build q-series from standard constructions, convert series to infinite product form (Andrews' algorithm), identify eta/theta/Jacobi products, find algebraic relations between series, and sift coefficients.

## Core Value

**Rogers-Ramanujan must work.** Given `Σ q^(n²) / (q;q)_n`, prodmake must recover the product form with denominators only at exponents ≡ ±1 (mod 5). If this test passes, the core math is correct. Everything else builds on it.

## Current Milestone: v4.0 Core Improvements

**Goal:** Improve REPL user experience with colored output, smarter completion, session management, and boost performance with Karatsuba multiplication and faster series ops.

**Target features:**
- Colored/styled ANSI output (series, products, errors)
- Smarter tab completion (auto-parentheses, argument hints)
- Save/load session (variables and state)
- Clear screen command
- Faster series multiplication (optimized inner loop)
- BigInt Karatsuba multiplication
- Benchmarking suite

---

## Previous Milestone: v3.0 Simple Website & Example Audit ✓

**Shipped:** Deleted old Astro/Wasm website, created single-page kangaroo-banner HTML (53-function reference, 4 examples), audited all docs examples (6 fixes), pushed to GitHub.

---

## Previous Milestone: v2.1 Website ✓

**Shipped:** Astro Starlight docs site with 17 pages, KaTeX math, xterm.js Wasm playground, GitHub Actions CI/CD for Cloudflare Pages. Replaced by v3.0 simple page approach.

---

## Previous Milestone: v2.0 Robustness & Tutorial Coverage ✓

**Shipped:** 9 robustness fixes (etaq/sift guards, div-by-zero, parser depth, int overflow, pow limit, safe variant access, Levenshtein suggestions, integer assignment), T_rn memoization, min/max builtins, version 2.0

---

## Previous Milestone: v1.9 Manual Update & Testing ✓

**Shipped:** Documentation for 10 v1.8 built-ins, acceptance-v18.sh with 11 tests, version strings to 1.9

---

## Previous Milestone: v1.8 More Math Functions ✓

**Shipped:** divisors, mobius, euler_phi, coeff, dissect, jacobi, kronecker, eisenstein, partition, qdiff; fixed Series::str() double-negative display bug

---

## Previous Milestone: v1.7 Polish & Documentation ✓

**Shipped:** Zero-warning build under -Wall -Wextra -Wpedantic; comprehensive MANUAL.md with all 37 built-ins, 4 workflow tutorials

---

## Previous Milestone: v1.6 etaq Performance ✓

**Shipped:** etaq memoization, Euler pentagonal optimization (O(√T) for etaq(1,T))

---

## Previous Milestone: v1.5 REPL improvements ✓

**Shipped:** Suppress output (colon), arrow-key line navigation, optional-arg audit, output-on-next-line, up/down arrow history navigation

---

## Previous: v1.4 Close more gaps ✓

**Shipped:** mprodmake, checkprod, checkmult, findmaxind

---

## Previous: v1.3 Quick wins ✓

**Shipped:** version, qdegree, lqdegree, jac2series, findlincombo, Makefile CXX default, acceptance-wins

---

## Previous: v1.1 Garvan Demo ✓

**Shipped:** Demo artifact, Rogers-Ramanujan, product conversion, relations, sifting/product identities

---

## Requirements

### Validated

- v1.0: All 9 SPEC acceptance tests pass; REPL fully functional

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

- ~~Memoization of etaq(k,T)~~ — now in scope (v1.6)
- ~~Euler pentagonal optimization for etaq(1,T)~~ — now in scope (v1.6)
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
*Last updated: 2026-02-28 after v3.0 milestone start*
