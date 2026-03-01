---
phase: 02-frac
plan: 01
subsystem: math
tags: [frac, rational, bigint, gcd, c++20]

# Dependency graph
requires:
  - phase: 01-bigint
    provides: BigInt, bigGcd for Frac numerator/denominator and reduction
provides:
  - Frac struct with exact rational arithmetic
  - reduce() via bigGcd in every ctor and op
  - Arithmetic +,-,*,/ and comparisons
  - str() for display
affects: [03-series, convert, linalg]

# Tech tracking
tech-stack:
  added: []
  patterns: [struct with methods, reduce-after-every-op, std::invalid_argument for zero denom]

key-files:
  created: [src/frac.h]
  modified: []

key-decisions:
  - "Use std::invalid_argument for Frac(n,0) and division by zero (consistency with BigInt)"
  - "den > 0, gcd(|num|,den)=1, 0/1 for zero per SPEC and RESEARCH"

patterns-established:
  - "reduce() in every constructor and operator — no deferral"
  - "Zero denominator throws before reduce()"

# Metrics
duration: ~15min
completed: 2025-02-24
---

# Phase 2 Plan 1: Frac Implementation Summary

**Exact rational arithmetic with BigInt, auto-reduce via GCD in every constructor and operator, zero-denom throws**

## Performance

- **Duration:** ~15 min
- **Tasks:** 3
- **Files created:** 1
- **Commits:** 3

## Accomplishments

- Frac struct with BigInt num, den; den > 0 and reduced form enforced
- Constructors: Frac(), Frac(int64_t), Frac(int64_t,int64_t), Frac(BigInt,BigInt); all call reduce()
- reduce() uses bigGcd; handles den<0 (flip signs), num=0 (0/1)
- Arithmetic: +,-,*,/; division by zero throws std::invalid_argument
- Comparisons: ==, !=, <, <=, >, >= via cross-multiply
- str(): integers as "7", fractions as "3/2", zero as "0", negatives as "-3/4"

## Task Commits

Each task was committed atomically:

1. **Task 1: Frac struct, constructors, reduce, helpers** - `ffdb3c1` (feat)
2. **Task 2: Arithmetic operators and comparisons** - `c54a046` (feat)
3. **Task 3: str() and long-chain growth** - `90046cc` (feat)

## Files Created/Modified

- `src/frac.h` - Frac struct with reduce(), constructors, arithmetic, comparisons, str()

## Decisions Made

None beyond plan — followed RESEARCH and SPEC. std::invalid_argument for zero denominator per consistency with BigInt (02-CONTEXT).

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- **Build verification:** g++ and make were not available in PATH in the execution environment (PowerShell). Code structure follows SPEC; verification should be run in a Cygwin or MinGW environment with `g++ -std=c++20 -O2 -o qseries src/main.cpp` or `make` after Plan 02-02 adds Frac tests to main.cpp.

## Next Phase Readiness

- frac.h ready for Plan 02-02 (test driver)
- Plan 02-02 will add #include "frac.h" and Frac tests to main.cpp
- Foundation for Series (Phase 3)

## Self-Check: PASSED

- FOUND: src/frac.h
- FOUND: ffdb3c1, c54a046, 90046cc

---
*Phase: 02-frac*
*Plan: 01*
*Completed: 2025-02-24*
