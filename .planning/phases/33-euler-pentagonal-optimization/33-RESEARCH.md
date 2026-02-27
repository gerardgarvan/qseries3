# Phase 33: Euler Pentagonal Optimization for etaq(1,T) - Research

**Researched:** 2026-02-26
**Domain:** Number theory — Euler's pentagonal number theorem applied to q-series computation
**Confidence:** HIGH

## Summary

Euler's pentagonal number theorem provides a closed-form expansion for the infinite product `∏_{n=1}^∞ (1 - q^n)`, which is exactly what `etaq(1, T)` computes. Instead of performing O(T) Series multiplications (each costing O(T) due to the growing number of terms), we can directly write down the coefficients: they are all 0, ±1, and nonzero only at the generalized pentagonal numbers `k(3k-1)/2` for `k = 0, ±1, ±2, ...`. Since there are only O(√T) such pentagonal numbers below T, the entire computation reduces to O(√T) map insertions — a dramatic improvement from the current O(T²) algorithm.

This is one of the most classical results in number theory (Euler, 1750) and is mathematically exact — no approximation, no heuristic. The optimization is a simple fast-path branch inside the existing `etaq` function, triggered when `k == 1` and the q argument is the standard formal variable. All other cases (`k != 1` or non-standard q) continue to use the current product-loop algorithm.

**Primary recommendation:** Add a fast-path branch at the top of `etaq()` that, when `k == 1 && standardQ`, directly generates the sparse pentagonal series instead of multiplying T binomials.

## Standard Stack

Not applicable — this is a pure algorithmic optimization within the existing codebase. No new libraries, no new dependencies. The implementation uses only the existing `Series`, `Frac`, and `std::map` primitives already in the project.

## Architecture Patterns

### Pattern: Fast-Path Branch Inside Existing Function

**What:** Add an `if (k == 1)` branch inside `etaq()` that replaces the product loop with a direct coefficient computation, then falls through to the same caching logic.

**When to use:** When a special case of an algorithm has a dramatically more efficient formulation.

**Structure of the modified `etaq` function:**

```cpp
inline Series etaq(const Series& q, int k, int T) {
    bool standardQ = /* existing check */;

    if (standardQ) {
        auto it = etaq_cache().find({k, T});
        if (it != etaq_cache().end()) return it->second;
    }

    Series result;

    if (k == 1 && standardQ) {
        // FAST PATH: Euler pentagonal theorem
        result = Series::zero(T);
        result.trunc = T;
        for (int j = 0; ; ++j) {
            int p1 = j * (3 * j - 1) / 2;
            int p2 = j * (3 * j + 1) / 2;
            if (p1 >= T && p2 >= T) break;
            Frac sign(j % 2 == 0 ? 1 : -1);
            if (p1 < T) result.setCoeff(p1, sign);
            if (p2 < T && p2 != p1) result.setCoeff(p2, sign);
        }
    } else {
        // EXISTING PATH: product loop
        result = Series::one(T);
        for (int n = 1; k * n < T; ++n) {
            Series qkn = q.truncTo(T).pow(k * n);
            Series factor = Series::one(T) - qkn;
            result = (result * factor).truncTo(T);
        }
        result.trunc = T;
    }

    if (standardQ) {
        etaq_cache()[{k, T}] = result;
    }
    return result;
}
```

### Anti-Patterns to Avoid

- **Separate function:** Don't create a separate `etaq_pentagonal()` function. The optimization must be transparent — callers (etamake, REPL, etc.) should not need to know or change anything.
- **Modifying Series representation:** Don't try to add a "sparse pentagonal" Series type. The existing `std::map<int, Frac>` is already sparse and handles this naturally.
- **Over-generalizing:** Don't try to extend the pentagonal optimization to `k != 1`. The pentagonal theorem specifically applies to `∏(1-q^n)` = `etaq(1,T)`. For `etaq(k,T)` with `k > 1`, the product is `∏(1-q^{kn})` which equals `etaq(1, T).subs_q(k)` in principle, but substituting q→q^k changes truncation semantics. The naive product loop is fine for `k > 1` since only ~T/k factors are needed.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Pentagonal number generation | Lookup table or precomputed array | Inline formula `j*(3*j±1)/2` in a simple loop | The formula is trivial, O(1) per number, and the loop terminates naturally when both pentagonal numbers exceed T |

## The Mathematics

### Euler's Pentagonal Number Theorem

**Statement:**

```
∏_{n=1}^∞ (1 - q^n) = Σ_{k=-∞}^{∞} (-1)^k q^{k(3k-1)/2}
```

**The generalized pentagonal numbers** are `k(3k-1)/2` for integer k:

| k | k(3k-1)/2 | (-1)^k |
|---|-----------|--------|
| 0 | 0 | +1 |
| 1 | 1 | -1 |
| -1 | 2 | -1 |
| 2 | 5 | +1 |
| -2 | 7 | +1 |
| 3 | 12 | -1 |
| -3 | 15 | -1 |
| 4 | 22 | +1 |
| -4 | 26 | +1 |
| 5 | 35 | -1 |
| -5 | 40 | -1 |

