# Phase 107: RR Identity Search — Research

**Researched:** 2025-03-06  
**Domain:** Rogers-Ramanujan / Ramanujan-Robins-Seltberg identities (findids types 3–10)  
**Confidence:** HIGH  

## Summary

Phase 107 extends `findids(type, T)` from types 1–2 to types 3–10. The reference is the Maple package `ramarobinsids` (gaps/ramarobinsids.txt). Types 1–2 use SUM forms (e.g. G(p)H(a)±G(a)H(p)); types 3 and 10 use QUOTIENT forms (num/den). Types 4–9 use SUM forms with GM/HM (Göllnitz-Gordon G*/H*) or special shapes.

**Primary recommendation:** Implement types **4** and **5** first for "at least 2 discoveries." They mirror types 1 and 2, use RRGstar/RRHstar, and return the same 3-tuple format `(a,p,c1)`. Extend `checkid` to accept negative `minExp` (Laurent series) for quotient types 3 and 10.

---

## User Constraints (from CONTEXT.md)

No CONTEXT.md exists. Research unconstrained.

---

## Types 3–10 Specification (from ramarobinsids.txt)

### Type 3 — QUOTIENT

| Field | Value |
|-------|-------|
| **SYMF** | (G(a1)G(p1) ± H(a1)H(p1)) / (G(a2)H(p2) ± H(a2)G(p2)) |
| **Parameters** | a1, p1, c1, a2, p2, c2 |
| **Constraints** | a1≤p1, a2>p2, gcd(a1,a2,p1,p2)=1, [a2/gcd(a2,p2), p2/gcd(a2,p2), c2] ∉ type1 |
| **COND1** | GE(a1)+GE(p1)−HE(a1)−HE(p1) ∈ ℤ (type-2 condition) |
| **COND2** | GE(a2)+HE(p2)−HE(a2)−GE(p2) ∈ ℤ (type-1 condition) |
| **Return tuple** | [a1, p1, c1, a2, p2, c2] (6-tuple) |
| **Loop** | n=2..T, p1,p2 ∈ divisors(n), a1=n/p1, a2=n/p2, c1,c2 ∈ {−1,1} |

### Type 4 — SUM (GM, HM)

| Field | Value |
|-------|-------|
| **SYMF** | GM(p)HM(a) ± GM(a)HM(p) |
| **Parameters** | p, a, c1 |
| **Constraints** | a≠p, a≤p, gcd(a,p)=1, (a mod 2 = 0) ∨ (p mod 2 = 0) |
| **ABCOND** | GE(p)+HE(a)−GE(a)−HE(p) ∈ ℤ (same as type 1) |
| **Return tuple** | [p, a, c1] (3-tuple) |
| **Loop** | n=2..T, p ∈ divisors(n), a=n/p, c1 ∈ {−1,1} |

### Type 5 — SUM (GM, HM)

| Field | Value |
|-------|-------|
| **SYMF** | GM(a)GM(p) ± HM(a)HM(p) |
| **Parameters** | a, p, c1 |
| **Constraints** | a≤p, gcd(a,p)=1, (a mod 2 = 0) ∨ (p mod 2 = 0) |
| **ABCOND** | GE(a)+GE(p)−HE(a)−HE(p) ∈ ℤ (same as type 2) |
| **Return tuple** | [a, p, c1] (3-tuple) |
| **Loop** | n=2..T, p ∈ divisors(n), a=n/p, c1 ∈ {−1,1} |

### Type 6 — SUM (G, HM, GM, H)

| Field | Value |
|-------|-------|
| **SYMF** | G(a)HM(p) ± GM(a)H(p) |
| **Parameters** | a, p, c1 |
| **Constraints** | a≥p, gcd(a,p)=1 |
| **ABCOND** | GE(a)+HE(p)−GE(a)−HE(p) = 0 (always integer) |
| **Return tuple** | [a, p, c1] (3-tuple) |
| **Loop** | n=1..T, p ∈ divisors(n), a=n/p, c1 ∈ {−1,1} |
| **Note** | Maple uses ACC=24 (higher than 10) for CHECKRAMIDF |

