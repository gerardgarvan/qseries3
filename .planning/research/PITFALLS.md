# Domain Pitfalls: Q-Series REPL

**Domain:** Arbitrary precision arithmetic, formal power series, q-series / Andrews-style series-to-product conversion  
**Researched:** 2025-02-24  
**Confidence:** HIGH (SPEC + authoritative sources; WebSearch for ecosystem)

---

## Critical Pitfalls

### Pitfall 1: Series Inverse Recurrence Index Error (j=0 vs j=1)

**What goes wrong:**  
Implementing `g[n] = -(1/c₀) Σ c_j · g[n-j]` with `j` from 0 to `n` instead of 1 to `n` produces wrong coefficients. Including `j=0` adds `c₀·g[n]` on the RHS, making the equation circular (you are solving for `g[n]`).

**Why it happens:**  
Off-by-one confusion: the recurrence arises from `(f·g)_n = δ_{n0}`. For n≥1, `Σ_{j=0}^n c_j g_{n-j} = 0`, so `c₀ g_n = -Σ_{j=1}^n c_j g_{n-j}`. Starting the loop at 0 is a natural coding mistake.

**Prevention:**  
- Implement the recurrence as `g[n] = -(1/c₀) Σ_{j=1}^{n} c[j]*g[n-j]` and add an assertion or comment: `// j MUST start at 1 — j=0 would include c₀*g[n] (circular)`.  
- Unit test: `(1-q) * (1/(1-q))` must equal 1 to truncation.  
- Reference: EmergentMind (2025), Math Stack Exchange 710252.

**Warning signs:**  
- `(1-q)^{-1}` produces wrong coefficients beyond the first few terms  
- Rogers-Ramanujan or partition function tests fail despite correct `aqprod`/`etaq`

**Phase to address:** Phase 3 (series.h)

---

### Pitfall 2: BigInt Division — Long Division Edge Cases

**What goes wrong:**  
Quotient or remainder is wrong for: (a) dividend &lt; divisor, (b) divisor with leading zeros, (c) negative operands, (d) base-boundary cases (`1000000000/1`, `999999999/1000000000`).

**Why it happens:**  
Long division in base 10⁹ is the hardest BigInt operation. Common mistakes: wrong iteration-dividend width (should be `n_d+1` digits when divisor has `n_d` digits), poor quotient-digit guess (use binary search over `[0,BASE-1]`), not normalizing out leading zeros, incorrect sign propagation.

**Prevention:**  
- Use binary search for each quotient digit: largest `k` s.t. `k*divisor ≤ remainder`.  
- Ensure `0 ≤ remainder < divisor` after each step.  
- Test: `1000000000/1`, `999999999/1000000000`, `123456789012345/123`, negative cases, `0` dividend.  
- Reference: janmr.com (2014), Knuth TAOCP Vol 2.

**Warning signs:**  
- Frac arithmetic or GCD produces wrong results for “nice” fractions  
- prodmake or etaq fails with odd rationals

**Phase to address:** Phase 1 (bigint.h)

---

### Pitfall 3: Frac Reduction Omitted — Exponential BigInt Growth

**What goes wrong:**  
Numerators and denominators blow up exponentially if `reduce()` is not called after every Frac construction and arithmetic operation. Operations like `a/b + c/d = (ad+bc)/(bd)` produce unreduced fractions; without GCD normalization, subsequent operations compound the size.

**Why it happens:**  
GNU MP Rational docs state: “casting out common factors at each stage is optimal… it’s better to perform several small GCDs than one large GCD later.” Skipping reduction “optimizes” by avoiding GCD, but the cost of one large GCD later far exceeds many small ones.

**Prevention:**  
- Call `reduce()` in every Frac constructor and at the end of every `operator+`, `operator-`, `operator*`, `operator/`.  
- Enforce: `den > 0`, `gcd(|num|, den) = 1`, and `0/1` for zero.  
- Reference: GMP Rational Internals, Python `fractions`.

**Warning signs:**  
- `etaq(1, 100)` or `prodmake` becomes very slow or runs out of memory  
- Coefficient strings grow unexpectedly (e.g. hundreds of digits)

**Phase to address:** Phase 2 (frac.h)

