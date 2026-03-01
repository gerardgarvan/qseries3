# Technology Stack: Half-Integer Jacobi Exponents & q-Shift Arithmetic Fixes

**Project:** q-series REPL — Milestone: Fix 3 Maple checklist failures + 3 dependent exercises
**Researched:** 2026-03-01
**Overall confidence:** HIGH (all changes are internal to existing types; no new dependencies)

---

## Executive Summary

All three checklist failures (Blocks 13, 14, 25) and three dependent exercise gaps (Exercises 4/b(q), 9/N(q), 10/findpoly) trace to two missing capabilities in the existing codebase:

1. **Series square root** — needed to reconstruct JAC factors with half-integer exponents
2. **q-shift normalization** — needed so series with integer-offset q-shifts can be added

No new types, no new files, no external dependencies. All fixes are extensions to `series.h` and `convert.h`, plus a new `Frac::isqrt()`-style helper. The existing `JacFactor = std::tuple<int, int, Frac>` already stores Frac exponents — the limitation is entirely in the reconstruction (`jac2series_impl`) and display (`jac2prod`) code that truncates non-integer exponents to zero.

---

## Recommended Stack Changes

### Core: series.h Extensions

| Addition | Type | Purpose | Why |
|----------|------|---------|-----|
| `Series::sqrt()` | New method | Formal power series square root via coefficient recurrence | Required by `jac2series_impl` to handle JAC exponents with den=2. The recurrence g[n] = (f[n] - Σ g[j]g[n-j]) / (2g[0]) is exact over Q when f[0] is a perfect rational square. |
| `Series::nthroot(int n)` | New method | Generalized n-th root (optional, sqrt is the urgent case) | Future-proofing for JAC exponents with den=3,4,... Not needed for current failures but trivial to add once sqrt works. |
| `Series::normalize_q_shift()` | New method | Absorb integer part of q_shift into coefficient map | Fixes Block 25 and Exercise 10. When q_shift has integer part k, shift all indices by k and set q_shift to fractional remainder. |
| `operator+` update | Modification | Call normalize before shift comparison | After normalization, series with q_shift=-1 and q_shift=0 become both q_shift=0, enabling addition. |

### Core: frac.h Extension

| Addition | Type | Purpose | Why |
|----------|------|---------|-----|
| `Frac::is_perfect_square()` | New method | Check if num and den are both perfect squares | Guard for Series::sqrt() — the constant term must have a rational square root. |
| `Frac::rational_sqrt()` | New method | Compute exact sqrt when possible | Returns Frac(sqrt(num), sqrt(den)) after verifying both are perfect squares. |

### Core: bigint.h Extension

| Addition | Type | Purpose | Why |
|----------|------|---------|-----|
| `BigInt::isqrt()` | New method | Integer square root via Newton's method | Required by Frac::rational_sqrt(). Base-10^9 Newton iteration: x_{k+1} = (x_k + n/x_k) / 2 until convergence. |
| `BigInt::is_perfect_square()` | New method | Check isqrt(n)^2 == n | Guard for rational sqrt. |

### Convert: convert.h Modifications

| Change | Type | Purpose | Why |
|--------|------|---------|-----|
| `jac2series_impl` fractional exponent handling | Modification | Handle exp with den=2 via sqrt | Currently: `int ex = 0; if (exp.den == BigInt(1)) ...` silently drops non-integer exponents. Fix: decompose p/2 as (f^p)^(1/2) or f.sqrt().pow(p). |
| `jac2prod` fractional display | Modification | Format `^(13/2)`, `√` notation | Currently: `int ex = 0; if (exp.den == BigInt(1)) ...` skips fractional exponents entirely. |
| `jacprodmake` half-integer allowance | Verification | Confirm decomposition already produces Frac x[a] | The existing logic uses Frac arithmetic throughout — x[a] = e[a] where e[a] = -prodmake_result[a]. If prodmake returns non-integer a[n], the decomposition naturally produces half-integer x[a]. Needs testing, not code change. |

---

## Detailed Algorithm: Series Square Root

Given f = Σ f[n] q^n with f[0] a perfect rational square, compute g = f^(1/2):

```
g[0] = rational_sqrt(f[0])
for n = 1 to T-1:
    sum = Σ_{j=1}^{n-1} g[j] * g[n-j]
    g[n] = (f[n] - sum) / (2 * g[0])
```

**Correctness:** g² = f holds coefficient-by-coefficient. The n-th coefficient of g² is:
`Σ_{j=0}^{n} g[j]g[n-j] = 2*g[0]*g[n] + Σ_{j=1}^{n-1} g[j]g[n-j]`
Setting this equal to f[n] and solving for g[n] yields the recurrence.

**Complexity:** O(T²) Frac operations — same as Series multiplication. No issue for T ≤ 500.

