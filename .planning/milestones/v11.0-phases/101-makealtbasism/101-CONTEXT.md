# Phase 101: makeALTbasisM — Context

**Phase:** 101  
**Goal:** User can call `makeALTbasisM(k, T)` — returns basis of M_k(SL_2(Z)) using Delta_12 instead of E4/E6  
**Research source:** `101-RESEARCH.md` (Frank Garvan Maple source in `gaps/wprogmodforms.txt`)

---

## Decisions (Research-Backed)

### 1. Basis structure
- **Formula:** Es * E6^(2r−2i) * DELTA12^i for i = 0, 1, …, r  
- **k = 12r + s**, s = k mod 12 ∈ {0, 2, 4, 6, 8, 10}
- **Es table:**

| s | Es (weight s) |
|---|---------------|
| 0 | 1             |
| 2 | E14           |
| 4 | E4            |
| 6 | E6            |
| 8 | E8            |
| 10| E10           |

### 2. Weight range
- **Valid:** k ≥ 0, even (including 0, 4, 6, 8, 10, 12, 14, …)
- **k = 2:** Return empty vector (dim M_2 = 0)
- **k < 0 or k odd:** Throw runtime_error, same message as `makebasisM`

### 3. Output and API
- **Return type:** `std::vector<Series>` (same as `makebasisM`)
- **Ordering:** i ascending (0 to r) — first element Eisenstein-like, last most cuspidal
- **Truncation:** Each series truncated to T; compute intermediates with extra precision (T+5 or T+10), then truncate final result

### 4. Error handling
- Reuse `makebasisM` style: `if (k < 0 || k % 2 != 0) throw std::runtime_error("makeALTbasisM: k must be nonnegative even integer");`
- k = 2: r = (2−14)/12 = −1 → loop `i = 0..r` runs 0 times → return empty vector

### 5. Building blocks (existing)
- `DELTA12(T)` from modforms.h
- `EISENq(d, T)` from qfuncs.h for d ∈ {4, 6, 8, 10, 14}

---

## Implementation sketch

```
makeALTbasisM(k, T):
  if k < 0 or k odd: throw
  s = k % 12
  r = (k - s) / 12   if s != 2
  r = (k - 14) / 12  if s == 2   # Es = E14 has weight 14
  Es = lookup(s)     # 1, E4, E6, E8, E10, E14
  E6 = EISENq(6, T+5), D = DELTA12(T+5)
  for i = 0 to r:
    push (Es * E6^(2r-2i) * D^i).truncTo(T)
  return FL
```

---

## Validation
- Same dimension as `makebasisM(k, T)`
- Span same space (optional: check linear independence via kernel of change-of-basis)
- Example: makeALTbasisM(12, 100) → [E6², DELTA12]