### Type 7 — SUM (GM, G, HM, H)

| Field | Value |
|-------|-------|
| **SYMF** | GM(a)G(p) ± HM(a)H(p) |
| **Parameters** | a, p, c1 |
| **Constraints** | a≤p, gcd(a,p)=1, a odd AND p odd |
| **ABCOND** | GE(a)+GE(p)−HE(a)−HE(p) ∈ ℤ (same as type 2) |
| **Return tuple** | [a, p, c1] (3-tuple; Maple IDLIST uses [p,a,c1]) |
| **Loop** | n=1..T, p ∈ divisors(n), a=n/p, c1 ∈ {−1,1} |

### Type 8 — SUM (special)

| Field | Value |
|-------|-------|
| **SYMF** | G(1)^p H(p) ± H(1)^p G(p) |
| **Parameters** | p, c1 (a=1 implicit) |
| **Constraints** | p≠1, p=2..T |
| **ABCOND** | 1 (always) |
| **Return tuple** | [p, c1] (2-tuple) |
| **Loop** | n=2..T, p=n, c1 ∈ {−1,1} |

### Type 9 — SUM (special, parametric)

| Field | Value |
|-------|-------|
| **SYMF** | G(1)^a H(1)^b − H(1)^a G(1)^b or G(1)^a H(1)^b − H(1)^a G(1)^b − 1 |
| **Parameters** | a, b, x (x=0 or 1) |
| **Constraints** | GE(a)+HE(b)=0 (solved via isolve; Maple uses _Z1=1) |
| **Return tuple** | [a, b, x] (3-tuple) |
| **Loop** | NOT a simple sweep — solves Diophantine GE(a)+HE(b)=0 |

### Type 10 — QUOTIENT

| Field | Value |
|-------|-------|
| **SYMF** | (G(a1)H(p1) ± H(a1)G(p1)) / (G(a2)HM(p2) ± H(a2)GM(p2)) |
| **Parameters** | a1, p1, c1, a2, p2, c2 |
| **Constraints** | a1>p1, a2>p2, gcd(a1,a2,p1,p2)=1, [a1/gcd(a1,p1), p1/gcd(a1,p1), c1] ∉ type1 |
| **COND1** | GE(a1)+HE(p1)−HE(a1)−GE(p1) ∈ ℤ |
| **COND2** | GE(a2)+HE(p2)−HE(a2)−GE(p2) ∈ ℤ |
| **Return tuple** | [a1, p1, c1, a2, p2, c2] (6-tuple) |
| **Loop** | n=2..T, p1,p2 ∈ divisors(n), a1=n/p1, a2=n/p2, c1,c2 ∈ {−1,1} |

---

## Standard Stack

| Component | Source | Purpose |
|-----------|--------|---------|
| RRG, RRH | rr_ids.h | Rogers-Ramanujan G, H |
| RRGstar, RRHstar | rr_ids.h | Göllnitz-Gordon G*, H* (GM, HM in Maple) |
| GE, HE | rr_ids.h | Ramanujan-Robins modulus-5 formulas |
| divisors | qfuncs.h | Positive divisors |
| addSeriesAligned | rr_ids.h | Add series with different q_shifts |
| checkid | rr_ids.h | CHECKRAMIDF: prodmake + etamake verification |
| prodmake, etamake | convert.h | Series→product, product→eta |

---

## Architecture Patterns

### findids dispatch

```cpp
std::vector<FindidVariant> findids(int type, int T, int acc = 10) {
    switch (type) {
        case 1: return findids_type1(T, acc);
        case 2: return findids_type2(T, acc);
        case 3: return findids_type3(T, acc);
        // ...
    }
    return {};
}
```

