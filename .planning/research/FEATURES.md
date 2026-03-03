# Feature Landscape: RootOf / Algebraic Numbers

**Domain:** q-series REPL — RootOf for cyclotomic fields (primitive cube root ω)
**Researched:** 2026-03-03
**Sources:** Maple RootOf docs (maplesoft.com), SymPy numberfields, Mathematica RootReduce, qseriesdoc §3.4, maple-checklist Block 10, project context

---

## Executive Summary

RootOf in symbolic systems (Maple, Mathematica, SymPy) represents algebraic numbers as roots of irreducible polynomials. For q-series parity, the critical case is **ω = RootOf(z²+z+1=0)** — the primitive cube root of unity used in a(q), b(q), c(q). Table stakes for v1: represent ω, arithmetic in Q(ω), simplification (ω³=1), and coefficients in Q(ω) for Series. Differentiators (general RootOf, indexed roots, radical conversion) should be deferred. Avoid nested RootOf and general polynomial factorization over Q(ω) for v1.

---

## How RootOf Works in Symbolic Systems

### Maple

- **Syntax:** `omega := RootOf(z^2+z+1=0)` or `alias(omega=RootOf(z^2+z+1=0))`
- **Arithmetic:** `evala(omega^4)` → reduces to `omega` (ω⁴ = ω³·ω = ω)
- **Canonical form:** Single-argument `RootOf(_Z²+_Z+1)`, primitive polynomial
- **Indexed roots:** `RootOf(expr, x, index=1)` selects by complex argument (counter-clockwise)
- **Conversion:** `convert(omega, radical)` → `(-1 + I√3)/2`

### Mathematica

- **Syntax:** `Root[#^2+#+1&, 1]` or `AlgebraicNumber` with field
- **Simplification:** `RootReduce` reduces algebraic combinations to canonical form
- **Operations:** Arithmetic on Root/AlgebraicNumber automatically normalized

### SymPy

- **Syntax:** `RootOf(x**2+x+1, 0)` or `QQ.algebraic_field(...)` for Q(θ)
- **Representation:** ANP (Algebraic Number Pair) — polynomial in θ mod minimal polynomial
- **Arithmetic:** Operations reduce modulo defining polynomial

### Common Patterns

| Aspect | Maple | Mathematica | SymPy |
|--------|-------|-------------|-------|
| Define | `RootOf(z²+z+1=0)` | `Root[#²+#+1&,1]` | `RootOf(x²+x+1,0)` |
| Simplify | `evala(expr)` | `RootReduce[expr]` | automatic in AlgebraicField |
| Display | alias / omega | Root or radical | ANP or RootOf |
| Substitution | symbolic | symbolic | symbolic |

---

## Expected Behavior for RootOf(z²+z+1=0)

### Mathematical Properties

- ω = exp(2πi/3), primitive cube root of unity
- **Minimal polynomial:** z² + z + 1 = 0
- **Reduction rules:** ω³ = 1, ω² = -ω - 1, ω + ω² = -1
- **Field:** Q(ω) = Q(√-3), degree 2 over Q
- **Elements:** a₀ + a₁ω with aᵢ ∈ Q

### Arithmetic in Q(ω)

- **Add:** (a₀ + a₁ω) + (b₀ + b₁ω) = (a₀+b₀) + (a₁+b₁)ω
- **Mul:** Use ω² = -ω - 1 to reduce; e.g. ω·(a₀+a₁ω) = a₀ω + a₁ω² = a₀ω + a₁(-ω-1) = -a₁ + (a₀-a₁)ω
- **Powers:** ω^k = ω^(k mod 3) — essential simplification for series

### Substitution into Series

- **b(q) = Σ Σ ω^(n-m) q^(n²+nm+m²):** Coefficients become elements of Q(ω)
- **Representation:** Each coefficient is (a₀, a₁) ∈ Q² meaning a₀ + a₁ω
- **Existing Series:** `std::map<int, Frac>` must extend to `std::map<int, Algnum>` where Algnum ∈ Q(ω)

### Display Format

| Format | Example | When to Use |
|--------|---------|-------------|
| Polynomial in ω | `1 + 2*omega` | Default for Q(ω) |
| Maple-style | `omega`, `omega^2` | With alias |
| Radical | `(-1 + I√3)/2` | Optional, requires sqrt(-3) |
| Cyclotomic | `ζ₃` or `ω` | Compact notation |