---

### Pitfall 4: Truncation Not Propagated Correctly

**What goes wrong:**  
Result series get `trunc` set incorrectly. Binary ops must use `result.trunc = min(a.trunc, b.trunc)`. If the result keeps the larger truncation, coefficients beyond the valid range are treated as meaningful, producing garbage in downstream computations.

**Why it happens:**  
Truncation is metadata that’s easy to forget. New code paths (e.g. `subs_q`, composition, custom functions) may copy only one operand’s trunc or use a default.

**Prevention:**  
- Centralize rule: for `f op g`, `result.trunc = min(f.trunc, g.trunc)`.  
- For `subs_q(k)`: `result.trunc = f.trunc * k` (exponents scale).  
- For `inverse()`: if `f` starts at exponent `m`, effective trunc is `T - m`.  
- Add a lint/comment checklist for every new Series-returning function.

**Warning signs:**  
- prodmake or relation finding behaves differently with “higher” truncation  
- Terms beyond trunc appear to influence results

**Phase to address:** Phase 3 (series.h), Phase 4 (qfuncs.h)

---

### Pitfall 5: prodmake Divisor Sum Includes d=n

**What goes wrong:**  
Implementing `a[n] = (c[n] - Σ_{d|n} d·a[d]) / n` instead of `Σ_{d|n, d<n}` yields wrong `a[n]`. Including `d=n` puts `n·a[n]` on the RHS, making the equation self-referential.

**Why it happens:**  
The recurrence solves `c[n] = Σ_{d|n} d·a[d]` for `a[n]`. Since `n|n`, we have `c[n] = n·a[n] + Σ_{d|n, d<n} d·a[d]`. Solving for `a[n]` requires excluding `d=n` from the sum.

**Prevention:**  
- Implement: `a[n] = (c[n] - Σ_{d|n, d<n} d·a[d]) / n`.  
- Use `divisors(n)` and filter `d < n` or use a loop that explicitly skips `d == n`.  
- Acceptance test: Rogers-Ramanujan must give denominators only at exponents ≡ ±1 (mod 5).

**Warning signs:**  
- prodmake output disagrees with known product forms  
- Rogers-Ramanujan identity test fails

**Phase to address:** Phase 5 (convert.h — prodmake)

---

### Pitfall 6: Series Inverse with Non-Zero Leading Exponent

**What goes wrong:**  
If `f` starts at exponent `m > 0` (e.g. `f = q^m + …`), naively applying the recurrence to the raw series fails. The recurrence assumes constant term `c₀ ≠ 0`. Applying it to `f` directly or with wrong indexing yields wrong inverse.

**Why it happens:**  
The recurrence `g[n] = -(1/c₀) Σ_{j=1}^{n} c_j g[n-j]` requires `c₀ = f[0] ≠ 0`. For `f = q^m · h(q)` with `h[0] ≠ 0`, you must invert `h` first, then `1/f = q^{-m} · (1/h)`.

**Prevention:**  
- Detect `minExp() > 0`, form `h = f / q^m` (shift exponents by `-m`), invert `h`, then shift result by `-m`.  
- Ensure effective truncation for the shifted series is `T - m`.  
- Test: invert `q * (1 - q)`, `q^2 * (1 + q + q^2)`.

**Warning signs:**  
- Division by series with no constant term crashes or returns nonsense  
- Eta/theta ratios produce wrong coefficients

**Phase to address:** Phase 3 (series.h)

---

### Pitfall 7: jacprodmake Periodicity Detection Too Strict

**What goes wrong:**  
Requiring `e[n+b] == e[n]` for 100% of indices leads to false negatives when the series is a valid Jacobi product but numerical noise or truncation causes minor mismatches.

**Why it happens:**  
Periodicity is a structural property; at finite truncation, boundary effects or rounding can break exact equality. Overfitting to perfect match rejects correct identifications.

**Prevention:**  
- Use ~80% match over a suitable range, then verify by reconstructing the series from the identified JAC factors and comparing to the original.  
- SPEC: “check that e[n+b] == e[n] holds for at least 80% of the test range, then verify.”

