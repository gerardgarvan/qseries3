# Technology Stack: Polynomial Factorization over Q (factor builtin)

**Project:** q-series REPL — Milestone v9.0 factor(t8)
**Researched:** 2026-03-03
**Scope:** Univariate polynomial factorization over Q for cyclotomic output. Zero external deps. No GMP/Flint/PARI.

---

## Executive Summary

Maple's `factor(t8)` factors T(8,8) (degree 42 poly in q) into cyclotomic polynomials. The `factor` builtin must do the same: input univariate poly in q with Frac coefficients, output product of cyclotomic Φ_n and irreducible factors. **Recommended pipeline: (1) Poly representation as `std::map<int,Frac>` (reuse Series), (2) cyclotomic extraction first via Φ_n division, (3) general Z[x] factorization via Berlekamp–Zassenhaus (finite-field step: Cantor–Zassenhaus; subset selection: Kronecker-style or Zassenhaus recombination).** For q-series polynomials like T(8,8) that factor entirely into cyclotomics, cyclotomic-first extraction suffices and avoids heavier machinery.

---

## Representation

### Poly with Frac Coefficients

| Approach | Structure | Purpose | Why |
|----------|-----------|---------|-----|
| **Reuse Series** | `std::map<int, Frac>` sparse, int exponents | Finite poly in q | Same as Series; factor input is polynomial (finite). `q_shift` must be 0; trunc = deg+1 or irrelevant for factor. |
| **Dense vector** | `std::vector<Frac>` index = exponent | Optional for division | Simpler for polynomial division/GCD; can convert from sparse. |

**Recommendation:** Use existing `Series` as Poly for factor input. Ensure integer exponents and `q_shift == 0`. For internal algorithms (division, GCD), convert to dense `vector<Frac>` over exponent range `[0, deg]` when beneficial.

### Output Format

| Format | Structure | Purpose |
|--------|-----------|---------|
| **Factor list** | `std::vector<std::pair<Series, int>>` (poly, multiplicity) | Internal representation |
| **Display** | Product of Φ_n(q) and irreducible polys | Maple parity: `factor(t8)` → cyclotomic form |

---

## Algorithm Pipeline

### 1. Normalization

- Clear denominators (multiply by LCM) → work in Z[x].
- Content removal: extract gcd of coefficients → primitive poly.
- Optional: square-free decomposition (Yun) to separate repeated factors. For cyclotomic factors, iteration of division handles multiplicity.

### 2. Cyclotomic Extraction (Primary Path for T(8,8))

**Strategy:** For each n with φ(n) ≤ deg(f), compute Φ_n, divide f by Φ_n while it divides; record multiplicity. Repeat until no cyclotomic factor remains.

**Φ_n formula (Möbius):**

```
Φ_n(x) = ∏_{d|n} (x^(n/d) - 1)^μ(d)    for n > 1
Φ_1(x) = x - 1
```

- Requires: `divisors(n)`, `mobius(n)` (already in project for prodmake), polynomial multiplication and exact division.
- Division: poly f / poly g over Q — standard long division, O(deg(f)·deg(g)) with Frac arithmetic.

**Cyclotomic detection:** A factor is cyclotomic iff it equals Φ_n for some n. Our approach: iterate n = 1,2,... with φ(n) ≤ deg(f), compute Φ_n, divide out. No separate "detection" step — we directly try division by Φ_n.

### 3. General Factorization over Z[x] (When Needed)

If cyclotomic extraction leaves a non-trivial remainder, factor it over Q. By Gauss's lemma, factorization over Q reduces to Z[x].

| Algorithm | Domain | Role | Complexity |
|-----------|--------|------|------------|
| **Cantor–Zassenhaus** | F_p[x] | Factor mod p | Polynomial time |
| **Berlekamp–Zassenhaus** | Z[x] | Full pipeline: mod p → Hensel lift → subset selection | Exponential in #factors (subset selection) |
| **Van Hoeij (LLL)** | Z[x] | Replaces subset selection with lattice reduction | Polynomial in practice |
| **Kronecker** | Z[x] | Evaluate, divisors, interpolate, test | Simple but slow; good for small degree |

