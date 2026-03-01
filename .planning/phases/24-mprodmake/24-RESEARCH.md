# Phase 24: mprodmake — Research

**Researched:** 2026-02-25  
**Domain:** q-series → product conversion (1+q^n1)(1+q^n2)...  
**Confidence:** HIGH

## Summary

`mprodmake` converts a q-series f into an infinite product of the form (1+q^n1)(1+q^n2)... that agrees with f to O(q^T). It is a Maple qseries function by Frank Garvan. The implementation can reuse `prodmake` and a consistency check on the exponent pattern.

---

## Maple Reference

**Source:** [qseries.org/fgarvan/qmaple/qseries/functions/mprodmake.html](https://qseries.org/fgarvan/qmaple/qseries/functions/mprodmake.html)

| Item | Detail |
|------|--------|
| **Calling sequence** | mprodmake(f, q, T) |
| **Parameters** | f — q-series; T — positive integer |
| **Output** | Product (1+q^n1)(1+q^n2)... or failure |

**Example (Maple):**
```
EP := etaq(q,2,100)^2/etaq(q,4,100)/etaq(q,1,100):
mprodmake(EP,q,17);
```
Output: `(1+q)(1+q³)(1+q⁵)(1+q⁷)(1+q⁹)(1+q¹¹)(1+q¹³)(1+q¹⁵)(1+q¹⁷)`

So `etaq(2)^2 / (etaq(4)*etaq(1))` equals ∏(1+q^n) for odd n. This is the canonical acceptance test.

---

## Mathematical Structure

**Relation:** (1+q^n) = (1-q^{2n})/(1-q^n).

Thus f = ∏_{n∈S} (1+q^n) = ∏_{n∈S} (1-q^{2n})/(1-q^n).

In prodmake form f = leading · ∏_j (1-q^j)^{-a[j]}:
- Factor (1-q^{2n})/(1-q^n) contributes: (1-q^n)^{-1} and (1-q^{2n})^1
- So a[n]=1 and a[2n]=-1 for each n in S.

Overlapping factors (e.g. (1+q)(1+q²)):
- (1+q): a[1]=1, a[2]=-1
- (1+q²): a[2]=1, a[4]=-1
- Combined: a[1]=1, a[2]=0, a[4]=-1

Recovery of S from a[]:
- S = {n : a[n]=1} ∪ {n : a[2n]=-1}
- For (1+q)(1+q²): S = {1} ∪ {2} = {1,2}. ✓
- For (1+q)(1+q³)(1+q⁵)...: S = {1,3,5,...}, a[odd]=1, a[2*odd]=-1. ✓

---

## Algorithm

1. Normalize: if f has minExp>0 or lead≠1, divide by leading and track q-power (mprodmake output typically has no q-power; if present, fail or report).
2. Run `prodmake(f, T)` → a[n].
3. Check: all a[n] ∈ {-1, 0, 1} and integer; if not, fail with "not an m-product".
4. Compute S = {n : a[n]=1} ∪ {n : a[2n]=-1}.
5. Consistency: for each n in S, (1+q^n) implies a[n]=1 and a[2n]=-1. So:
   - if n in S and 2n < T, require a[2n] = -1;
   - if a[2n]=-1, n must be in S (automatically satisfied by S definition).
   - Remove redundancy: S is already minimal; no duplicates.
6. Sort S and return as product (1+q^n) for n in S.

**Edge cases:**
- f constant 1 → S = ∅, output "1".
- f has no constant term or b[0]=0 → fail (same as prodmake).
- Non-integer or a[n] ∉ {-1,0,1} → fail.

---

## Implementation Location

| Component | File | Notes |
|-----------|------|-------|
| mprodmake(f,T) | convert.h | New function; calls prodmake |
| formatMprodmake / display | repl.h | New formatter or extend existing display |
| REPL dispatch | repl.h | Add "mprodmake" built-in |

---

## API Design

**Recommendation:** `mprodmake(f, T)` — two arguments (f and T); q is implicit (the series variable).

**Return type options:**
- `std::vector<int>` — sorted list of exponents n in S; empty = failure or constant 1
- `std::optional<std::vector<int>>` — nullopt on failure
- Custom struct with success flag + exponents

**Recommendation:** Return `std::vector<int>`; empty vector means failure (or constant 1). Emit "not an m-product" to stderr on failure so caller can distinguish. For constant 1, return {} and optionally a separate "is_one" hint, or treat {} as "1" (no factors) — matches Maple output "1" for constant 1.

---

## Display Format

**Maple style:** `(1 + q)(1 + q³)(1 + q⁵)...`

- Match prodmake conventions: use `(1+q)` for n=1, `(1+qⁿ)` for n>1.
- Use `Series::expToUnicode` for exponents.
- Truncate with "..." when many factors (e.g. >20); or show all up to T (Maple example shows all up to 17).

**Recommendation:** Show all factors with exponent < T; no truncation with "..." for typical T (≤100). If S is very large, consider a cap (e.g. 50 factors) and "..." — defer to implementation phase.

---

## Dependencies

| Dependency | Location | Purpose |
|------------|----------|---------|
| prodmake(f,T) | convert.h | Andrews algorithm; get a[n] |
| Series | series.h | truncTo, coeff, minExp |
| divisors(n) | qfuncs.h | Used by prodmake internally |
| formatProdmake-style | repl.h | Pattern for formatMprodmake |

No new headers. No subs_q or f(-q) needed; prodmake on f directly suffices.

---

## Acceptance Test

```
ep := etaq(2,100)^2 / etaq(4,100) / etaq(1,100)
mprodmake(ep, 17)
```
Expected: `(1+q)(1+q³)(1+q⁵)(1+q⁷)(1+q⁹)(1+q¹¹)(1+q¹³)(1+q¹⁵)(1+q¹⁷)`

(Adjust syntax to match REPL: `etaq(2,100)` etc.)

---

## Design Decisions (for CONTEXT)

| Decision | Recommendation | Rationale |
|----------|----------------|-----------|
| API | mprodmake(f, T) | Matches prodmake; q implicit |
| Return | std::vector<int> | Simple; empty = failure or 1 |
| Failure | stderr + return {} | Consistent with prodmake, etamake |
| Display | (1+q)(1+q³)... | Match Maple; expToUnicode |
| Constant 1 | return {} → display "1" | Natural |

---

## SEE ALSO

prodmake, etamake, jacprodmake — all in convert.h / qseries package.