**Warning signs:**  
- jacprodmake fails on known Jacobi products (e.g. Rogers-Ramanujan)  
- Output is empty when a plausible period exists

**Phase to address:** Phase 5 (convert.h — jacprodmake)

---

### Pitfall 8: etamake Infinite Loop on Non-Eta Input

**What goes wrong:**  
If the input is not an eta product, the algorithm never converges to 1 and loops forever.

**Why it happens:**  
etamake repeatedly cancels the smallest nonzero exponent by multiplying by the appropriate `etaq` power. For non-eta series, some terms never cancel.

**Prevention:**  
- Enforce a maximum iteration count (e.g. `T` or `2*T`).  
- On exit without convergence, report “not an eta product” instead of looping.  
- SPEC: “Add a maximum iteration count and report failure.”

**Warning signs:**  
- REPL hangs on `etamake` for arbitrary series  
- No timeout or failure message

**Phase to address:** Phase 5 (convert.h — etamake)

---

## Moderate Pitfalls

### Series Multiplication: Skipping Truncation Early

**What goes wrong:**  
Inner loop accumulates `c1*c2` at `e1+e2` even when `e1+e2 >= trunc`. This wastes Frac multiplies and can pollute a dense coefficient map.

**Prevention:**  
Skip pairs where `e1+e2 >= min(trunc_a, trunc_b)` before the multiply. SPEC: “Skip if e1+e2 >= trunc EARLY (inner loop optimization).”

**Phase to address:** Phase 3 (series.h)

---

### prodmake c[n] Recurrence Sum Bounds

**What goes wrong:**  
The recurrence `c[n] = n·b[n] - Σ_{j=1}^{n-1} b[n-j]·c[j]` uses `j` from 1 to `n-1`. Using `j` from 1 to `n` would include `b[0]·c[n]` and make the recurrence circular.

**Prevention:**  
Use `Σ_{j=1}^{n-1}`. Add a comment referencing Andrews’ logarithmic derivative derivation.

**Phase to address:** Phase 5 (convert.h — prodmake)

---

### findhom / findnonhom Matrix Size Blow-Up

**What goes wrong:**  
For `k` series and degree `n`, monomial count is `C(n+k-1, k-1)`. With k=6, n=4 this is 126 monomials; the coefficient matrix grows quickly. Kernel computation can become slow or memory-bound.

**Prevention:**  
Document the scaling. Consider truncation limits or degree caps for interactive use. SPEC: “make sure the kernel computation handles this.”

**Phase to address:** Phase 7 (relations.h)

---

## Minor Pitfalls

### BigInt Sign Handling

**What goes wrong:**  
Zero should be non-negative. Edge cases: `(-a)*(-b)=ab`, `0*anything=0`, division by zero, `abs()` of negative.

**Prevention:**  
Test sign rules explicitly. Normalize zero to `neg=false`.

**Phase to address:** Phase 1 (bigint.h)

---

### subs_q Truncation Update

**What goes wrong:**  
`subs_q(k)` multiplies exponents by `k`. Truncation must scale: `result.trunc = f.trunc * k`, otherwise high exponents are incorrectly included or dropped.

**Prevention:**  
Always set `trunc = f.trunc * k` in `subs_q(k)`.

**Phase to address:** Phase 3 (series.h)

---

## Technical Debt Patterns

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Skip Frac::reduce() in “hot” paths | Fewer GCD calls | Exponential BigInt growth, slowdowns | Never |
| Hardcode truncation instead of min(T₁,T₂) | Simpler code | Wrong results in composed expressions | Never |
| Use j=0 in series inverse sum | “Symmetric” loop | Wrong coefficients, failed tests | Never |
| Relax jacprodmake to 100% match | Simpler logic | Misses valid Jacobi products | Never — use 80% + verify |
| No max iterations in etamake | One less parameter | Infinite loop on bad input | Never |

---

## Performance Traps

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Dense series mult without trunc skip | Slow mult, large maps | Skip pairs with e1+e2 ≥ trunc | T ~ 200+ |
| Unreduced Fracs in series coeffs | Gradual slowdown, huge strings | Always reduce after Frac ops | After 10–20 mults |
| findhom with many series/degree | Long kernel computation | Cap degree or document scaling | k≥4, n≥3 |
| Naive etaq(1,T) as product of (1-q^n) | O(T·√T) for pentagonal | Consider Euler pentagonal theorem (stretch) | T ~ 500+ |