**Regrouping by |k| = j ≥ 0:**
- For k = +j: pentagonal number = `j(3j-1)/2`
- For k = -j: pentagonal number = `j(3j+1)/2`
- Sign for both = `(-1)^j`

So the expansion is:
```
1 - q - q² + q⁵ + q⁷ - q¹² - q¹⁵ + q²² + q²⁶ - q³⁵ - q⁴⁰ + ...
```

**Key properties of the coefficients:**
- Every coefficient is exactly 0, +1, or -1
- Signs come in pairs: `(-1)^j` for both `j(3j-1)/2` and `j(3j+1)/2`
- Only O(√T) coefficients are nonzero below degree T (since pentagonal numbers grow as ~3j²/2)

### Complexity Analysis

**Current algorithm (naive product):**
- Performs T-1 multiplications: `result *= (1 - q^n)` for n = 1, ..., T-1
- Each multiplication: result has up to T nonzero terms, multiplied by a binomial → O(T) operations
- Total: O(T²) Frac arithmetic operations
- For T = 500: ~250,000 Frac multiplications and additions

**Pentagonal algorithm:**
- Enumerate pentagonal numbers below T: need j from 0 to ~√(2T/3)
- For each j: two O(1) integer computations, two O(1) map insertions (Frac(±1) is trivial)
- Total: O(√T) operations (with O(log T) per map insertion, so O(√T · log T) strictly)
- For T = 500: ~18 iterations (j up to 18, since 18×55/2 = 495), ~36 map insertions
- Speedup: ~7000x fewer operations compared to naive

**Note on "O(T√T)" mentioned in phase description:** This complexity applies to computing the *partition function* p(n) via the pentagonal recurrence `p(n) = Σ (-1)^{k+1} p(n - k(3k-1)/2)`. For computing `etaq(1,T)` itself (the product, not its reciprocal), the pentagonal theorem gives the answer directly in O(√T).

### Why This Is Exact

The pentagonal number theorem is an identity — it holds as a formal power series equality. Truncating both sides to O(q^T) gives exact agreement through degree T-1. Since all coefficients are integers (in fact ±1 or 0), the Frac representation involves no reduction — `Frac(1)` and `Frac(-1)` have trivial GCD.

## Common Pitfalls

### Pitfall 1: Duplicate Coefficient at j=0

**What goes wrong:** When j=0, both pentagonal formulas give the same exponent: `0(3·0-1)/2 = 0` and `0(3·0+1)/2 = 0`. If you set the coefficient twice, the second `setCoeff` overwrites the first (same value, no harm), but if using `+=` instead of `=`, you'd double-count.

**How to avoid:** Use `setCoeff` (which sets, not adds). Or add a `p2 != p1` guard on the second insertion. Both approaches are correct.

### Pitfall 2: Sign Pattern

**What goes wrong:** Getting the sign wrong. The sign is `(-1)^k` where k ranges over all integers. When regrouping by `j = |k|`, both `k = +j` and `k = -j` have sign `(-1)^j` (since `(-1)^{-j} = (-1)^j` for integers).

**How to avoid:** Use `j % 2 == 0 ? 1 : -1` for both pentagonal numbers generated by the same j. Verify against the known sequence: coeff(0)=1, coeff(1)=-1, coeff(2)=-1, coeff(5)=1, coeff(7)=1.

### Pitfall 3: Integer Overflow in Pentagonal Computation

**What goes wrong:** `j * (3 * j - 1) / 2` could overflow `int` for very large j. With 32-bit int, overflow occurs at j ≈ 26,754, corresponding to pentagonal number ~1.07 billion. Since T is realistically ≤ 10,000 in this REPL (j ≤ ~82), this is not a practical concern.

**How to avoid:** Use `int` — safe for any realistic T. For paranoia, cast to `int64_t` before multiplication.

### Pitfall 4: Loop Termination

**What goes wrong:** The loop `for (int j = 0; ; ++j)` must terminate when BOTH pentagonal numbers exceed T. Since `p1 = j(3j-1)/2` < `p2 = j(3j+1)/2` for j > 0, checking `p1 >= T` alone is sufficient — but only for the first pentagonal number. If p1 < T but p2 >= T, we must still insert p1.

**How to avoid:** Check `if (p1 >= T && p2 >= T) break;` then guard each insertion with `if (p1 < T)` and `if (p2 < T)`.

### Pitfall 5: Forgetting the k != 1 Case