**Edge cases:**
- f[0] = 0: not a valid input (leading coefficient must be nonzero)
- f[0] = 1: g[0] = 1, simplifies to g[n] = (f[n] - Σ g[j]g[n-j]) / 2
- f[0] not a perfect square: throw runtime_error (not representable in exact Q arithmetic)
- q_shift handling: result.q_shift = f.q_shift / Frac(2)

---

## Detailed Algorithm: q-Shift Normalization

Given Series s with q_shift = p/d (a Frac):

```
int_part = floor(p/d)    // integer part
frac_part = p/d - int_part

new_coefficients = {}
for each (exponent e, coefficient c) in s.c:
    new_coefficients[e + int_part] = c

s.c = new_coefficients
s.q_shift = frac_part     // always in [0, 1) after normalization
```

**Floor for Frac:** `floor(p/d) = p / d` using integer division rounding toward -∞:
- If p ≥ 0: floor = p.num / p.den (integer division)
- If p < 0: floor = -((-p.num + p.den - 1) / p.den) (ceiling of |p|/d, negated)

**When to normalize:**
- After multiplication (q_shift = a.q_shift + b.q_shift can exceed 1)
- After division/inverse (q_shift = a.q_shift - b.q_shift can be negative)
- After pow (q_shift = a.q_shift * n can grow arbitrarily)
- Before addition (to make shifts compatible)

**Tradeoff:** Normalize eagerly (in *, /, pow) vs. lazily (only before +). Eager is safer — prevents q_shift from growing and makes addition always work without explicit user action. The cost is negligible: one Frac floor operation + coefficient map rebuild.

**Recommendation: Eager normalization.** Apply in operator*, operator/, inverse(), pow(), subs_q(). This makes q_shift always in [0, 1), and addition between compatible series "just works."

---

## Detailed Fix: jac2series_impl Fractional Exponents

Current code (lines 353-361 of convert.h):
```cpp
int ex = 0;
if (exp.den == BigInt(1) && exp.num.d.size() == 1 && exp.num.d[0] <= 1000)
    ex = exp.num.neg ? -static_cast<int>(exp.num.d[0]) : static_cast<int>(exp.num.d[0]);
if (ex > 0) { /* pow */ } else if (ex < 0) { /* inv then pow */ }
```

This silently ignores non-integer exponents (ex stays 0, factor is skipped).

**Fix approach:**
1. Check if `exp.den == BigInt(2)` (half-integer case)
2. Extract integer numerator p from exp = p/2
3. Compute `fac.sqrt().pow(p)` if p > 0, or `fac.inverse().sqrt().pow(-p)` if p < 0
4. General case: `fac.nthroot(den).pow(num)` (den extracted as int from exp.den)

For the Slater identity (Block 13), the exponents are 13/2 and -1/2, both with den=2.

---

## Block 25 Trace: Why q-Shift Normalization Fixes It

```
theta2(q, 100)         → q_shift = 1/4, coeffs at 0, 2, 6, 12, ...
theta2(q, 100)^2       → q_shift = 1/2 (via multiplication: 1/4 + 1/4)
theta2(q^3, 40)        → q_shift = 3/4, coeffs at 0, 6, 18, ...
theta2(q^3, 40)^2      → q_shift = 3/2 (via multiplication: 3/4 + 3/4)
x1 = t2(q)^2 / t2(q^3)^2  → q_shift = 1/2 - 3/2 = -1  [INTEGER!]

theta3(q, 100)^2       → q_shift = 0
theta3(q^3, 40)^2      → q_shift = 0
x2 = t3(q)^2 / t3(q^3)^2  → q_shift = 0

x1 + x2 → FAILS: q_shift -1 ≠ 0
```

After normalization of x1: q_shift -1 → absorb into coefficients (shift all indices by -1), q_shift becomes 0. Now x1 + x2 works.

Similarly for the `a` computation in Block 25:
```
theta2(q)*theta2(q^3)  → q_shift = 1/4 + 3/4 = 1  [INTEGER!]
theta3(q)*theta3(q^3)  → q_shift = 0

After normalizing the theta2 product: q_shift 1 → absorbed, becomes 0.
Addition now works.
```

And for `c^3` where `c = 3*q^(1/3)*etaq(3,T)^3/etaq(1,T)`:
```
c has q_shift = 1/3
c^3 has q_shift = 1  [INTEGER!]
After normalization: q_shift becomes 0.
```

**All q-shift issues in Block 25 reduce to integer offsets after arithmetic.** Normalization handles them all.

---

## Exercise Fixes Enabled by Stack Changes

| Exercise | Current Status | Fix | Stack Change Used |
|----------|---------------|-----|-------------------|
| 4 (b(q)) | Partial — ω unsupported | Define `bq := etaq(1,T)^3 / etaq(3,T)` directly (known eta product result) | None — workaround using known answer |
| 6 (relations) | Partial — depends on b(q) | Use b(q) workaround from Exercise 4 | None |
| 9 (N(q)) | Partial — findnonhomcombo infeasible | Use larger truncation; may need to accept partial result | None — performance issue, not missing capability |
| 10 (findpoly) | Blocked — q-shift addition | After q-shift normalization, compute m = θ₃²/θ₃(q³)² and y = c³/a³, then findpoly(x, y, ...) | q-shift normalization |

