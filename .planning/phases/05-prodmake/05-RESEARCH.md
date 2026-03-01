# Phase 5: prodmake — Research

**Researched:** 2025-02-25
**Domain:** Andrews' series-to-product algorithm, Rogers-Ramanujan identity
**Confidence:** HIGH (SPEC, qseriesdoc, Garvan reference, PITFALLS)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **Output representation:** Match Garvan prodmake display; return a[n] exponent array for downstream (jacprodmake, etamake)
- **Input handling:** Match Garvan behavior (research edge cases)
- **Non-product:** Warn (don't throw), return best-effort
- **Rogers-Ramanujan:** T default 50, sum terms default n=0..8 (allow higher); verification scope and acceptance threshold — research needed

### Claude's Discretion
- Exact a[n] return type (std::map<int,Frac>, std::map<int,int>, struct)
- Display string format details
- Warning delivery (stderr, callback, etc.)

### Deferred Ideas (OUT OF SCOPE)
- etamake, jacprodmake, qfactor — Phase 6 (convert-extended)
- REPL display formatting — Phase 10
</user_constraints>

---

## Summary

Phase 5 implements Andrews' algorithm for recovering infinite product form from a q-series. Rogers-Ramanujan is the canary test: `Σ q^(n²)/(q;q)_n` must yield denominators only at exponents ≡ ±1 (mod 5). The algorithm is well-specified in SPEC and Andrews/Garvan: extract b[n], compute c[n] via logarithmic-derivative recurrence, extract a[n] via Möbius-like inversion. **Critical:** divisor sum excludes `d=n`; c[n] recurrence uses `j=1..n-1`. Garvan uses series(x,q,50) and prodmake(x,q,40) with sum n=0..8; recommend T=40–50, verify a[n]==0 for n ≢ ±1 (mod 5) in range 1..T-1. Edge cases: b[0]=0 (normalization fails — warn+return empty); b[0]<0 (divide by |b[0]|, track sign); non-integer a[n] (warn). Return `std::map<int,Frac>` for a[n] so jacprodmake can use e[n]=-a[n] and detect non-integer. Place prodmake in `convert.h` for Phase 6 etamake/jacprodmake.

**Primary recommendation:** Implement Andrews' algorithm with locked recurrence/formula; use Rogers-Ramanujan (T=40–50, n=0..8) as acceptance test; warn on edge cases and return best-effort.

---

## Dependencies

| Dependency | Status | Notes |
|------------|--------|-------|
| bigint.h | Done | Phase 1 |
| frac.h | Done | Phase 2 |
| series.h | Done | Phase 3 — coeff, minExp, truncTo, trunc propagation |
| qfuncs.h | Done | Phase 4 — aqprod, etaq, divisors |
| convert.h | **Does not exist** | Phase 5 creates it |

**Required from qfuncs.h:** `divisors(int n)` returns sorted positive divisors. Used in step 3 to compute `Σ_{d|n, d<n} d·a[d]`.

**Required from series.h:** `coeff(int n)`, `minExp()`, `truncTo(int T)`, `trunc` field. prodmake reads b[0..T-1] from the Series.

---

## Andrews Algorithm

**Source:** SPEC §5.1, §Mathematical Reference; qseriesdoc §3.1; Andrews "q-series" section 10.7.

### Steps (locked)

1. **Extract coefficients b[0..T-1]** from f. Normalize so `b[0] = 1` (divide all coeffs by b[0]).

2. **Compute c[n]** for n=1..T-1:
   ```
   c[n] = n·b[n] - Σ_{j=1}^{n-1} b[n-j]·c[j]
   ```
   - **Locked:** j runs 1..n-1, *not* 1..n. Including j=n would add b[0]·c[n] and make the recurrence circular (PITFALLS §prodmake c[n] recurrence).

3. **Extract a[n]** for n=1..T-1:
   ```
   a[n] = (c[n] - Σ_{d|n, d<n} d·a[d]) / n
   ```
   - **Locked:** divisor sum excludes d=n. Including d=n yields `a[n] = (c[n] - n·a[n] - ...)/n` (self-referential) — PITFALLS §prodmake Divisor Sum.

4. **Result:** f = (leading) · Π_{n=1}^{T-1} (1 - q^n)^{-a[n]}.

### Implementation notes

- b[n] = f.coeff(n) for n=0..T-1. Truncate input to T first: `f.truncTo(T)`.
- For each n, loop over divisors of n: `for (int d : divisors(n)) if (d < n) sum += d * a[d];`
- Division by n: use Frac arithmetic; a[n] may be non-integer if input is not product form.

---

## Rogers-Ramanujan Test

**Source:** qseriesdoc §3.1 (Output 2), SPEC Test 1, Garvan prodmake reference.

### Garvan workflow

```
x := add(q^(n^2)/aqprod(q,q,n), n=0..8)
series(x, q, 50)
prodmake(x, q, 40)
```

- **Sum terms:** n=0..8 (9 terms)
- **Series truncation:** 50
- **prodmake truncation T:** 40

### Expected output (denom exponents)

1, 4, 6, 9, 11, 14, 16, 19, 21, 24, 26, 29, 31, 34, 36, 39 — all ≡ ±1 (mod 5).

### Verification recommendation

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| T (prodmake) | 40–50 | Garvan uses 40; CONTEXT allows range |
| Sum terms | n=0..8 default, allow higher | Garvan; higher n improves accuracy |
| Check | a[n] == 0 for n ≢ ±1 (mod 5), n ∈ [1, T-1] | Structural requirement |
| Acceptance | Exact Frac: a[n] integer; for n ≡ ±1 (mod 5), a[n] = 1 | Product form: (1-q^n)^{-1} at those n |

**Threshold:** With exact rational arithmetic, a[n] should be exactly integer for product-form input. No tolerance needed. If Frac yields non-integer a[n], treat as non-product and warn.

---

## Edge Cases

### b[0] = 0 (series has no constant term)

**Andrews assumption:** b[0] = 1 (normalized). If b[0]=0, normalization fails.

**Recommendation:** Detect `b[0].isZero()` (or minExp() > 0). **Warn** (per CONTEXT: don't throw). Return empty `std::map<int,Frac>` or best-effort. Do not divide by zero.

### b[0] < 0

**Recommendation:** Divide by |b[0]|, track sign in leading factor. Normalize to b[0]=1 for recurrence; display can show negative leading if needed.

### Non-integer b[n]

Andrews assumes integer coefficients. With Frac, b[n] can be rational. The recurrence still yields rational c[n], a[n]. **Recommendation:** If any a[n] is non-integer (den != 1), **warn** — input likely not product form. Return raw a[n] anyway.

### Empty / trivial input

f with no coefficients or trunc=0. **Recommendation:** Warn, return empty map.

### T vs input truncation

If f.trunc < T, we only have b[0..f.trunc-1]. **Recommendation:** Use effective T = min(T, f.trunc). Garvan's prodmake agrees with f to O(q^T); if input is shorter, limit accordingly.

---

## Garvan Alignment

### Display format

Garvan prodmake output (qseriesdoc Output 2):
```
1 / ((1-q)(1-q⁴)(1-q⁶)(1-q⁹)(1-q¹¹)...(1-q³⁹))
```

- Numerator: 1 (or leading factor)
- Denominator: product of (1-q^n) for n where -a[n] > 0 (i.e. a[n] > 0)
- Exponents with a[n]=0 are omitted

### Garvan prodmake reference page

`prodmake(f,q,T)` converts q-series f to infinite product agreeing to O(q^T). No explicit edge-case documentation; CONTEXT mandates research and sensible defaults.

### jacprodmake downstream

jacprodmake uses prodmake's a[n]. Product exponents e[n] = -a[n]. For JAC notation, periodicity search needs structured a[n]. **Recommendation:** Return `std::map<int,Frac>` — Frac allows exact rationals and detection of non-integer (den != 1 ⇒ warn).

---

## Pitfalls

### Pitfall 1: Divisor sum includes d=n

**What goes wrong:** Using `Σ_{d|n}` instead of `Σ_{d|n, d<n}` yields wrong a[n].

**Prevention:** `for (int d : divisors(n)) if (d < n) sum += d * a[d];`

**Ref:** PITFALLS §5, .cursorrules §4

### Pitfall 2: c[n] recurrence uses j=n

**What goes wrong:** Including j=n adds b[0]·c[n], making recurrence circular.

**Prevention:** Loop j=1..n-1 only.

**Ref:** PITFALLS §prodmake c[n] Recurrence Sum Bounds

### Pitfall 3: Forgetting Frac::reduce

**What goes wrong:** Intermediate Fracs grow; slowdown, wrong results.

**Prevention:** Frac ops auto-reduce if constructors/ops call reduce(). Ensure no raw Frac build without reduce.

**Ref:** PITFALLS §3

### Pitfall 4: Truncation mismatch

**What goes wrong:** Using f without truncTo(T), or T larger than available coefficients.

**Prevention:** `Series g = f.truncTo(T)`; iterate n=1..T-1; skip if coeffs missing.

---

## File Layout

**Recommendation:** Put prodmake in `convert.h`.

- Phase 6 will add etamake, jacprodmake, qfactor to the same header.
- SPEC file structure lists convert.h for product conversion.
- Single header keeps conversion logic together.

**Signature (recommended):**
```cpp
// convert.h
#include "series.h"
#include "qfuncs.h"

// Returns a[n] for n=1..T-1. Keys are exponents; value is exponent in (1-q^n)^{-a[n]}.
// Warns (stderr) on: b[0]=0, non-integer a[n], etc. Still returns best-effort.
std::map<int, Frac> prodmake(const Series& f, int T);
```

---

## References

### Primary (HIGH confidence)
- SPEC.md — §5.1 prodmake, §Mathematical Reference, §Common Pitfalls
- qseriesdoc.md — §3.1 prodmake, Rogers-Ramanujan example (Output 2)
- Garvan prodmake: https://qseries.org/fgarvan/qmaple/qseries/functions/prodmake.html
- .planning/research/PITFALLS.md — prodmake divisor sum, c[n] recurrence

### Secondary (MEDIUM confidence)
- Andrews, "q-series: their development and application" (CBMS 1986), §10.7
- .cursorrules — Common Bugs to Avoid, Andrews algorithm summary

### Tertiary
- CONTEXT.md — locked decisions, Rogers-Ramanujan parameters

---

## Metadata

**Confidence breakdown:**
- Andrews algorithm: HIGH — SPEC, qseriesdoc, PITFALLS agree; formulas locked
- Rogers-Ramanujan test: HIGH — Garvan example explicit; verification criteria clear
- Edge cases: MEDIUM — Garvan docs sparse; CONTEXT provides policy (warn, best-effort)
- File layout: HIGH — SPEC specifies convert.h; Phase 6 adds to same file

**Research date:** 2025-02-25
**Valid until:** 30 days (algorithm stable)

---

## RESEARCH COMPLETE
