---
phase: 26-findmaxind
plan: 01
subsystem: relations
tags: [findmaxind, RREF, linear-algebra, q-series, Maple-compat]

# Dependency graph
requires:
  - phase: 08-relations
    provides: build_matrix, transpose, findhom
  - phase: 07-linalg
    provides: gauss_to_rref
provides:
  - findmaxind(L, topshift) built-in
  - FindmaxindResult struct (subset, 1-based indices)
  - acceptance-findmaxind.sh tests
affects: [10-repl, 14-relations-demo]

# Tech tracking
tech-stack:
  added: []
  patterns: [RREF pivot columns = maximal independent subset]

key-files:
  created: [tests/acceptance-findmaxind.sh]
  modified: [src/linalg.h, src/relations.h, src/repl.h]

key-decisions:
  - "Indices 1-based (Maple compatible) per CONTEXT"
  - "Output indices only (concise, Maple primary output)"

patterns-established:
  - "findmaxind: M = build_matrix(L, numCols), MT = transpose(M), pivot_cols = gauss_to_rref(copy)"

# Metrics
duration: ~12min
completed: 2026-02-26
---

# Phase 26 Plan 01: findmaxind Summary

**findmaxind(L, topshift) implemented with RREF on transpose of coefficient matrix; returns maximal linearly independent subset and 1-based indices; closes GAP-04.**

## Performance

- **Duration:** ~12 min
- **Tasks:** 3
- **Files modified:** 4 (3 modified, 1 created)

## Accomplishments

- Moved gauss_to_rref out of anonymous namespace in linalg.h
- Added FindmaxindResult and findmaxind(L, topshift) in relations.h
- REPL: EvalResult variant, formatFindmaxind, display, dispatch, getHelpTable
- Acceptance tests: independent list (3 eta), dependent (duplicate), Rogers-Ramanujan G1,G2

## Task Commits

1. **Task 1: Expose gauss_to_rref, add findmaxind in relations.h** - `3654293` (feat)
2. **Task 2: EvalResult, format, display, dispatch, help in repl.h** - `a9d1575` (feat)
3. **Task 3: Acceptance test for findmaxind** - `8bc8632` (test)

## Files Created/Modified

- `src/linalg.h` - gauss_to_rref moved out of anonymous namespace
- `src/relations.h` - FindmaxindResult struct, findmaxind(L, topshift)
- `src/repl.h` - EvalResult variant, formatFindmaxind, display case, dispatch, help
- `tests/acceptance-findmaxind.sh` - 3 tests (independent, dependent, RR)

## Decisions Made

- Output format: indices only (e.g. "indices: [1, 2, 3]") per Maple primary output
- Acceptance tests use inline lists (findmaxind([...], 0)) since list assignment not implemented

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Git commit -m fails with "option trailer requires a value" (global config); used gsd-tools commit

## Next Phase Readiness

- findmaxind ready for use in relations demos
- GAP-04 closed (Maple findmaxind parity)

## Self-Check: PASSED

---
*Phase: 26-findmaxind*
*Completed: 2026-02-26*