**What goes wrong:** The pentagonal theorem only gives `∏(1-q^n)` = etaq(1,T). For etaq(k,T) with k > 1, the product is `∏(1-q^{kn})`. One might be tempted to compute etaq(1, T) and then substitute q → q^k, but this changes the effective truncation (you'd need T/k terms of etaq(1, T/k)).

**How to avoid:** Only apply the pentagonal fast path when `k == 1`. Leave the existing product loop for `k > 1`. The k > 1 case is already fast enough since only ~T/k factors are multiplied.

## Code Examples

### Core Implementation

```cpp
if (k == 1 && standardQ) {
    result = Series::zero(T);
    result.trunc = T;
    for (int j = 0; ; ++j) {
        int p1 = j * (3 * j - 1) / 2;
        int p2 = j * (3 * j + 1) / 2;
        if (p1 >= T && p2 >= T) break;
        Frac sign(j % 2 == 0 ? 1 : -1);
        if (p1 < T) result.setCoeff(p1, sign);
        if (p2 < T && p2 != p1) result.setCoeff(p2, sign);
    }
}
```

### Verification: First 15 Coefficients

The result of `etaq(1, 15)` should be:
```
q^0:  1
q^1: -1
q^2: -1
q^3:  0
q^4:  0
q^5:  1
q^6:  0
q^7:  1
q^8:  0
q^9:  0
q^10: 0
q^11: 0
q^12:-1
q^13: 0
q^14: 0
```

### Loop Bound Analysis

For truncation T, the maximum j needed satisfies `j(3j-1)/2 < T`, i.e., `j < (1 + √(1+24T)) / 6 ≈ √(2T/3)`:

| T | max j | pentagonal numbers generated |
|---|-------|------------------------------|
| 50 | 6 | 12 entries |
| 100 | 8 | 16 entries |
| 200 | 12 | 24 entries |
| 500 | 18 | 36 entries |
| 1000 | 26 | 52 entries |

## Integration Points

### Where etaq(1,T) Is Called

1. **REPL direct:** `etaq(1, 50)` — user explicitly computing the Euler product
2. **etamake:** calls `etaq(q_var, k, T)` with various k including k=1 in the inner loop
3. **Acceptance Test 2:** `1/etaq(1,50)` — partition generating function
4. **Acceptance Test 6:** `etaq(2,200)/etaq(1,200)` — Rødseth computation
5. **Acceptance Test 8:** `etaq(7,100)/etaq(1,100)` and `etaq(49,100)/etaq(1,100)` — Watson
6. **Acceptance Test 9:** `etaq(1,500)` — the big one, Euler pentagonal dissection

### Interaction with Phase 32 Memoization

The pentagonal fast path produces a result that is then cached by the existing memoization logic (Phase 32). This is correct and desired:
- First call: fast pentagonal computation → O(√T), result stored in cache
- Subsequent calls: O(1) cache hit, same as before

The optimization makes the *first* call dramatically faster. Cache hits remain O(1) as before. No changes to cache logic are needed.

### What Must NOT Change

- Function signature: `inline Series etaq(const Series& q, int k, int T)` — unchanged
- Return value: identical Series object for all inputs — byte-for-byte same coefficients
- Cache behavior: still keyed on `{k, T}`, still cleared by `clear_etaq_cache()`
- Non-standard q: still bypasses both the pentagonal path and the cache

## Testing Strategy

1. **Coefficient verification:** Compute `etaq(1, T)` with the pentagonal path, compare every coefficient against the naive product path. They must be identical.
2. **Acceptance tests:** All 9 SPEC tests must pass, especially Test 9 (`etaq(1, 500)`).
3. **Edge cases:** `etaq(1, 1)` = 1 + O(q), `etaq(1, 2)` = 1 - q + O(q²), `etaq(1, 3)` = 1 - q - q² + O(q³).
4. **k != 1 unchanged:** Verify `etaq(2, 100)`, `etaq(7, 50)` produce same results as before (they use the old path).
5. **Performance:** `etaq(1, 500)` should be near-instantaneous (microseconds vs. potentially seconds for naive).

## Open Questions

None. This is a textbook optimization with no ambiguity. The pentagonal number theorem is mathematically exact, the implementation is straightforward, and the integration is a simple branch inside the existing function.

## Sources

### Primary (HIGH confidence)

- Euler's pentagonal number theorem — foundational result in number theory (Euler, 1750). Present in every number theory textbook (Hardy & Wright, Andrews' "The Theory of Partitions", etc.)
- SPEC.md stretch goals section — mentions this optimization explicitly
- Current `src/qfuncs.h` — existing etaq implementation with Phase 32 memoization cache
- Current `src/series.h` — Series data structure (std::map<int, Frac>, setCoeff, trunc semantics)

## Metadata

**Confidence breakdown:**
- Algorithm: HIGH — Euler's pentagonal theorem is a 275-year-old proven identity
- Implementation: HIGH — the code pattern is trivial (a single loop with two map insertions per iteration)
- Integration: HIGH — simple branch inside existing function, no API changes, transparent to all callers
- Testing: HIGH — exact integer arithmetic, deterministic, easy to verify against known sequences

**Research date:** 2026-02-26
**Valid until:** Indefinite — the mathematics does not change
