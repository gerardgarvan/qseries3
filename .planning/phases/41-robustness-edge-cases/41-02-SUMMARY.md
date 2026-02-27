---
phase: 41-robustness-edge-cases
plan: 02
subsystem: repl
tags: [error-handling, levenshtein, variant-safety, acceptance-tests]

requires:
  - phase: 41-robustness-edge-cases plan 01
    provides: etaq/sift/pow/parser guards
provides:
  - Division by zero error for integer and series expressions
  - Safe variant access via toSeries() helper (eliminates bad_variant_access)
  - Levenshtein-based builtin name suggestions for typos
  - Integer assignment support (int64_t results wrap in Series::constant)
  - Comprehensive acceptance test script for all 9 robustness fixes
affects: [42-garvan-tutorial-coverage]

tech-stack:
  added: []
  patterns: [toSeries helper for EvalResult->Series conversion, levenshteinDistance for fuzzy matching]

key-files:
  created: [tests/acceptance-robustness.sh]
  modified: [src/repl.h, src/series.h]

key-decisions:
  - "Moved pow magnitude limit from series.h to REPL evalExpr layer — internal math functions (etamake) need unrestricted pow"
  - "Added Series division-by-zero check (r.c.empty()) in evalExpr alongside evalToInt guard"
  - "toSeries() takes optional T parameter for int64_t-to-Series promotion"

patterns-established:
  - "toSeries(result, context, T): safe EvalResult->Series with auto-promotion from int64_t"
  - "levenshteinDistance: reusable fuzzy string matching for suggestions"

duration: 15min
completed: 2026-02-27
---

# Phase 41 Plan 02: REPL Hardening & Acceptance Tests Summary

**Division-by-zero guard, safe variant access via toSeries(), Levenshtein typo suggestions, integer assignment, and 9-test acceptance script**

## Performance

- **Duration:** ~15 min
- **Started:** 2026-02-27T14:00:21Z
- **Completed:** 2026-02-27T14:15:00Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments
- Integer division by zero in evalToInt and zero-series division in evalExpr both produce clear "division by zero" error
- All bare std::get<Series> calls replaced with toSeries() helper — bad_variant_access can never leak to user
- Unknown builtin names suggest up to 2 closest matches (Levenshtein distance <= 3): e.g., "etamke" suggests "etamake etaq"
- Integer-valued builtins (mobius, legendre, etc.) can now be assigned to variables: x := mobius(6) works
- Comprehensive acceptance-robustness.sh covering all 9 fixes from Plans 01 and 02

## Task Commits

Each task was committed atomically:

1. **Task 1: Fix division by zero, safe variant access, Levenshtein suggestions, integer assignment** - `29189d4` (feat)
2. **Task 2: Create acceptance test script for all 9 robustness fixes** - `35b6eb3` (test)

## Files Created/Modified
- `src/repl.h` - Added toSeries(), levenshteinDistance(), div-by-zero guards, Levenshtein suggestions, int64_t assignment
- `src/series.h` - Removed pow magnitude limit (moved to REPL layer)
- `tests/acceptance-robustness.sh` - 9-test acceptance script covering all robustness fixes

## Decisions Made
- Moved pow magnitude limit from Series::pow() in series.h to evalExpr BinOp::Pow in repl.h — etamake internally needs large exponents that exceeded the 10000 limit
- Added zero-series check (r.c.empty()) for Series division in evalExpr, separate from the integer division guard in evalToInt — both paths need coverage
- toSeries() helper accepts optional T parameter (default 50) for int64_t auto-promotion to constant series

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Series division by zero not caught by evalToInt guard**
- **Found during:** Task 1 (division by zero verification)
- **Issue:** `1/0` goes through Series division path (evalExpr), not evalToInt — error was "constant term zero" instead of "division by zero"
- **Fix:** Added `if (r.c.empty()) throw "division by zero"` in evalExpr BinOp::Div case
- **Files modified:** src/repl.h
- **Verification:** `echo "1/0" | ./qseries` → "division by zero"
- **Committed in:** 29189d4

**2. [Rule 1 - Bug] pow magnitude limit broke etamake (Test 6 Rødseth)**
- **Found during:** Task 1 (acceptance test verification)
- **Issue:** Plan 01's pow limit of 10000 in series.h broke etamake which internally uses pow() with very large exponents
- **Fix:** Removed limit from series.h, added equivalent limit in repl.h evalExpr BinOp::Pow — user expressions limited, internal math unrestricted
- **Files modified:** src/series.h, src/repl.h
- **Verification:** All 10 acceptance tests pass; `etaq(1,20)^20000` still blocked
- **Committed in:** 29189d4

**3. [Rule 3 - Blocking] Series::constant requires T parameter**
- **Found during:** Task 1 (build)
- **Issue:** Plan specified `Series::constant(Frac(val))` but API requires `Series::constant(Frac(val), T)`
- **Fix:** Added T parameter to toSeries() and evalStmt integer assignment
- **Files modified:** src/repl.h
- **Committed in:** 29189d4

---

**Total deviations:** 3 auto-fixed (2 bugs, 1 blocking)
**Impact on plan:** All fixes necessary for correctness. No scope creep.

## Issues Encountered
None beyond the deviations above.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- All 9 robustness requirements (ROBUST-01 through ROBUST-09) implemented and tested
- Ready for Phase 42 (Garvan tutorial coverage)

## Self-Check: PASSED

- All 3 key files exist (tests/acceptance-robustness.sh, src/repl.h, src/series.h)
- Both task commits found (29189d4, 35b6eb3)
- Acceptance test script: 103 lines (min 60)

---
*Phase: 41-robustness-edge-cases*
*Completed: 2026-02-27*
