---
phase: 66-exercise-solutions-regression
plan: 01
subsystem: testing
tags: [acceptance-tests, regression, exercises, etamake, findnonhomcombo, findpoly]

requires:
  - phase: 63-q-shift-arithmetic-fix
    provides: q-shift normalization for fractional power arithmetic
  - phase: 64-fractional-power-infrastructure
    provides: powFrac for Series, rational exponentiation in Frac/BigInt
  - phase: 65-jacobi-half-integer-exponents
    provides: jacprodmake/jac2series half-integer exponent support
provides:
  - Exercise verification test script (tests/acceptance-exercises.sh)
  - Full regression confirmation for Milestone v4.2
affects: []

tech-stack:
  added: []
  patterns: [exercise-test-script]

key-files:
  created: [tests/acceptance-exercises.sh]
  modified: []

key-decisions:
  - "Exercise 9 uses T=100 truncation — completes in ~46s, feasible for CI"
  - "Grep for Unicode subscript X₁/X₂ in findpoly/findnonhomcombo output, not ASCII X[1]/X[2]"
  - "Grep for UTF-8 η character in etamake output, not ASCII 'eta'"

patterns-established:
  - "Exercise test pattern: capture output to variable for multi-assertion checks"

duration: ~8min
completed: 2026-03-01
---

# Phase 66 Plan 01: Exercise Solutions & Regression Summary

**Exercise verification test script (4/9/10) with full regression: 10/10 acceptance, 6/6 powfrac, 33/33 maple-checklist, 7/7 exercises — zero failures across all suites**

## Performance

- **Duration:** ~8 min
- **Started:** 2026-03-01
- **Completed:** 2026-03-01
- **Tasks:** 2
- **Files modified:** 1

## Accomplishments
- Created tests/acceptance-exercises.sh covering Exercises 4, 9, and 10 from qseriesdoc.md
- Exercise 4: verified b(q) = η(τ)³/η(3τ) series starts "1 - 3q" and etamake identifies eta quotient
- Exercise 9: confirmed findnonhomcombo produces N = a⁶(1 - 20x - 8x²) cubic AGM identity
- Exercise 10: confirmed findpoly produces cubic relation polynomial with coefficients 27, 81, etc.
- Full regression suite verified: zero failures across all 4 test suites (56 total tests)

## Task Commits

Each task was committed atomically:

1. **Task 1: Create exercise verification test script** - `35f8d8a` (feat)
2. **Task 2: Full regression suite verification** - no changes needed (verification-only)

## Files Created/Modified
- `tests/acceptance-exercises.sh` - Exercise verification tests for Exercises 4, 9, 10 (7 subtests)

## Decisions Made
- Exercise 9 computation uses T=100 truncation and completes in ~46 seconds — feasible for automated testing
- Output format uses Unicode subscripts (X₁, X₂) not ASCII X[1]/X[2] — grep patterns adjusted accordingly
- etamake output uses Unicode η character — grep uses UTF-8 byte sequence \xCE\xB7

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None.

## User Setup Required

None - no external service configuration required.

## Test Matrix

| Suite | Pass | Fail | Skip | Total |
|-------|------|------|------|-------|
| acceptance.sh | 10 | 0 | 0 | 10 |
| acceptance-powfrac.sh | 6 | 0 | 0 | 6 |
| maple-checklist.sh | 33 | 0 | 8 | 41 |
| acceptance-exercises.sh | 7 | 0 | 0 | 7 |
| **Total** | **56** | **0** | **8** | **64** |

## Next Phase Readiness
- Milestone v4.2 (Fix Block Failures) is complete
- All exercises from qseriesdoc.md that depend on Phases 63-65 are verified
- No blockers or concerns

---
*Phase: 66-exercise-solutions-regression*
*Completed: 2026-03-01*
