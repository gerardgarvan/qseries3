---
phase: 38-math-enrichment
plan: 01
status: COMPLETED
duration: ~12min
---

## What Was Done

Added 3 math enrichment built-ins to the REPL:

1. **`eisenstein(k, T)`** — normalized Eisenstein series E_{2k}(q) = 1 - (4k/B_{2k}) Σ σ_{2k-1}(n) q^n
   - Hardcoded Bernoulli numbers B_2 through B_20 as exact rationals
   - Uses Frac arithmetic for σ computation to avoid int64_t overflow
   - Supports k=1..10 (E_2 through E_20)

2. **`partition(n)`** — partition number p(n) via generating function
   - Reuses pentagonal-optimized etaq(1,T) and Series::inverse()
   - Returns exact integer via Frac::str()

3. **`qdiff(f)`** — formal q-derivative θ_q f = Σ n·a_n·q^n
   - Multiplies each coefficient by its exponent
   - Returns composable Series

## Files Modified

- `src/qfuncs.h` — `bernoulli_even()`, `eisenstein()`, `partition_number()`, `qdiff()` functions
- `src/repl.h` — 3 help table entries + 3 dispatch handlers
- `src/series.h` — fixed pre-existing display bug with negative coefficients (double negatives like "- -24q")

## Verification

- Build: clean under `-Wall -Wextra -Wpedantic` (zero warnings)
- Smoke tests:
  - `eisenstein(2, 10)` → 1 + 240q + 2160q² + 6720q³ + ... (E_4) ✓
  - `eisenstein(1, 10)` → 1 - 24q - 72q² - 96q³ - ... (E_2, no double negatives) ✓
  - `eisenstein(3, 10)` → 1 - 504q - 16632q² - ... (E_6) ✓
  - `partition(0)` → 1, `partition(5)` → 7, `partition(100)` → 190569292 ✓
  - `qdiff(1 + q + q^2 + q^3)` → q + 2q² + 3q³ ✓
- All 10 acceptance tests pass
- All 7 quick-win tests pass

## Deviation

- Fixed pre-existing `Series::str()` display bug: negative coefficients after the first term rendered as double negatives (e.g., "- -24q" instead of "- 24q"). Root cause: sign prefix applied unconditionally instead of only for count==0.
- `partition_number()` needed explicit `Series::q(T)` construction since `etaq` requires `(const Series&, int, int)` — not `(int, int)`.
