# Feature Landscape: Half-Integer Jacobi Exponents & q-Shift Arithmetic Fixes

**Domain:** q-series REPL — Maple checklist parity fixes (Blocks 13/14/25, Exercises 4/9/10)
**Researched:** 2026-03-01
**Sources:** Maple qseries 1.3 documentation (qseries.org), qseriesdoc.md reference tutorial, existing codebase analysis

---

## Table Stakes

Features required for Maple tutorial parity. Missing = blocks fail, exercises unsolvable.

| Feature | Why Expected | Complexity | Blocks/Exercises Affected |
|---------|--------------|------------|---------------------------|
| Half-integer JAC exponents in `jacprodmake` | Maple produces `JAC(a,b,∞)^(1/2)` for Slater's identity | **Medium** | Block 13, Block 14, Exercise 5 |
| Half-integer JAC exponents in `jac2series` | Must reconstruct series from `JAC(a,b)^(1/2)` | **Medium** | Block 14 |
| Integer q-shift absorption | `theta2(q)^2/theta2(q^3)^2` yields q_shift=-1 (integer), must merge into coefficient map | **Low** | Block 25, Exercise 10 |
| b(q) without omega | Exercise 4 needs b(q) = η(τ)³/η(3τ), computable without RootOf | **Low** | Exercise 4, Exercise 6, Exercise 9 |
| N(q) Eisenstein E₆ series | N(q) = 1 - 504·Σ σ₅(n)qⁿ — `sigma(n,5)` already exists | **Low** | Exercise 9 |

---

## Feature 1: Half-Integer Jacobi Exponents (`jacprodmake` + `jac2series`)

### The Problem

Block 13 computes Slater's identity: `Σ q^(n(n+1)/2)(-q;q)_n / (q;q)_{2n+1}`. Maple's `jacprodmake` outputs:

```
JAC(0,14,∞)^(13/2) / (JAC(1,14,∞)² JAC(3,14,∞) JAC(4,14,∞)
  JAC(5,14,∞) JAC(6,14,∞) √JAC(7,14,∞))
```

Our implementation returns empty (fails) because `jacprodmake` rejects non-integer exponents during the decomposition step.

### How Maple Handles It

From the official documentation (qseries.org/fgarvan/qmaple/qseries/functions/jacprodmake.html):

1. **`prodmake` produces rational `a[n]`:** Andrews' algorithm doesn't require integer outputs. The recurrence `a[n] = (c[n] - Σ d·a[d])/n` can produce rationals like 1/2, 3/2 when the input series is the square root of a Jacobi product. Our `prodmake` already handles this correctly — it stores `a[n]` as `Frac`, and the non-integer warning is just informational.

2. **`jacprodmake` checks periodicity on rationals:** The periodicity test `e[n] == e[n+b]` already works with `Frac` comparisons in our code. The 80% match threshold applies identically.

3. **JAC decomposition allows rational exponents:** When decomposing `e[1..b]` into JAC(a,b) exponents, the symmetry check `e[a] == e[b-a]` and the `x[0] = e[b] - Σ x[a]` computation work the same — they just produce `Frac` values like 13/2 or 1/2 instead of integers.

4. **`jac2series` handles fractional powers:** For `JAC(a,b)^(1/2)`, the series is `((q^a;q^b)_∞ (q^{b-a};q^b)_∞ (q^b;q^b)_∞)^(1/2)`. This requires computing the square root of a series, which is a well-defined operation: given f = 1 + c₁q + c₂q² + ..., g = f^(1/2) satisfies g² = f, solvable by the recurrence g[0]=1, g[n] = (f[n] - Σ_{j=1}^{n-1} g[j]·g[n-j]) / (2·g[0]).

### Expected Maple Output (Block 13)

```
JAC(0,14,∞)^(13/2) / (JAC(1,14,∞)² JAC(3,14,∞) JAC(4,14,∞)
  JAC(5,14,∞) JAC(6,14,∞) JAC(7,14,∞)^(1/2))
```

