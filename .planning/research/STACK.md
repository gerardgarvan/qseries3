# Stack Additions: RootOf / Algebraic Numbers (Q(ω))

**Project:** q-series REPL — Milestone v8.0 RootOf
**Researched:** 2026-03-03
**Scope:** Zero-dependency C++; BigInt and Frac already available. Focus on cyclotomic Q(ω₃) for Block 10 and a(q), b(q), c(q).

---

## Executive Summary

For Maple parity (Block 10) and Exercise 4’s b(q) = Σ ω^(n-m) q^(n²+nm+m²), we need exact arithmetic in Q(ω) where ω = exp(2πi/3) = RootOf(z²+z+1=0). **Recommendation: cyclotomic-specific representation (a,b) with ω² = -ω-1.** Avoid general RootOf and full polynomial machinery. No GMP, no Boost.

---

## How Symbolic Systems Represent Algebraic Numbers

### Maple

- **RootOf(expr)** — placeholder for roots of one-variable polynomials
- **Elements of extension:** polynomials in α reduced modulo the defining polynomial
- **Canonical form:** primitive polynomial in _Z; eval in `evala` context
- **Source:** [Maple RootOf help](https://www.maplesoft.com/support/help/Maple/view.aspx?path=RootOf)

### PARI/GP

- **t_POLMOD** — algebraic number as (representative polynomial) mod (defining polynomial T)
- **Cyclotomics:** Mod(t, Φₙ(t)), with Mod(t, Φₙ(t)) ↦ exp(2πi/n)
- **Source:** PARI/GP General number fields docs

### SymPy

- **AlgebraicNumber** — minimal polynomial + choice of root (index or approximation)
- **Field elements:** polynomials in the extension generator
- **Source:** SymPy `polys.numberfields`

### GAP

- **Cyclotomics** — stored in the smallest cyclotomic field (conductor n)
- **Basis:** Zumbroich base; coefficients are rationals
- **Arithmetic:** done in Q_lcm(n,m), then reduced to minimal field
- **Source:** GAP3 Manual 13 Cyclotomics

### Regina (C++)

- **Cyclotomic** class — exact cyclotomic arithmetic; depends on `polynomial.h` and `rational.h`
- **Operations:** +, -, *, / with Rational scalars
- **Source:** Regina engine docs (cyclotomic.h)

---

## Recommended Stack Additions

### Core Representation: Cyclotomic Q(ω₃) Only

| Strategy | Data Structure | Purpose | Why |
|----------|----------------|---------|-----|
| **Omega3** | `struct { Frac a, b; }` for a + b·ω | Scalar type for Q(ω) | ω²+ω+1=0 → ω² = -ω-1. Multiplication: (a+bω)(c+dω) = (ac-bd) + (ad+bc-bd)ω. No polynomial ring, no reduction step. |
| **Arithmetic** | `Omega3::operator+`, `*`, `-`, `/` | Exact field operations | Uses only BigInt/Frac; division via conjugate: 1/(a+bω) = (a+bω²)/(a³+b³-3ab(a+b)) when a+bω≠0. |
| **Parser** | `omega` literal or `RootOf(z^2+z+1=0)` | Maple parity | Block 10 expects `omega := RootOf(z^2+z+1=0)`. |
| **SeriesAlg** | `std::map<int, Omega3>` + trunc + q_shift | Series with cyclotomic coefficients | Parallel to `Series` (map<int,Frac>). b(q) has coefficients in Q(ω). |

**Multiplication rule for ω²:**
```
(a + bω)(c + dω) = ac + (ad+bc)ω + bd·ω²
                 = ac + (ad+bc)ω + bd·(-ω-1)
                 = (ac - bd) + (ad + bc - bd)ω
```

**Inverse (a + bω)⁻¹:** Use 1/(a+bω) = (a+bω²) / N(a+bω) where the norm N(a+bω) = (a+bω)(a+bω²) = a² - ab + b² ∈ Q. So (a+bω)⁻¹ = ((a-b) - bω) / (a² - ab + b²).

### Supporting Implementation Patterns

| Pattern | Purpose | When to Use |
|---------|---------|-------------|
| **Omega3::fromRational(Frac)** | Embed Q into Q(ω) | Constant terms, rational coefficients |
| **Omega3::realPart()**, `omegaPart()` | Extract a, b | Debugging, display |
| **str()** | Display as `a + b*omega` or `RootOf(...)` form | REPL output |
| **Conjugate** | ω̄ = ω² | Norm, division, simplification |

### No New External Dependencies

- BigInt: existing arbitrary-precision integers
- Frac: existing exact rationals
- Omega3: pair of Frac with custom arithmetic
- SeriesAlg: same shape as Series, different coefficient type

---

## Alternatives Considered

| Recommended | Alternative | When to Use Alternative |
|-------------|-------------|--------------------------|
| **Cyclotomic-specific (a,b)** | General RootOf(minpoly, element) | If we later need √2, √5, or other algebraic numbers |
| **Omega3 struct** | `std::pair<Frac,Frac>` | Struct gives named `.a`, `.b` and clearer intent |
| **SeriesAlg parallel type** | Template `Series<Coeff>` | Template refactor touches many call sites; parallel type is lower risk |
| **Single extension Q(ω)** | Arbitrary cyclotomic Q(ζₙ) | For a(q),b(q),c(q) only ω₃ is required; φ(3)=2 keeps implementation small |

---

## What NOT to Use

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| **GMP, NTL, Flint, PARI** | External dependencies; project is zero-deps | BigInt + Frac + Omega3 |
| **General polynomial ring Q[x]** | Unnecessary for Q(ω₃); adds complexity | Cyclotomic-specific (a,b) |
| **Polynomial factorization** | Not needed for fixed Φ₃(z)=z²+z+1 | Hardcode ω² = -ω-1 |
| **Full RootOf(minpoly)** | Requires polynomial ops and reduction for every extension | Omega3 for Q(ω₃) only |
| **Floating-point approximations** | Requirement is exact arithmetic | Frac-based Omega3 |
| **Boost.Multiprecision** | External dependency | BigInt/Frac |

---

## Representation Options Comparison

### Option A: Cyclotomic-Indexed (GAP-style)

- **Structure:** conductor n + coefficient vector in Zumbroich (or power) basis
- **Pros:** Natural for many cyclotomics; embed Q(ζₙ) and Q(ζₘ) in Q(ζ_lcm(n,m))
- **Cons:** Basis and reduction for general n are nontrivial; overkill for n=3

### Option B: Minimal Polynomial + Element in Q(α)

- **Structure:** (minpoly, polynomial in α) reduced mod minpoly
- **Pros:** General; matches Maple, PARI, SymPy
- **Cons:** Needs polynomial ring, reduction; heavier than needed for ω₃

### Option C: Cyclotomic-Specific (a,b) for Q(ω₃) — Recommended

- **Structure:** `{ Frac a, Frac b }` for a + bω
- **Pros:** Minimal; no polynomial machinery; direct arithmetic; fits existing Frac/BigInt
- **Cons:** Only Q(ω₃); adding Q(ζ₅) etc. would need more cases or refactor

**Rationale for C:** Block 10 and Exercise 4 need only ω = exp(2πi/3). Cyclotomic-specific representation keeps scope small and integrates cleanly with Series.

---

## Integration with Existing Types

| Existing | Integration |
|----------|-------------|
| **Frac** | Omega3(a, 0) = rational embedding; Omega3 uses Frac for a, b |
| **BigInt** | Via Frac; no direct use in Omega3 |
| **Series** | Series stays Q-valued. Add SeriesAlg = map<int, Omega3> for b(q) etc. |
| **qfuncs** | b(q) double-sum uses Omega3 coefficients; a(q), c(q) stay over Q |
| **prodmake / etamake** | Remain over Q; b(q) as eta product η(τ)³/η(3τ) is Q-valued and already supported |
| ** relations (findhom, findpoly)** | Kernel over Q; no change. Optional: extend to Q(ω) later if relations with ω appear |

**Design choice:** Keep Series (Q-coefficients) and add SeriesAlg (Q(ω)-coefficients) as a sibling. Avoid templating Series on coefficient type to limit refactoring.

---

## Implementation Outline

1. **omega3.h** (new): `struct Omega3 { Frac a, b; }` with +, -, *, /, str(), fromRational(Frac).
2. **Parser:** Accept `omega` or `RootOf(z^2+z+1=0)`; bind to Omega3(0,1).
3. **qfuncs:** Add `bq_direct(int T)` or extend double-sum to accept Omega3 coefficients for ω^(n-m).
4. **repl.h:** Add Omega3 to EnvValue; SeriesAlg if needed for b(q) as series; display for Omega3.
5. **No changes** to prodmake, etamake, jacprodmake, relations, convert — they stay over Q.

---

## Sources

- Maple RootOf — [maplesoft.com/support/help/Maple/view.aspx?path=RootOf](https://www.maplesoft.com/support/help/Maple/view.aspx?path=RootOf) — canonical form, polynomial-in-α representation
- PARI/GP t_POLMOD — General number fields documentation — mod T representation
- GAP3 Cyclotomics — [webusers.imj-prg.fr/~jean.michel/gap3/htm/chap013.htm](https://webusers.imj-prg.fr/~jean.michel/gap3/htm/chap013.htm) — conductor, Zumbroich base
- Regina Cyclotomic — regina-normal.github.io/engine-docs/cyclotomic_8h.html — C++ exact cyclotomic arithmetic
- Wikipedia Cyclotomic field — degree φ(n), minimal polynomial Φₙ
- Project: qseriesdoc.md, maple_checklist.md, .planning/STATE.md — Block 10, a(q), b(q), c(q) context

---
*Stack research for: RootOf / Q(ω) in zero-dependency C++ q-series REPL*
