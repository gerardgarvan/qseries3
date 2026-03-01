---
phase: 02-frac
plan: 02
subsystem: testing
tags: [frac, bigint, cpp, test-driver, reduce, spec]

# Dependency graph
requires:
  - phase: 02-01
    provides: Frac implementation (frac.h)
provides:
  - main.cpp runs BigInt + Frac tests
  - SPEC cases validated: 6/4→3/2, 0/5→0/1, zero-denom throws
  - Arithmetic and sign test coverage
  - Long-chain growth test (no exponential BigInt growth)
affects: [03-series]

# Tech tracking
tech-stack:
  added: []
  patterns: [CHECK macro for assertions, try/catch for exception tests]

key-files:
  created: []
  modified: [src/main.cpp]

key-decisions:
  - "Single commit for all 3 tasks - sequential edits to main.cpp"
  - "Long-chain test: add chain 1+50*(1/2)=26, mul chain (3/2)^20 reduced"

patterns-established:
  - "Frac test groups: SPEC reduction, zero-denom, arithmetic, sign, long-chain"

# Metrics
duration: ~8min
completed: 2025-02-24
---

# Phase 02 Plan 02: Frac Test Driver Summary

**main.cpp extended with Frac test suite covering SPEC cases, add/sub/mul/div, sign handling, and long-chain growth verification**

## Performance

- **Duration:** ~8 min
- **Tasks:** 3 completed
- **Files modified:** 1 (src/main.cpp)

## Accomplishments

- Added `#include "frac.h"` and Frac Phase 2 test driver section
- SPEC reduction: 6/4→3/2, 0/5→0/1, 0/5→num=0 den=1
- Zero denominator throws: Frac(1,0) and Frac(1,2)/Frac(0)
- Arithmetic: 1/2+1/3=5/6, 1/2-1/3=1/6, 1/2*2/3=1/3, 1/2/2/3=3/4, (-1/2)+(1/2)=0, (-1/2)*2=-1
- Sign handling: Frac(-3,4), Frac(3,-4)→"-3/4", comparisons
- str() integer form: Frac(7)→"7", Frac(0)→"0"
- Long-chain: f=1; for 50 iterations f=f+1/2 → f=26/1; (3/2)^20 stays reduced

## Task Commits

1. **feat(02-02): add Frac test driver** - `1fba5cc`
   - Tasks 1–3 implemented in one commit (sequential edits to main.cpp)

## Files Created/Modified

- `src/main.cpp` - Added Frac include, Frac Phase 2 test groups (SPEC, zero-denom, arithmetic, sign, long-chain)

## Decisions Made

- Followed plan: SPEC reduction, zero-denom throws, add/sub/mul/div, sign, long-chain
- Long-chain formulation: add chain 1+50*(1/2)=26 (den=1), mul chain (3/2)^20 with gcd(num,den)=1 check

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Build/run via Cygwin bash: `c:\cygwin64\bin\bash.exe -lc "cd /home/ggarv/qseries3 && g++ ... && ./qseries"`
- Git commit via gsd-tools with Cygwin bin in PATH (git not in default PowerShell PATH)

## Self-Check: PASSED

- FOUND: .planning/phases/02-frac/02-02-SUMMARY.md
- FOUND: 1fba5cc in git log

## Next Phase Readiness

- Frac validated; ready for Phase 3 (Series)
- main.cpp runs BigInt + Frac tests, exits 0 when all pass

---
*Phase: 02-frac*
*Plan: 02*
*Completed: 2025-02-24*
