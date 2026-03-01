# Plan 33-01 Summary: Euler Pentagonal Fast Path

**Status:** COMPLETE
**Date:** 2026-02-26

## What Was Done

Added Euler pentagonal number theorem fast path to `etaq(1,T)` in `src/qfuncs.h`.

When `k == 1` and `q` is the standard formal variable, `etaq()` now directly generates the sparse pentagonal coefficients instead of performing T-1 Series multiplications. The coefficients are all 0, +1, or -1 at generalized pentagonal numbers `j(3j±1)/2`, yielding O(√T) complexity instead of O(T²).

## Changes

| File | Change |
|------|--------|
| `src/qfuncs.h` | Added `if (k == 1 && standardQ)` branch with pentagonal loop; existing product loop moved to `else` branch |

## Verification

- Build: clean (no new warnings)
- `etaq(1,15)` = `1 - q - q² + q⁵ + q⁷ - q¹² + O(q¹⁵)` — correct pentagonal coefficients
- `etaq(1,500)` — near-instant, correct sparse output with ~36 nonzero terms
- `etaq(2,50)`, `etaq(7,50)` — correct output via existing product loop (k≠1 unchanged)
- Core acceptance: 10/10 pass
- Wins: 7/7 pass
- Optional-args: 5/5 pass
- Suppress-output: 4/4 pass

## Requirements Satisfied

- **PERF-02**: etaq(1,T) uses Euler's pentagonal number theorem — O(√T)
- **PERF-03**: All existing acceptance tests pass (26/26)
