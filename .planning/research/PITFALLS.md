# Domain Pitfalls — Half-Integer Jacobi Exponents & Q-Shift Arithmetic Fixes

**Domain:** Adding half-integer Jacobi product exponents, mixed q-shift addition, and double-sum support to existing C++20 q-series REPL
**Researched:** 2026-03-01
**Overall confidence:** HIGH (verified against codebase, reference doc qseriesdoc.md, and Maple output)

---

## Critical Pitfalls

Mistakes that cause silent wrong answers, crashes, or require multi-file rewrites.

---

### Pitfall 1: `jac2prod` and `jac2series_impl` Silently Discard Fractional Exponents

**What goes wrong:** Both `jac2prod` (display) and `jac2series_impl` (series reconstruction) extract `int ex` from the Frac exponent with a guard `exp.den == BigInt(1)`. When the exponent is 13/2 or 1/2, this check fails and `ex` stays 0. The factor is silently skipped — no error, no warning, just wrong output.

**Where it is:**

- `convert.h:354` — `jac2series_impl`: `if (exp.den == BigInt(1) && exp.num.d.size() == 1 && exp.num.d[0] <= 1000)` → extracts int, else ex=0 → factor is identity
- `convert.h:438` — `jac2prod`: same pattern, `if (exp.den == BigInt(1) ...)` → ex=0 → `continue` skips the factor

**Consequences:**
- `jac2prod(jp)` for Slater (46) shows nothing or a partial product — the 13/2 and 1/2 exponents vanish
- `jac2series(jp, 500)` reconstructs the wrong series — missing factors means coefficients are wrong
- No error is thrown, so users trust the output

**Prevention:**
1. In `jac2prod`: display fractional exponents using `^(num/den)` notation. E.g., `JAC(0,14,∞)^(13/2)`. For exponent 1/2, display as `√JAC(...)` or `JAC(...)^(1/2)`.
2. In `jac2series_impl`: implement `Series::pow(Frac)` for half-integer exponents, or decompose JAC(a,b)^(1/2) into its constituent (q^a;q^b)^(1/2) factors and use the identity `f^(1/2) = exp(1/2 · log f)` via formal power series logarithm.
3. Alternative for `jac2series_impl`: extract numerator and denominator separately. For exponent p/q, compute `fac^p` then take the q-th root (via Newton iteration on formal power series).

**Detection:** Any `JacFactor` with `exp.den != BigInt(1)` that reaches `jac2prod` or `jac2series_impl` will be silently dropped.

**Severity:** CRITICAL — silent wrong output, no error thrown
**Phase:** Half-integer Jacobi exponents phase (Task 1: display, Task 2: series reconstruction)

---

### Pitfall 2: `jacprodmake` Decomposition Cannot Produce Fractional x[a] Values

**What goes wrong:** The jacprodmake algorithm decomposes prodmake exponents e[n] into JAC(a,b) factors by assigning `x[a] = e[a]` directly. Since prodmake always returns integer a[n], and e[n] = -a[n], all x[a] values are integers. But Block 13 (Slater 46) needs JAC(0,14)^(13/2) and JAC(7,14)^(1/2) — half-integer exponents that cannot arise from the current algorithm.

**Why it happens:** The mathematical issue is subtle. For a JAC factor with `a > 0`, `JAC(a,b)^e` contributes exponent `e` to residues `a`, `b-a`, and `b` (mod b). When `2a = b` (the middle residue), `JAC(b/2, b)^e = (q^{b/2};q^b)^{2e} (q^b;q^b)^e` — note the doubling for the middle element because `a = b-a`. The current code assigns `x[b/2] = e[b/2]`, but `JAC(b/2, b)^{x[b/2]}` contributes `2·x[b/2]` to the prodmake exponent at residue `b/2`. So `x[b/2]` should be `e[b/2] / 2`, which is fractional when `e[b/2]` is odd.

**Where it is:** `convert.h:395-408` — the decomposition loop handles `b % 2 == 0` case with `x[half] = e[half]` but doesn't account for the doubling that JAC(b/2, b) produces at residue b/2.

**Consequences:**
- Block 13 returns empty result (verification `recon == f` at line 422 fails)
- Exercise 5 happens to have modulus 20 with all even-residue exponents, so it works by accident

