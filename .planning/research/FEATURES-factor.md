# Feature Research: factor(poly)

**Domain:** Polynomial factorization for Block 4 parity (Maple factor(t8))  
**Researched:** 2026-03-03  
**Context:** SUBSEQUENT MILESTONE — factor(t8) + Block 4. Existing: T(r,n) builtin, qfactor for q-products.

---

## Executive Summary

`factor(t8)` in Maple (qseriesdoc line 288) produces cyclotomic factorization of T(8,8). This is distinct from `qfactor(t8, 20)`, which yields q-product form. Block 4 currently skips factor(t8). To achieve Maple parity, add `factor(expr)` that factors univariate polynomials in q into cyclotomic polynomials over Q.

**Recommendation:** API `factor(expr)` taking a Series (polynomial); output as a formatted string of cyclotomic factors; scope Block 4 = T(8,8) via qfactor → cyclotomic expansion; general univariate deferred unless needed.

---

## 1. API Design

### Input: Series or Poly?

| Option | Pros | Cons |
|--------|------|------|
| **factor(expr)** where expr → Series | Same pattern as qfactor, prodmake; no new type | Series is already the polynomial representation |
| **factor(poly)** with dedicated poly type | Explicit type | Introduces redundant type; REPL has no poly, only Series |

**Recommendation:** `factor(expr)` — expr evaluates to Series. The REPL's polynomial representation *is* Series (sparse `map<int,Frac>`). All built-ins (qfactor, prodmake, etamake) take Series. No separate poly type.

**Signature:** `factor(f)` or `factor(f, T?)`. T is only needed if f is an infinite series (not a polynomial); for polynomials like T(8,8), truncation is implicit in the series. For consistency with qfactor: `factor(f)` using `f.trunc` or `factor(f, T)` if caller wants to bound.

**Edge cases:** Non-polynomial input (infinite series with nonzero tail) — either reject or truncate and treat as polynomial. Block 4: t8 = T(8,8) is finite degree 42.

---

## 2. Output Format

### Options

| Format | Example | Use |
|--------|---------|-----|
| **String** | `"Φ₁²·Φ₂·Φ₃²·Φ₄·Φ₅·Φ₉·Φ₁₀·Φ₁₁·Φ₁₆"` | Display; matches Maple "factored into cyclotomics" |
| **List of (n, e)** | `[(1,2),(2,1),(3,2),...]` for Φₙ^e | Programmatic; compact |
| **Struct** | `FactorResult { std::map<int,int> cyclotomic; Frac content; }` | Both display and downstream |

**Maple behavior:** factor() returns an expression (product form). Display is typically cyclotomic factors; Maple may use `Cyclotomic(n,q)` or expanded form.

**Recommendation:** 
- **Primary:** Formatted string for display (REPL prints it). Use Φₙ or `Cyclotomic(n,q)` notation.
- **Internal:** Struct with `std::map<int,int> cyclotomic` (n → exponent) and optional `Frac content` for leading constant. Display builds string from this.
- **Rationale:** Same pattern as QFactorResult → formatQfactor: struct for logic, string for REPL output.

---

## 3. Cyclotomic vs General Irreducible

### Mathematical Context

Over Q, a univariate polynomial factors into:
- **Content** (rational constant)
- **Product of irreducibles** — either cyclotomic Φₙ(q) or non-cyclotomic (e.g. q²−2)

For polynomials arising from q-products (e.g. T(8,8)):
- T(8,8) = q⁶ · (1-q⁹)(1-q¹⁰)(1-q¹¹)(1-q¹⁶) / ((1-q)(1-q²)(1-q³)(1-q⁴))
- Each (1-qⁿ) = ∏_{d|n} Φ_d(q)
- So the factorization over Q is **purely cyclotomic**

### Scope Options

| Scope | Algorithm | Effort |
|-------|-----------|--------|
| **Cyclotomic only** | Convert qfactor output: (1-qⁿ)^e → ∏ Φ_d^e for d|n | Low — reuse qfactor |
| **General univariate** | Full factorization over Q (Berlekamp, Cantor-Zassenhaus, etc.) | High — new algorithm, no external libs |

**Recommendation for Block 4:** Cyclotomic-only. T(8,8) and any polynomial that qfactor can handle (finite q-product structure) factor into cyclotomics. Use:

