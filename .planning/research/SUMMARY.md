# Project Research Summary

**Project:** q-series REPL — Half-Integer Jacobi Exponents & q-Shift Arithmetic Fixes
**Domain:** C++20 q-series computation (number theory / modular forms)
**Researched:** 2026-03-01
**Confidence:** HIGH

## Executive Summary

Three Maple checklist failures (Blocks 13, 14, 25) and three dependent exercise gaps (Exercises 4/b(q), 9/N(q), 10/findpoly) share two root causes: the Series arithmetic layer cannot compute fractional powers (needed for half-integer Jacobi product exponents), and integer-valued q-shifts are not absorbed into coefficient maps (blocking theta-function quotient addition). All fixes are internal to existing types — zero new files, zero new structs, zero external dependencies — requiring approximately 70–120 lines of changes across `series.h`, `convert.h`, `bigint.h`, and `frac.h`.

The recommended approach is bottom-up: first fix q-shift normalization (independent, low complexity, unblocks Block 25 and Exercise 10), then implement `Series::powFrac(Frac)` via generalized binomial coefficients over Q (enables half-integer Jacobi reconstruction), then fix the `jacprodmake` middle-element decomposition and `jac2series_impl` dispatch (unblocks Blocks 13–14). The exercises require no additional code changes — only correct REPL commands using existing functions plus the eta identity `b(q) = η(τ)³/η(3τ)`.

The primary risk is a subtle decomposition bug in `jacprodmake`: when `a = b/2`, the JAC factor contributes *double* the exponent to that residue class, so `x[b/2]` must be `e[b/2] / 2` — producing the half-integer values. Missing this causes silent empty output with no error. All other risks are low: the algorithms are textbook, the data structures already support `Frac` exponents in their types, and performance is adequate for T ≤ 500.

## Key Findings

### Recommended Stack

No new technologies needed. All changes extend existing types within the current zero-dependency C++20 architecture.

**Core extensions:**
- `BigInt::isqrt()` — Integer square root via Newton's method in base 10⁹, needed by `Frac::rational_sqrt()`
- `Frac::rational_sqrt()` / `is_perfect_square()` — Guard for `Series::powFrac` constant-term validation
- `Series::powFrac(Frac)` — Generalized binomial series `(1+g)^α = Σ C(α,k)·gᵏ` for exact rational fractional powers
- `Series::normalize_q_shift()` — Absorb integer q_shift into coefficient map indices, keep q_shift ∈ [0,1)
- `jac2series_impl` fractional dispatch — Route non-integer exponents through `powFrac` instead of binary exponentiation

**Key design decision:** Use generalized binomial coefficients (ARCHITECTURE.md approach) rather than the simpler sqrt-specific recurrence (STACK.md approach). The binomial approach handles arbitrary rational exponents (1/2, 13/2, future n-th roots) with one method, while the recurrence only handles square roots. Both are O(T²) in Frac operations. The binomial series terminates naturally when `gᵏ` exceeds truncation.

### Expected Features

**Must have (table stakes):**
- Half-integer JAC exponents in `jacprodmake` output — Maple produces `JAC(0,14,∞)^(13/2)` for Slater's identity
- Half-integer JAC exponents in `jac2series` reconstruction — must rebuild series from `JAC(a,b)^(1/2)`
- Integer q-shift absorption — `theta2(q)²/theta2(q³)²` yields q_shift=-1, must merge into coefficients
- `jac2prod` fractional exponent display — show `^(13/2)` or `√` notation

**Should have (correctness):**
- Eager q-shift normalization in `operator*`, `operator/`, `pow()`, `inverse()` — prevents q-shift accumulation
- `operator+` reconciliation for integer-different q-shifts — shift coefficient maps when delta is integer

**Defer (v2+):**
- General n-th root (`Series::nthroot(int n)`) — only den=2 appears in reference document
- Full algebraic number support (RootOf/ω) — use eta identity for b(q) instead
- Maple `radsimp` equivalent — integer q-shift absorption handles the actual need
- Double-sum performance optimization — exercises work at moderate T (≤ 200)

### Architecture Approach

The fix touches four components with clear boundaries: `series.h` gets `powFrac` and `normalize_q_shift`; `convert.h` gets fractional dispatch in `jac2series_impl` and middle-element fix in `jacprodmake`; `bigint.h` and `frac.h` get integer/rational square root helpers. No changes to `parser.h`, `qfuncs.h`, or `linalg.h`. The `JacFactor = std::tuple<int, int, Frac>` type already stores Frac exponents — the limitation is entirely in algorithms that truncate to `int`.