---

## Table Stakes (Users Expect These)

Features required for Maple Block 10 parity and qseriesdoc Exercise 4. Missing = product feels incomplete.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| `omega := RootOf(z^2+z+1=0)` | Maple Block 10, qseriesdoc §3.4 hint | LOW | Parser + symbol binding; can be built-in constant initially |
| Arithmetic in Q(ω): add, mul | b(q) = Σ ω^(n-m) q^(…) requires ω^k in coefficients | MEDIUM | New type `Algnum` (a₀, a₁) or FracPair; reduce via ω²=-ω-1 |
| Simplification: ω^k → ω^(k mod 3) | Coefficients must normalize; ω⁴ = ω | LOW | Part of Algnum mul/pow |
| Series with coefficients in Q(ω) | b(q) is a series; a(q), c(q) are rational | HIGH | Series needs optional `std::map<int, Algnum>` or unified `Coeff` variant |
| `sum` / theta with ω in exponent | b(q) = Σ Σ ω^(n-m) q^(n²+nm+m²) | MEDIUM | sum() must evaluate ω^(n-m) → Algnum |
| Display: `a + b*omega` | Users must see coefficients | LOW | Format Algnum as string |
| etamake / prodmake on Q(ω) series | Maple can run etamake on b(q) | MEDIUM | May need to restrict to rational-only for v1 (see Anti-Features) |

---

## Differentiators (Competitive Advantage)

Valuable but not required for Block 10 / Exercise 4 parity.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| General `RootOf(poly)` | Arbitrary algebraic extensions | HIGH | Requires polynomial ring, gcd, minimal polynomial handling |
| Indexed root selection | Choose specific root (e.g. ω vs ω²) | MEDIUM | Maple index=1,2; affects display/choice only for z²+z+1 |
| `convert(expr, radical)` | Explicit (-1+I√3)/2 | LOW | Requires √(-3) as symbolic or display convention |
| `evala`-style explicit reduce | User-triggered simplification | LOW | Nice for debugging |
| Cyclotomic ζₙ for n>3 | 5th, 7th roots of unity | MEDIUM | Q(ζ₅) has degree 4; different reduction rules |

---

## Anti-Features (Commonly Requested, Often Problematic)

| Anti-Feature | Why Requested | Why Problematic | Alternative |
|--------------|---------------|-----------------|-------------|
| Full algebraic number tower | "Support any RootOf" | Nested extensions (Q(ω)(√2)), field membership, primitive element — major scope | Restrict v1 to Q(ω) only |
| prodmake/etamake on Q(ω) series | Maple can do it | Andrews' algorithm assumes rational coefficients; exponent recurrence uses division | Use identity b(q)=η(τ)³/η(3τ) for prodmake; or defer Q(ω) prodmake |
| Floating-point evaluation of ω | "Get numeric value" | Project is exact-only; evalf conflicts with zero-dependency | Omit; keep exact |
| Maple `alias` in REPL | Compact omega display | Alias is session-global; complicates parser | Use `omega` as built-in name for RootOf(z²+z+1=0) |
| General polynomial factorization over Q(ω) | factor(t8) in Block 4 | Requires factor over number field; major subsystem | Deferred (per PROJECT.md) |

---

## Feature Dependencies

```
RootOf(z²+z+1=0) symbol
    └── requires → Algnum type (a₀ + a₁ω)
                      └── requires → ω² = -ω - 1 reduction
    └── requires → Parser: omega, omega^2 as identifiers

Algnum
    └── requires → add, mul, pow (ω^k reduction)
    └── enhances → Series (coefficients)

Series with Q(ω) coefficients
    └── requires → sum() evaluating ω^exponent → Algnum
    └── requires → theta / aqprod with Algnum coefficients (if used in sum)
    └── optionally requires → prodmake/etamake (defer if complex)

b(q) = Σ Σ ω^(n-m) q^(n²+nm+m²)
    └── requires → sum(sum(omega^(n-m)*q^(n*n+n*m+m*m), ...))
    └── requires → omega^(n-m) → Algnum
```

---

## Maple Parity Checklist (Block 10)

