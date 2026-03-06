# Phase 63: Q-Shift Arithmetic Fix - Research

**Researched:** 2026-03-01
**Domain:** Truncated formal power series with rational q-power offsets (q_shift normalization)
**Confidence:** HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

#### Normalization Strategy
- **Eager normalization** — After every operation that changes q_shift (multiply, divide, pow, inverse), immediately absorb integer part into coefficient map
- q_shift always kept in **[0, 1)** range — matches Maple convention
- With eager normalization, both operands of operator+ will already have matching q_shifts if their fractional parts are the same — no special alignment logic needed in operator+

#### Error Handling for Non-Integer Differences
- **Throw for non-integer differences** — e.g., q_shift 1/4 vs 1/3 is a genuine error
- Common denominator approach rejected
- All known Maple checklist failures involve integer differences only

#### Display Format
- **Expanded individual terms** — `2q^(1/4) + 2q^(5/4) + 2q^(9/4) + ...` instead of `q^(1/4) * (2 + 2q + 2q³ + ...)`
- **Parenthesized notation** for fractional exponents — `q^(1/4)` not Unicode fractions
- **Integer-exponent series unchanged** — When q_shift is 0, display stays as `1 + 2q + 3q² + ...`
- **O() term uses integer truncation** — `O(q^50)` not `O(q^(50+1/4))`

### Claude's Discretion
None specified — all decisions locked.

### Deferred Ideas (OUT OF SCOPE)
None — discussion stayed within phase scope.
</user_constraints>

## Summary

This phase fixes a concrete bug: `operator+` throws when two series have q_shifts that differ by an integer, which breaks Block 25 of the Maple checklist (and any computation mixing theta2-derived series with integer-shift series). The fix is **eager normalization** — a `normalize_q_shift()` method that absorbs the integer part of `q_shift` into the coefficient index map after every operation that modifies q_shift.

The implementation touches three areas: (1) a new `normalize_q_shift()` method on Series, (2) calls to it from `operator*`, `inverse()`, `pow()`, and `subs_q()`, and (3) a rewritten `str()` method that displays expanded fractional-exponent terms instead of the current `q^(frac) * (...)` wrapper format.

The mathematical invariant is: after normalization, `q_shift ∈ [0, 1)` always. Since `operator+` only needs to check `q_shift == o.q_shift`, and both operands will already be normalized, addition "just works" for the integer-difference case. Non-integer-difference addition (a genuine mathematical error) still throws.

**Primary recommendation:** Implement `normalize_q_shift()` first with a standalone unit test, then wire it into all q_shift-producing operations, then update `str()` display, then verify Block 25.

## Standard Stack

Not applicable — zero-dependency C++20 project. All changes are within `src/series.h` and `src/repl.h`.

## Architecture Patterns

### Pattern 1: normalize_q_shift() — The Core Algorithm

**What:** After any operation produces a new q_shift, decompose it into integer + fractional parts. Absorb the integer part by shifting all coefficient indices.

**Mathematical basis:** A Series represents `q^{q_shift} · Σ c[n] q^n`. Coefficient at index `n` contributes exponent `n + q_shift`. To normalize with floor `F = ⌊q_shift⌋`:
- Each coefficient moves: `c'[n + F] = c[n]`
- `trunc' = trunc + F`
- `q_shift' = q_shift - F` (now in [0, 1))

The real-exponent truncation is preserved: `trunc' + q_shift' = (trunc + F) + (q_shift - F) = trunc + q_shift`.

**Implementation:**

```cpp
void normalize_q_shift() {
    // Fast path: q_shift is 0 (the overwhelmingly common case)
    if (q_shift.isZero()) return;

    // Compute F = floor(q_shift)
    // BigInt::divmod truncates toward zero; adjust for negative values
    auto [quot, rem] = BigInt::divmod(q_shift.num, q_shift.den);
    // If remainder is negative, floor is one less than truncated quotient
    if (!rem.isZero() && rem.neg)
        quot = quot - BigInt(1);

    // If F == 0, q_shift is already in [0, 1)
    if (quot.isZero()) return;

    // Extract F as int (safe: q_shift values are small in practice)
    int F = 0;
    if (!quot.isZero()) {
        F = quot.d.empty() ? 0 : static_cast<int>(quot.d[0]);
        if (quot.neg) F = -F;
    }

    // Shift all coefficient indices by F
    std::map<int, Frac> new_c;
    for (const auto& [e, v] : c)
        new_c[e + F] = v;
    c = std::move(new_c);

    // Adjust truncation and q_shift
    trunc += F;
    q_shift = q_shift - Frac(BigInt(quot), BigInt(1));

    // Safety: if trunc went non-positive, the series is effectively empty
    if (trunc <= 0) {
        c.clear();
        trunc = 0;
    }

    clean();
}
```

