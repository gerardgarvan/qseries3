# Phase 101: makeALTbasisM — Research

**Researched:** 2025-03-05  
**Domain:** Modular forms M_k(SL_2(Z)), alternative Delta-based basis  
**Confidence:** HIGH

## Summary

Frank Garvan's Maple modforms package (in `gaps/wprogmodforms.txt`) already implements `makeALTbasisM`. The basis uses **Es * E6^(2r-2i) * DELTA12^i** for i = 0..r, where k = 12r + s and Es is the Eisenstein series of weight s. This matches the mathematical decomposition M_k = Eisenstein part ⊕ cusp part (Delta * M_{k-12}). The C++ codebase has DELTA12, EISENq, and makebasisM; implementing makeALTbasisM by porting the Maple logic is straightforward.

**Primary recommendation:** Port Garvan's makeALTbasisM algorithm from Maple to C++. Use Es (E4, E6, E8, E10, E14, or 1) and E6, DELTA12 as building blocks. Validate output against makebasisM (same span) and Maple examples.

## User Constraints

*No CONTEXT.md exists for this phase. Research is unconstrained.*

## Standard Stack

### Core (existing in codebase)
| Component | Location | Purpose |
|-----------|----------|---------|
| DELTA12 | modforms.h | q * eta(q)^24, cusp form of weight 12 |
| EISENq(d,T) | qfuncs.h | Eisenstein E_d for d ∈ {2,4,6,8,10,14,...} |
| makebasisM | modforms.h | E4^a * E6^b with 4a+6b=k |

### Dependencies
- EISENq supports d ≤ 20 (bernoulli_even table). E14 = EISENq(14,T) is supported.

## Basis Structure (from Maple + Math)

### Formula

For k nonnegative even, write **k = 12r + s** where **s = k mod 12** ∈ {0, 2, 4, 6, 8, 10}:

| s | Es (weight s) |
|---|----------------|
| 0 | 1 |
| 2 | E14 |
| 4 | E4 |
| 6 | E6 |
| 8 | E8 |
| 10 | E10 |

**Basis elements:** Es * E6^(2r-2i) * DELTA12^i for **i = 0, 1, ..., r**.

**Weight check:** s + 6(2r-2i) + 12i = s + 12r = k ✓

### Ordering

Elements are listed with **i ascending** (0 to r). First element has no Delta (Eisenstein-like); last has Delta^r (most cuspidal).

### Dimension

- dim M_k = floor(k/12) + 1 if k ≢ 2 (mod 12)
- dim M_k = floor(k/12) if k ≡ 2 (mod 12)
- M_2 = 0

The formula yields r+1 elements except for k≡2: s=2, r=(k-14)/12 = floor(k/12)-1, so r+1 = floor(k/12) ✓

## Weight Range and Error Handling

| k | s | r | Basis size |
|---|---|---|------------|
| 0 | 0 | 0 | 1 |
| 2 | 2 | (2-14)/12 = -1 | 0 |
| 4 | 4 | 0 | 1 |
| 6 | 6 | 0 | 1 |
| 8 | 8 | 0 | 1 |
| 10 | 10 | 0 | 1 |
| 12 | 0 | 1 | 2 |
| 14 | 2 | 0 | 1 |

- **k < 0 or odd:** Error (same as makebasisM).
- **k = 2:** Return empty vector (dim M_2 = 0).
- **k ≥ 0 even, k ≠ 2:** Valid; no fallback to E4/E6 needed.

## Maple Reference (Garvan)

Source: `gaps/wprogmodforms.txt` (Maple modforms package, Jan 2021)

```maple
modforms[makeALTbasisM]:=proc(k,T)
   # basis elements: Es*E6^(2*r-2*i)*DELTA12^i i=0..r, where k=12*r+s
   s12:=modp(k,12):
   if s12=0 then Es:=1: r:=k/12: fi:
   if s12=2 then Es:=locE14: r:=(k-14)/12: fi:
   if s12=4 then Es:=locE4: r:=(k-4)/12: fi:
   if s12=6 then Es:=locE6: r:=(k-6)/12: fi:
   if s12=8 then Es:=locE8: r:=(k-8)/12: fi:
   if s12=10 then Es:=locE10: r:=(k-10)/12: fi:
   for i from 0 to r do
       func:=series(Es*locE6^(2*r-2*i)*locDELTA12^i,q,T+1):
       FL:=[op(FL),func]:
   od:
   RETURN(FL):
end:
```

**Maple example (from mfhelp):**
- makeALTbasisM(12,100) → [E6^2, DELTA12]
- B12[1] = E6^2, B12[2] = DELTA12 = eta^24

## Implementation Checklist

1. Reuse existing `DELTA12(T)` and `EISENq(d,T)`.
2. Compute Es and r from k (s = k mod 12, then table lookup).
3. Loop i = 0..r, push Es * E6^(2r-2i) * DELTA12^i (truncated to T).
4. Error if k < 0 or k odd; for k=2 return empty vector.
5. Register `makeALTbasisM` in REPL (similar to makebasisM).

## Common Pitfalls

### Pitfall 1: k=2 edge case
- **What goes wrong:** r = (2-14)/12 = -1. Loop i=0..-1 must yield no iterations.
- **How to avoid:** In C++, use `for (int i = 0; i <= r; ++i)` — when r=-1 this runs 0 times. Ensure r is computed as integer division.

### Pitfall 2: Truncation
- Compute E4,E6,E8,E10,E14,DELTA12 with extra precision (e.g. T+10) before products, then truncate final series to T. Match makebasisM style.

### Pitfall 3: E14 availability
- EISENq(14,T) requires bernoulli_even(14); the codebase supports up to B_20, so k=14 is fine.

## Uniqueness

The basis is **canonical** given the choice of Es and the monomial form. Representation of an arbitrary f ∈ M_k as a linear combination of these elements is unique. The ordering (by Delta exponent i) is conventional and matches Garvan's Maple.

## Sources

### Primary (HIGH confidence)
- `gaps/wprogmodforms.txt` — Maple makeALTbasisM source and mfhelp examples
- `src/modforms.h` — DELTA12, makebasisM
- `src/qfuncs.h` — EISENq, eisenstein

### Secondary (MEDIUM confidence)
- Wikipedia: Ring of modular forms (C[E4,E6])
- Math Stack Exchange: Basis of M_12 (E4³, Δ) and (E6², Δ)
- Stein: Dimension formulas for M_k(Γ(1))

### Tertiary (LOW confidence)
- General web search on modular form basis structure

## Metadata

**Confidence breakdown:**
- Basis formula: HIGH — from Maple source and dimension check
- Weight range: HIGH — dimension formula and Maple handling of k=2
- Ordering: HIGH — explicit in Maple

**Research date:** 2025-03-05  
**Valid until:** Stable (modular forms theory)