**What CAS use:**
- **Maple:** Combination of methods; factor() produces cyclotomic form when applicable. (LOW confidence — not verified in source.)
- **PARI/GP:** Cantor–Zassenhaus for F_p; Berlekamp–Zassenhaus style for Z[x]. `polcyclofactors(f)` extracts cyclotomics (no multiplicity); full `factor()` handles multiplicities. (MEDIUM confidence — Stack Overflow, PARI docs.)
- **SymPy:** Uses Flint/other backends when available; fallback may use Kronecker or similar. (LOW confidence — docs don't specify.)

**Recommendation for zero-dependency C++:**
- **Phase 1 (Block 4):** Cyclotomic extraction only. Sufficient for factor(t8) and many q-series polynomials.
- **Phase 2 (if needed):** Add Berlekamp–Zassenhaus: (a) Factor in F_p with Cantor–Zassenhaus, (b) Hensel lift to Z/p^k, (c) Zassenhaus recombination (test subsets). Kronecker is an alternative if degree is small (e.g. ≤20) and factor count is low.
- **Avoid:** LLL/van Hoeij — complex, typically requires external libs.

---

## Cyclotomic Polynomial Computation

**Formula (equivalent forms):**

```
Φ_n(x) = ∏_{d|n} (x^d - 1)^μ(n/d)    [Wikipedia]
       = ∏_{d|n} (x^(n/d) - 1)^μ(d)  [MathWorld]
```

**Implementation outline:**
1. For each divisor d of n, compute `x^(n/d) - 1` as a sparse/dense poly.
2. Raise to power μ(d) (usually ±1 or 0).
3. Multiply results. Handle μ(d)=0 by skipping.
4. Result is Φ_n with integer coefficients.

**Range:** For deg(f) ≤ D, only n with φ(n) ≤ D can appear. Max n such that φ(n) ≤ D grows roughly like n ~ O(D log log D). For D=42, n up to ~100–200 suffices.

---

## Implementation Checklist

| Component | Required | Notes |
|-----------|----------|-------|
| `divisors(n)` | ✓ | Already in project |
| `mobius(n)` | ✓ | Already in project |
| `euler_phi(n)` | ✓ | Already in project |
| Poly multiplication | ✓ | Series `operator*` |
| Poly exact division | New | f / g when g | f; return quotient |
| Poly GCD | New | Euclidean algorithm; needed for square-free and possibly Hensel |
| Φ_n computation | New | Möbius product formula |
| Cyclotomic extraction loop | New | n=1..N, divide by Φ_n while dividing |
| Hensel lifting | Phase 2 | For general B–Z |
| C–Z over F_p | Phase 2 | For general B–Z |

---

## Scope for Block 4

**Goal:** `factor(t8)` produces cyclotomic factorization matching Maple Output (4).

**Sufficient implementation:**
1. `cyclotomic(n)` → Φ_n as Series (or dense poly)
2. `factor_poly(f)` → list of (Φ_n or irreducible, multiplicity)
   - Loop: for n with φ(n) ≤ deg(f), divide f by Φ_n repeatedly, record multiplicity
   - When no more cyclotomic factors: remainder is 1 (success) or needs general factorization
3. For T(8,8) and similar: cyclotomic extraction alone should reduce f to 1
4. Display: product of Φ_n(q)^e

**If remainder ≠ 1:** Either (a) treat as irreducible and report, or (b) implement Kronecker/B–Z for Phase 2. Block 4 can accept (a) for now.

---

## Alternatives Considered

| Recommended | Alternative | Why Not |
|-------------|-------------|---------|
| Cyclotomic extraction first | Full B–Z from the start | T(8,8) and most q-series polys are products of cyclotomics; B–Z is heavier |
| Reuse Series as Poly | New Poly struct | Duplication; Series already has Frac coeffs, sparse map |
| Zassenhaus subset selection | LLL/van Hoeij | LLL is complex; zero-deps constraint |
| Möbius formula for Φ_n | Recursive Φ_n = (x^n-1)/∏_{d\|n,d<n} Φ_d | Equivalent; Möbius form is explicit and uses existing mobius() |

---

## Sources

- Wikipedia: [Berlekamp–Zassenhaus algorithm](https://en.wikipedia.org/wiki/Berlekamp%E2%80%93Zassenhaus_algorithm) — Z[x], Hensel, subset selection
- Wikipedia: [Cantor–Zassenhaus algorithm](https://en.wikipedia.org/wiki/Cantor%E2%80%93Zassenhaus_algorithm) — F_p factorization
- Wikipedia: [Cyclotomic polynomial](https://en.wikipedia.org/wiki/Cyclotomic_polynomial) — Φ_n, Möbius formula
- Wikipedia: [Kronecker's method](https://en.wikipedia.org/wiki/Factorization_of_polynomials) — classical Z[x] factorization
- PARI: polcyclofactors, polcyclo — cyclotomic extraction; factor uses C–Z + B–Z style
- qseriesdoc.md §3.2 — factor(t8) → cyclotomic; qfactor → q-product
- Project: SPEC.md, convert.h (qfactor, prodmake), series.h (Series = map<int,Frac>)

---

*Stack research for: factor(poly) over Q, cyclotomic factorization, zero-dependency C++*
