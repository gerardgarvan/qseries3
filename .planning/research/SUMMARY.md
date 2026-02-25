# Project Research Summary

**Project:** Q-Series REPL (zero-dependency C++ mathematical software)
**Domain:** q-series / partition theory / modular forms computation
**Researched:** 2025-02-24
**Confidence:** HIGH

## Executive Summary

This is a zero-dependency C++ REPL for q-series computation (number theory, partition theory, modular forms). Experts build such tools as layered exact-arithmetic stacks: BigInt → Frac → Series → domain functions (eta, theta, products) → conversion (prodmake, etamake, jacprodmake) → relations (findhom, findpoly). The Maple qseries package (Frank Garvan) is the reference; this project reimplements its core in a single static binary.

**Recommended approach:** Build bottom-up with header-only C++20. Use base-10⁹ BigInt with schoolbook multiplication (no Karatsuba — numbers rarely exceed ~100 digits). Implement Frac with `reduce()` on every operation. Implement Andrews' prodmake as the central acceptance gate; Rogers-Ramanujan (sum → product with denominators at ±1 mod 5) is the litmus test. Build each layer, validate it, then move up. Do not skip validation: bugs in Series inverse or prodmake are the main failure modes.

**Key risks:** (1) Series inverse recurrence uses j=1..n, not j=0..n — off-by-one causes wrong coefficients. (2) prodmake divisor sum must exclude d=n. (3) Skipping Frac::reduce() causes exponential BigInt growth. Mitigate by unit-testing each layer before the next and running the 9 SPEC acceptance tests early and often.

---

## Key Findings

### Recommended Stack

All code in header files; single `main.cpp`; compile with `g++ -std=c++20 -O2 -static -Wall -Wextra -o qseries main.cpp`. No GMP, Boost, or package managers — roll BigInt and Frac from scratch for portability, reproducibility, and binary size (< 2MB target).

**Core technologies:**
- **GCC 11+ (13.3 recommended):** Full C++20 support; concepts, `[[nodiscard]]`, structured bindings
- **BigInt base 10⁹:** Digits in `std::vector<uint32_t>`, schoolbook multiply; long division with binary-search quotient per digit; reference: cp-algorithms, janmr.com
- **Frac:** `struct { BigInt num, den; }`; `reduce()` after every constructor and arithmetic op — never skip
- **Recursive-descent parser:** ~200–400 lines; no flex/bison/Spirit

**What not to use:** Karatsuba (premature), float/double in math pipeline, base 10 or 2⁶⁴ for BigInt, separate .cpp files.

### Expected Features

**Must have (table stakes):**
- Exact rational arithmetic (BigInt → Frac → Series), no floats
- **prodmake** — Andrews' algorithm; series → infinite product; Rogers-Ramanujan is canonical test
- etaq, aqprod, theta2/3/4, qbin — q-series building blocks
- etamake, jacprodmake — product identification (eta, Jacobi)
- sift, qfactor — coefficient extraction, finite factorization
- findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly — polynomial relations via kernel over Q
- tripleprod, quinprod, winquist — product identities
- Series arithmetic (+, -, *, /, inverse, pow, subs_q)
- REPL: variables, expressions, sum/add, assignment
- All 9 SPEC acceptance tests

**Should have (differentiators):**
- Zero deps, single binary — run anywhere
- Maple qseries-compatible syntax
- Built-in help, script mode (`qseries < script.qs`)

**Defer (v2+):**
- LaTeX output, tab completion, memoization, Euler pentagonal optimization

### Architecture Approach

Layered bottom-up: BigInt → Frac → Series → qfuncs (aqprod, etaq, theta, products) → convert (prodmake, etamake, jacprodmake) → linalg (kernel over Q) → relations → parser → repl. Data flows up: user input → parser → eval → domain functions → Series arithmetic. Use `std::map<int, Frac>` for Series (sparse, ordered iteration). Propagate truncation: `result.trunc = min(a.trunc, b.trunc)` for all binary ops.

