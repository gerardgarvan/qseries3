# Architecture Research: factor(poly) — Polynomial Factorization Over Q

**Domain:** q-series REPL — polynomial factorization (Maple-style factor into cyclotomics)
**Researched:** 2026-03-03
**Confidence:** HIGH (architecture) / MEDIUM (algorithm choice — zero-dependency limits options)

---

## Executive Summary

The `factor(poly)` builtin should factor a univariate polynomial in q over Q into irreducible factors (cyclotomic polynomials for T(8,8)-style inputs). **Series is the polynomial representation** — no new Poly type is needed. Put factorization logic in a new `polyfactor.h` (or `factor.h`) to keep convert.h focused on q-product conversions. Parser already supports `factor(expr)` via `Call`; the REPL needs dispatch. Build order: **factorization algorithm first** → **polyfactor.h** → **parser dispatch**.

---

## 1. Where Does factor Fit?

### Option A: New poly.h / polyfactor.h — **Recommended**

| Aspect | Rationale |
|--------|-----------|
| **Separation of concerns** | `convert.h` = q-series → q-product form (prodmake, qfactor, etamake, jacprodmake). Polynomial factorization over Q is algebra, not q-product conversion. |
| **Naming** | `factor` in Maple/Sage factors polynomials. `qfactor` factors q-products. Clear distinction. |
| **Dependencies** | polyfactor needs: Series (or `std::vector<Frac>`), Frac, BigInt. No need for qfuncs, eta, theta. |
| **Testing** | Isolated unit tests for factorization without loading convert stack. |

**Recommendation:** Create `polyfactor.h` with `factorPoly(const Series& f)` or `factorPoly(const std::vector<Frac>& coeffs)`. Keep convert.h unchanged.

### Option B: Extend convert.h

| Aspect | Rationale |
|--------|-----------|
| **Pro** | One less file; all "conversion" in one place. |
| **Con** | convert.h is already large (450+ lines). factor and qfactor are semantically different: qfactor → (1-q^n) product; factor → cyclotomic Φ_d(q) product. Mixing them blurs responsibilities. |

**Recommendation:** Avoid. Keep convert.h for q-product conversions only.

---

## 2. Polynomial Representation: Series = Poly

### No New Poly Type Needed

A polynomial P(q) = c₀ + c₁q + … + c_d q^d with coefficients in Q is represented **exactly** by `Series` when:

- `minExp() >= 0` (no negative powers)
- `q_shift == 0` (no fractional q-power)
- Coefficients `c[i]` = `f.coeff(i)` for i = 0..d

Existing facilities:

- `Series::coeffList(from, to)` → `std::vector<Frac>` (used by relations.h)
- `Series::trunc`, `Series::minExp()`, `Series::maxExp()`

So: **`factor(expr)` where `expr` evaluates to Series**. If the Series has negative exponents or q_shift ≠ 0, either reject or normalize (e.g. multiply by q^{-minExp} and treat as polynomial with rational leading term — but typically factor expects polynomial with non-negative exponents).

### Semantic Contract

```
factor(f)  where f is Series
  Precondition: f.minExp() >= 0, f.q_shift == 0 (or reject with clear error)
  Input:  P(q) = Σ c[n] q^n  (finite polynomial, c[n] ∈ Q)
  Output: product of irreducible factors over Q (cyclotomics Φ_d(q), etc.)
```

### Return Type

Options for factored form:

| Type | Example | Pros | Cons |
|------|---------|------|------|
| `std::map<int, int>` | Φ_d → multiplicity | Compact; easy to format | Loses non-cyclotomic factors |
| `std::vector<std::pair<Series, int>>` | (factor, mult) | General | Heavier; factor may be non-monic |
| `std::vector<std::pair<std::vector<Frac>, int>>` | (coeffs, mult) | Explicit | Same |
| `struct FactorResult { std::vector<...> factors; }` | Custom | Clear API | More boilerplate |

For Maple parity with Block 4 (factor(t8) → cyclotomics), the output is typically shown as `Φ_5(q)·Φ_7(q)·…`. A compact representation:

```cpp
struct FactorResult {
    Frac leading;                              // scalar factor
    std::vector<std::pair<int, int>> cyclotomic;  // (n, mult) for Φ_n(q)^mult
    // If general factors needed later: std::vector<std::pair<std::vector<Frac>, int>> other;
};
```

For v1, supporting only cyclotomic factorization (as in qseriesdoc Output 4) is sufficient. General irreducibles can be deferred.

---

## 3. Parser and REPL Integration

### Parser: No Change

`factor(expr)` parses as `Expr::makeCall("factor", {expr})`. The parser already handles arbitrary `identifier(args)`. No grammar change.

### REPL Dispatch

In `repl.h` `evalCall`:

1. `if (name == "factor")` …
2. Evaluate `args[0]` → must be `Series` (or promote from int via `Series::constant`)
3. Validate: `minExp() >= 0`, `q_shift == 0`
4. Call `factorPoly(series)` from polyfactor.h
5. Add `FactorResult` to `EvalResult` variant
6. Add `formatFactor(FactorResult)` for display

### EvalResult Variant

```cpp
// Add to EvalResult
FactorResult,   // factor(poly)
```

`formatFactor` prints e.g. `Phi[5](q)*Phi[7](q)^2*...` or Maple-style `(q^4+q^3+q^2+q+1)*(...)`.