### Pattern 2: Call Sites for Eager Normalization

**What:** Every operation that produces a new `q_shift` value must call `normalize_q_shift()` on the result before returning.

**Call sites in series.h:**

| Method | How q_shift changes | Normalize? |
|--------|-------------------|------------|
| `operator*` (Series × Series) | `q_shift = a.q_shift + b.q_shift` | YES — sum of two [0,1) values can reach [0,2) |
| `operator*` (Series × Frac) | q_shift unchanged | NO |
| `inverse()` | `q_shift = -q_shift` | YES — negation of [0,1) gives (-1,0] |
| `pow(n)` | Accumulated via multiply | YES (each multiply normalizes, but final result should too) |
| `subs_q(k)` | `q_shift *= k` | YES — multiplication by k can produce any value |
| `operator+` | Checks match | NO — operands already normalized |
| `operator-()` (unary) | q_shift unchanged | NO |
| `truncTo()` / `withTrunc()` | q_shift copied | NO |

**Critical insight for `pow()`:** Since `pow()` uses binary exponentiation via `operator*`, and each `operator*` call normalizes, the final result will already be normalized. No extra call needed inside `pow()` itself.

**Critical insight for `inverse()`:** The current `inverse()` has two paths — one for `minExp() != 0` (shifts coefficients, recurses) and one for `minExp() == 0` (direct recurrence). Both set `q_shift = -q_shift`. After normalization, this becomes `1 - q_shift` when q_shift was in (0,1), or stays 0 when q_shift was 0.

### Pattern 3: Updated str() Display Format

**What:** When q_shift != 0, display each term with its full fractional exponent instead of wrapping in `q^(frac) * (...)`.

**Old format:** `q^(1/4) * (2 + 2q + 2q² + ...)`
**New format:** `2q^(1/4) + 2q^(5/4) + 2q^(9/4) + ...`

**Algorithm for each term at index `n`:**
1. Compute actual exponent: `exp = Frac(n) + q_shift`
2. If exp is zero: display just the coefficient (e.g., `3`)
3. If exp is an integer: use unicode superscript (e.g., `q²`)
4. If exp is a non-integer fraction: use parenthesized notation (e.g., `q^(5/4)`)

**O() term:** Always `O(q^trunc)` using the integer `trunc` field, regardless of q_shift.

### Pattern 4: Block 25 Trace (Verification of Correctness)

The critical test path:

```
theta2(q, 100)         → q_shift = 1/4, coeffs at 0,2,6,12,20,...
theta2(q, 100)^2       → q_shift = 1/4 + 1/4 = 1/2 (in [0,1), no normalize needed)
theta2(q^3, 40)        → q_shift = 3/4, coeffs at 0,6,18,...
theta2(q^3, 40)^2      → q_shift = 3/4 + 3/4 = 3/2 → NORMALIZE → q_shift = 1/2, shift +1
inverse(theta2(q^3,40)^2) → q_shift = -(1/2) → NORMALIZE → q_shift = 1/2, shift -1

x1 = theta2(q,100)^2 / theta2(q^3,40)^2
   = theta2(q,100)^2 * inverse(theta2(q^3,40)^2)
   → q_shift = 1/2 + 1/2 = 1 → NORMALIZE → q_shift = 0, shift +1

theta3(q, 100)^2       → q_shift = 0
theta3(q^3, 40)^2      → q_shift = 0
x2 = theta3(q,100)^2 / theta3(q^3,40)^2 → q_shift = 0

x = x1 + x2            → both have q_shift = 0 → SUCCEEDS
```

After `x1 + x2`, run `findpoly(x, x, 3, 3)` or similar as Block 25 expects. The addition no longer throws.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Floor of Frac | Custom integer floor from scratch | `BigInt::divmod` + sign adjustment | Divmod already exists and handles all edge cases |
| BigInt-to-int conversion | Ad-hoc bit fiddling | Extract from `d[0]` with sign, same pattern used in `str()`, `fracExpStr()`, and `evalExpr` | Consistent with 5+ existing call sites |
| Coefficient map rebuild | In-place key mutation | `std::map` swap with new map | map keys are immutable; must rebuild |

