---
phase: 37-convenience-functions
plan: 01
status: COMPLETED
duration: ~10min
---

## What Was Done

Added 4 convenience built-ins to the REPL:

1. **`coeff(f, n)`** — extract coefficient of q^n from series f, displayed as bare rational
2. **`dissect(f, m, T)`** — display all m siftings of f labeled k=0..m-1
3. **`jacobi(a, n)`** — Jacobi symbol (a/n) for odd positive n, using quadratic reciprocity
4. **`kronecker(a, n)`** — Kronecker symbol extending Jacobi to all integers n

## Files Modified

- `src/qfuncs.h` — new `jacobi()` and `kronecker()` functions
- `src/repl.h` — 4 help table entries + 4 dispatch handlers

## Verification

- Build: clean under `-Wall -Wextra -Wpedantic` (zero warnings)
- Smoke tests: all 4 functions produce correct results
  - `coeff(etaq(1,30), 5)` → `1`, `coeff(etaq(1,30), 0)` → `1`
  - `dissect(etaq(1,30), 3, 10)` → 3 labeled sifted series
  - `jacobi(2, 15)` → `1`, `jacobi(2, 3)` → `-1`
  - `kronecker(2, 6)` → `0`, `kronecker(-1, -1)` → `-1`, `kronecker(1, 0)` → `1`
- `help(coeff)`, `help(dissect)`, `help(jacobi)`, `help(kronecker)` all work
- All 10 acceptance tests pass
- All 7 quick-win tests pass

## Deviation

None.
