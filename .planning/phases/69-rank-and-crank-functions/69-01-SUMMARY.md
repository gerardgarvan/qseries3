# Phase 69-01 Summary: Rank and Crank Generating Functions

## Status: COMPLETE

## What was done

1. **Implemented `compute_partition_numbers(T)`** in `qfuncs.h` — shared helper computing p(0)..p(T-1) via Euler's pentagonal recurrence in O(T√T)

2. **Implemented `rankgf(m, T)`** in `qfuncs.h` — generates Σ N(m,n) q^n using the partition-number convolution formula with pentagonal exponents k(3k-1)/2. Includes δ_{m,0} correction for the empty partition.

3. **Implemented `crankgf(m, T)`** in `qfuncs.h` — generates Σ M(m,n) q^n using triangular exponents k(k-1)/2. No correction needed (M(0,0)=1 arises naturally; M(0,1)=-1 is the Andrews-Garvan convention).

4. **Added REPL dispatch** in `repl.h` — both functions callable as `rankgf(m, T)` and `crankgf(m, T)` with help entries.

5. **Created acceptance test suite** `tests/acceptance-rank-crank.sh` — 11 tests covering symmetry, specific coefficients, and partition count identity.

## Verification

- All 11 acceptance tests pass
- maple-checklist regression: 33 passed, 0 failed, 8 skipped (unchanged)
- Coefficients match verified tables from research (cross-checked against Garvan's Maple RANK/CRANK packages)

## Files modified

- `src/qfuncs.h` — added `compute_partition_numbers`, `rankgf`, `crankgf`
- `src/repl.h` — added help entries and dispatch for `rankgf`, `crankgf`
- `tests/acceptance-rank-crank.sh` — new acceptance test script