### Return format: variable-length tuples

Types 1,2,4,5,6,7 use 3-tuple; type 8 uses 2-tuple; types 3,10 use 6-tuple; type 9 uses 3-tuple `(a,b,x)`.

**Option A:** `std::vector<int>` for variable-length representation; REPL prints `[a,p,c1]` or `[a1,p1,c1,a2,p2,c2]` etc.  
**Option B:** Tagged union / variant; keeps type-safe representation.

**Recommendation:** Use `std::vector<int>` for simplicity; REPL and tests can format per type.

---

## Checkid for Quotient Forms

**Issue:** Quotient forms (types 3, 10) can yield series with negative `minExp` (Laurent). Current `checkid` rejects `ldq < 0`:

```cpp
int ldq = f.minExp();
if (ldq < 0) return out;  // ← rejects Laurent
```

Maple CHECKRAMIDF uses `S2 = S/q^LDQ` and allows negative LDQ. Same pipeline (prodmake, etamake) works after normalizing by `q^LDQ`.

**Fix:** Remove the `ldq < 0` rejection. Use `g = f.truncTo(T)` and `h = g / (c0 * q_var.pow(ldq))` for any integer `ldq`. `Series::pow` supports negative exponents (via `inverse()`). Ensure `h` has `minExp==0` and `coeff(0)==1` before prodmake/etamake.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead |
|---------|-------------|-------------|
| Diophantine GE(a)+HE(b)=0 | Custom solver | Type 9: hardcode known (a,b) or defer |
| RRGstar(n), RRHstar(n) for n>1 | Ad-hoc formulas | Get Geta-style formulas for GG modulus (e.g. 8) or restrict search to n=1 |
| Laurent series normalization | Custom logic | Existing `Series::operator/` and `q.pow(k)` for negative k |

---

## Common Pitfalls

### Pitfall 1: RRGstar/RRHstar for n>1
**What goes wrong:** Types 4–7 use GM(p), HM(a), etc. with p,a > 1. Current `RRGstar(n)` and `RRHstar(n)` return `RRGstar1(T)` for n>1.  
**Why:** n>1 was deferred in phase 86.  
**How to avoid:** Either extend RRGstar/RRHstar with Geta-style formulas for Göllnitz-Gordon, or constrain the search so that only n=1 is used (narrower discovery).

### Pitfall 2: Tuple format mismatch
**What goes wrong:** Types 3 and 10 return 6-tuples; type 8 returns 2-tuple. Using `FindidTuple = std::tuple<int,int,int>` breaks.  
**How to avoid:** Use `std::vector<int>` or a variant; REPL formats per type.

### Pitfall 3: myramtype1 exclusion
**What goes wrong:** Type 3 excludes `[a2/dd2, p2/dd2, c2]` if it lies in type-1 identities; type 10 excludes `[a1/dd1, p1/dd1, c1]` similarly. Without this filter, duplicates or degenerate cases appear.  
**How to avoid:** Run `findids_type1(T, acc)` first, store results in a set, and filter in type 3/10 loops.

### Pitfall 4: Type 9 isolve
**What goes wrong:** Type 9 uses `isolve(GE(a)+HE(b)=0)` — a parametric solver, not a simple loop.  
**How to avoid:** Defer type 9 or use a small hardcoded table of (a,b) satisfying GE(a)+HE(b)=0.

---

## Recommendation: Which 2+ Types First

For "findids discovers identities for at least 2 of types 3–10":

1. **Type 4** — SUM, 3-tuple, mirrors type 1 with GM/HM. Easiest; requires RRGstar/RRHstar for n>1 (or restrict to n where both =1).
2. **Type 5** — SUM, 3-tuple, mirrors type 2 with GM/HM. Same dependencies as type 4.
3. **Type 6** — SUM, 3-tuple, uses G, H, GM, HM; constraint a≥p; always passes ABCOND.
4. **Type 7** — SUM, 3-tuple, both odd; same structure as type 5/2.