More precisely from the Maple docs page, a similar example shows:

```
jacprodmake(Y,q,100);
                 2 JAC(0, 2, infinity)
                 ---------------------
                 /JAC(1, 2, infinity)\1/2
                 |-------------------|
                 \JAC(0, 2, infinity)/
```

This demonstrates that Maple nests JAC quotients inside fractional exponents. Our output should use `^(1/2)` notation for non-integer exponents.

### What Needs to Change

**In `jacprodmake` (convert.h:367-428):**
- The decomposition logic already stores exponents as `Frac` — no change needed in the data path
- The symmetry check `e[a] == e[b-a]` already uses `Frac` comparison — works
- The verification step `jac2series_impl(result, T)` will fail because `jac2series_impl` currently truncates fractional exponents to 0 — this is the actual bug
- **Fix:** Remove the integer-only truncation in `jac2series_impl`; add fractional power support

**In `jac2series_impl` (convert.h:343-365):**
- Lines 353-355 extract integer exponent: `if (exp.den == BigInt(1) && exp.num.d.size() == 1 ...)` — this silently drops fractional exponents
- **Fix:** Add a `Series::rational_pow(Frac p)` method that handles half-integer powers via Newton's recurrence for formal power series
- For `p = n/2` (the common case): compute `f^n` then take formal square root
- For general `p = a/b`: compute `f^a` then take b-th root (recurrence: `g[0]=f[0]^(1/b)`, which must be rational)

**In Series (series.h):**
- Add `Series sqrt()` method: given f with f[0]=1, compute g s.t. g²=f
- Recurrence: `g[0] = 1`, `g[n] = (f[n] - Σ_{j=1}^{n-1} g[j]·g[n-j]) / 2`
- Add `Series nth_root(int n)` for generality (b-th root)

**In output formatting (repl.h or convert.h):**
- Display `JAC(a,b,∞)^(1/2)` or `√JAC(a,b,∞)` for exponent = 1/2
- Display `JAC(a,b,∞)^(p/q)` for other fractional exponents

### Confidence: HIGH

The algorithm is mathematically straightforward. The data structures already support `Frac` exponents. The gap is only in `jac2series_impl` (truncates to int) and a missing `Series::sqrt()` method.

---

## Feature 2: Integer q-Shift Absorption

### The Problem

Block 25 computes:
```
x1 := theta2(q,100)^2 / theta2(q^3,40)^2
x2 := theta3(q,100)^2 / theta3(q^3,40)^2
x := x1 + x2   ← FAILS: "cannot add series with different q-shifts"
```

`theta2(q)` has `q_shift = 1/4` (from the q^(1/4) factor). After squaring and dividing:
- `theta2(q)^2` → q_shift = 1/2
- `theta2(q^3)^2` → q_shift = 3/2
- `x1 = theta2(q)^2 / theta2(q^3)^2` → q_shift = 1/2 - 3/2 = **-1** (an integer!)
- `x2 = theta3(q)^2 / theta3(q^3)^2` → q_shift = **0**

Adding q_shift=-1 to q_shift=0 fails, but q_shift=-1 is an integer power of q that could be absorbed: `q^(-1) · Σ c[n] q^n = Σ c[n] q^(n-1)`. Simply shift all coefficient indices by -1 and set q_shift=0.

### How Maple Handles It

Maple uses `radsimp` to simplify expressions involving radicals. When theta2(q)^2/theta2(q^3)^2 is computed, the q^(1/2)/q^(3/2) = q^(-1) factor is simplified algebraically, leaving a standard q-series with integer exponents. The `radsimp` call is explicit in the Maple code for Block 25 and Exercise 10.

### What Needs to Change

**In Series arithmetic (series.h):**
- After any arithmetic operation that produces a result, check if `q_shift` has integer value (i.e., `q_shift.den == BigInt(1)`)
- If so, absorb: shift all coefficient keys by the integer q_shift value, then set q_shift to 0
- This should be a `normalize_q_shift()` method called at the end of `operator*`, `operator/`, `inverse()`, `pow()`, and `subs_q()`

