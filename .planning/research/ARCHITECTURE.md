# Architecture: Half-Integer Jacobi Exponents & q-Shift Arithmetic

**Project:** qseries3
**Researched:** 2026-03-01
**Overall confidence:** HIGH
**Scope:** Fixes for 3 Maple checklist block failures (13, 14, 25) and 3 dependent exercises (5, 9, 10)

---

## Executive Summary

Three checklist failures share a common root cause: the Series arithmetic layer assumes integer exponents and matching q-shifts everywhere, but certain Jacobi products and theta-function quotients produce half-integer exponents and mismatched fractional q-shifts. Fixing these requires three targeted changes to three files, with clear dependency ordering:

1. **Series::pow(Frac)** — Generalized binomial series for fractional powers (enables half-integer Jacobi exponents)
2. **jac2series_impl** — Use `pow(Frac)` instead of integer iteration (enables Block 14)
3. **jacprodmake** — Detect when fractional exponents are needed in the decomposition (enables Block 13)
4. **Series::operator+** — Reconcile mismatched q-shifts when their difference is an integer (enables Block 25, Exercise 10)

---

## Current Architecture

### Data Flow: Series → Product → Jacobi

```
User expression
    │
    ▼
eval() → Series { map<int,Frac> c, int trunc, Frac q_shift }
    │
    ├── prodmake(f, T) → map<int, Frac>     // Andrews' algorithm: a[n] exponents
    │       │
    │       ├── jacprodmake(f, T)            // Detect period b, decompose into residues
    │       │       → vector<JacFactor>      // JacFactor = tuple<int a, int b, Frac e>
    │       │
    │       └── etamake(f, T)                // Detect eta-product form
    │               → vector<pair<int, Frac>>
    │
    └── jac2series(jac, T) → Series          // Reconstruct from Jacobi factors
```

### Key Struct: JacFactor

```cpp
using JacFactor = std::tuple<int, int, Frac>;
//                            a    b    exponent
// Represents JAC(a, b, ∞)^e = (q^a; q^b)_∞^e · (q^{b-a}; q^b)_∞^e · (q^b; q^b)_∞^e
// When a=0: JAC(0, b, ∞)^e = (q^b; q^b)_∞^e
```

The exponent field is already `Frac`, so the **type** supports half-integer exponents. The **algorithms** do not.

### Key Struct: Series

```cpp
struct Series {
    std::map<int, Frac> c;   // coefficients indexed by integer exponent
    int trunc;                // truncation order
    Frac q_shift;             // series represents q^{q_shift} · Σ c[n] q^n
};
```

The `q_shift` field handles fractional q-powers (e.g., theta2 sets `q_shift = 1/4`). Arithmetic propagation rules:
- **Multiply:** result.q_shift = a.q_shift + b.q_shift
- **Inverse:** result.q_shift = -a.q_shift
- **Add:** REQUIRES a.q_shift == b.q_shift (currently throws otherwise)

---

## Failure Analysis

### Block 13–14: Half-Integer Jacobi Exponents

**Input:** `sum(q^(n*(n+1)/2)*aqprod(-q,q,n,50)/aqprod(q,q,2*n+1,50), n, 0, 10)`

**Expected (from qseriesdoc.md):**
```
JAC(0,14,∞)^(13/2) / (JAC(1,14,∞)² JAC(3,14,∞) JAC(4,14,∞)
                       JAC(5,14,∞) JAC(6,14,∞) √JAC(7,14,∞))
```

Note exponents `13/2` and `1/2` (the √). The current `jacprodmake` produces integer exponents via the decomposition `x[a] = e[a]` where `e[n]` comes from `prodmake`. When the underlying series has a Jacobi product with half-integer exponents, `prodmake` produces non-integer `a[n]` values, and the symmetry check `e[a] == e[b-a]` may fail or produce fractional `x[a]`.

**Root cause chain:**
1. `prodmake` correctly produces `a[n]` as `Frac` — some are half-integers like `1/2`, `13/2`
2. `jacprodmake` decomposition already stores `x[a]` as `Frac` — this part works
3. `jac2series_impl` truncates the `Frac` exponent to `int` at line 353–355 — **this is where it breaks**
4. With int-truncated exponents, the reconstruction `jac2series_impl(result, T)` doesn't match the input
5. `jacprodmake` returns `{}` (empty — no valid Jacobi decomposition found)

