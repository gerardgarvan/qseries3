# Feature Landscape

**Domain:** q-series / partition theory / modular forms computation tools  
**Researched:** 2025-02-24  
**Confidence:** HIGH (SPEC.md, qseriesdoc.md, Maple qseries reference)

## Executive Summary

Mathematicians working in q-series, partition theory, and modular forms expect a tool that can **build** series from standard constructions, **convert** them to infinite products (Andrews' algorithm), **identify** eta/Jacobi products, and **find** polynomial relations. The Maple qseries package (Frank Garvan) is the reference implementation; Sage and Mathematica offer partial coverage. Table stakes are exact arithmetic, prodmake, core q-series building blocks, and the relation-finding family. Differentiators are zero dependencies, single binary, Maple-compatible syntax, and focused scope.

---

## Table Stakes

Features users expect. Missing = product feels incomplete or unusable.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| **Exact rational arithmetic** | Number theorists demand exactness; floats contaminate coefficients and relations | High | BigInt → Frac → Series. No floats in math pipeline |
| **prodmake** (Andrews' algorithm) | THE central operation: series → infinite product. Rogers-Ramanujan is the canonical test | High | Uses logarithmic derivative recurrence + divisor extraction |
| **etaq** | Dedekind eta products; partition function `1/η(τ)`; building block for most identities | Medium | `Π(1-q^{kn})`; workhorse function |
| **aqprod** | Rising q-factorial `(a;q)_n`; Rogers-Ramanujan left side, q-binomials | Medium | `Π(1-a·q^k)` for k=0..n-1 |
| **theta2, theta3, theta4** | Theta functions; Gauss AGM, modular forms, Jacobi products | Low | Sparse series; `Σ q^{n²}` etc. |
| **etamake** | Identify series as eta products; connect to modular form theory | Medium | Iteratively multiply by etaq until series = 1 |
| **jacprodmake** | Jacobi product form; Rogers-Ramanujan right side in JAC notation | High | Uses prodmake + periodicity detection; 80% match threshold |
| **sift** | Extract every n-th coefficient; Ramanujan dissections, quintuple product | Low | `Σ a_{ni+k} q^i` from f |
| **findhom / findnonhom** | Find polynomial relations; Gauss AGM, modular equations | High | Matrix kernel over Q; monomial enumeration |
| **findhomcombo / findnonhomcombo** | Express one series as polynomial in others; Eisenstein, Watson | High | Same as findhom but with extra column |
| **findpoly** | Polynomial relation between two series; cubic modular identities | Medium | Kernel with bounded degrees |
| **Series arithmetic** | +, -, *, /, pow, inverse, subs_q | High | Inverse recurrence is error-prone; truncation propagation critical |
| **REPL basics** | Variables, expressions, function calls, assignment | Medium | `x := etaq(1, 50)`; `prodmake(x, 40)` |
| **sum / add** | Build series from sums; Rogers-Ramanujan left side | Medium | `sum(q^(n^2)/aqprod(q,q,n), n, 0, 8)` |
| **series / coeffs display** | Inspect truncated series and coefficients | Low | Must match qseriesdoc output format |
| **qbin** | Gaussian polynomial; q-binomials in identities | Medium | Use product formula to avoid division instability |
| **tripleprod, quinprod, winquist** | Standard product identities; Euler pentagonal, Macdonald | Medium | Spec defines exact formulas |
| **qfactor** | Finite q-product factorization; T(r,n) examples | Medium | Uses prodmake internally |
| **Number theory helpers** | divisors, mobius, legendre, sigma, phi | Low | prodmake needs divisors; relations need legendre |

---

## Differentiators

Features that set the product apart. Not required, but valued.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| **Zero dependencies, single binary** | Run anywhere; no Maple/Sage/Python install; Windows/Linux portability | N/A (constraint) | Compiles with `g++ -static -O2` |
| **Maple qseries-compatible syntax** | Mathematicians can copy-paste from qseriesdoc.md and Garvan's package | Low | `aqprod(q,q,n,T)`, `prodmake(x,40)` |
| **C++ performance** | Faster than Maple for large truncations; T=500 comfortably | Medium | Series multiplication is hot path |
| **Focused scope** | No general CAS bloat; only q-series, partition, modular forms | N/A | Deliberate anti-feature avoidance |
| **Reproducible acceptance tests** | Every qseriesdoc example runs; Rogers-Ramanujan, Gauss AGM, Watson | Medium | SPEC defines 9 acceptance tests |
| **Built-in help** | `help`, `help(prodmake)` for quick reference | Low | Mathematicians expect it |

---

## Anti-Features

Features to explicitly NOT build.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|-------------------|
| **Floating point in math pipeline** | Corrupts coefficients; relations become approximate; mathematicians reject it | Exact rationals only; BigInt/Frac throughout |
| **External libraries (GMP, Boost)** | Violates single-binary, zero-dependency constraint | Roll BigInt from scratch (base 10^9) |
| **General-purpose CAS** | Polynomial calculus, Groebner bases, symbolic integration → scope creep | Stay within q-series / partition / modular forms |
| **Full readline / libedit** | External dependency; overkill for REPL | Simple line history buffer, optional termios |
| **Scripting language** | Loops, conditionals, modules → complexity | Script mode: read commands from file (`qseries < script.qs`) is stretch |
| **LaTeX output** | Nice-to-have, not table stakes | Defer to stretch; plain text first |
| **Arbitrary q-difference / recurrence guessers** | qFunctions (Mathematica) has this; different domain | Focus on prodmake/etamake/jacprodmake/relations |
| **Cylindric partitions, weighted words** | qFunctions specialty; outside Garvan package scope | Not in spec; defer indefinitely |

---

## Feature Dependencies

```
BigInt
  └── Frac (requires BigInt, gcd)
        └── Series (requires Frac)
              ├── qfuncs: aqprod, etaq, theta2/3/4, tripleprod, quinprod, winquist, qbin
              │     └── All require Series arithmetic (+, *, inverse, subs_q)
              ├── convert: prodmake (requires Series inverse, divisors, coeff extraction)
              │     ├── etamake (requires etaq, prodmake logic)
              │     ├── jacprodmake (requires prodmake output, periodicity search)
              │     └── qfactor (requires prodmake)
              ├── sift (requires Series coeff access)
              └── relations: findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly
                    └── linalg: kernel over Q (Gaussian elimination)
                          └── Requires Frac, matrix ops

Number theory helpers: divisors, mobius, legendre, sigma, phi
  └── prodmake needs divisors(n)
  └── findhomcombo (Eisenstein) needs legendre

Parser + REPL
  └── Requires all built-in functions, variable env, sum/add
```

### Dependency Notes

- **prodmake requires Series::inverse():** Division of series, qbin denominator handling, normalization. The inverse recurrence is the #1 source of bugs (j from 1 to n, not 0).
- **jacprodmake requires prodmake:** Extracts exponents a[n] from prodmake, then searches for periodicity in the exponent sequence.
- **findhom/findnonhom require linalg::kernel():** Build matrix of monomial q-expansions, compute rational null space.
- **etamake requires etaq:** Iteratively multiplies by etaq(k,T)^c to cancel terms.
- **All qfuncs require Series::subs_q(k):** theta3(q^2) = theta3(q).subs_q(2); common pattern.

---

## MVP Definition

### Launch With (v1)

Minimum viable product — reproducible qseriesdoc workflows.

- [x] **BigInt, Frac, Series** — foundation; no math without these
- [x] **etaq, aqprod, theta2/3/4, qbin** — building blocks
- [x] **prodmake** — Rogers-Ramanujan must pass
- [x] **etamake, jacprodmake** — product identification
- [x] **sift** — coefficient extraction
- [x] **findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly** — relation finding
- [x] **tripleprod, quinprod, winquist** — product identities
- [x] **qfactor** — finite factorization
- [x] **REPL:** variables, expressions, sum/add, series/coeffs
- [x] **All 9 SPEC acceptance tests** — Rogers-Ramanujan, partition, theta→eta, Jacobi, Gauss AGM, Rødseth, qfactor, Watson, Euler pentagonal

### Add After Validation (v1.x)

- [ ] **Script mode:** `qseries < script.qs` — batch runs
- [ ] **help / help(func)** — built-in documentation
- [ ] **Timing per command** — user tunes truncation
- [ ] **Multi-line input** — backslash continuation

### Future Consideration (v2+)

- [ ] **LaTeX output** — optional formatting
- [ ] **Tab completion** — function/variable names
- [ ] **Memoization** — cache etaq(k,T) by (k,T)
- [ ] **Euler pentagonal optimization** — O(T) for etaq(1,T) via pentagonal theorem

---

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| prodmake | HIGH | HIGH | P1 |
| etaq, aqprod, theta2/3/4 | HIGH | MEDIUM | P1 |
| Series + inverse | HIGH | HIGH | P1 |
| etamake, jacprodmake | HIGH | MEDIUM | P1 |
| findhom, findnonhom family | HIGH | HIGH | P1 |
| sift, qfactor | MEDIUM | LOW | P1 |
| tripleprod, quinprod, winquist | MEDIUM | MEDIUM | P1 |
| REPL, parser, sum | HIGH | MEDIUM | P1 |
| qbin | MEDIUM | MEDIUM | P1 |
| help | MEDIUM | LOW | P2 |
| Script mode | MEDIUM | LOW | P2 |
| LaTeX output | LOW | MEDIUM | P3 |
| Tab completion | LOW | LOW | P3 |
| Memoization | LOW | LOW | P3 |

**Priority key:** P1 = Must have for launch; P2 = Should have; P3 = Nice to have

---

## Competitor Feature Analysis

| Feature | Maple qseries | Sage (q_analogues) | Mathematica qFunctions | Our Approach |
|---------|---------------|--------------------|------------------------|--------------|
| prodmake | ✓ core | Partial (different API) | — | ✓ Andrews algorithm |
| etamake, jacprodmake | ✓ | — | — | ✓ Full reimplementation |
| findhom, findnonhom | ✓ | — | — | ✓ Rational kernel |
| aqprod, etaq, theta | ✓ | q_factorial, etc. | Some | ✓ Maple-compatible |
| Exact rational | ✓ | ✓ (QQ) | ✓ | ✓ Custom BigInt/Frac |
| Zero deps | ✗ (Maple) | ✗ (Python stack) | ✗ (Mathematica) | ✓ Single binary |
| q-diff / recurrence guessers | — | — | ✓ | ✗ Anti-feature (scope) |

---

## Mathematician Expectations

From qseriesdoc.md, SPEC acceptance tests, and CAS norms:

1. **Exactness:** Coefficients and relations must be exact rationals. No "0.999999999" in output.
2. **Rogers-Ramanujan:** Any q-series tool must recover the product form from the sum side. This is the litmus test.
3. **Product conversion chain:** prodmake → etamake (eta) or jacprodmake (Jacobi). Both workflows must work.
4. **Relation finding:** Given theta3, theta4, theta3(q²), theta4(q²), find Gauss AGM relations. Linear algebra over Q.
5. **Sift + identify:** Extract mod-n coefficients, then identify as eta/Jacobi product. Rødseth example.
6. **Syntax familiarity:** Maple-like `aqprod(q,q,n,T)`, `prodmake(x,40)` so papers and tutorials transfer.
7. **Truncation control:** User specifies T; tool respects it. Display `O(q^T)`.
8. **Reproducibility:** Same input → same output. No randomness, no floats.

---

## Sources

- SPEC.md — project specification, acceptance tests, architecture
- qseriesdoc.md — Frank Garvan q-series Maple package tutorial
- .cursorrules — layer ordering, common bugs
- Maple qseries package (qseries.org)
- Andrews, *q-Series* (CBMS 66) — prodmake algorithm
- Web search: q-series CAS tools, Sage q_analogues, Mathematica qFunctions