**Major components affected:**
1. `series.h` — `powFrac(Frac)`, `normalize_q_shift()`, modified `operator+` for integer-delta q-shifts
2. `convert.h` — `jac2series_impl` fractional dispatch, `jacprodmake` middle-element fix (`x[b/2] = e[b/2] / 2`), `jac2prod` Frac display
3. `bigint.h` — `isqrt()`, `is_perfect_square()` (~20 lines)
4. `frac.h` — `rational_sqrt()`, `is_perfect_square()` (~15 lines)

### Critical Pitfalls

1. **Silent fractional exponent discard (CRITICAL)** — Both `jac2prod` and `jac2series_impl` extract `int ex` from Frac exponents; when den≠1, ex stays 0 and the factor is silently skipped. No error, no warning. Fix: replace int extraction with Frac-aware dispatch.

2. **Middle-element doubling in jacprodmake (CRITICAL)** — When `a = b/2`, `JAC(b/2, b)^e` contributes `2e` to residue `b/2` (because `a = b-a`). Current code assigns `x[b/2] = e[b/2]` but should assign `x[b/2] = e[b/2] / 2`. This is why Block 13 (period b=14, middle element a=7) fails — x[7] must be 1/2, not 1.

3. **Integer q-shift throw blocks addition (CRITICAL)** — `operator+` throws when q_shifts differ, even when the difference is an integer (e.g., -1 vs 0). Fix: compute delta, shift coefficients if delta is integer, throw only for non-integer delta.

4. **Coefficient shift direction matters (CRITICAL)** — When reconciling q-shifts, always shift the series with the LARGER q_shift downward (increasing its exponents). Shifting the wrong direction creates negative exponents that break `prodmake` (expects `b[0] ≠ 0`) and `etamake`.

5. **Verification rejects correct decomposition (MAJOR)** — `jacprodmake` verifies via `jac2series_impl` reconstruction. If reconstruction can't handle fractional exponents, it rejects correct decompositions. Fix: implement `powFrac` before or simultaneously with decomposition fix.

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: Q-Shift Arithmetic Fix
**Rationale:** Independent of all other changes, low complexity (~35 lines), unblocks 2 checklist items and 1 exercise immediately. Also prevents q-shift accumulation bugs that would affect Phase 2 testing.
**Delivers:** Block 25 passes, Exercise 10 unblocked, `operator+` works for theta2+theta3 quotients
**Addresses:** Features: integer q-shift absorption, eager normalization, operator+ reconciliation
**Avoids:** Pitfalls 3 (integer q-shift throw), 4 (shift direction), 9 (propagation), 10 (etamake), 13 (multiply after add)

### Phase 2: Fractional Power Infrastructure
**Rationale:** Foundational capability needed before jacprodmake can be fixed. Bottom-up dependency: BigInt::isqrt → Frac::rational_sqrt → Series::powFrac.
**Delivers:** `Series::powFrac(Frac)` method, verified with `(1-q)^(1/2)` and `f^(1/2)² == f` tests
**Addresses:** Features: Series square root / rational power, BigInt/Frac sqrt helpers
**Avoids:** Pitfalls 5 (missing sqrt algorithm), 12 (binomial truncation discipline)

### Phase 3: Jacobi Half-Integer Exponents
**Rationale:** Depends on Phase 2 (powFrac). Fixes the jacprodmake decomposition bug and jac2series_impl dispatch. Unblocks Blocks 13-14.
**Delivers:** Blocks 13-14 pass, `jacprodmake` correctly produces half-integer JAC factors, `jac2series` reconstructs them
**Addresses:** Features: half-integer JAC in jacprodmake + jac2series, jac2prod fractional display
**Avoids:** Pitfalls 1 (silent discard), 2 (middle-element doubling), 6 (display formatting), 8 (verification rejection)

### Phase 4: Exercise Solutions & Verification
**Rationale:** All code changes complete; this phase verifies with REPL-level exercise solutions. No new code needed — only correct commands using b(q) = η(τ)³/η(3τ) identity and existing functions.
**Delivers:** Exercises 4, 5, 9, 10 solutions verified, full acceptance test suite passes
**Addresses:** Features: b(q) workaround, N(q) computation, findpoly with theta quotients
**Avoids:** Pitfall 7 (double-sum performance — use moderate T and eta identity workaround)