**The fix is in the reconstruction**, not the decomposition. Once `jac2series_impl` can handle `Frac` exponents, the existing decomposition logic will find a match.

### Block 25: Mixed q-Shift Addition

**Input:** Block 25 computes `x1 + x2` where:
- `x1 = theta2(q,100)^2 / theta2(q^3,40)^2` → `q_shift = 2·(1/4) - 2·(3/4) = -1`
- `x2 = theta3(q,100)^2 / theta3(q^3,40)^2` → `q_shift = 0`

**Error:** `"cannot add series with different q-shifts"` (q_shift -1 ≠ 0)

**Key insight:** The difference `x2.q_shift - x1.q_shift = 0 - (-1) = 1` is an integer. This means we can reconcile by shifting one series' coefficient map. Specifically, `x2 = q^0 · Σ d_n q^n = q^{-1} · Σ d_n q^{n+1}`, so we shift x2's exponents by +1 and use x1's q_shift.

**When reconciliation is impossible:** If the q_shift difference is NOT an integer (e.g., `1/4` vs `0`), the series genuinely cannot be added as integer-indexed power series. The current error is correct for that case.

### Exercise Dependencies

| Exercise | Blocked By | Fix Required |
|----------|-----------|--------------|
| 5 (Slater 46) | Block 13 | Half-integer Jacobi exponents in jacprodmake + jac2series |
| 9 (N(q) in terms of a,x) | sigma(n,k) performance | Already implemented; needs double-sum optimization |
| 10 (findpoly m vs y) | Block 25 | q-shift reconciliation for theta2/theta3 quotient addition |

---

## Proposed Changes

### Change 1: Series::pow(Frac) — Generalized Binomial Series

**File:** `src/series.h`
**Type:** New method on Series

**Algorithm:** For `f^α` where `f(0) = c₀ ≠ 0`:

1. Factor out `c₀`: let `g = f/c₀ - 1` (so `g(0) = 0`)
2. Compute `c₀^α` (exact rational power — only works when `c₀` is a perfect power or `α` produces a rational result; for `c₀ = 1` this is trivial)
3. Apply generalized binomial: `(1+g)^α = Σ_{k=0}^{trunc} C(α,k) · g^k`
4. Where `C(α,0) = 1` and `C(α,k) = C(α,k-1) · (α-k+1) / k`
5. Multiply by `c₀^α`
6. Set `q_shift = α · f.q_shift`

**Constraint:** When `c₀ ≠ 1` and `α` is non-integer, `c₀^α` may not be rational. For Jacobi products, the factors `(q^a;q^b)_∞` always have constant term 1, so this is not a practical concern — but the code should throw a clear error if `c₀^α` is irrational.

```cpp
Series powFrac(Frac alpha) const {
    if (alpha.isZero()) return one(trunc);
    if (alpha.den == BigInt(1)) {
        int n = /* extract int from alpha */;
        return pow(n);  // delegate to existing integer pow
    }
    Frac c0 = coeff(minExp());
    if (!c0.isOne())
        throw std::runtime_error("powFrac: non-unit constant term with fractional exponent");

    // g = f - 1 (constant term is 0)
    Series g = *this;
    g.c.erase(0);  // or adjust for minExp
    g.q_shift = Frac(0);

    Series result = one(trunc);
    Series gk = one(trunc);        // g^0
    Frac binom_coeff(1);           // C(alpha, 0)

    for (int k = 1; k < trunc; ++k) {
        binom_coeff = binom_coeff * (alpha - Frac(k - 1)) / Frac(k);
        if (binom_coeff.isZero()) break;
        gk = (gk * g).truncTo(trunc);
        if (gk.c.empty()) break;
        result = result + gk * binom_coeff;
    }
    result.q_shift = q_shift * alpha;  // not Frac(0) — propagate
    result.clean();
    return result;
}
```

**Performance note:** This is O(trunc²) in series multiplications for the `gk = gk * g` step. For trunc = 50, that's 50 iterations with ~50-term series multiplications each — fast enough. For trunc = 500 (Block 14's `jac2series(jp, 500)`), this is ~500 iterations with growing series — potentially slow but tractable since each `g^k` term gains at least 1 in minimum exponent, causing convergence.