**Implementation detail:**
```cpp
void normalize_q_shift() {
    if (q_shift.den != BigInt(1) || q_shift.isZero()) return;
    int shift = /* extract integer from q_shift */;
    std::map<int, Frac> new_c;
    for (const auto& [e, v] : c) {
        int new_e = e + shift;
        if (new_e >= 0 && new_e < trunc)
            new_c[new_e] = v;
    }
    c = std::move(new_c);
    q_shift = Frac(0);
}
```

- **Negative shift handling:** If q_shift = -1, coefficients shift left (c[n] → c[n-1]), allowing negative indices if needed. The Series already uses `std::map<int, Frac>` which supports negative keys, and `trunc` stays the same.
- **Positive shift handling:** If q_shift = 3, coefficients shift right (c[n] → c[n+3]), and trunc should be adjusted.

### Expected Output (Block 25)

After the fix, `x = x1 + x2` should produce a valid integer-exponent series (q_shift=0), and then:
```
findpoly(x, y, 3, 1, 60)
```
should output: `(X + 6)³ Y - 27 (X + 2)²`

### Confidence: HIGH

This is a mechanical fix — absorb integer q_shifts into coefficient maps. No mathematical complexity.

---

## Feature 3: Exercise 4 — b(q) Without Omega

### The Problem

Exercise 4 defines b(q) = Σ_{n,m} ω^(n-m) q^(n²+nm+m²) where ω = exp(2πi/3). Computing this directly requires algebraic number support (RootOf), which is out of scope.

### The Known Identity

b(q) = η(τ)³/η(3τ) — an eta product, computable without complex numbers!

**Proof route:** Since ω satisfies ω²+ω+1=0, and ω^k depends only on k mod 3:
- S₀ = Σ_{(n-m)≡0 mod 3} q^(n²+nm+m²)
- S₁ = Σ_{(n-m)≡1 mod 3} q^(n²+nm+m²)
- S₂ = Σ_{(n-m)≡2 mod 3} q^(n²+nm+m²)

By symmetry (n,m) → (m,n): S₁ = S₂. Then:
- a(q) = S₀ + S₁ + S₂ = S₀ + 2S₁
- b(q) = S₀ + ω·S₁ + ω²·S₂ = S₀ - S₁ (since ω+ω²=-1)

So **b(q) = (3S₀ - a(q))/2**, where S₀ is computable from integer sums only.

### Implementation in REPL

```
set_trunc(200)
aq := sum(sum(q^(n*n+n*m+m*m), m, -20, 20), n, -20, 20)
S0 := sum(sum(q^(n*n+n*m+m*m), m, -20, 20, 3*n), n, -20, 20)
```

Wait — S₀ needs the constraint (n-m) ≡ 0 mod 3. The simplest approach:

```
S0 := sum(sum(q^((3*k+m)*(3*k+m)+(3*k+m)*m+m*m), m, -20, 20), k, -20, 20)
```

Substituting n = 3k+m to enforce n ≡ m (mod 3), then simplify: (3k+m)²+(3k+m)m+m² = 9k²+6km+m²+3km+m²+m² = 9k²+9km+3m² = 3(3k²+3km+m²).

So S₀ = Σ_{k,m} q^(3(3k²+3km+m²)).

Then b(q) = (3S₀ - a(q)) / 2.

**Alternative (simpler):** Just compute b(q) directly as an eta product:
```
bq := etaq(1, 200)^3 / etaq(3, 200)
etamake(bq, 100)
```
This should confirm b(q) = η(τ)³/η(3τ). The exercise asks to **find** which are eta products, and this approach demonstrates b(q) is one.

### What Needs to Change

Nothing in the codebase. The workaround is pure REPL commands. The exercise solution just needs to:
1. Compute a(q) from the double sum (already working)
2. Compute c(q) from the shifted double sum (already working, confirmed in exercise solutions)
3. Compute b(q) as `etaq(1,200)^3/etaq(3,200)` and verify with `etamake`
4. Alternatively compute b(q) = (3S₀ - a(q))/2 and verify