**Major components:**
1. **bigint.h** — Arbitrary-precision integers; base 10⁹; schoolbook multiply/divide
2. **frac.h** — Exact rationals; `reduce()` after every op
3. **series.h** — Truncated power series; inverse recurrence (j=1..n); truncation propagation
4. **qfuncs.h** — etaq, aqprod, theta2/3/4, qbin, tripleprod, quinprod, winquist
5. **convert.h** — prodmake (Andrews), etamake, jacprodmake, qfactor
6. **linalg.h** — Gaussian elimination, kernel over Q
7. **relations.h** — findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly
8. **parser.h** — Recursive-descent expression parser
9. **repl.h** — REPL loop, variable env, help

### Critical Pitfalls

1. **Series inverse index j=1..n, not j=0..n** — Including j=0 makes the recurrence circular. Test: `(1-q)*(1/(1-q)) = 1` to truncation.

2. **prodmake divisor sum excludes d=n** — Use `Σ_{d|n, d<n} d·a[d]`; including d=n is self-referential. Test: Rogers-Ramanujan yields denominators only at ±1 mod 5.

3. **Frac::reduce() on every construction and op** — Skipping causes exponential BigInt growth. Test: 6/4 → 3/2; long chains stay small.

4. **BigInt division edge cases** — Dividend < divisor, negatives, base boundaries (10⁹/1, 999999999/10⁹). Use binary-search quotient digit.

5. **etamake max iterations** — Non-eta input loops forever. Cap iterations; report "not an eta product" on failure.

6. **jacprodmake 80% match** — Requiring 100% periodicity causes false negatives. Use ~80% match, then verify by reconstruction.

---

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: Foundation — bigint.h
**Rationale:** No other layer can proceed without BigInt. Division is the hardest operation.
**Delivers:** Arbitrary-precision integers; +, -, *, divmod, gcd; base 10⁹
**Avoids:** Division edge cases (binary-search quotient, remainder invariant)
**Verification:** Division test suite (0, negatives, base boundaries)

### Phase 2: Rationals — frac.h
**Rationale:** Series and all math require Frac.
**Delivers:** Exact rationals; reduce() after every op
**Avoids:** Exponential BigInt growth from unreduced Fracs
**Verification:** 6/4 → 3/2; 0/5 → 0/1; no growth in long chains

### Phase 3: Series — series.h
**Rationale:** qfuncs and convert operate on Series.
**Delivers:** Truncated power series; +, -, *, /, inverse, pow, subs_q; truncation propagation
**Avoids:** Inverse j=0 bug; truncation drift; mult without early trunc skip
**Verification:** `(1-q)*(1/(1-q)) = 1`; invert q·(1-q); skip pairs with e1+e2 ≥ trunc

### Phase 4: qfuncs — qfuncs.h
**Rationale:** prodmake and convert need etaq, aqprod, theta; prodmake is blocked until qfuncs exist.
**Delivers:** aqprod, etaq, theta2/3/4, qbin, tripleprod, quinprod, winquist; number-theory helpers (divisors, mobius, etc.)
**Avoids:** Truncation errors in subs_q; wrong pentagonal coefficients
**Verification:** etaq(1,T) matches Euler pentagonal; theta3 coefficients

### Phase 5: convert — prodmake, etamake, jacprodmake, qfactor
**Rationale:** prodmake is the central acceptance gate; etamake and jacprodmake depend on it.
**Delivers:** Andrews prodmake, etamake, jacprodmake, qfactor
**Avoids:** Divisor sum d=n; c[n] recurrence j=n; etamake infinite loop; jacprodmake 100% match
**Verification:** Rogers-Ramanujan → denominators at ±1 mod 5; jacprodmake on Rogers-Ramanujan; etamake failure on non-eta

### Phase 6: linalg — linalg.h
**Rationale:** relations need kernel over Q; linalg is independent of convert.
**Delivers:** Gaussian elimination, kernel computation over `vector<vector<Frac>>`
**Verification:** Kernel on small rational matrices