**Integration with existing pow(int):** The existing `pow(int n)` uses binary exponentiation. Add a `powFrac(Frac)` method and modify `pow` to dispatch:

```cpp
Series pow(int n) const { /* existing binary exp */ }
Series powFrac(Frac alpha) const { /* new generalized binomial */ }
```

### Change 2: jac2series_impl — Use powFrac

**File:** `src/convert.h`
**Type:** Modify existing function

**Current code (lines 353–361):**
```cpp
int ex = 0;
if (exp.den == BigInt(1) && ...) ex = /* truncate to int */;
if (ex > 0) { for (int i = 0; i < ex; ++i) prod = prod * fac; }
else if (ex < 0) { /* inverse loop */ }
```

**New code:**
```cpp
if (exp.den == BigInt(1)) {
    // Integer exponent — use existing fast binary pow
    int ex = /* extract int */;
    prod = (prod * fac.pow(ex)).truncTo(T);
} else {
    // Fractional exponent — use generalized binomial
    prod = (prod * fac.powFrac(exp)).truncTo(T);
}
```

This is the minimal change. The `fac` series (from `jac_factor_series`) always has constant term 1, so `powFrac` will succeed.

### Change 3: jacprodmake — Allow Fractional Exponents in Decomposition

**File:** `src/convert.h`
**Type:** Modify existing function (lines 367–428)

The current decomposition logic already stores `x[a]` as `Frac` and handles non-integer values correctly. The **only** reason it fails is that `jac2series_impl` can't reconstruct with fractional exponents, so the verification step (`recon.coeff(n) != f.coeff(n)`) fails and it returns `{}`.

Once Change 2 is applied, **no change to jacprodmake itself may be needed**. The existing decomposition + reconstruction should work. However, verify with the Slater (46) test case. If the decomposition logic produces incorrect fractional exponents, it may need refinement in the symmetry check or the `x[0]` remainder calculation.

**Potential issue:** The `prodmake` step produces `a[n]` values. For the Slater (46) case, some `a[n]` will be `1/2`, `13/2`, etc. The warning `"prodmake: non-integer a[n]"` fires (line 124–126) — this is informational, not an error. The jacprodmake decomposition uses these non-integer values directly. The symmetry check `e[a] == e[b-a]` should still hold because the Jacobi product structure is inherently symmetric.

### Change 4: Series::operator+ — q-Shift Reconciliation

**File:** `src/series.h`
**Type:** Modify existing operator (lines 131–149)

**Current behavior:** Throws if `q_shift` differs and both series are non-empty.

**New behavior:** If `q_shift` differs, compute `delta = o.q_shift - q_shift`. If `delta` is an integer, shift `o`'s coefficient map by `delta` and use `this->q_shift`. If `delta` is not an integer, throw (unchanged).

```cpp
Series operator+(const Series& o) const {
    if (!(q_shift == o.q_shift) && !c.empty() && !o.c.empty()) {
        Frac delta = o.q_shift - q_shift;
        if (delta.den != BigInt(1))
            throw std::runtime_error("cannot add series with incompatible q-shifts ("
                + q_shift.str() + " vs " + o.q_shift.str() + ")");
        int shift = /* extract int from delta */;
        // Use whichever q_shift is smaller (more negative = more terms visible)
        Frac result_shift = (q_shift < o.q_shift) ? q_shift : o.q_shift;
        int self_offset = /* q_shift - result_shift as int */;
        int other_offset = /* o.q_shift - result_shift as int */;
        Series s;
        s.trunc = min(trunc, o.trunc);
        s.q_shift = result_shift;
        for (auto& [e, v] : c)
            if (e + self_offset < s.trunc) s.setCoeff(e + self_offset, v);
        for (auto& [e, v] : o.c) {
            int ne = e + other_offset;
            if (ne < s.trunc) s.setCoeff(ne, s.coeff(ne) + v);
        }
        s.clean();
        return s;
    }
    // ... existing code for matching q_shifts ...
}
```

