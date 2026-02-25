# Architecture Patterns: Mathematical Computation Systems

**Domain:** Layered mathematical computation (CAS, symbolic math, formal power series)
**Researched:** 2025-02-24
**Confidence:** MEDIUM–HIGH (project-specific layers from SPEC; ecosystem patterns from SymPy, FLINT, NTL)

## Standard Architecture

### System Overview

Mathematical computation systems—CAS, number theory libraries, and formal power series tools—share a layered architecture. Data flows upward: each layer consumes types from the layer below and produces types consumed by the layer above.

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        USER / REPL / PARSER                              │
│  Commands, expressions, variable bindings, help                          │
├─────────────────────────────────────────────────────────────────────────┤
│                     ALGORITHM / DOMAIN LAYER                             │
│  prodmake, etamake, jacprodmake, findhom, findnonhom, findpoly           │
│  (Series → product/relation representations)                             │
├─────────────────────────────────────────────────────────────────────────┤
│                     FUNCTION / BUILDING-BLOCK LAYER                      │
│  aqprod, qbin, etaq, theta2/3/4, tripleprod, quinprod, winquist, sift   │
│  (Series in → Series out)                                                │
├─────────────────────────────────────────────────────────────────────────┤
│                     SERIES / POLYNOMIAL LAYER                            │
│  Truncated power series: map<int, Coeff> + trunc                         │
│  Arithmetic: +, -, *, /, inverse, pow, compose                           │
├─────────────────────────────────────────────────────────────────────────┤
│                     RATIONAL / COEFFICIENT LAYER                         │
│  Exact rationals (Frac), auto-reduced via GCD                            │
├─────────────────────────────────────────────────────────────────────────┤
│                     INTEGER / FOUNDATION LAYER                           │
│  BigInt: arbitrary precision signed integer                              │
└─────────────────────────────────────────────────────────────────────────┘
```

### Component Boundaries

| Component | Responsibility | Typical Implementation | Communicates With |
|-----------|----------------|------------------------|-------------------|
| **BigInt** | Arbitrary-precision integer arithmetic (+, -, *, divmod, gcd) | Base-10⁹ digits, `vector<uint32_t>`, schoolbook multiply/divide | Nothing (foundation) |
| **Frac** | Exact rationals; normalize via GCD on every op | `num/den` with `BigInt`; `reduce()` after construction | BigInt |
| **Series** | Truncated formal power series; sparse coeffs; propagate truncation | `map<int, Frac>` + `int trunc`; ordered iteration | Frac |
| **qfuncs** | q-series building blocks (eta, theta, products) | Iterative products/sums producing Series | Series |
| **convert** | Series → product forms (prodmake, etamake, jacprodmake) | Andrews recurrence, divisor extraction, period detection | Series, qfuncs, number-theory helpers |
| **linalg** | Linear algebra over Q (Gaussian elimination, kernel) | Row reduction over `vector<vector<Frac>>` | Frac |
| **relations** | Polynomial relations between series (findhom, findpoly) | Monomial generation, matrix build, kernel → relations | linalg, Series |
| **parser** | Tokenize and parse expressions; recursive descent | AST; no math types | — |
| **repl** | Read-eval-print loop; variable env; dispatch to functions | Orchestrates parser + eval + display | parser, all math |

### Data Flow

**Upward flow (computation):**
```
User input (string)
    ↓
Parser → AST
    ↓
Eval: resolve vars, call built-ins
    ↓
qfuncs / convert / relations → Series (or product/relation representation)
    ↓
Series arithmetic (uses Frac → BigInt)
    ↓