---

## Alternatives Considered

| Decision | Chosen | Alternative | Why Not |
|----------|--------|-------------|---------|
| Series sqrt algorithm | Coefficient recurrence | Newton iteration on series | Newton converges quadratically but requires more complex code; recurrence is O(T²) either way and simpler |
| q-shift normalization timing | Eager (in *, /, pow) | Lazy (only in +) | Eager prevents accumulation of large integer offsets and makes all code paths consistent |
| Half-integer scope | den=2 only initially | General rational exponents | Only den=2 appears in the reference document; general case can be added later via nthroot |
| BigInt isqrt algorithm | Newton's method in base 10^9 | Binary search | Newton converges in O(log(digits)) iterations; binary search is O(digits × log(digits)) |
| b(q) computation | Direct eta product formula | Algebraic number support for ω | Adding algebraic extension fields (Z[ω]) would require a new number type, new arithmetic, new parser support — massive scope for one exercise |

---

## No New Dependencies

All changes use existing types:
- `BigInt` for integer square root
- `Frac` for rational square root
- `Series` for formal power series sqrt
- `std::map<int, Frac>` for coefficient storage (unchanged)
- `JacFactor = std::tuple<int, int, Frac>` for JAC factors (unchanged — already uses Frac exponent)

**Zero new headers. Zero new structs. Zero external packages.**

---

## Implementation Ordering

```
1. BigInt::isqrt() + is_perfect_square()     [bigint.h, ~20 lines]
   └─ test: isqrt(1000000000000000000) = 1000000000
   
2. Frac::rational_sqrt() + is_perfect_square()  [frac.h, ~15 lines]
   └─ test: Frac(9,4).rational_sqrt() = Frac(3,2)
   
3. Series::sqrt()                            [series.h, ~20 lines]
   └─ test: (1+q)^2 → sqrt → should give 1+q

4. Series::normalize_q_shift()               [series.h, ~15 lines]
   └─ test: {q_shift=-1, c[0]=1} → {q_shift=0, c[-1]=1}

5. Eager normalization in *, /, pow, inverse  [series.h, ~8 lines per method]
   └─ test: theta2(q,20)^2 / theta2(q^3,10)^2 has q_shift=0

6. jac2series_impl half-integer support       [convert.h, ~15 lines]
   └─ test: JAC(0,14,∞)^(13/2) reconstructs to expected series

7. jac2prod fractional display                [convert.h, ~10 lines]
   └─ test: exponent 1/2 displays as √, exponent 13/2 displays as ^(13/2)

8. Verify jacprodmake produces half-integer results  [testing only]
   └─ test: Block 13 Slater series → JAC(0,14)^(13/2) output
```

Steps 1-3 are the sqrt chain (bottom-up dependency).
Steps 4-5 are the q-shift chain (independent of sqrt).
Steps 6-7 require step 3.
Step 8 requires steps 6-7.

**Steps 1-3 and 4-5 can be developed in parallel.**

---

## Risk Assessment

| Risk | Likelihood | Impact | Mitigation |
|------|-----------|--------|------------|
| prodmake returns exact half-integers for Slater series | LOW risk (algorithm is exact over Q) | HIGH — if a[n] aren't exact half-integers, jacprodmake can't identify the product | Verify with explicit computation: compute prodmake for the Block 13 series and inspect exponents |
| Coefficient index shift in normalize produces negative indices | MEDIUM — series already supports negative keys in std::map | LOW — map handles negative keys naturally | Test with theta2 products that produce q_shift = -1 |
| sqrt of non-perfect-square constant term | LOW — JAC(a,b,∞) series always start with 1 | MEDIUM — throws at runtime | Guard with is_perfect_square() check + clear error message |
| Performance regression from eager normalization | LOW — one map rebuild per operation | LOW — negligible compared to O(T²) multiply | Profile if any test becomes noticeably slower |

---

## Sources

- **Block 13-14 expected output:** `qseriesdoc.md` lines 526-530 — `JAC(0,14,∞)^(13/2)` with `√JAC(7,14,∞)`
- **Block 25 Maple commands:** `qseriesdoc.md` lines 916-926 — theta2/theta3 quotients + findpoly
- **Formal power series square root algorithm:** Standard result in combinatorics/computer algebra — coefficient recurrence from g² = f
- **q-shift design:** Existing `series.h` q_shift field (line 15) — already Frac-typed, just missing normalization
- **JacFactor type:** `convert.h` line 329 — `std::tuple<int, int, Frac>` already supports fractional exponents in the type
- **Phase 62 verification:** `.planning/phases/62-maple-checklist/62-VERIFICATION.md` — documents all three failures as "known limitations"

**Confidence:** HIGH — all algorithms are well-known, all types already exist, all changes are local to 3 files.