| Maple Block 10 Item | Required for Parity | v1 Scope |
|---------------------|---------------------|----------|
| `omega := RootOf(z^2+z+1=0)` | Yes | Implement as `omega` built-in or `:= RootOf(...)` |
| Use omega in b(q) sum | Yes | `sum(sum(omega^(n-m)*q^(n*n+n*m+m*m), m, -N, N), n, -N, N)` |
| etamake(b(q), q, T) | Optional | Use b(q)=η(τ)³/η(3τ) workaround if prodmake over Q(ω) deferred |
| Display omega, omega^2 | Yes | Table stakes |

---

## MVP Definition

### Launch With (v1)

- [ ] **omega** — Built-in symbol for RootOf(z²+z+1=0); no general RootOf(poly) parser yet
- [ ] **Algnum** — Type (a₀, a₁) with add, mul, pow; ω^k reduces to ω^(k mod 3)
- [ ] **Series over Q(ω)** — Coefficients `std::map<int, Algnum>` or variant; add/mul for such series
- [ ] **sum(..., omega^exponent, ...)** — Evaluate ω^exponent to Algnum
- [ ] **Display** — `a + b*omega` or `a - b - b*omega` (reduced form)
- [ ] **b(q)** — Compute via sum with omega^(n-m)

### Add After Validation (v1.x)

- [ ] **convert(omega, radical)** — Display (-1+I√3)/2
- [ ] **Indexed RootOf** — RootOf(z²+z+1=0, index=1) vs index=2 (ω vs ω²)
- [ ] **etamake(b(q))** — If feasible without major prodmake rewrite

### Future Consideration (v2+)

- [ ] General RootOf(poly) for arbitrary irreducible polynomials
- [ ] Cyclotomic ζₙ for n > 3
- [ ] factor over Q(ω)

---

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| omega symbol | HIGH | LOW | P1 |
| Algnum add/mul/pow | HIGH | MEDIUM | P1 |
| Series with Q(ω) coeffs | HIGH | HIGH | P1 |
| sum(omega^k) evaluation | HIGH | MEDIUM | P1 |
| Display a+b*omega | HIGH | LOW | P1 |
| convert to radical | MEDIUM | LOW | P2 |
| General RootOf(poly) | LOW | HIGH | P3 |
| prodmake on Q(ω) | MEDIUM | HIGH | P3 |

**Priority key:** P1 = Must have for Block 10; P2 = Should have; P3 = Nice to have

---

## Competitor Feature Analysis

| Feature | Maple | Mathematica | SymPy | Our v1 Approach |
|---------|-------|-------------|-------|-----------------|
| Define ω | RootOf(z²+z+1=0) | Root[#²+#+1&,1] | RootOf(x²+x+1,0) | Built-in omega or RootOf(z²+z+1=0) |
| Arithmetic | evala | RootReduce | AlgebraicField | Algnum with explicit reduction |
| Display | omega, omega^2 | Root or radical | ANP | a + b*omega |
| Series coeffs | Symbolic | Symbolic | Symbolic | std::map<int, Algnum> |
| General poly | Full | Full | Full | Omit |

---

## Dependencies on Existing Frac/Series

| Existing Component | RootOf Integration |
|--------------------|--------------------|
| Frac | Algnum = (Frac, Frac) for Q(ω); Frac embeds as (f, 0) |
| Series::c | Extend to hold Algnum or use variant; Series over Q needs merge with Series over Q(ω) |
| sum() | Must recognize omega, evaluate omega^int_expr → Algnum |
| theta, aqprod | Return Series; if coefficients can be Algnum, series ops need Algnum add/mul |
| prodmake | Works on std::map<int, Frac>; Q(ω) version would need std::map<int, Algnum> — defer |
| findhom, findnonhom | Linear algebra over Q; Q(ω) would need linalg over Q(ω) — defer |

---

## Sources

- Maple RootOf documentation: https://www.maplesoft.com/support/help/Maple/view.aspx?path=RootOf (HIGH — official)
- Maple evala: algebraic evaluation for RootOf (HIGH)
- SymPy numberfields: https://docs.sympy.org/latest/modules/polys/numberfields.html (HIGH)
- Mathematica RootReduce, AlgebraicNumbers guide (MEDIUM — Wolfram docs)
- qseriesdoc.md §3.4, Exercise 4 (HIGH — project reference)
- maple_checklist.md Block 10 (HIGH — project parity target)
- PROJECT.md v8.0 RootOf milestone (HIGH)
- Cyclotomic field Q(ω): ω²+ω+1=0, ω³=1 (HIGH — standard)