---

## 4. Build Order

### Recommended Order

1. **Cyclotomic polynomial Φ_n(q)** — Implement `cyclotomic(n)` returning `std::vector<Frac>` or `Series` (coefficients of Φ_n). Required for both factoring and recognizing cyclotomic factors.
2. **Integer polynomial utilities** — Clear denominators (Q → Z), extract content, primitive part. Use existing Frac/BigInt.
3. **Factorization algorithm** — For cyclotomic detection: check if P | (q^n - 1) for some n ≤ degree. Factor q^n - 1 = ∏_{d|n} Φ_d(q). Extract Φ_d by division. Alternative: Kronecker's method for small degrees.
4. **polyfactor.h** — `factorPoly(const Series&)` → `FactorResult`. Thin wrapper around algorithm.
5. **REPL dispatch** — Add `factor` to evalCall, FactorResult to EvalResult, formatFactor.

### Why This Order

- Cyclotomic construction is independent and testable.
- Factorization depends on cyclotomic and integer polynomial ops.
- polyfactor.h depends only on Series, Frac, BigInt, and the algorithm — no REPL.
- REPL changes are last and localized.

### Not Recommended

- **Poly type first**: Unnecessary; Series suffices. Adding a Poly type would duplicate Series semantics.
- **Parser first**: Parser already supports `factor(expr)`. No change.
- **Dispatch first**: Would require stubs; better to have working factorPoly before wiring.

---

## 5. Algorithm: Zero-Dependency Constraint

**Constraint:** No GMP, NTL, or other external libs. All in-house with BigInt, Frac.

### Cyclotomic Factorization Path

For polynomials that split into cyclotomics (e.g. T(8,8)):

1. **Compute Φ_n(q)** — Euler's formula: Φ_n(q) = ∏_{d|n} (q^d - 1)^{μ(n/d)}. Or use recurrence. Coefficients are integers.
2. **Trial division** — For each d = 1, 2, … up to degree(P): if Φ_d divides P (polynomial division over Q), record (d, multiplicity), replace P by P/Φ_d.
3. **Content and primitive part** — Factor out integer content; factor primitive part. Remaining scalar can be absorbed into "leading" in FactorResult.

### General Factorization (Deferred)

For polynomials with non-cyclotomic irreducible factors:

- **Kronecker's method**: Evaluate P at deg(P)+1 points, factor integer values, interpolate candidate factors. Exponential in degree but works for small degrees.
- **Berlekamp / Cantor–Zassenhaus**: Requires more infrastructure (finite field, lifting). Defer.

**Recommendation:** Implement cyclotomic-only factorization first. T(8,8) and Block 4 are covered. If P has a non-cyclotomic factor, return "cannot factor" or the product of cyclotomic factors found plus "remaining factor" — document as limitation.

---

## 6. Component Boundaries

| Component | Responsibility | Depends On |
|-----------|----------------|------------|
| **polyfactor.h** (NEW) | `cyclotomic(n)`, `factorPoly(Series)` → FactorResult | series.h, frac.h, bigint.h |
| **convert.h** | qfactor, prodmake, etamake, jacprodmake (unchanged) | series.h, frac.h, qfuncs.h |
| **repl.h** | `factor` builtin dispatch, formatFactor, EvalResult | polyfactor.h, parser |
| **parser.h** | No change | — |

### Data Flow

```
User: factor(t8)
  → parse: Call("factor", [Var("t8")])
  → eval: lookup t8 → Series
  → factorPoly(series) → FactorResult
  → formatFactor(result) → string
  → display
```

---

## 7. Summary: Direct Answers

| Question | Answer |
|----------|--------|
| **Where does factor fit?** | New `polyfactor.h`. Do not extend convert.h. |
| **New poly.h?** | No. Use Series as polynomial (coeffs = series coefficients, minExp ≥ 0). |
| **Parser: factor(expr)?** | Yes. expr evaluates to Series; coefficients = polynomial coefficients. |
| **Build order** | Cyclotomic Φ_n → factorization algorithm → polyfactor.h → REPL dispatch. Poly type is unnecessary. |
| **factor vs qfactor** | factor = polynomial factorization over Q (cyclotomics). qfactor = q-product form (1-q^n). Different semantics, different headers. |

---

## 8. Pitfalls

- **Confusing factor with qfactor**: Users may expect `factor` to behave like `qfactor`. Document: factor = Maple's factor (cyclotomic); qfactor = q-product form.
- **Negative exponents**: Reject or error clearly if Series has minExp < 0. Don't silently treat q^{-1} + 1 as polynomial 1 + q.
- **Non-cyclotomic factors**: First version may not fully factor. Return best-effort cyclotomic factorization and document limitation.
- **Degree explosion**: Cyclotomic polynomials can have large coefficients. Use BigInt throughout; avoid int overflow.

---

## Sources

- qseriesdoc.md §3.2 — factor(t8) Output (4) vs qfactor Output (5)
- PROJECT.md — v9.0 factor(t8) + Block 4
- convert.h — qfactor, prodmake; Series usage
- repl.h — evalCall pattern, EvalResult, qfactor dispatch
- SPEC.md — zero external libs, BigInt/Frac/Series stack
- Web: NTL, polyfact, Factoring-with-Cyclotomic-Polynomials (Sage) — external libs not applicable; algorithm design reference only
