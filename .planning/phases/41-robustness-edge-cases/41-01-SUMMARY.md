---
phase: 41-robustness-edge-cases
plan: 01
subsystem: math-engine
tags: [validation, error-handling, parser, robustness]

requires:
  - phase: 10-repl
    provides: "REPL try/catch for std::runtime_error"
provides:
  - "etaq k>0 guard preventing infinite loop on k=0"
  - "sift n>0 guard preventing division-by-zero/hang on n=0"
  - "pow exponent magnitude limit (10000) preventing INT_MIN UB"
  - "Parser recursion depth limit (256) preventing stack overflow"
  - "Integer literal overflow detection preventing silent wraparound"
affects: [42-garvan-tutorial-coverage]

tech-stack:
  added: []
  patterns: [throw std::runtime_error at function entry for invalid input]

key-files:
  created: []
  modified: [src/qfuncs.h, src/convert.h, src/series.h, src/parser.h]

key-decisions:
  - "Limit pow exponent to ±10000 (generous for any realistic computation)"
  - "Parser nesting limit 256 (well above any hand-typed expression)"
  - "Integer overflow check uses INT64_MAX arithmetic before accumulation"

patterns-established:
  - "Guard pattern: validate at function entry, throw descriptive std::runtime_error"

duration: 8min
completed: 2026-02-27
---

# Phase 41 Plan 01: Robustness & Edge Cases — Input Validation Guards

**Five critical crash/hang bugs fixed: etaq(0) hang, sift(0) hang, pow(INT_MIN) UB, deep nesting stack overflow, integer literal silent overflow**

## Performance

- **Duration:** 8 min
- **Started:** 2026-02-27T13:52:51Z
- **Completed:** 2026-02-27T14:01:00Z
- **Tasks:** 2
- **Files modified:** 4

## Accomplishments
- etaq, sift, and pow now throw descriptive errors on invalid inputs instead of hanging/crashing
- Parser rejects expressions nested beyond 256 levels with clear error message
- Integer literals exceeding int64 range detected at parse time, not silently overflowed
- All 10 existing acceptance tests continue to pass

## Task Commits

Each task was committed atomically:

1. **Task 1: Add validation guards to etaq, sift, and pow** - `cfd06df` (feat)
2. **Task 2: Add parser recursion depth limit and integer overflow detection** - `82325ea` (feat)

## Files Created/Modified
- `src/qfuncs.h` - Added k>0 guard at top of etaq()
- `src/convert.h` - Added n>0 guard at top of sift()
- `src/series.h` - Added magnitude guard in pow() after n==0 check, before n<0 branch
- `src/parser.h` - Added depth member, recursion tracking in LPAREN, overflow check in INT parsing, #include <climits>

## Decisions Made
- Placed pow magnitude guard after n==0 (so pow(0) still works) but before n<0 (prevents -n on INT_MIN)
- Used INT64_MAX arithmetic check (`v > (INT64_MAX - digit) / 10`) rather than string length heuristic for precision
- Added `#include <climits>` for INT64_MAX definition

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered
- Git 2.51.0 `--trailer` config required `-c trailer.ifexists=doNothing` workaround for commit commands

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- Five of six ROBUST bugs fixed; ROBUST-03 (division by zero in series inverse) deferred to Plan 02
- Ready for Plan 41-02 execution

---
*Phase: 41-robustness-edge-cases*
*Completed: 2026-02-27*
