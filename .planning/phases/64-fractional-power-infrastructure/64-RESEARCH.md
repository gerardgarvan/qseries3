# Phase 64 Research: Fractional Power Infrastructure

## Current State

### What exists
- `Series::pow(int n)` — binary exponentiation for integer powers (lines 266–280 of series.h)
- `Series::inverse()` — recurrence-based inverse with q_shift handling (lines 217–260)
- `normalize_q_shift()` — absorbs integer part of q_shift into coefficient indices (lines 100–126)
- `q^(frac)` special case in REPL — already handles `q^(1/2)` via q_shift (lines 996–1014 of repl.h)
- Parser already handles `f^(1/2)` syntactically — `1/2` parsed as `BinOp::Div(IntLit(1), IntLit(2))`

### What doesn't exist
- `Series::powFrac(Frac)` — not implemented
- `BigInt::iroot(int k)` — integer k-th root
- `Frac::rational_pow(Frac)` — rational power of rational number
- REPL dispatch for fractional exponents — `^` calls `evalToInt` which throws "non-integer division" on `1/2`

## Algorithm: Coefficient Recurrence

If `h = (1 + g)^α` where `g` has zero constant term and `α` is any rational, then `(1+g)·h' = α·g'·h`. This yields:

```
h[0] = 1
h[m] = α·g[m] + (1/m) · Σ_{j=1}^{m-1} ((α+1)·j - m) · g[j] · h[m-j]
```

**Verification** with `g = x`, `α` arbitrary:
- `h[1] = α·1 = α` ✓ matches `C(α,1) = α`
- `h[2] = 0 + (1/2)(α-1)·α = α(α-1)/2` ✓ matches `C(α,2)`
- `h[3] = 0 + (1/3)(α-2)·α(α-1)/2 = α(α-1)(α-2)/6` ✓ matches `C(α,3)`

**Complexity:** O(T²) — each h[m] needs O(m) work, summed over m = 1..T-1.

## Implementation Design

### `Series::powFrac(Frac alpha)`

```
1. If alpha.den == 1: delegate to pow(int)
2. Extract leading power: k = minExp()
   - Shift all coefficients: c[e-k] = c[e]
   - New q_shift contribution: k * alpha (added to result q_shift)
3. Factor out constant: c0 = coeff(0)
   - If c0 is zero: throw "powFrac: zero series"
   - Compute scale = c0^alpha via rational_pow (throw if irrational)
   - Normalize: g[j] = coeff(j) / c0 for j >= 1
4. Coefficient recurrence:
   h[0] = 1
   for m = 1 to trunc-1:
     h[m] = alpha * g[m] + (1/m) * Σ_{j=1}^{m-1} ((alpha+1)*j - m) * g[j] * h[m-j]
5. Build result:
   - Multiply all h[m] by scale
   - Set result.q_shift = original_q_shift + Frac(k) * alpha
   - Set result.trunc = trunc
6. normalize_q_shift()
7. Return
```

### `BigInt::iroot(int k)` — Integer k-th root

Binary search for `r` such that `r^k == n`. Throw if not perfect k-th root.

```
low = 0, high = n (or tighter bound)
while low <= high:
  mid = (low + high) / 2
  p = mid^k
  if p == n: return mid
  if p < n: low = mid + 1
  else: high = mid - 1
throw "not a perfect k-th root"
```

For k=2, Newton's method converges faster but binary search is simpler and sufficient for our BigInt sizes (series coefficients, not cryptographic).

### `Frac::rational_pow(Frac alpha)`

Compute `(a/b)^(p/q)` where result must be rational:
1. If q == 1: return `Frac(a^p, b^p)` (just integer power)
2. Compute `num_p = a^|p|`, `den_p = b^|p|` (BigInt::pow)
3. Take q-th root: `num_root = iroot(num_p, q)`, `den_root = iroot(den_p, q)` — throws if not perfect
4. If p < 0: swap num/den
5. Return `Frac(num_root, den_root)`

### REPL Dispatch Fix

Current code at lines 1025–1030 of repl.h:
```cpp
case BinOp::Pow: {
    int64_t expVal = evalToInt(e->right.get(), env, sumIndices);
    ...
    return l.pow(static_cast<int>(expVal));
}
```

Fix: catch `evalToInt` failure, evaluate exponent as expression, extract constant Frac:
```cpp
case BinOp::Pow: {
    try {
        int64_t expVal = evalToInt(e->right.get(), env, sumIndices);
        if (expVal > 10000 || expVal < -10000)
            throw std::runtime_error("pow: exponent too large");
        return l.pow(static_cast<int>(expVal));
    } catch (...) {
        EnvValue rv = eval(e->right.get(), env, sumIndices);
        Series rs = toSeries(rv, "pow exponent", env.T);
        if (rs.c.size() == 1 && rs.c.count(0) && rs.q_shift.isZero()) {
            return l.powFrac(rs.c.at(0));
        }
        throw std::runtime_error("pow: exponent must be integer or rational constant");
    }
}
```

## Test Strategy

### Roundtrip test
`(1-q)^(1/2)` squared should equal `(1-q)` to truncation.

### Known coefficients
`(1-x)^(1/2)` = `1 - x/2 - x²/8 - x³/16 - 5x⁴/128 - 7x⁵/256 - ...`

### Negative fractional exponent
`(1-q)^(-1/2)` = `1 + q/2 + 3q²/8 + 5q³/16 + 35q⁴/128 + 63q⁵/256 + ...`

### Constant term ≠ 1
`(4-4q)^(1/2)` = `2·(1-q)^(1/2)` — should scale correctly.

### Leading q-power
`(q-q²)^(1/2)` = `q^(1/2)·(1-q)^(1/2)` — q_shift should be 1/2.

## Files to Modify

| File | Changes |
|------|---------|
| `src/bigint.h` | Add `static BigInt iroot(BigInt n, int k)` and `static BigInt bigpow(BigInt base, int exp)` |
| `src/frac.h` | Add `Frac rational_pow(Frac alpha) const` |
| `src/series.h` | Add `Series powFrac(Frac alpha) const` |
| `src/repl.h` | Update BinOp::Pow dispatch to fall back to powFrac |
| `tests/acceptance-powfrac.sh` | New test script for fractional powers |