Display: str(), series(), prodmake output, etc.
```

**Algorithm-specific flows:**

1. **prodmake (Andrews):** `Series` coeffs `b[0..T-1]` → recurrence `c[n]` → divisor extraction `a[n]` → product form. Depends on `divisors(n)`, `Frac` arithmetic.
2. **findhom:** List of `Series` → monomial products → coefficient matrix → `kernel(M)` → relation polynomials. Depends on `linalg` kernel over `Frac`.
3. **etamake:** `Series` → greedy cancellation with `etaq(k,T)` → eta-product representation. Depends on `qfuncs::etaq`.

## Recommended Project Structure

For Q-Series REPL (header-only, single binary):

```
bigint.h       — Foundation: BigInt
frac.h         — Rationals: Frac (uses BigInt)
series.h       — Power series: Series (uses Frac)
qfuncs.h       — aqprod, qbin, etaq, theta*, tripleprod, quinprod, winquist
convert.h      — prodmake, etamake, jacprodmake, qfactor
linalg.h       — Gaussian elimination, kernel over Q
relations.h    — findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly
parser.h       — Tokenizer + recursive-descent parser
repl.h         — REPL loop, variable env, help
main.cpp       — Entry point
```

**Structure rationale:**
- **bigint.h / frac.h / series.h:** Pure type + arithmetic. No domain logic. Each layer only imports the one below.
- **qfuncs.h / convert.h:** Domain logic; depend only on Series (and number-theory helpers). No linalg.
- **linalg.h:** Generic over Frac; used by relations only.
- **relations.h:** Orchestrates Series + linalg + qfuncs (for etamake display).
- **parser.h / repl.h:** Can be developed largely in parallel; depend on math APIs.

## Build Order and Dependencies

**Recommended build order (bottom-up, each layer testable before the next):**

| Phase | Components | Rationale |
|-------|------------|-----------|
| 1 | bigint.h | Foundation; no deps. Test division, sign handling, edge cases. |
| 2 | frac.h | Needs BigInt. Test reduction, arithmetic. |
| 3 | series.h | Needs Frac. Test (1-q)*(1/(1-q)) = 1, inverse recurrence. |
| 4 | qfuncs.h | Needs Series. Test etaq(1,T) pentagonal, theta3, aqprod. |
| 5 | convert.h | Needs Series, qfuncs (etaq), divisors. **prodmake is acceptance-critical** — Rogers–Ramanujan. |
| 6 | linalg.h | Needs Frac. Test kernel over small rational matrices. |
| 7 | relations.h | Needs linalg, Series, qfuncs. Test findhom for theta relations. |
| 8 | parser.h | No math deps. Test expression parsing. |
| 9 | repl.h | Needs parser + all math. Test full REPL. |
| 10 | main.cpp | Entry point. |

**Dependency DAG:**
```
BigInt
  ↓
Frac
  ↓
Series ← divisors, mobius (small helpers)
  ↓
qfuncs (aqprod, etaq, theta, ...)
  ↓
convert (prodmake, etamake, jacprodmake)
  ↓
relations (findhom, findpoly, ...)
  ↑
linalg (kernel over Q) — depends on Frac only
```

**Critical constraint:** prodmake and relations cannot be tested meaningfully until Series and qfuncs are correct. The Rogers–Ramanujan prodmake test is the primary acceptance gate.

## Architectural Patterns

### Pattern 1: Bottom-Up Type Stack

**What:** Each layer defines types whose coefficients or components come from the layer below (BigInt → Frac → Series).

**When to use:** Any exact-arithmetic math system.

**Trade-offs:** Clear dependencies; changes at the bottom can ripple up. Benefit: correctness of BigInt/Frac/Series guarantees correctness of higher layers.

**Example:**
```cpp
// Frac uses BigInt
struct Frac { BigInt num, den; };