### Phase Ordering Rationale

- **Phase 1 before Phase 2:** Q-shift fix is independent and immediately testable. It also ensures theta2/theta3 computations work correctly when testing powFrac in Phase 2.
- **Phase 2 before Phase 3:** `jac2series_impl` needs `powFrac` to reconstruct half-integer exponents. Without it, the `jacprodmake` verification step rejects correct decompositions (Pitfall 8).
- **Phase 3 before Phase 4:** Exercises 4 and 5 depend on working jacprodmake output. Exercise 10 depends on Phase 1 only, so it could be tested earlier.
- **Phases 1 and 2 are parallelizable** — they modify different methods in `series.h` and have no data dependencies. A single developer would do them sequentially, but they could be developed on separate branches.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 2:** The generalized binomial approach vs. sqrt recurrence tradeoff. STACK.md recommends the simpler recurrence; ARCHITECTURE.md recommends the more general binomial. Both are O(T²). Recommend binomial for generality, but implementation should be validated against known coefficients of `(1-q)^(1/2)`.
- **Phase 3:** The middle-element decomposition fix (Pitfall 2) needs careful testing with Block 13 specifically. The `x[b/2] = e[b/2] / 2` formula needs verification against Maple's actual jacprodmake output for Slater (46).

Phases with standard patterns (skip research-phase):
- **Phase 1:** Q-shift normalization is mechanical — absorb integer part, keep fractional remainder. Well-defined semantics with clear test cases.
- **Phase 4:** Exercise solutions use existing REPL commands. The b(q) = η(τ)³/η(3τ) identity is well-established in the literature.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | All changes extend existing types; zero new dependencies; algorithms are textbook |
| Features | HIGH | Table stakes clearly identified from Maple checklist failures; anti-features well-reasoned |
| Architecture | HIGH | Component boundaries clean; JacFactor already stores Frac exponents; changes are local |
| Pitfalls | HIGH | 15 pitfalls identified from code inspection + failure analysis; ordering dependencies clear |

**Overall confidence:** HIGH

### Gaps to Address

- **powFrac approach selection:** Two viable algorithms (binomial vs recurrence). Recommend implementing binomial series and validating first 20 coefficients of `(1-q)^(1/2)` against known values: 1, -1/2, -1/8, -1/16, -5/128, -7/256, ...
- **Performance at T=500:** Block 14 calls `jac2series(jp, 500)` which triggers `powFrac` with trunc=500. The binomial series computes up to 500 terms of `gᵏ`. Each `g^k` series multiplication is O(k) because `g.minExp() ≥ 1` means `gᵏ` has only terms at exponents ≥ k. Total work is O(T²), acceptable but untested at that scale.
- **Exercise 9 (N(q)) feasibility:** The `findnonhomcombo` computation with σ₅ coefficients produces very large rationals. May need T=150+ for reliable results. The existing exercise solution notes this is "infeasible within practical truncation limits." Could remain partially unsolved.

## Sources

### Primary (HIGH confidence)
- `qseriesdoc.md` Output (15) — Confirms `JAC(0,14,∞)^(13/2)` expected output for Slater (46)
- `maple_checklist.md` — Documents Block 13/14/25 failures with exact error messages
- `src/convert.h:343-460` — Direct code inspection of jac2series_impl and jacprodmake
- `src/series.h:131-148` — Direct code inspection of operator+ q_shift handling
- Phase 62 verification (`.planning/phases/62-maple-checklist/62-VERIFICATION.md`) — Documents all three failures as known limitations

### Secondary (MEDIUM confidence)
- Maple qseries package v1.3 documentation (qseries.org) — jacprodmake and findpoly function references
- Borwein-Borwein-Garvan cubic theta identities — Exercise 10 expected output `y = 27(m-1)(m+1)⁴/(m²+6m-3)³`
- Generalized binomial series / formal power series square root — standard mathematical references (Knuth TAOCP Vol. 2 §4.7)

### Tertiary (LOW confidence)
- Exercise 9 N(q) = a(q)⁶(1 - 540x) — from Borwein-Borwein-Garvan; needs validation via `findnonhomcombo` at sufficient truncation

---
*Research completed: 2026-03-01*
*Ready for roadmap: yes*
