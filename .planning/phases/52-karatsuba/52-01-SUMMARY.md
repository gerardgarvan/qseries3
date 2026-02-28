---
phase: 52-karatsuba
plan: 01
subsystem: math
tags: [bigint, karatsuba, multiplication, performance]

requires:
  - phase: 01-bigint
    provides: BigInt struct with operator* (schoolbook)
provides:
  - Karatsuba O(n^1.585) multiplication for large BigInt operands
  - Hybrid dispatch (schoolbook below threshold, Karatsuba above)
  - Raw digit-vector helpers (mulSchoolbook, addVec, subVec)
affects: [series, qfuncs, convert, relations]

tech-stack:
  added: []
  patterns: [hybrid-dispatch, recursive-divide-and-conquer]

key-files:
  created: []
  modified: [src/bigint.h]

key-decisions:
  - "KARATSUBA_THRESHOLD=32 limbs (~288 decimal digits)"
  - "Vector-level helpers (addVec/subVec/mulSchoolbook) avoid BigInt construction overhead in recursion"
  - "operator* dispatches through karatsubaMultiply which auto-selects schoolbook below threshold"

patterns-established:
  - "Hybrid multiply: karatsubaMultiply dispatches to mulSchoolbook for small operands"

duration: 4min
completed: 2026-02-28
---

# Phase 52 Plan 01: Karatsuba Multiplication Summary

**Hybrid Karatsuba/schoolbook multiplication in BigInt with 32-limb threshold, O(n^1.585) for large operands**

## Performance

- **Duration:** 4 min
- **Started:** 2026-02-28T21:01:57Z
- **Completed:** 2026-02-28T21:05:48Z
- **Tasks:** 1
- **Files modified:** 1

## Accomplishments
- Extracted existing operator* body into `mulSchoolbook` static method operating on raw digit vectors
- Added `addVec` and `subVec` digit-vector helpers for carry-propagating arithmetic without BigInt overhead
- Implemented recursive `karatsubaMultiply` with 3-multiplication Karatsuba algorithm (z0, z1, z2 decomposition)
- Updated `operator*` to dispatch through `karatsubaMultiply` — automatic hybrid selection
- All 10 core + 45 supplementary acceptance tests pass; partition(200) = 3972999029388 verified correct

## Task Commits

Each task was committed atomically:

1. **Task 1: Implement Karatsuba multiplication** - `cfd311c` (feat)

## Files Created/Modified
- `src/bigint.h` - Added mulSchoolbook, addVec, subVec, karatsubaMultiply static methods; updated operator* to dispatch

## Decisions Made
- KARATSUBA_THRESHOLD set to 32 limbs (~288 decimal digits in base 10^9) — standard crossover point
- Helpers work on raw `std::vector<uint32_t>` (not BigInt objects) to avoid sign handling and normalization overhead during recursion
- `subVec` assumes a >= b (unsigned subtraction) — guaranteed by Karatsuba identity: (x0+x1)(y0+y1) >= x0*y0 + x1*y1

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- BigInt multiply performance improved for large operands
- Ready for Series optimization (phase 53) and benchmarking (phase 54)
- All existing functionality preserved — zero regressions

---
*Phase: 52-karatsuba*
*Completed: 2026-02-28*