## Common Pitfalls

### Pitfall 1: Forgetting to normalize in inverse() non-zero-minExp path
**What goes wrong:** The `inverse()` method has two branches. The `minExp() != 0` branch shifts coefficients internally and recurses. It sets `result.q_shift = Frac(0) - q_shift`. If you only normalize in the `minExp() == 0` branch, the recursive path won't normalize.
**How to avoid:** Add `normalize_q_shift()` call at the very end of `inverse()`, after both branches converge, or at the end of each branch.

### Pitfall 2: Truncation adjustment causing empty series
**What goes wrong:** If `trunc + F <= 0`, the series becomes empty. E.g., a series with `trunc = 5` and `q_shift = -10` would produce `trunc = -5`.
**How to avoid:** After normalization, if `trunc <= 0`, clear all coefficients and set `trunc = 0`. Then call `clean()`.

### Pitfall 3: Display regression for q_shift = 0 case
**What goes wrong:** Changing `str()` to display expanded fractional exponents breaks the display for the vastly more common case where `q_shift = 0`.
**How to avoid:** The new `str()` must check `q_shift.isZero()` first and use the exact existing display logic for that case. Only use the new fractional-exponent display when `q_shift != 0`.

### Pitfall 4: Coefficient sign display with fractional exponents
**What goes wrong:** The current `str()` logic handles sign display carefully — the first term shows a leading minus, subsequent terms show ` - ` or ` + `. When rewriting for fractional exponents, this sign logic must be preserved exactly.
**How to avoid:** Factor the sign/coefficient display into a shared helper, or carefully duplicate the logic in the fractional-exponent branch.

### Pitfall 5: clean() removing valid negative-index coefficients
**What goes wrong:** After normalization with negative F, some coefficients may have negative indices. The current `clean()` only removes `e >= trunc`, which is fine. But if any code elsewhere assumes non-negative indices, it would break.
**How to avoid:** Audit callers of `coeff()`, `coeffList()`, `minExp()`. These already handle negative indices via `map<int, Frac>`. The display code iterates `c` in order, which handles negative keys automatically.

### Pitfall 6: subs_q with negative k
**What goes wrong:** `subs_q(k)` multiplies q_shift by k. If k is negative, q_shift could become negative. The normalization handles this correctly (floor of a negative fraction), but the existing `subs_q` also multiplies exponents by k, which reverses coefficient ordering when k < 0.
**How to avoid:** Verify that `subs_q` with negative k still produces correct results after normalization. The existing code handles `absK` for truncation but uses `k` for exponent multiplication, which is correct.

### Pitfall 7: etamake and prodmake with non-zero q_shift after normalization
**What goes wrong:** After normalization, series from theta2-based computations may still have q_shift in (0,1). Functions like `etamake` already handle this (stores q_shift as key -1 in result). But `prodmake` works on integer indices and requires the series to start at index 0 with value 1. A non-zero q_shift doesn't affect prodmake's algorithm, but the user needs to know the result is multiplied by `q^{q_shift}`.
**How to avoid:** This is already handled. No changes needed in convert.h for this phase.

## Code Examples

### Example 1: normalize_q_shift() integration into operator*

```cpp
Series operator*(const Series& o) const {
    Series s;
    int t = std::min(trunc, o.trunc);
    s.trunc = t;
    s.q_shift = q_shift + o.q_shift;
    for (const auto& [e1, c1] : c) {
        for (const auto& [e2, c2] : o.c) {
            int exp = e1 + e2;
            if (exp >= t) break;
            Frac prod = c1 * c2;
            if (!prod.isZero()) {
                Frac old = s.coeff(exp);
                s.setCoeff(exp, old + prod);
            }
        }
    }
    s.clean();
    s.normalize_q_shift();  // <-- NEW
    return s;
}
```

### Example 2: Floor computation for Frac

```cpp
// Returns floor(q_shift) as int
// Precondition: q_shift.den > 0 (guaranteed by Frac::reduce())
static int q_shift_floor(const Frac& f) {
    if (f.isZero()) return 0;
    auto [quot, rem] = BigInt::divmod(f.num, f.den);
    if (!rem.isZero() && rem.neg)
        quot = quot - BigInt(1);
    if (quot.isZero()) return 0;
    int v = quot.d.empty() ? 0 : static_cast<int>(quot.d[0]);
    if (quot.neg) v = -v;
    return v;
}
```