1. Run qfactor(f, T) → q^e · ∏(1-qⁿ)^{±e_n}
2. For each (1-qⁿ), expand: (1-qⁿ) = ∏_{d|n} Φ_d(q)
3. Aggregate exponents: each Φ_d gets sum of contributions from all n with d|n
4. Output: q^e · ∏ Φ_d^a_d

**General univariate:** Defer. If a future milestone needs factor(q²−2) or similar, that requires full polynomial factorization. Block 4 does not.

---

## 4. Scope: T(8,8) Only vs General Univariate

| Scope | Delivers | Risk |
|-------|----------|------|
| **T(8,8) only** | Block 4 parity; factor(t8) works | Limited; any other T(r,n) or q-product would also work with same approach |
| **All q-product polynomials** | factor works for any f where qfactor works | Same implementation — qfactor + cyclotomic expansion |
| **General univariate** | factor(q²−2), etc. | New algorithm; out of scope for Block 4 |

**Recommendation:** Implement for **all polynomials that qfactor can factor** — i.e., finite q-product structure. That covers T(8,8), T(4,4), T(6,6), dixson, etc. No need to artificially restrict to T(8,8). If qfactor fails (not a q-product), factor can either error or fall back to "irreducible" / "cannot factor" — document as Block 4 scope.

---

## 5. Maple Parity for Block 4

### Current State

- **Block 3:** T(8,8) computation — PASS
- **Block 4:** factor(t8) — SKIP (Maple-only)
- **Block 5:** qfactor(t8, 20) — PASS

### Maple factor vs qfactor

| Function | Output type | Example for T(8,8) |
|----------|-------------|--------------------|
| **qfactor(t8, 20)** | q-product | q⁶·(1-q⁹)(1-q¹⁰)(1-q¹¹)(1-q¹⁶)/((1-q)(1-q²)(1-q³)(1-q⁴)) |
| **factor(t8)** | Cyclotomic | Product of Φ_d(q) factors |

Both are valid factorizations; different representations. qseriesdoc line 288: "Observe how we used factor to factor t8 into cyclotomic polynomials. However, qfactor was able to factor t8 as a q-product."

### Parity Definition

Block 4 parity = `factor(t8)` produces output equivalent to Maple's cyclotomic factorization. Exact character-level match is not required; mathematical equivalence is. Use Φₙ or Cyclotomic(n,q) notation.

---

## 6. Implementation Sketch

```
factor(f):
  1. f ← truncate to effective degree (or use f.trunc)
  2. qf ← qfactor(f, T)  where T = 4*maxExp+3 or f.trunc
  3. If qfactor fails (e.g. non-monic, non-q-product): return error or "irreducible"
  4. Build cyclotomic exponents:
     - content = qf.q_power (handled as q^e factor)
     - For each n in qf.num_exponents: e = exponent
       - For each d|n: cyclotomic[d] += e
     - For each n in qf.den_exponents: e = exponent
       - For each d|n: cyclotomic[d] -= e
  5. Format: q^e · ∏ Φ_d^a_d for d with a_d ≠ 0
  6. Return FactorResult or string
```

**Dependency:** qfactor, divisors (for d|n). Both exist.

---

## 7. Feature Summary Table

| Aspect | Recommendation |
|--------|----------------|
| **API** | `factor(expr)` — expr → Series; same as qfactor |
| **Output** | Struct + formatted string; Φₙ or Cyclotomic(n,q) notation |
| **Algorithm** | Cyclotomic via qfactor + (1-qⁿ) = ∏_{d|n} Φ_d expansion |
| **Scope** | All polynomials that qfactor factors (q-product structure); T(8,8) is primary target |
| **General irreducible** | Out of scope for Block 4 |
| **Maple parity** | factor(t8) produces cyclotomic factorization; Block 4 pass |

---

## 8. Sources

- qseriesdoc.md lines 284–308 (factor vs qfactor, T(8,8))
- SPEC.md §5.4 (qfactor), Test 7
- tests/maple-checklist.sh Block 4 (currently skip)
- Maple factor() docs: maplesoft.com/support/help/Maple/view.aspx?path=factor
- .planning/phases/06-convert-extended/06-03-PLAN.md (qfactor, T(r,n))
- Cyclotomic expansion: (1-qⁿ) = ∏_{d|n} Φ_d(q) — standard identity