### Phase 7: relations — relations.h
**Rationale:** findhom/findnonhom family needs linalg and Series.
**Delivers:** findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly
**Avoids:** Matrix size blow-up (monomial count C(n+k-1,k-1))
**Verification:** Gauss AGM relations from theta3, theta4, etc.

### Phase 8: parser — parser.h
**Rationale:** REPL needs parsed expressions; parser is stateless and has no math deps.
**Delivers:** Tokenizer, recursive-descent parser; AST for expressions
**Verification:** Parse assignments, function calls, sum/add

### Phase 9: repl — repl.h + main.cpp
**Rationale:** Integrates parser with all math; final REPL.
**Delivers:** Read-eval-print loop; variable env; dispatch to qfuncs/convert/relations; help
**Verification:** All 9 SPEC acceptance tests; Rogers-Ramanujan, partition, theta→eta, Jacobi, Gauss AGM, Rødseth, qfactor, Watson, Euler pentagonal

### Phase Ordering Rationale

- **Dependencies:** BigInt → Frac → Series → qfuncs → convert; linalg depends only on Frac; relations needs linalg + Series.
- **Acceptance gate:** prodmake (Phase 5) is the primary gate; Rogers-Ramanujan must pass before claiming success.
- **Parallelization:** linalg can be developed in parallel with convert; parser in parallel with qfuncs.

### Research Flags

**Phases needing deeper research during planning:**
- **Phase 5 (convert):** jacprodmake periodicity detection and etamake iteration strategy — SPEC gives 80% threshold; may need algorithm refinement.
- **Phase 7 (relations):** findhom monomial enumeration and kernel scaling — C(n+k-1,k-1) grows fast; may need degree/list caps.

**Phases with standard patterns (skip research-phase):**
- **Phase 1 (bigint):** cp-algorithms, janmr.com provide clear algorithms.
- **Phase 2 (frac):** Standard GCD/reduce pattern.
- **Phase 3 (series):** Inverse recurrence and truncation rules are well-documented.
- **Phase 4 (qfuncs):** Formulas in SPEC and qseriesdoc.
- **Phase 6 (linalg):** Standard Gaussian elimination over Q.
- **Phase 8 (parser):** Recursive-descent is textbook.

---

## Confidence Assessment

| Area       | Confidence | Notes                                                                 |
|-----------|------------|-----------------------------------------------------------------------|
| Stack     | HIGH       | SPEC, .cursorrules, cp-algorithms, janmr.com; zero-deps constraint clear |
| Features  | HIGH       | SPEC acceptance tests, qseriesdoc, Maple qseries as reference          |
| Architecture | HIGH    | SPEC layer ordering; SymPy/FLINT/NTL confirm layered pattern           |
| Pitfalls  | HIGH       | SPEC and .cursorrules enumerate them; phase-to-pitfall mapping given   |

**Overall confidence:** HIGH

### Gaps to Address

- **jacprodmake threshold:** 80% match is specified; optimal range and verification strategy may need tuning during implementation.
- **findhom scaling:** Document monomial count; decide whether to cap degree/list size in REPL.
- **Windows static linking:** MinGW-w64 may pull libwinpthread; test on target.

---

## Sources

### Primary (HIGH confidence)
- SPEC.md — project specification, acceptance tests, architecture, common pitfalls
- .cursorrules — layer ordering, common bugs
- qseriesdoc.md — Frank Garvan q-series Maple package tutorial
- cp-algorithms Big Integer — base 10⁹, schoolbook algorithms
- janmr.com Multiple-Precision Long Division — quotient digit, remainder invariant

### Secondary (MEDIUM confidence)
- EmergentMind, Math Stack Exchange — series inverse recurrence
- GMP Rational Internals — GCD at each stage optimal
- GCC C++ Standards Support — C++20 feature availability
- Stack Overflow — schoolbook vs Karatsuba for moderate digit counts
- Andrews q-series, qseries.org — prodmake algorithm structure

### Tertiary (LOW confidence)
- SymPy, FLINT, NTL — architecture patterns (different ecosystem)

---
*Research completed: 2025-02-24*
*Ready for roadmap: yes*