**Subtlety — choosing the result q_shift:** Use the minimum (most negative) q_shift so that both series' coefficients map to non-negative or least-negative integer exponents. For `q_shift = -1` and `q_shift = 0`:
- result_shift = -1
- self_offset = (-1) - (-1) = 0 (no change)
- other_offset = 0 - (-1) = 1 (shift o's exponents up by 1)

This correctly produces `q^{-1} · (c_0 + (c_1 + d_0)q + (c_2 + d_1)q² + ...)`.

**operator- follows automatically** since `a - b = a + (-b)` and negation preserves q_shift.

### Change 5 (Optional): Double-Sum Performance for sigma-Based Exercises

**File:** `src/repl.h` or `src/qfuncs.h`
**Type:** Optimization, not required for correctness

For `N(q) = 1 - 504*sum(sigma(n,5)*q^n, n, 1, 50)`, the computation is:
```
for n = 1 to 50:
    compute sigma(n, 5)              // fast: O(√n) divisors
    create Series::constant * q^n    // fast: single-term series
    add to accumulator               // fast: O(1) map insert
```

This is O(50 · √50) ≈ 350 operations — trivially fast. No optimization needed.

For nested double sums like `sum(sum(legendre(m,7)*n^2*q^(m*n), n, 1, 50), m, 1, 50)`:
- Inner loop: 50 iterations, each creating a 1-term series (q^{m*n})
- Outer loop: 50 iterations, each adding a ~50-term series
- Total: 2500 series additions — each O(1) for single-term series

This is also tractable (~seconds). **No architectural change needed** for sigma or double-sum exercises.

---

## Component Boundaries

| Component | Responsibility | Modified? | New Methods |
|-----------|---------------|-----------|-------------|
| `series.h` | Series struct, arithmetic operators | **Yes** | `powFrac(Frac)`, modified `operator+` |
| `convert.h` | prodmake, jacprodmake, jac2series | **Yes** | Modified `jac2series_impl` dispatch |
| `qfuncs.h` | theta, eta, q-product functions | No | — |
| `repl.h` | REPL evaluation, display formatting | Possibly | `jac2prod` display for fractional exponents |
| `parser.h` | Tokenizer, expression parser | No | — |
| `frac.h` | Exact rational arithmetic | No | — |
| `bigint.h` | Arbitrary precision integers | No | — |

### Display Formatting for Fractional Exponents

The `jac2prod()` display function (lines 434–460 in `convert.h`) already uses `exp.str()` for exponent display. For half-integer exponents:
- `Frac(13, 2).str()` → `"13/2"` — acceptable but not the `^(13/2)` format Maple uses
- Consider displaying `√` for exponent 1/2 and `^(n/2)` for other half-integers

The `formatProdmake` function in `repl.h` uses `Series::expToUnicode()` for integer exponents. Fractional exponents need `Series::fracExpStr()` which already exists (line 284 of series.h).

---

## Data Flow Changes

### Before (integer-only path)

```
jacprodmake
  → prodmake → a[n] (Frac, but expected integer)
  → decompose → x[a] (Frac)
  → jac2series_impl → TRUNCATES to int → loop multiply
  → verify → FAILS for half-integer cases → returns {}
```

### After (fractional path)

```
jacprodmake
  → prodmake → a[n] (Frac, half-integers OK)
  → decompose → x[a] (Frac, half-integers OK)
  → jac2series_impl → dispatch: int → binary pow, frac → powFrac
  → verify → PASSES for half-integer cases → returns factors
```

### q-Shift Reconciliation

```
Before:
  theta2(q)^2/theta2(q^3)^2  →  q_shift = -1
  theta3(q)^2/theta3(q^3)^2  →  q_shift = 0
  x1 + x2  →  THROW "cannot add"

After:
  x1 + x2  →  delta = 0 - (-1) = 1 (integer!)
           →  result q_shift = -1
           →  shift x2 exponents by +1
           →  add coefficient maps
           →  SUCCESS
```

---

## Implementation Order (Dependency-Driven)

### Phase 1: Series::powFrac (series.h)

**No dependencies.** This is a pure addition to the Series struct. Test independently:
```
f = 1 - q + q² (trunc=20)
f.powFrac(Frac(1,2)) should give (1-q+q²)^{1/2} via binomial
Verify: f.powFrac(Frac(1,2))^2 == f  (within truncation)
```

### Phase 2: jac2series_impl dispatch (convert.h)

**Depends on Phase 1.** Modify the exponent handling to call `powFrac` for non-integer exponents. Test:
```
// Manually construct JacFactor with Frac(1,2) exponent
jac = {(0, 5, Frac(1,2))}  // JAC(0,5,∞)^{1/2}
jac2series(jac, 50)         // should produce (q^5;q^5)_∞^{1/2}
```

### Phase 3: jacprodmake verification (convert.h)

**Depends on Phase 2.** Run Block 13 test case:
```
x := sum(q^(n*(n+1)/2)*aqprod(-q,q,n,50)/aqprod(q,q,2*n+1,50), n, 0, 10)
jp := jacprodmake(x, 50)
// Should produce: JAC(0,14)^{13/2}, JAC(1,14)^{-2}, JAC(3,14)^{-1}, ...
```

If the decomposition fails, debug the symmetry check in jacprodmake (the `e[a] == e[b-a]` condition with fractional values).

### Phase 4: jac2prod display (convert.h or repl.h)

**Depends on Phase 3.** Ensure `jac2prod(jp)` displays fractional exponents correctly:
```
// Expected: (q^14,q^14)_∞^(13/2) / ((q,q^14)_∞² ...)
```

### Phase 5: q-shift reconciliation (series.h)

**Independent of Phases 1–4.** Modify `operator+` and test:
```
a = Series with q_shift = Frac(-1), c = {0: 1, 1: -2}
b = Series with q_shift = Frac(0),  c = {0: 3, 1: 4}
a + b should produce q_shift = -1, c = {0: 1, 1: -2+3, 2: 4}
```

### Phase 6: Block 25 integration test

**Depends on Phase 5 + existing theta q-argument fix.** Run Block 25:
```
x1 := theta2(q,100)^2/theta2(q^3,40)^2
x2 := theta3(q,100)^2/theta3(q^3,40)^2
x := x1 + x2
// Should succeed (q_shift difference is integer)
```

### Dependency Graph

```
Phase 1: Series::powFrac
    │
    ▼
Phase 2: jac2series_impl dispatch
    │
    ▼
Phase 3: jacprodmake verification ──► Phase 4: jac2prod display
    │
    (Blocks 13-14 fixed)

Phase 5: q-shift reconciliation (independent)
    │
    ▼
Phase 6: Block 25 integration test
    │
    (Block 25 + Exercise 10 fixed)
```

---

## Patterns to Follow

### Pattern 1: Generalized Binomial Coefficients over Q

For `(1 + g)^α` where `α ∈ Q` and `g(0) = 0`:

```
C(α, 0) = 1
C(α, k) = C(α, k-1) · (α - k + 1) / k
```

All operations are Frac arithmetic — stays exact. The series converges (truncates) because `g^k` gains minimum exponent with each multiplication, eventually exceeding `trunc`.

**Key invariant:** `g.minExp() ≥ 1` ensures `g^k.minExp() ≥ k`, so after `k ≥ trunc` iterations the contribution is zero. This guarantees termination.

### Pattern 2: q-Shift Reconciliation Guard

Only reconcile when `delta.den == 1` (integer difference). For non-integer differences, the series represent genuinely incompatible power series (e.g., `q^{1/4} · f(q)` + `q^{1/3} · g(q)` cannot be combined without introducing `q^{1/12}` granularity, which would require reindexing all coefficients).

### Pattern 3: powFrac Constant Term Validation

For `f^α`, if `f` has non-unit leading coefficient, the fractional power of a rational number may be irrational (e.g., `2^{1/3}`). The code must validate this and throw a clear error rather than producing wrong results. In practice, all Jacobi factor series `(q^a;q^b)_∞` have constant term 1, so this check is defensive.

---

## Anti-Patterns to Avoid

### Anti-Pattern 1: Reindexing to Sub-Integer Granularity

**Don't** try to handle `q_shift = 1/4 + q_shift = 1/3` by reindexing to `q^{1/12}` granularity. This would multiply the coefficient map size by 12 and cascade through all downstream operations. The integer-difference guard is the correct boundary.

### Anti-Pattern 2: Float Approximation for powFrac

**Don't** approximate `Frac(1,2)` as `0.5` and use floating-point pow. The project requires exact rational arithmetic everywhere. The generalized binomial approach stays exact.

### Anti-Pattern 3: Separate Half-Integer Path in jacprodmake

**Don't** add a separate code path in `jacprodmake` for "half-integer mode." The existing decomposition already produces `Frac` exponents. The only fix needed is downstream (jac2series reconstruction). Adding a parallel path would duplicate logic and create maintenance burden.

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| powFrac convergence too slow for T=500 | LOW | Block 14 slow | Early termination when g^k exceeds trunc; minExp grows linearly |
| prodmake warns on non-integer a[n] | NONE | Cosmetic | Suppress warning when jacprodmake is the caller, or make it conditional |
| q-shift reconciliation edge cases | LOW | Wrong coefficients | Comprehensive test: verify a+b then (a+b)-a == b |
| c₀^α irrational | NONE | Jacobi factors always have c₀=1 | Defensive throw for non-unit case |
| jacprodmake symmetry check fails with fractions | LOW | Block 13 empty result | Frac equality is exact; should work if prodmake values are correct |

---

## Scalability Notes

| Concern | At T=50 | At T=500 | At T=5000 |
|---------|---------|----------|-----------|
| powFrac iterations | 50 binomial terms | 500 terms (each with growing series mult) | Likely too slow; would need Newton's method or exp/log approach |
| q-shift reconciliation | Instant | Instant | Instant (O(n) map traversal) |
| jacprodmake + powFrac reconstruction | < 1s | ~10s (acceptable for Block 14) | Would need optimization |

For the current use cases (T ≤ 500 from Block 14), the generalized binomial approach is sufficient. If T=5000+ is ever needed, a Newton's method approach for series square root (`f^{1/2}` via iterative refinement) would be faster but more complex to implement.

---

## Confidence Assessment

| Area | Confidence | Rationale |
|------|------------|-----------|
| powFrac algorithm | HIGH | Generalized binomial over Q is textbook; exact arithmetic guarantees correctness |
| jac2series_impl fix | HIGH | Minimal change — dispatch on exp.den, call powFrac |
| jacprodmake compatibility | HIGH | Decomposition already produces Frac; fix is downstream only |
| q-shift reconciliation | HIGH | Integer-difference check is straightforward; well-defined semantics |
| jac2prod display | HIGH | fracExpStr already exists for Frac display |
| Performance at T=500 | MEDIUM | Binomial convergence depends on minimum exponent growth; should be fine for Jacobi factors but untested |

---

## Test Validation Plan

### Unit Tests (per phase)

1. **powFrac:** `(1-q)^{1/2}` squared equals `1-q` (mod trunc). `(1-q)^{-1/2}` times `(1-q)^{1/2}` equals 1.
2. **jac2series fractional:** `jac2series([{0,5,Frac(1,2)}], 50)` squared equals `jac2series([{0,5,Frac(1)}], 50)`.
3. **jacprodmake Slater:** Block 13 input produces non-empty result with mod-14 period and half-integer exponents.
4. **q-shift add:** Series with q_shift=-1 + Series with q_shift=0 produces correct coefficients.
5. **Block 25 end-to-end:** `theta2(q,100)^2/theta2(q^3,40)^2 + theta3(q,100)^2/theta3(q^3,40)^2` succeeds and `findpoly` produces the expected relation.

### Regression

Run existing `tests/acceptance.sh` and `tests/maple-checklist.sh` to confirm no regressions in the 30 currently-passing blocks.

---

## Sources

- Direct source code audit: `src/series.h`, `src/convert.h`, `src/qfuncs.h`, `src/repl.h` (HIGH confidence)
- Maple expected outputs: `qseriesdoc.md` lines 524–536 (HIGH confidence)
- Maple checklist: `maple_checklist.md` failure analysis for Blocks 13, 14, 25 (HIGH confidence)
- Phase 62 verification: `.planning/phases/62-maple-checklist/62-VERIFICATION.md` (HIGH confidence)
- Generalized binomial series: standard mathematical reference (HIGH confidence)
