# Phase 65: Jacobi Half-Integer Exponents - Research

**Researched:** 2026-03-02
**Domain:** q-series Jacobi product identification and reconstruction with fractional exponents
**Confidence:** HIGH

## Summary

Phase 65 fixes three functions in `src/convert.h` that silently ignore fractional (half-integer) exponents in Jacobi product factors. The JacFactor type already stores exponents as `Frac`, and `prodmake` already produces fractional values for series like Slater's identity (46). The failure chain is: `jacprodmake` decomposes correctly into `Frac` exponents → calls `jac2series_impl` for verification → `jac2series_impl` truncates exponents to `int`, defaulting to 0 for fractions → reconstruction fails → `jacprodmake` returns empty.

Phase 64 (completed) already provides `Series::powFrac(Frac)` via generalized binomial coefficient recurrence, `Frac::rational_pow(Frac)`, and `BigInt::iroot`. The infrastructure is fully in place. This phase only needs to wire `powFrac` into the Jacobi reconstruction path and fix the display function.

The scope is small and well-defined: modify `jac2series_impl` to dispatch to `powFrac` for non-integer exponents, fix `jac2prod` to display fractional exponents, and verify that `jacprodmake` produces correct results for Blocks 13-14 without any changes to its decomposition logic.

**Primary recommendation:** Fix `jac2series_impl` and `jac2prod` in `convert.h` to handle `Frac` exponents via `powFrac`; `jacprodmake` decomposition needs no change.

## Standard Stack

Not applicable — this is a zero-dependency C++20 project. All code is header-only in `src/`. No external libraries or tools beyond `g++ -std=c++20 -O2`.

### Existing Infrastructure (from Phase 64)

| Component | Location | Purpose | Status |
|-----------|----------|---------|--------|
| `Series::powFrac(Frac)` | `src/series.h:282-345` | Generalized binomial series for fractional powers | Complete, tested |
| `Frac::rational_pow(Frac)` | `src/frac.h:110-154` | Exact rational exponentiation (handles c0^alpha) | Complete, tested |
| `BigInt::iroot` | `src/bigint.h` | Integer k-th root via binary search | Complete, tested |
| `Series::fracExpStr(Frac)` | `src/series.h:382-389` | Display helper for fractional exponents `^(n/d)` | Complete |

## Architecture Patterns

### Current Code Flow (Broken)

```
jacprodmake(f, T)
  → prodmake(f, T) → a[n] as Frac (some are 1/2, 13/2, etc.)
  → negate: e[n] = -a[n]
  → find period b, decompose e[1..b] into JAC(a,b) exponents x[a] as Frac
  → VERIFY: jac2series_impl(result, T)
       → for each JacFactor: extract int ex from Frac exp
       → exp.den != 1 → ex stays 0 → factor SKIPPED
       → reconstruction wrong → verification fails
  → return {} (empty)
```

### Fixed Code Flow

```
jacprodmake(f, T)
  → prodmake(f, T) → a[n] as Frac (half-integers OK)
  → decompose → x[a] as Frac (half-integers OK)
  → VERIFY: jac2series_impl(result, T)
       → for each JacFactor: check exp.den
       → exp.den == 1 → use existing pow(int) [fast binary exponentiation]
       → exp.den != 1 → use powFrac(exp)  [generalized binomial recurrence]
       → reconstruction correct → verification passes
  → return factors
```

### Pattern: Integer vs Fractional Exponent Dispatch

The fix follows a consistent pattern in both `jac2series_impl` and `jac2prod`:

```cpp
// BEFORE: integer-only extraction, silently ignores fractions
int ex = 0;
if (exp.den == BigInt(1) && exp.num.d.size() == 1 && exp.num.d[0] <= 1000)
    ex = exp.num.neg ? -static_cast<int>(exp.num.d[0]) : static_cast<int>(exp.num.d[0]);
// ex == 0 for fractions → factor skipped

// AFTER: dispatch on integer vs fractional
if (exp.den == BigInt(1)) {
    // Integer path — extract int, use pow(int) or integer display
} else {
    // Fractional path — use powFrac(exp) or fractional display
}
```

### Anti-Patterns to Avoid

- **Don't add a separate "half-integer mode" to jacprodmake.** The decomposition already works with `Frac` values. The fix is entirely in the downstream functions (`jac2series_impl` and `jac2prod`).
- **Don't suppress the prodmake non-integer warning globally.** The warning `"prodmake: non-integer a[n]"` fires for legitimate reasons in other contexts. It's informational when called from `jacprodmake`.
- **Don't use floats or approximations.** All arithmetic must remain exact rational.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Fractional series power | Custom half-integer logic | `Series::powFrac(Frac)` | Already implemented and tested in Phase 64 |
| Rational number exponentiation | Manual root extraction | `Frac::rational_pow(Frac)` | Handles all edge cases (sign, even roots, exact roots) |
| Fractional exponent display | New formatting code | `Series::fracExpStr(Frac)` | Already produces `^(n/d)` notation for non-integer Fracs |