### Example 3: Updated str() for fractional exponents

```cpp
std::string str(int maxTerms = 30) const {
    // Integer q_shift: use existing display logic unchanged
    if (q_shift.isZero()) {
        // ... existing code exactly as-is ...
    }

    // Fractional q_shift: expanded term display
    std::string out;
    int count = 0;
    for (const auto& [e, v] : c) {
        if (count >= maxTerms) break;
        Frac actual_exp = Frac(e) + q_shift;
        // Format: [sign] [coeff] q^(exp)
        if (count > 0) {
            out += (v < Frac(0)) ? " - " : " + ";
        }
        Frac av = v.abs();
        bool showNeg = (count == 0 && v < Frac(0));
        if (showNeg) out += "-";
        if (actual_exp.isZero()) {
            out += av.isOne() ? "1" : av.str();
        } else {
            if (!av.isOne()) out += av.str();
            // Format exponent
            if (actual_exp.den == BigInt(1)) {
                // Integer exponent: use unicode superscript
                int iexp = /* extract int from actual_exp */;
                out += (iexp == 1) ? "q" : ("q" + expToUnicode(iexp));
            } else {
                // Fractional exponent: parenthesized
                out += "q^(" + actual_exp.str() + ")";
            }
        }
        ++count;
    }
    out += " + O(q" + expToUnicode(trunc) + ")";
    return out;
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| q_shift raw, checked only at addition | Eager normalization to [0,1) | This phase | Fixes Block 25 and all integer-difference addition failures |
| `q^(frac) * (...)` display | Expanded terms `cq^(n+frac)` | This phase | Matches Maple convention |

## Open Questions

1. **Session save/load for q_shift**
   - What we know: The `saveSession` function in repl.h serializes Series coefficients but does NOT serialize `q_shift`. After eager normalization, most intermediate results will have q_shift absorbed, so this is usually fine. But series with genuinely fractional q_shift (e.g., raw theta2 result) would lose their q_shift on save/load.
   - Recommendation: Out of scope for this phase. Could be addressed later by adding q_shift to the serialization format. In practice, users save results after arithmetic (which normalizes q_shift), not raw theta2 outputs.

2. **coeffList() and coeff() with fractional q_shift**
   - What we know: These functions use integer indices. With q_shift != 0, the "actual" exponent is n + q_shift. Users calling `coeff(f, 3)` on a series with q_shift = 1/4 get the coefficient at q^{3+1/4} = q^{13/4}.
   - Recommendation: Document this behavior. No code change needed — it's mathematically correct and matches how coefficients are stored.

## Sources

### Primary (HIGH confidence)
- **src/series.h** — Direct inspection of Series struct, all arithmetic operators, q_shift usage in operator*, inverse(), pow(), subs_q(), str()
- **src/qfuncs.h** — theta2() sets q_shift = Frac(k, 4); theta3/theta4 have q_shift = 0
- **src/convert.h** — etamake() handles q_shift via key -1; sift() operates on integer indices
- **src/repl.h** — evalExpr() handles q^(frac) by setting q_shift on Series::one(); display() delegates to str()
- **src/bigint.h** — BigInt::divmod truncates toward zero, remainder has same sign as dividend (lines 311-378)
- **src/frac.h** — Frac always has den > 0 after reduce(); no existing floor/isInteger methods

### Secondary (HIGH confidence)
- **tests/maple-checklist.sh** — Block 25 (lines 222-230): tests `theta2(q,100)^2/theta2(q^3,40)^2 + theta3(q,100)^2/theta3(q^3,40)^2`; currently fails with "error: cannot add series with different q-shifts"
- **.planning/phases/63-q-shift-arithmetic-fix/63-CONTEXT.md** — Locked decisions from user discussion

## Metadata

**Confidence breakdown:**
- Normalization algorithm: HIGH — derived from first principles, verified by tracing Block 25
- BigInt::divmod behavior: HIGH — read directly from source (lines 311-378 of bigint.h)
- Display format changes: HIGH — requirements fully specified in CONTEXT.md
- Edge cases (negative trunc, negative indices): MEDIUM — theoretical analysis, needs testing

**Research date:** 2026-03-01
**Valid until:** indefinite (internal C++ project, no external dependency drift)