**First-choice pair: Types 4 and 5.** They reuse the type-1/type-2 structure, same 3-tuple, and RRGstar/RRHstar are present. Main gap: RRGstar(n) and RRHstar(n) for n>1. If n>1 is not yet implemented, types 4 and 5 may still find identities when a,p=1 in allowed configurations (type 4 needs a≠p so both 1 is excluded; type 5 allows a=p=1). Verify constraints: type 4 has (a mod 2 = 0 or p mod 2 = 0), so (1,1) is excluded. Smallest case for type 4 is (p,a)=(2,1) — needs RRGstar(2), RRHstar(1). Type 5: (1,1) gives GM(1)GM(1)±HM(1)HM(1), needs RRGstar(1), RRHstar(1) — both exist.

**Conclusion:** Implement **types 4 and 5** first. If RRGstar(n), RRHstar(n) for n>1 are missing, type 5 with (a,p)=(1,1) still yields a valid identity. Type 4 requires at least one of p,a > 1, so RRGstar(2) or RRHstar(2) may be needed for early hits. Plan should include extending RRGstar/RRHstar for n>1 (Geta for modulus 8 or equivalent) as a prerequisite or parallel task.

---

## Code Examples

### Type 5 (GM(a)GM(p) ± HM(a)HM(p))
```cpp
// Same structure as findids_type2, use RRGstar/RRHstar
Series t1 = (RRGstar(a, T) * RRGstar(p, T)).truncTo(T);
Series t2 = (RRHstar(a, T) * RRHstar(p, T)).truncTo(T);
Series symf = addSeriesAligned(t1, t2, c1, T);
```

### Type 4 (GM(p)HM(a) ± GM(a)HM(p))
```cpp
Series t1 = (RRGstar(p, T) * RRHstar(a, T)).truncTo(T);
Series t2 = (RRGstar(a, T) * RRHstar(p, T)).truncTo(T);
Series symf = addSeriesAligned(t1, t2, c1, T);
```

### Type 3 quotient (numerator / denominator)
```cpp
Series num = addSeriesAligned(
    (RRG(a1,T)*RRG(p1,T)).truncTo(T),
    (RRH(a1,T)*RRH(p1,T)).truncTo(T), c1, T);
Series den = addSeriesAligned(
    (RRG(a2,T)*RRH(p2,T)).truncTo(T),
    (RRH(a2,T)*RRG(p2,T)).truncTo(T), c2, T);
Series symf = (num / den).truncTo(T);
```

---

## Acceptance Test (acceptance-rr-id.sh)

The script currently checks:
- prodmake(RRG(1)), prodmake(RRH(1))
- RRGstar(1), RRHstar(1)
- geta(1,5,2,50)
- help(RRG)
- checkid(RRG(1)*RRH(1), 50)
- checkid(1+2*q, 50) → "not an eta product"
- findids(1, 20), findids(2, 20)

Phase 107 should extend acceptance-rr-id.sh to:
- `findids(3, T)` through `findids(10, T)` where applicable
- At least 2 of types 3–10 discovering ≥1 identity each (or document which types are implemented and expected counts)

---

## Sources

### Primary (HIGH confidence)
- gaps/ramarobinsids.txt — Maple findtype3..findtype10, CHECKRAMIDF, latexprinttype*
- src/rr_ids.h — findids_type1/2, checkid, RRG, RRH, RRGstar, RRHstar, addSeriesAligned

### Secondary
- tests/acceptance-rr-id.sh — current acceptance tests

---

## Metadata

**Confidence breakdown:**
- Type specs: HIGH — direct extraction from ramarobinsids.txt
- checkid quotient: HIGH — code inspection of rr_ids.h and series.h
- RRGstar/RRHstar n>1: MEDIUM — n>1 deferred; Geta for GG needs verification

**Research date:** 2025-03-06  
**Valid until:** 30 days