## Common Pitfalls

### Pitfall 1: jac2series_impl silently skipping fractional factors
**What goes wrong:** When `exp.den != 1`, the extracted `int ex` stays 0 and the entire factor is multiplied as identity (skipped).
**Why it happens:** The original code was written before fractional exponents were needed; it only handled integers.
**How to avoid:** Check `exp.den == BigInt(1)` as the branching condition, not as a filter.
**Warning signs:** `jacprodmake` returns empty `{}` for series known to have Jacobi structure; `jac2series` returns `1 + O(q^T)` for a product with only fractional exponents.

### Pitfall 2: jac2prod skipping fractional exponents in display
**What goes wrong:** Factors with fractional exponents don't appear in the display string.
**Why it happens:** Same integer-extraction pattern as Pitfall 1.
**How to avoid:** Use `Frac` comparison for sign (positive/negative) instead of `int ex`.
**Warning signs:** `jac2prod(jp)` shows fewer factors than `jp` contains; display shows `1` for a non-trivial product.

### Pitfall 3: prodmake stderr warning noise
**What goes wrong:** When `jacprodmake` calls `prodmake` on a Slater-type series, prodmake emits `"prodmake: non-integer a[n]"` to stderr.
**Why it happens:** prodmake line 124 warns on first non-integer exponent.
**How to avoid:** This is cosmetic. Either leave it (it's one warning per call) or add a `bool quiet` parameter to `prodmake`. The warning is correct — the a[n] values genuinely aren't integers.
**Warning signs:** Extra stderr output in test runs; can cause false failures in grep-based tests that check stderr.

### Pitfall 4: Exponent sign handling in jac2prod display
**What goes wrong:** Fractional exponents need to be split into numerator/denominator parts correctly for display.
**Why it happens:** The existing code uses `int ex` for sign: `ex > 0 → numerator, ex < 0 → denominator`. With `Frac`, must use `exp > Frac(0)` or `exp < Frac(0)`.
**How to avoid:** Use `Frac` comparison operators which are already defined.
**Warning signs:** Factors appear in wrong position (numerator vs denominator).

### Pitfall 5: powFrac with leading q-power
**What goes wrong:** `jac_factor_series(a, b, T)` starts with constant term 1, so `powFrac` works directly. But if someone constructs a JacFactor that produces a series with non-unit constant term, `powFrac` would need `rational_pow` for the constant.
**Why it happens:** `powFrac` extracts leading coefficient and calls `c0.rational_pow(alpha)`.
**How to avoid:** This is already handled by Phase 64's implementation. Not a concern for Jacobi factors specifically.
**Warning signs:** None expected — defensive check already in place.

## Code Examples

### Current jac2series_impl (lines 343-365 of convert.h)

```cpp
inline Series jac2series_impl(const std::vector<JacFactor>& jac, int T) {
    Series q = Series::q(T);
    Series prod = Series::one(T);
    for (const auto& [a, b, exp] : jac) {
        Series fac;
        if (a == 0) {
            fac = jac_factor_series(b, b, T);
        } else {
            fac = (jac_factor_series(a, b, T) * jac_factor_series(b - a, b, T)
                   * jac_factor_series(b, b, T)).truncTo(T);
        }
        // BUG: truncates to int, silently skips fractional exponents
        int ex = 0;
        if (exp.den == BigInt(1) && exp.num.d.size() == 1 && exp.num.d[0] <= 1000)
            ex = exp.num.neg ? -static_cast<int>(exp.num.d[0]) : static_cast<int>(exp.num.d[0]);
        if (ex > 0) {
            for (int i = 0; i < ex; ++i) prod = (prod * fac).truncTo(T);
        } else if (ex < 0) {
            auto inv = fac.inverse();
            for (int i = 0; i < -ex; ++i) prod = (prod * inv).truncTo(T);
        }
        // when ex == 0 (fractional exp), nothing happens — factor skipped
    }
    prod.trunc = T;
    return prod;
}
```

### Fixed jac2series_impl

```cpp
inline Series jac2series_impl(const std::vector<JacFactor>& jac, int T) {
    Series q = Series::q(T);
    Series prod = Series::one(T);
    for (const auto& [a, b, exp] : jac) {
        if (exp.isZero()) continue;
        Series fac;
        if (a == 0) {
            fac = jac_factor_series(b, b, T);
        } else {
            fac = (jac_factor_series(a, b, T) * jac_factor_series(b - a, b, T)
                   * jac_factor_series(b, b, T)).truncTo(T);
        }
        if (exp.den == BigInt(1)) {
            // Integer exponent — use binary exponentiation
            int64_t p64 = 0;
            if (!exp.num.d.empty()) p64 = static_cast<int64_t>(exp.num.d[0]);
            if (exp.num.neg) p64 = -p64;
            prod = (prod * fac.pow(static_cast<int>(p64))).truncTo(T);
        } else {
            // Fractional exponent — use powFrac
            prod = (prod * fac.powFrac(exp)).truncTo(T);
        }
    }
    prod.trunc = T;
    return prod;
}
```

### Current jac2prod display (lines 434-460 of convert.h)

```cpp
inline std::string jac2prod(const std::vector<JacFactor>& jac) {
    std::vector<std::string> num_parts, den_parts;
    for (const auto& [a, b, exp] : jac) {
        // BUG: truncates to int, skips fractional exponents
        int ex = 0;
        if (exp.den == BigInt(1) && exp.num.d.size() == 1 && exp.num.d[0] <= 100)
            ex = exp.num.neg ? -static_cast<int>(exp.num.d[0]) : static_cast<int>(exp.num.d[0]);
        if (ex == 0) continue;  // ← skips fractions!
        // ... display logic uses int ex ...
        int absex = (ex > 0) ? ex : -ex;
        if (absex > 1) part += "^" + std::to_string(absex);
        // ...
    }
}
```

### Fixed jac2prod display

```cpp
inline std::string jac2prod(const std::vector<JacFactor>& jac) {
    std::vector<std::string> num_parts, den_parts;
    for (const auto& [a, b, exp] : jac) {
        if (exp.isZero()) continue;

        std::string part;
        // ... build (q^a, q^b)_∞ string as before ...

        // Exponent display: handle both integer and fractional
        Frac absexp = exp.abs();
        if (!(absexp == Frac(1))) {
            if (absexp.den == BigInt(1)) {
                // Integer exponent > 1: use superscript
                int ae = absexp.num.d.empty() ? 0 : static_cast<int>(absexp.num.d[0]);
                if (ae > 1) part += "^" + std::to_string(ae);
            } else {
                // Fractional exponent: use ^(n/d) notation
                part += "^(" + absexp.str() + ")";
            }
        }

        if (exp > Frac(0))
            num_parts.push_back(part);
        else
            den_parts.push_back(part);
    }
    // ... rest unchanged ...
}
```

### Expected Slater (46) Output

From qseriesdoc.md Output (15):
```
JAC(0,14,∞)^(13/2) / (JAC(1,14,∞)² JAC(3,14,∞) JAC(4,14,∞)
      JAC(5,14,∞) JAC(6,14,∞) JAC(7,14,∞)^(1/2))
```

In our `jac2prod` format (q-product form):
```
(q^14,q^14)_∞^(13/2) / ((q,q^14)_∞ (q^13,q^14)_∞ (q^14,q^14)_∞)² 
(q^3,q^14)_∞ (q^11,q^14)_∞ (q^14,q^14)_∞ ... ^(1/2))
```

## Detailed Analysis of Each Function

### 1. jacprodmake Decomposition (lines 367-428)

The decomposition logic:
1. Calls `prodmake(f, T)` → gets `a_map` (map from int → Frac)
2. Computes `e[n] = -a[n]` (all Frac values)
3. Searches for period `b` with 80% match threshold
4. For each candidate `b`:
   - Checks symmetry: `e[a] == e[b-a]` for `a = 1..floor((b-1)/2)`
   - Sets `x[a] = e[a]` (Frac)
   - Computes `x[0] = e[b] - sum_x` (Frac)
   - Creates JacFactor entries with Frac exponents
   - **Verifies**: calls `jac2series_impl(result, T)`, compares to input

The decomposition itself is correct for fractional values — all variables are `Frac`. The **only** failure is at the verification step because `jac2series_impl` can't reconstruct.

**Prediction:** Once `jac2series_impl` is fixed, `jacprodmake` will work for Slater (46) without any changes to the decomposition logic.

### 2. jac2series_impl Reconstruction (lines 343-365)

Current behavior with fractional exponents:
- `exp = Frac(13, 2)`: `exp.den == BigInt(2)` → condition fails → `ex = 0` → factor skipped
- `exp = Frac(-1, 2)`: same → skipped
- `exp = Frac(-2, 1)`: `exp.den == BigInt(1)` → `ex = -2` → works correctly

Fix: Branch on `exp.den == BigInt(1)` to choose integer pow vs powFrac.

**Key insight:** `jac_factor_series(a, b, T)` always produces `(q^a; q^b)_∞` which has constant term 1. Therefore `powFrac` will always succeed (no irrational constant term issue).

### 3. jac2prod Display (lines 434-460)

Current behavior:
- Fractional exponents → `ex = 0` → `continue` → factor not displayed
- Integer exponents work fine

Fix: Use `Frac` comparisons for sign determination and `exp.str()` or `^(n/d)` for display.

### 4. REPL Display Path (repl.h line 1429)

When `jacprodmake` returns, the result type is `std::vector<JacFactor>`. The display path is:
```cpp
} else if constexpr (std::is_same_v<T, std::vector<JacFactor>>) {
    std::cout << jac2prod(arg) << std::endl;
}
```

This automatically uses the fixed `jac2prod`. No change needed in `repl.h`.

## Block 13-14 Test Analysis

### Block 13 (maple-checklist.sh lines 112-119)

```bash
OUT13=$(run "x := sum(q^(n*(n+1)/2)*aqprod(-q,q,n,100)/aqprod(q,q,2*n+1,100), n, 0, 10)" \
            "jacprodmake(x, 50)")
if echo "$OUT13" | grep -qE 'JAC'; then
    pass "Block 13: jacprodmake Slater"
fi
```

The test checks that `jacprodmake` output contains "JAC". Currently returns empty → no JAC in output → FAIL.

**Note:** The test uses `aqprod(-q,q,n,100)` which computes `(-q;q)_n`. The truncation T=100 for the series and T=50 for jacprodmake are important — enough terms for the 80% periodicity check with period b=14.

### Block 14 (maple-checklist.sh lines 121-129)

```bash
OUT14=$(run "x := sum(..., n, 0, 10)" "jp := jacprodmake(x, 50)" "jac2series(jp, 100)")
LAST14=$(echo "$OUT14" | tail -1)
if echo "$LAST14" | grep -qE '2q|3q|5q'; then
    pass "Block 14: jac2series Slater"
fi
```

This tests that `jac2series` on the jacprodmake result produces a non-trivial series. Since Block 13 fails (jp is empty), jac2series on empty gives `1 + O(q^T)` → no `2q`/`3q`/`5q` → FAIL.

## Open Questions

1. **prodmake stderr warning**
   - What we know: prodmake emits `"prodmake: non-integer a[n]"` once per call when any a[n] is fractional
   - What's unclear: Whether this warning causes test failures (grep-based tests might pick it up on stderr)
   - Recommendation: Leave the warning. Tests grep stdout, not stderr. The `2>&1` redirect in the test runner merges both, but the grep patterns look for specific content like 'JAC', not error messages.

2. **Display format consistency**
   - What we know: qseriesdoc uses `√JAC(7,14,∞)` for exponent 1/2 and `^(13|2)` for 13/2
   - What's unclear: Whether our `^(1/2)` and `^(13/2)` notation is acceptable
   - Recommendation: Use `^(1/2)` and `^(13/2)` consistently. The `√` notation is Maple-specific and would require special-casing. The `^(n/d)` format is clear and general.

3. **Performance of powFrac in reconstruction**
   - What we know: For T=50 (Block 13), powFrac on a ~50-term series is instant. For T=100 (Block 14's jac2series), it's still fast. For T=500 (qseriesdoc's example), it might take a few seconds.
   - What's unclear: Exact timing for T=500 with multiple JAC factors
   - Recommendation: Implement and measure. The O(T²) recurrence should be fine for T≤500.

## Sources

### Primary (HIGH confidence)
- Direct source code audit: `src/convert.h` (JacFactor, jacprodmake, jac2series_impl, jac2prod)
- Direct source code audit: `src/series.h` (powFrac, fracExpStr)
- Direct source code audit: `src/frac.h` (rational_pow)
- Direct source code audit: `src/repl.h` (display dispatch, REPL jac2prod/jac2series handlers)
- Direct source code audit: `tests/maple-checklist.sh` (Blocks 13-14 test logic)

### Secondary (HIGH confidence)
- `qseriesdoc.md` Output (15): Expected Slater (46) Jacobi product form
- `.planning/research/ARCHITECTURE.md`: Prior analysis of failure chain and fix strategy
- `.planning/phases/64-fractional-power-infrastructure/64-01-SUMMARY.md`: Phase 64 completion status

## Metadata

**Confidence breakdown:**
- Root cause analysis: HIGH — confirmed by direct code reading; the int-truncation pattern is unambiguous
- Fix strategy: HIGH — straightforward dispatch to existing powFrac infrastructure
- jacprodmake compatibility: HIGH — decomposition already produces Frac exponents; only downstream functions need fixing
- Display formatting: HIGH — fracExpStr already exists; just needs wiring
- Test expectations: HIGH — Block 13-14 tests are simple grep checks; expected outputs are documented in qseriesdoc.md

**Research date:** 2026-03-02
**Valid until:** Stable — this is project-internal C++ code, not external dependencies