**Prevention:**
1. Fix the decomposition: for `a = b/2`, set `x[a] = e[a] / 2` (since JAC(a,b) contributes 2·x[a] to residue a when a = b-a)
2. For `a = 0`, `JAC(0,b)^e` contributes `e` to residue 0 only. So `x[0] = e[b] - Σ_{a>0} x[a]` (current formula is correct IF the other x[a] are correct)
3. For general `a ≠ b/2, a > 0`, `JAC(a,b)^e` contributes `e` to both residues `a` and `b-a`. The symmetry check `e[a] == e[b-a]` is correct; `x[a] = e[a]` is correct
4. After the fix, x[a] values can be Frac. Store them as Frac and pass through to JacFactor

**Detection:** Block 13 with period b=14: JAC(7,14) has a=7=b/2, and the prodmake exponent at residue 7 is odd, so x[7] must be 1/2.
**Severity:** CRITICAL — blocks a documented acceptance test
**Phase:** Half-integer Jacobi exponents phase (must fix decomposition BEFORE display/reconstruction)

---

### Pitfall 3: `operator+` Throws on Integer-Different Q-Shifts That Are Mathematically Compatible

**What goes wrong:** `Series::operator+` throws `"cannot add series with different q-shifts"` when q_shifts differ, even when the difference is an integer. For Block 25, `x1 = theta2(q)²/theta2(q³)²` has `q_shift = -1` and `x2 = theta3(q)²/theta3(q³)²` has `q_shift = 0`. Their sum `x1 + x2` is mathematically well-defined (just shift x1's coefficients by +1), but the code throws.

**Why it matters:** The q_shift arithmetic is correct:
- `theta2(q).q_shift = 1/4`, so `theta2(q)².q_shift = 1/2`
- `theta2(q³).q_shift = 3/4`, so `theta2(q³)².q_shift = 3/2`
- `x1.q_shift = 1/2 - 3/2 = -1` (integer!)
- `x2.q_shift = 0 - 0 = 0`
- Difference = 1 → should be addable by index shifting

**Where it is:** `series.h:132-133` — `if (!(q_shift == o.q_shift) && !c.empty() && !o.c.empty()) throw ...`

**Consequences:**
- Block 25 fails (findpoly on theta2/theta3 quotients)
- Exercise 10 fails (same root cause)
- Any identity mixing theta2 (which has q_shift) with theta3/theta4 (which don't) is blocked

**Prevention:**
1. Compute `delta = q_shift - o.q_shift`
2. If `delta` is an integer (i.e., `delta.den == BigInt(1)`), shift coefficients by `int(delta)` to align
3. If `delta` is non-integer, still throw — these series genuinely can't be added term-by-term
4. Choose the smaller q_shift as the result's q_shift
5. Apply same logic to `operator-` (which goes through `operator+` via `*this + (-o)` — already correct)

**Detection:** Any expression like `theta2(q)^n / theta2(q^k)^n + theta3(q)^n / theta3(q^k)^n` will trigger the error.
**Severity:** CRITICAL — blocks two checklist items and a key exercise
**Phase:** Q-shift arithmetic phase (should be Phase 1 — other changes depend on this working)

---

### Pitfall 4: Shifting Coefficients During Addition Can Create Exponents Below `minExp` Expectations

**What goes wrong:** When aligning q_shifts by shifting coefficients, some exponents may move into negative territory or beyond the truncation bound. Downstream code like `prodmake`, `etamake`, and `inverse()` has assumptions about coefficient ranges.

**Concrete example:** Series A has q_shift=-2 with coefficients at {0,1,2,...}. Series B has q_shift=0 with coefficients at {0,1,2,...}. To add them, shift A's coefficients by +2 (to align to q_shift=0). Now A has coefficients at {2,3,4,...}. This is fine. But the reverse: shift B's coefficients by -2, giving B coefficients at {-2,-1,0,1,...}. Now B has negative exponents.

**Where assumptions break:**
- `prodmake` (convert.h:88): requires `b[0] != 0` and normalizes by `b[0]`. If the series has been shifted so the "constant term" is at a negative exponent, prodmake sees `b[0] = 0` and fails
- `Series::inverse()` (series.h:189): handles negative minExp by shifting, but the shift logic assumes the shift brings minExp to 0. If additions introduced unexpected negative exponents, the inverse's internal normalization may be wrong
- `clean()` (series.h:89): erases coefficients with `e >= trunc` but NOT negative exponents — negative-exponent coefficients persist silently
- `etamake` (convert.h:46): iterates `k = 1; while k < T` — negative exponents are invisible to this scan

**Prevention:**
1. When shifting coefficients during addition, always shift the series with the LARGER q_shift downward (i.e., increase its exponents). This way no new negative exponents are introduced
2. Choose `result.q_shift = min(a.q_shift, b.q_shift)` as the base
3. Shift the other series' coefficients by `int(other.q_shift - result.q_shift)` — this is always non-negative, so exponents only increase
4. Add an assertion: after addition, no coefficient should have an exponent below what the input series had

**Detection:** Unit test `(q^(-1) * f) + g` where f and g are simple polynomials — check that result has correct exponents.
**Severity:** CRITICAL — silent coefficient misalignment causes wrong answers in downstream algorithms
**Phase:** Q-shift arithmetic phase (same phase as Pitfall 3)

---

## Major Pitfalls

Mistakes that cause wrong answers or significant usability problems.

---

### Pitfall 5: `jac2series_impl` Needs Formal Power Series Square Root — Non-Trivial Algorithm

**What goes wrong:** To reconstruct a series from JAC(a,b)^(1/2), you need the square root of a formal power series. The naive approach `f.pow(1)` uses integer exponentiation (via binary exponentiation), which can't compute half-integer powers. There's no `Series::sqrt()` method.

**Why it's non-trivial:** The formal power series square root of `1 + a₁q + a₂q² + ...` is computed via Newton iteration:
```
g₀ = 1
gₙ₊₁ = (gₙ + f/gₙ) / 2
```
doubling precision each step. This requires division (already implemented) but the convergence must be tracked carefully — each iteration must be truncated to avoid exponential coefficient growth.

**Alternative:** Use the identity `f^(1/2) = exp(1/2 · log(f))` where log and exp are formal power series operations:
```
log(1 + g) = g - g²/2 + g³/3 - ...
exp(h) = 1 + h + h²/2! + h³/3! + ...
```
This works but requires implementing `Series::log()` and `Series::exp()`.

**Simpler alternative for JacFactor specifically:** For exponent p/2, compute the integer-exponent parts separately. `JAC(a,b)^(3/2) = JAC(a,b) · JAC(a,b)^(1/2)`. Only the ^(1/2) part needs a square root. And JAC(a,b) itself decomposes into `(q^a;q^b)_∞ · (q^{b-a};q^b)_∞ · (q^b;q^b)_∞`, each of which is an Euler-type product. The square root of an Euler product can be computed via prodmake's framework: if `f = Π(1-q^n)^{a_n}`, then `f^(1/2) = Π(1-q^n)^{a_n/2}`, where each factor `(1-q^n)^{a_n/2}` can be expanded term-by-term using the binomial series for `(1-x)^r` with rational r.

**Prevention:**
1. Implement `Series::pow(Frac r)` using Newton square root or the binomial series approach
2. Test with `(etaq(1,T))^(1/2)` — should satisfy `f² = etaq(1,T)` to truncation
3. Handle sign carefully: `(1-q)^(1/2)` has alternating sign coefficients that grow

**Detection:** `jac2series(jp, 100)` where jp contains a 1/2 exponent — compare against direct series computation.
**Severity:** MAJOR — blocking feature, but alternative computation paths exist
**Phase:** Half-integer Jacobi exponents phase (Task 2, after decomposition and display are fixed)

---

### Pitfall 6: `formatJacprodmake` Display Doesn't Handle Fractional Exponents

**What goes wrong:** The `jac2prod` function in `convert.h:434-460` constructs display strings like `(q^a,q^b)_∞^{ex}`. The exponent `ex` is extracted as `int`, so fractional exponents show as nothing (factor omitted) or `^0` (factor present but exponent missing).

**Where it is:** `convert.h:438-439` — the int extraction:
```cpp
int ex = 0;
if (exp.den == BigInt(1) && exp.num.d.size() == 1 && exp.num.d[0] <= 100)
    ex = exp.num.neg ? -static_cast<int>(exp.num.d[0]) : static_cast<int>(exp.num.d[0]);
```

**Prevention:**
1. Replace the int extraction with Frac-aware display
2. For integer exponents: continue using `^n` notation (current behavior)
3. For half-integer exponents: use `^(p/q)` notation, e.g., `(q^14,q^14)_∞^(13/2)`
4. Special case: exponent 1/2 → display as `√(...)` for readability (matching Maple's `√JAC(7,14,∞)`)
5. The `abs_ex` calculation for num/den partitioning must also use Frac comparison (`exp > Frac(0)` vs `exp < Frac(0)`)

**Detection:** Create a JacFactor with Frac(1,2) exponent and call `jac2prod` — currently shows empty or wrong output.
**Severity:** MAJOR — display is wrong but computation would still be possible if other code is fixed
**Phase:** Half-integer Jacobi exponents phase (Task 1: display)

---

### Pitfall 7: Double-Sum Performance for b(q) and N(q) — O(T²) Frac Operations

**What goes wrong:** The REPL's `sum()` evaluator (repl.h:1042-1053) creates a Series for each iteration and calls `operator+`, making nested double sums O(T²) in Series additions, each O(T) in coefficient count, giving O(T³) Frac operations total. For T=200 this means ~10⁸ BigInt multiplications.

**Concrete cases:**
- **b(q) = Σ_m Σ_n ω^(m-n) · q^(m²+mn+n²)**: requires ω = exp(2πi/3) which is irrational. This is fundamentally impossible in exact rational arithmetic — not a performance issue but an algebraic one.
- **N(q) = Σ_{m,n} (−1)^(m+n) (2m+1)(2n+1) · q^((m²+m+n²+n)/2)**: double sum with ~√T iterations per sum. For T=100: ~10×10 = 100 iterations, each creating a Series. Feasible.
- **UE function from Block 17**: `Σ_m Σ_n legendre(m,5)·n⁵·q^(m·n)`. For T=50: outer loop m=1..50, inner n=1..floor(50/m). Total ~230 terms. Feasible if each is a simple q-power addition.

**Why it's slow:** Each sum body evaluation creates a fresh Series, and `acc = (acc + term).truncTo(T)` copies all coefficients. With k iterations, this is O(k·T) work. For a double sum with O(√T) × O(√T) iterations, total is O(T²).

**Prevention:**
1. For double sums that just accumulate `coeff * q^exp` terms, bypass Series arithmetic entirely — accumulate into a single coefficient map directly. This is O(#terms) instead of O(#terms × T).
2. In the REPL evaluator, detect the pattern `sum(sum(c * q^e, ...), ...)` and optimize to direct coefficient accumulation.
3. Alternatively, accept the O(T²) cost for moderate T (≤ 200) and document that T > 500 with double sums may be slow.
4. For b(q) specifically: implement it as a specialized function `bq(T)` that uses the eta product identity `b(q) = η(τ)³/η(3τ)` instead of the double sum definition. This avoids ω entirely.

**Detection:** Time `sum(sum(q^(m*n), n, 1, 100), m, 1, 100)` with set_trunc(200) — should complete in < 5 seconds. If > 30 seconds, optimization is needed.
**Severity:** MAJOR — blocks exercise completion for large T
**Phase:** Double-sum optimization (can be deferred — exercises work at smaller T)

---

### Pitfall 8: Verification Step in `jacprodmake` Fails for Fractional Exponents

**What goes wrong:** After decomposing prodmake exponents into JAC factors, `jacprodmake` reconstructs the series via `jac2series_impl(result, T)` and checks `recon.coeff(n) != f.coeff(n)` for all n. If `jac2series_impl` can't handle fractional exponents (Pitfall 5), the verification fails and jacprodmake returns empty — even though the decomposition was correct.

**Where it is:** `convert.h:420-426`:
```cpp
Series recon = jac2series_impl(result, T);
bool ok = true;
for (int n = 0; n < T && ok; ++n) {
    if (recon.coeff(n) != f.coeff(n)) ok = false;
}
if (ok) return result;
```

**Consequences:** Even if the decomposition algorithm is fixed (Pitfall 2), the verification will reject correct results because reconstruction fails.

**Prevention:**
1. Fix `jac2series_impl` to handle fractional exponents BEFORE or IN PARALLEL with the decomposition fix
2. Alternatively, skip verification when fractional exponents are detected and rely on the decomposition algebra being correct
3. If skipping verification, add a confidence flag to the output so users know the result wasn't verified

**Detection:** Fix Pitfall 2 but not Pitfall 5 → jacprodmake still returns empty for Block 13.
**Severity:** MAJOR — creates a dependency between display/decomposition fix and reconstruction fix
**Phase:** Half-integer Jacobi exponents phase (ordering constraint: reconstruction must work before or with decomposition)

---

## Moderate Pitfalls

Mistakes that cause confusing behavior or partial failures.

---

### Pitfall 9: Q-Shift Propagation Through `subs_q(k)` With Integer-Shifted Series

**What goes wrong:** After allowing integer-different q_shift addition, a series may have a non-zero integer q_shift (e.g., q_shift = -1). When `subs_q(k)` is called, it does `q_shift *= Frac(k)`. For q_shift = -1 and k = 3, the result has q_shift = -3. This is correct, but downstream code that later adds this to a series with q_shift = 0 now has a larger integer gap to bridge, potentially shifting many coefficients.

**Prevention:**
1. After integer-shift addition, normalize the result: absorb the integer q_shift into the coefficient map by shifting all exponents by `int(q_shift)` and setting q_shift to 0
2. This normalization should happen at the end of `operator+`, not deferred
3. Add a method `Series::normalize_q_shift()` that does: for each (e, v) in c, move to (e + int_q_shift, v), then set q_shift to the fractional remainder

**Detection:** Compute `x := theta2(q)^2/theta2(q^3)^2 + theta3(q)^2/theta3(q^3)^2`, then `subs_q(x, 5)` — check that q_shift is handled correctly.
**Severity:** MODERATE — surprising q_shift accumulation, not wrong but confusing
**Phase:** Q-shift arithmetic phase (enhancement after core fix)

---

### Pitfall 10: `etamake` Breaks When Input Has Non-Zero Integer Q-Shift

**What goes wrong:** `etamake` starts with `if (!(f.q_shift == Frac(0))) result.push_back({-1, f.q_shift})` and then normalizes the series. If the input has q_shift = -1 (from an integer-shift addition), etamake records this as a fractional q-power prefix. But q_shift = -1 should be absorbed into the coefficient exponents, not treated as a separate factor.

**Where it is:** `convert.h:30-31`

**Prevention:**
1. At the start of `etamake`, normalize the input: if q_shift is integer, shift coefficients and set q_shift = 0
2. Only record q_shift as a {-1, q_shift} entry when q_shift is genuinely fractional (non-integer)
3. This avoids `etamake` producing spurious `q^(-1)` prefixes

**Detection:** Compute a series with q_shift = -1 via integer-shifted addition, then run `etamake` — should identify eta product correctly, not show `q^(-1) * eta(...)`.
**Severity:** MODERATE — wrong display but underlying identification may still work
**Phase:** Q-shift arithmetic phase (must normalize before passing to etamake)

---

### Pitfall 11: Session Save/Load Doesn't Preserve Integer-Shifted Coefficient Maps

**What goes wrong:** `saveSession` serializes series as `S varname trunc exp1:coeff1 exp2:coeff2 ...` and separately records q_shift. If a series has been integer-shifted (q_shift = 0, but coefficients were moved), the save/load round-trip is fine. But if the series retains a non-zero integer q_shift (because normalization wasn't applied), the q_shift is NOT saved in the current format.

**Where it is:** `repl.h:210-215` — the S line format doesn't include q_shift:
```cpp
f << "S " << varname << " " << s.trunc;
for (const auto& [exp, coeff] : s.c)
    f << " " << exp << ":" << coeff.str();
```

**Consequences:** Series with non-zero q_shift lose their q_shift on save/load. After loading, the series is wrong.

**Prevention:**
1. Either normalize q_shift before saving (convert integer q_shift to coefficient shifts)
2. Or add q_shift to the S line format: `S varname trunc q_shift_num/q_shift_den exp1:coeff1 ...`
3. Option 1 is simpler and avoids format changes

**Detection:** Save a session with a series that has q_shift ≠ 0, load it, compare output.
**Severity:** MODERATE — data loss on save/load
**Phase:** Q-shift arithmetic phase (must fix save format or normalize before save)

---

### Pitfall 12: Binomial Series for `(1-q^n)^r` with Rational r Diverges — Need Truncation Discipline

**What goes wrong:** Implementing `Series::pow(Frac r)` via the binomial series:
```
(1-x)^r = Σ_{k=0}^∞ C(r,k) (-x)^k
```
where `C(r,k) = r(r-1)...(r-k+1)/k!`. For non-integer r, this is an infinite series. Without careful truncation, computing too many terms wastes time, and computing too few gives wrong coefficients.

**Prevention:**
1. The generalized binomial coefficient `C(r,k)` for rational `r = p/q` is `(p/q)(p/q - 1)...(p/q - k + 1) / k!`. Each factor is a Frac operation. For k up to T, this is O(T) Frac multiplications.
2. Truncate at k where the first exponent of `(-x)^k` exceeds T. For x = q^n, this means k > T/n.
3. Watch out: the generalized binomial coefficients grow factorially in denominator but the numerator also grows. For r = 1/2: C(1/2, k) = (-1)^(k+1) / (2^(2k-1) · k) · C(2k-2, k-1). These are rational but denominators grow exponentially. Frac::reduce() handles this but may be slow for large k.
4. The binomial series for (1-q)^(1/2) converges formally (all coefficients are well-defined rationals), so truncation to O(q^T) is mathematically correct.

**Detection:** Compute `(1-q)^(1/2)` to 20 terms. First few coefficients should be: 1, -1/2, -1/8, -1/16, -5/128, -7/256, ...
**Severity:** MODERATE — performance concern, not correctness (if truncated properly)
**Phase:** Half-integer Jacobi exponents phase (part of Series::pow(Frac) implementation)

---

### Pitfall 13: Q-Shift Arithmetic in `operator*` After Integer-Shifted Addition

**What goes wrong:** After two series with integer-different q_shifts are added (Pitfall 3 fix), the result has some q_shift. Multiplying this result with another series adds q_shifts. If the addition result wasn't normalized (q_shift still non-zero integer), the multiplication produces a q_shift that might be unexpected.

**Example:** `x = (f with q_shift=-1) + (g with q_shift=0)` → result has q_shift = -1. Then `x * h` (q_shift=0) → product has q_shift = -1. But if `x` had been normalized (q_shift=0, coefficients shifted), `x * h` would have q_shift = 0. The coefficients differ between these two representations.

**Prevention:**
1. Always normalize integer q_shifts into coefficient exponents immediately after addition
2. This ensures q_shift is always either 0 or genuinely fractional throughout the computation
3. Invariant to maintain: `q_shift.den == BigInt(1) implies q_shift == Frac(0)` (after normalization)

**Detection:** Compute `x := theta2(q)^2/theta2(q^3)^2 + theta3(q)^2/theta3(q^3)^2`, then `x * etaq(1,100)`. Check that output matches the separately computed `(theta2^2/theta2_3^2) * eta + (theta3^2/theta3_3^2) * eta`.
**Severity:** MODERATE — produces correct answer in wrong representation, may confuse downstream
**Phase:** Q-shift arithmetic phase (normalize step)

---

## Minor Pitfalls

Mistakes that cause cosmetic issues or minor confusion.

---

### Pitfall 14: Display of Half-Integer Exponents in `str()` Method

**What goes wrong:** `Series::str()` uses `expToUnicode(int n)` for exponent display, which only handles integers. If coefficients are at integer exponents but q_shift is fractional (e.g., 1/4), the `fracExpStr(Frac)` method handles the prefix. But if a future change introduces non-integer coefficient exponents (e.g., from normalizing a half-integer q_shift), `str()` would crash or produce garbage.

**Prevention:**
1. Keep the invariant that coefficient map keys (`std::map<int, Frac>`) are always integers
2. Non-integer powers belong in q_shift, never in the map keys
3. Document this invariant

**Severity:** MINOR — unlikely to manifest if q_shift normalization is done correctly
**Phase:** Q-shift arithmetic phase (documentation)

---

### Pitfall 15: `clean()` and `truncTo()` Don't Interact With Q-Shift Normalization

**What goes wrong:** `clean()` erases coefficients with `e >= trunc`. After integer-shift normalization (shifting coefficients by delta), some coefficients that were `e < trunc` become `e + delta >= trunc` and should be cleaned. If normalization and clean() happen in different steps, coefficients leak past truncation.

**Prevention:**
1. Call `clean()` after every q_shift normalization
2. Or perform normalization inside `clean()` — if q_shift is integer, shift coefficients and reset q_shift, then erase out-of-bounds

**Severity:** MINOR — only causes extra coefficients beyond truncation, which most operations ignore
**Phase:** Q-shift arithmetic phase (implementation detail)

---

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation | Ref |
|---|---|---|---|
| jacprodmake decomposition | Middle residue (a = b/2) doubles contribution | Divide by 2 for middle element: `x[b/2] = e[b/2] / 2` | #2 |
| jacprodmake decomposition | Verification rejects correct fractional results | Fix jac2series_impl first, or skip verification for fractional | #8 |
| jac2prod display | Fractional exponents silently dropped | Replace int extraction with Frac-aware display | #1, #6 |
| jac2series reconstruction | No Series::pow(Frac) method | Implement via Newton sqrt or binomial series | #5, #12 |
| Series::operator+ q_shift fix | Negative exponents from wrong shift direction | Always shift the larger q_shift downward | #4 |
| Series::operator+ q_shift fix | Non-normalized q_shift propagates | Normalize integer q_shifts immediately after addition | #9, #13 |
| etamake with shifted input | Integer q_shift treated as fractional prefix | Normalize input q_shift before running etamake | #10 |
| Session save/load | q_shift not preserved in file format | Normalize before saving, or extend format | #11 |
| Double sums | O(T²) Frac operations for large T | Use smaller T, or implement coefficient accumulation | #7 |
| b(q) exercise | ω = exp(2πi/3) is irrational | Use eta identity b(q) = η(τ)³/η(3τ) instead | #7 |

---

## Integration Pitfalls

### Ordering Dependency: Decomposition → Reconstruction → Display

The three jacprodmake fixes have a dependency chain:
1. **Decomposition** (Pitfall 2): fix `x[b/2] = e[b/2] / 2` to produce fractional x[a]
2. **Reconstruction** (Pitfall 5): implement `jac2series_impl` for Frac exponents, so verification works
3. **Display** (Pitfall 6): fix `jac2prod` to show Frac exponents

If you fix (1) without (2), the verification step rejects the correct decomposition and jacprodmake returns empty. If you fix (1) and (3) without (2), display works but verification still fails. You must fix (2) before or simultaneously with (1).

### Q-Shift Fix Interacts With Existing Theta Identity Tests

The Block 25 fix (allowing integer-different q_shift addition) changes the behavior of `operator+`. Existing tests that rely on the throw (e.g., accidentally adding incompatible series and getting an error) will now silently produce a result instead. Verify that all existing acceptance tests still pass after the change — especially any test that involves theta2 in combination with theta3/theta4.

### Double-Sum Optimization vs. Generality

A specialized double-sum evaluator would be faster but adds complexity. The simpler approach (accept O(T²) for moderate T) may be sufficient for the exercises, which use T ≤ 200. Don't over-optimize unless profiling shows a real bottleneck.

### Q-Shift Normalization Must Be Idempotent

If normalization is called multiple times (e.g., in nested additions), it must produce the same result. Since normalization absorbs integer q_shift into coefficient indices and sets q_shift to the fractional remainder, calling it again on the result is a no-op (q_shift is already fractional or zero). This is naturally idempotent.

---

## Recommended Task Ordering

Based on dependency analysis:

1. **Q-shift arithmetic fix** (Pitfalls 3, 4, 9, 10, 13) — unblocks Block 25 and Exercise 10. No dependency on other fixes. Test immediately with theta2/theta3 quotient sums.

2. **jac2series_impl fractional power** (Pitfall 5, 12) — implement Series::pow(Frac) via binomial series. Test with `(1-q)^(1/2)`. Required before jacprodmake fix can be verified.

3. **jacprodmake decomposition fix** (Pitfall 2, 8) — fix middle-element doubling. Now verification via jac2series_impl works. Test with Block 13 (Slater 46).

4. **jac2prod display fix** (Pitfalls 1, 6) — display fractional exponents. Cosmetic but needed for user verification.

5. **Double-sum optimization** (Pitfall 7) — defer unless profiling shows need. Exercises work at moderate T.

---

## Sources

- `qseriesdoc.md` Output (15): JAC(0,14,∞)^(13|2) — confirms half-integer Jacobi exponents are expected — HIGH confidence
- `src/convert.h:328-460` — current JacFactor implementation — HIGH confidence (code inspection)
- `src/series.h:131-148` — current operator+ q_shift handling — HIGH confidence (code inspection)
- `src/repl.h:1042-1053` — sum evaluator loop — HIGH confidence (code inspection)
- `maple_checklist.md` Blocks 13, 14, 25 — documented failures — HIGH confidence (verified)
- `exercises_solutions.md` Exercises 4, 6, 10 — documented blockers — HIGH confidence (verified)
- Formal power series square root via Newton iteration — standard algorithm, see Knuth TAOCP Vol. 2 §4.7 — HIGH confidence
- Generalized binomial coefficients for rational exponents — standard combinatorics — HIGH confidence