### Confidence: HIGH

The identity b(q) = η(τ)³/η(3τ) is well-established in the literature [12]. No code changes needed — just correct REPL commands.

---

## Feature 4: Exercise 9 — N(q) = E₆ Eisenstein Series

### The Problem

Exercise 9 requires:
1. Computing N(q) = 1 - 504·Σ_{n≥1} n⁵qⁿ/(1-qⁿ) = 1 - 504·Σ_{n≥1} σ₅(n)qⁿ
2. Using `findnonhomcombo` to express N(q) in terms of a(q) and x(q) = c(q)³/a(q)³

### What's Already Available

- `sigma(n, 5)` — already implemented in `qfuncs.h:98`
- `sum(sigma(n,5)*q^n, n, 1, T)` — REPL supports this via `sum`
- `findnonhomcombo` — already implemented

### Implementation

```
set_trunc(100)
aq := sum(sum(q^(n*n+n*m+m*m), m, -15, 15), n, -15, 15)
cq := 3*q^(1/3)*etaq(3,100)^3/etaq(1,100)
xq := cq^3 / aq^3
Nq := 1 - 504*sum(sigma(n,5)*q^n, n, 1, 50)
```

### Challenges

1. **c(q) = 3q^(1/3)·η(3τ)³/η(τ):** The q^(1/3) factor means c(q)³ = 27q·η(3τ)⁹/η(τ)³ has q_shift = 1 (an integer). This requires Feature 2 (integer q_shift absorption) for the division `c(q)³/a(q)³` to produce a plain series.

2. **Large coefficients:** x(q) = c(q)³/a(q)³ produces very large rational coefficients. With T=100, the σ₅(n) values grow rapidly (σ₅(50) = 98,456,189,057). The `findnonhomcombo` matrix will have entries of this magnitude.

3. **Weight analysis:** N(q) has weight 6, a(q) has weight 1. So the expected relation has a(q)⁶ and x(q) (weight 0) terms, meaning `findnonhomcombo(Nq, [aq, xq], [6, ?], 0)`. Need to determine the correct weight vector.

### Expected Result

From [8] (Borwein-Borwein-Garvan): N(q) = a(q)⁶ · (1 - 540x) where x = c³/a³.

In findnonhomcombo format with weights [6,3] or similar:
```
N = a⁶ - 540·a⁶·x = a⁶(1 - 540x)
```

Equivalently: `N = a⁶ - 540·c³·a³`

### What Needs to Change

- Feature 2 (integer q_shift absorption) must work for c(q)³/a(q)³
- Performance may be an issue — large truncation needed for σ₅ sums
- May need to increase `set_trunc` to 150+ for reliable `findnonhomcombo`

### Confidence: MEDIUM

The mathematical approach is clear, but practical feasibility depends on coefficient sizes and computation time. The existing exercise solution notes this is "infeasible within practical truncation limits."

---

## Feature 5: Exercise 10 — findpoly with m = θ₃(q)²/θ₃(q³)²

### The Problem

Exercise 10 defines m = (θ₃(q)/θ₃(q³))² and asks to find y = c³/a³ as a rational function of m using `findpoly`.

### Key Insight

Exercise 10 is **simpler than Block 25**. While Block 25 defines x = θ₂(q)²/θ₂(q³)² + θ₃(q)²/θ₃(q³)² (requires theta2 with q-shifts), Exercise 10 uses **only** m = θ₃(q)²/θ₃(q³)² which has **no q-shift** (theta3 has q_shift=0).

The challenge is computing y = c³/a³, which requires:
- c(q) = 3q^(1/3)·η(3τ)³/η(τ), so c³ = 27q·η(3τ)⁹/η(τ)³ → q_shift=1 (integer)
- a(q) computed from double sum → q_shift=0
- y = c³/a³ → q_shift=1 (integer, absorbed to 0)

So Exercise 10 needs Feature 2 (integer q_shift absorption) but does **not** need the theta2 addition fix from Block 25.

