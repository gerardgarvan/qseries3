---
phase: 01-bigint
plan: 02
subsystem: core
tags: [BigInt, divmod, GCD, binary-search, C++20]

# Dependency graph
requires:
  - phase: 01-bigint
    provides: BigInt struct, add/sub/mul, comparisons, str
provides:
  - BigInt divmod (binary-search quotient)
  - operator/, operator%
  - bigGcd (Euclidean via divmod)
affects: [frac, linalg]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Binary-search quotient digit selection for long division"
    - "divmod on absolutes with sign rules: q_neg = a.neg != b.neg, r_neg = a.neg"

key-files:
  created: []
  modified: [src/bigint.h, src/main.cpp]

key-decisions:
  - "std::invalid_argument for division by zero (consistent with string ctor)"
  - "Remainder invariant 0 ≤ r < |b| per standard convention"

patterns-established:
  - "Long division: n_d+1 digit partial, binary search [0,BASE-1] for k s.t. k*divisor ≤ remainder"
  - "bigGcd uses abs() for negatives; Euclidean loop via divmod"

# Metrics
duration: ~15min
completed: 2026-02-24
---

# Phase 01 Plan 02: BigInt divmod and GCD Summary

**BigInt divmod with binary-search quotient, operator/, operator%, and bigGcd; division by zero throws std::invalid_argument**

## Performance

- **Duration:** ~15 min
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments

- divmod with binary-search quotient digit selection (per SPEC/RESEARCH)
- Remainder invariant 0 ≤ r < |b| enforced
- operator/ and operator% delegate to divmod; division by zero throws
- bigGcd via Euclidean algorithm (abs for negatives, gcd(0,x)=|x|)

## Task Commits

1. **Task 1 + Task 2** - `6d94a10` (feat: divmod, operator/, operator%, bigGcd)

**Note:** Both tasks committed together; implementation is intertwined in src/bigint.h.

## Files Created/Modified

- `src/bigint.h` - divmod (binary search), operator/, operator%, bigGcd
- `src/main.cpp` - Verification tests for divmod, /, %, bigGcd, division-by-zero

## Decisions Made

- std::invalid_argument for division by zero (consistent with BigInt string constructor)
- Remainder sign follows dividend (r_neg = a.neg); quotient sign is a.neg XOR b.neg

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Build tools (g++ / x86_64-w64-mingw32-g++) not in PATH in execution shell; user should run `make` in Cygwin or MinGW environment for verification.

## User Setup Required

None - no external service configuration required.

## Verification

Run in Cygwin or MinGW environment:
```bash
make
./qseries.exe
```
Expected: "Plan 01-02: divmod, /, %, bigGcd OK"

Critical SPEC cases: 1000000000/1, 999999999/1000000000, 123456789012345/123, bigGcd(48,18)==6.

## Next Phase Readiness

BigInt API complete. Ready for frac.h (Frac uses divmod and bigGcd for reduce()).

## Self-Check: PASSED

- FOUND: .planning/phases/01-bigint/01-02-SUMMARY.md
- FOUND: commit 6d94a10

---
*Phase: 01-bigint*
*Completed: 2026-02-24*