// Series uses Frac
struct Series { std::map<int, Frac> c; int trunc; };
```

### Pattern 2: Truncation Propagation

**What:** Every binary operation on truncated series sets `result.trunc = min(a.trunc, b.trunc)`. Unary ops preserve trunc. Never produce coefficients at or beyond trunc.

**When to use:** Formal power series with explicit precision.

**Trade-offs:** Simpler reasoning; no implicit precision. Must thread truncation through all Series APIs.

**Example:**
```cpp
Series operator*(const Series& a, const Series& b) {
    Series r;
    r.trunc = std::min(a.trunc, b.trunc);
    for (auto& [e1, c1] : a.c)
        for (auto& [e2, c2] : b.c)
            if (e1 + e2 < r.trunc)
                r.c[e1 + e2] += c1 * c2;
    return r;
}
```

### Pattern 3: Sparse Series with Ordered Map

**What:** Store only nonzero coefficients in `std::map<int, Coeff>`. Iteration is in exponent order.

**When to use:** q-series and many special functions; coefficients are sparse.

**Trade-offs:** Memory-efficient for sparse series; ordered iteration needed for display, prodmake, and coefficient extraction. `std::map` (not `unordered_map`) is required.

### Pattern 4: Immutable Expression Trees (CAS-Style)

**What:** Expressions as trees; `expr.func(*expr.args) == expr`; immutability enables hashing and caching. (SymPy pattern.)

**When to use:** Full CAS with symbolic simplification. **Not used in Q-Series REPL** — we evaluate to Series immediately.

**Trade-offs:** Flexible, extensible; heavier than direct Series evaluation. Q-Series REPL uses parse → eval → Series, not symbolic trees.

### Pattern 5: Algorithm Selection by Operand Size

**What:** Choose algorithms (schoolbook vs Karatsuba vs FFT) based on operand size. (FLINT/NTL pattern.)

**When to use:** High-performance libraries. **Deferred for Q-Series** — schoolbook is sufficient for typical truncations (T ≤ 500).

## Anti-Patterns to Avoid

### Anti-Pattern 1: Skipping Frac Reduction

**What people do:** Build Fracs without calling `reduce()` after each operation.

**Why it's wrong:** Numerators and denominators grow exponentially; GCD and display become expensive; risk of overflow.

**Do this instead:** Call `reduce()` in every Frac constructor and after every arithmetic operation.

### Anti-Pattern 2: Wrong Inverse Recurrence Index

**What people do:** In `Series::inverse()`, use `Σ_{j=0}^{n} c_j * g[n-j]` (including j=0).

**Why it's wrong:** Mathematically incorrect; the recurrence is `g[n] = -(1/c₀) Σ_{j=1}^{n} c_j g[n-j]`.

**Do this instead:** Loop j from 1 to n, not 0 to n.

### Anti-Pattern 3: Divider Sum Including d=n in prodmake

**What people do:** In prodmake step 3, use `Σ_{d|n} d*a[d]` including `d=n`.

**Why it's wrong:** The inversion formula requires `Σ_{d|n, d<n} d*a[d]`; `a[n]` is solved from that.

**Do this instead:** Exclude `d = n` in the divisor sum.

### Anti-Pattern 4: Dense Series for Sparse q-Series

**What people do:** Use `vector<Frac>` indexed by exponent.

**Why it's wrong:** Wastes memory; many q-series have few nonzero terms up to truncation.

**Do this instead:** Use `std::map<int, Frac>` for sparse storage with ordered iteration.

### Anti-Pattern 5: Building Layers Out of Order

**What people do:** Implement prodmake or relations before Series arithmetic is verified.

**Why it's wrong:** Bugs in prodmake are indistinguishable from bugs in Series inverse or multiplication.

**Do this instead:** Validate each layer before the next. Use `(1-q)*(1/(1-q)) = 1` as a Series sanity check.

## Data Flow Summary

| Flow | Direction | Key Types |
|------|-----------|-----------|
| User → Eval | Down | string → AST |
| Eval → Series | Down | AST → Series (via qfuncs/convert) |
| Series → Product | Algorithm | Series coeffs → a[n] → product representation |
| Relations | Algorithm | List&lt;Series&gt; → matrix → kernel → polynomials |
| Display | Up | Series / product / relation → string |

## Integration Points

### Internal Boundaries

| Boundary | Communication | Notes |
|----------|---------------|-------|
| Parser ↔ REPL | AST, variable names | Parser is stateless; REPL holds env |
| REPL ↔ Math | Function calls with Series/params | REPL resolves names, dispatches to qfuncs/convert/relations |
| convert ↔ qfuncs | etaq(k,T) calls | etamake uses etaq for cancellation |
| relations ↔ linalg | Matrix of Frac, kernel result | relations builds matrix, linalg returns kernel basis |

### Number Theory Helpers

Small helpers (`divisors`, `mobius`, `sigma`, `euler_phi`) are used by prodmake and optionally elsewhere. They belong in a lightweight utility layer (or inline in convert.h) — no BigInt needed for typical T.

## Scaling Considerations

For a single-binary REPL targeting T ≤ 500:

| Scale | Architecture Adjustments |
|-------|--------------------------|
| T ≤ 100 | Current design is sufficient. Schoolbook BigInt, no caching. |
| T ~ 500 | Memoize etaq(k,T) for repeated k,T. Consider lazy coefficient evaluation only if needed. |
| T > 1000 | Karatsuba for BigInt multiply; FFT-based series multiply; batch eta product construction. |

**First bottleneck:** Series multiplication (O(T²) coefficient pairs, each a Frac multiply). Optimize inner loop (skip e1+e2 ≥ trunc early).

**Second bottleneck:** findhom monomial count C(n+k-1, k-1) and matrix size. Limit degree n and list size k in REPL.

## Sources

- [SymPy Architecture](https://www.cfm.brown.edu/people/dobrush/am33/SymPy/architecture.html) — expression trees, Basic class, immutability
- [FLINT fmpz_poly](https://flintlib.org/doc/fmpz_poly.html) — polynomials over integers, coefficient types
- [FLINT overview](https://flintlib.org/index.html) — layered rings: integers → rationals → polynomials
- [NTL Library](https://libntl.org/) — C++ number theory, polynomials, similar layering
- SPEC.md, .cursorrules — Q-Series REPL layer definitions and constraints

---
*Architecture research for: Layered mathematical computation (CAS, formal power series)*
*Researched: 2025-02-24*