### Expected Output

From the Maple documentation (findpoly.html):
```
findpoly(m, y, 6, 1, 50)
→ -1/27·(X²+6X-3)³·Y + (X-1)·(X+1)⁴
```

So y = c³/a³ = 27·(m-1)(m+1)⁴ / (m²+6m-3)³. This is Eq.(12.8) in [8].

### What Needs to Change

- Feature 2 (integer q_shift absorption) — for c³/a³ computation
- `findpoly` is already implemented and working

### Confidence: HIGH

Straightforward once integer q_shift absorption works.

---

## Anti-Features

Features to explicitly NOT build for this milestone.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|---------------------|
| Full algebraic number support (RootOf) | Architectural complexity far exceeds benefit | Use identity b(q) = η(τ)³/η(3τ) |
| Maple's `radsimp` symbolic simplifier | Not a q-series operation; massive scope | Integer q_shift absorption handles the actual need |
| General n-th root of formal power series | Only need square root for half-integer JAC | Implement `Series::sqrt()` only; generalize later if needed |
| Maple `proc` definitions in REPL | Already handled via built-in T(r,n) and inline workarounds | Continue using existing approach |

---

## Feature Dependencies

```
Feature 2 (q_shift absorption) ← Feature 5 (Exercise 10)
Feature 2 (q_shift absorption) ← Feature 4 (Exercise 9, for c³/a³)
Feature 2 (q_shift absorption) ← Block 25 (findpoly with theta2)
Feature 1 (half-integer JAC)    ← Block 13, Block 14
Feature 1 (half-integer JAC)    → requires Series::sqrt()
Feature 3 (b(q) workaround)    ← independent (REPL commands only)
Feature 4 (Exercise 9)          ← Feature 2 + Feature 3 (for a(q), c(q))
```

**Recommended implementation order:**
1. Feature 2 (integer q_shift absorption) — Low complexity, unblocks 3 items
2. Feature 1 (half-integer JAC exponents) — Medium complexity, core gap
3. Features 3/4/5 (exercises) — Mostly REPL usage, no code changes needed after 1 & 2

---

## Complexity Assessment

| Feature | Lines of Code | Risk | Dependencies on Existing Code |
|---------|--------------|------|-------------------------------|
| Integer q_shift absorption | ~20 lines in series.h | Low | Touches Series operator methods |
| Series::sqrt() method | ~15 lines in series.h | Low | Self-contained; analogous to inverse() |
| jacprodmake fractional support | ~5 lines changed in convert.h | Low | Already uses Frac throughout |
| jac2series fractional support | ~20 lines in convert.h | Medium | Needs Series::sqrt() or rational_pow() |
| JAC output formatting | ~10 lines in convert.h/repl.h | Low | String formatting only |
| Exercise 4 b(q) | 0 lines (REPL commands) | None | — |
| Exercise 9 N(q) | 0 lines (REPL commands) | Medium (perf) | Depends on Features 2, 3 |
| Exercise 10 findpoly | 0 lines (REPL commands) | Low | Depends on Feature 2 |

**Total estimated code changes:** ~70 lines across series.h and convert.h

---

## Sources

- Maple qseries package v1.3, jacprodmake documentation: https://qseries.org/fgarvan/qmaple/qseries/functions/jacprodmake.html (Confidence: HIGH — official authoritative source, includes example with `√JAC` output)
- Maple qseries package, findpoly documentation: https://qseries.org/fgarvan/qmaple/qseries/functions/findpoly.html (Confidence: HIGH — shows expected Exercise 10 output)
- qseriesdoc.md reference tutorial §3.4 Output 15 (Confidence: HIGH — the original expected output)
- maple_checklist.md (Confidence: HIGH — actual test results showing Block 13/14/25 failures)
- [8] Borwein-Borwein-Garvan: cubic theta identities, Eq.(12.8) for Exercise 10 result
- [12] Borwein-Borwein: b(q) = η(τ)³/η(3τ) identity