---

## "Looks Done But Isn't" Checklist

- [ ] **Series inverse:** Off-by-one in sum index (j=1..n) — verify with `(1-q)*(1/(1-q))=1`
- [ ] **prodmake:** Divisor sum excludes d=n — verify with Rogers-Ramanujan
- [ ] **Truncation:** All binary ops use `min(T₁,T₂)` — grep for `trunc =`
- [ ] **Frac reduction:** Every constructor and op calls `reduce()` — grep for `reduce`
- [ ] **BigInt division:** Handles 0, negatives, base boundaries — run division test suite
- [ ] **etamake:** Has max iteration and failure reporting — try on non-eta series
- [ ] **jacprodmake:** Uses ~80% match + reconstruction check — verify on Rogers-Ramanujan

---

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Series inverse j=0 bug | LOW | Change loop to j=1..n; re-run `(1-q)*(1/(1-q))` test |
| BigInt division wrong | MEDIUM | Add targeted division tests; fix quotient digit guess and remainder |
| Frac growth | LOW | Add `reduce()` everywhere; may need to refactor Frac ops |
| Truncation drift | LOW | Audit all Series-returning code; enforce `min(trunc)` rule |
| prodmake divisor sum | LOW | Change sum to exclude d==n; re-run Rogers-Ramanujan |
| etamake infinite loop | LOW | Add max iterations and failure path |
| jacprodmake too strict | LOW | Implement 80% threshold and reconstruction check |

---

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Series inverse index (j=1) | Phase 3 (series.h) | `(1-q)*(1/(1-q)) = 1` to truncation |
| BigInt division edge cases | Phase 1 (bigint.h) | Division test suite (SPEC §Common Pitfalls) |
| Frac reduction | Phase 2 (frac.h) | 6/4 → 3/2; 0/5 → 0/1; no growth in long chains |
| Truncation propagation | Phase 3, 4 (series, qfuncs) | prodmake, findhom stable across truncations |
| prodmake divisor sum (d<n) | Phase 5 (convert) | Rogers-Ramanujan → ±1 mod 5 denominators |
| Inverse with minExp>0 | Phase 3 (series.h) | Invert q·(1-q), q²·(1+q) |
| jacprodmake periodicity | Phase 5 (convert) | jacprodmake on Rogers-Ramanujan |
| etamake infinite loop | Phase 5 (convert) | etamake on non-eta series returns failure |
| Series mult trunc skip | Phase 3 (series.h) | Benchmark mult at T=200 |
| findhom matrix size | Phase 7 (relations) | Document/compute monomial count |

---

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation |
|-------------|----------------|------------|
| bigint.h | Division, sign, base boundaries | Test 10⁹/1, 999999999/10⁹, negatives |
| frac.h | Missing reduce, den≤0 | Reduce in every constructor and op |
| series.h | Inverse j=0, trunc propagation, mult skip | Unit tests, checklist |
| qfuncs.h | Truncation in subs_q, products | Propagate trunc in all helpers |
| prodmake | Divisor sum d=n, c[n] sum j=n | Rogers-Ramanujan acceptance test |
| etamake | No max iterations | Cap iterations, report failure |
| jacprodmake | 100% periodicity | 80% match + reconstruction verify |
| relations | Matrix explosion | Document C(n+k-1,k-1), consider caps |

---

## Sources

- SPEC.md — Common Pitfalls and Warnings (project specification)
- .cursorrules — Common Bugs to Avoid
- EmergentMind: Multiplicative Inverse of Power Series (2025) — recurrence c_n = -(1/b₀) Σ_{k=1}^n b_k c_{n-k}
- Math Stack Exchange 710252 — Power series reciprocal recurrence
- janmr.com: Basic Multiple-Precision Long Division (2014) — iteration-dividend width, remainder invariant
- GMP Rational Internals — GCD at each stage optimal; delay causes large GCDs
- Andrews q-series (qseries.org, arXiv) — prodmake algorithm structure
