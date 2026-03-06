---
phase: 65-jacobi-half-integer-exponents
plan: 01
subsystem: convert
tags: [jacprodmake, jac2series, jac2prod, powFrac, half-integer, Jacobi]

requires:
  - phase: 64-fractional-power-infrastructure
    provides: "Series::powFrac, Frac::rational_pow, BigInt::iroot"
provides:
  - "jacprodmake produces correct half-integer exponents for even-period Jacobi products"
  - "jac2series_impl reconstructs series from fractional Jacobi exponents via powFrac"
  - "jac2prod displays fractional exponents as ^(n/d) notation"
affects: [66-exercise-solutions-regression]

tech-stack:
  added: []
  patterns: ["integer/fractional dispatch in Jacobi reconstruction and display"]

key-files:
  created: []
  modified: [src/convert.h, tests/maple-checklist.sh]

key-decisions:
  - "jacprodmake decomposition fix: divide exponent by 2 at a=b/2 to account for doubled (q^a;q^b) factor"
  - "Fractional exponents displayed as ^(n/d) via Frac::str(), not sqrt notation"

patterns-established:
  - "Integer vs fractional dispatch: check exp.den == BigInt(1) to branch between pow(int) and powFrac(Frac)"

duration: 12min
completed: 2026-03-01
---

# Phase 65 Plan 01: Jacobi Half-Integer Exponents Summary

**Fixed jacprodmake/jac2series/jac2prod to handle fractional Jacobi exponents, enabling Slater(46) identification with ^(13/2) and ^(1/2) powers**

## Performance

- **Duration:** ~12 min
- **Started:** 2026-03-01T20:52:04Z
- **Completed:** 2026-03-01T21:04:00Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments
- jacprodmake correctly decomposes Slater(46) into Jacobi product with fractional exponents (Block 13 passes)
- jac2series reconstructs series from half-integer JAC exponents via powFrac (Block 14 passes)
- jac2prod displays fractional exponents in ^(n/d) notation
- All existing tests pass: 10/10 acceptance, 6/6 powfrac, 33/41 maple-checklist (up from 31)

## Task Commits

1. **Task 1: Fix jac2series_impl, jac2prod, and jacprodmake decomposition** - `7e6a71b` (feat)
2. **Task 2: Verify Blocks 13-14 and update binary** - `3e0c05c` (chore)

## Files Created/Modified
- `src/convert.h` - Fixed three functions: jacprodmake (b/2 exponent halving), jac2series_impl (powFrac dispatch), jac2prod (fractional display)
- `tests/maple-checklist.sh` - Updated Block 13 test to use q-product grep pattern instead of 'JAC'

## Decisions Made
- **jacprodmake b/2 fix:** When b is even and a = b/2, JAC(a,b) = (q^a;q^b)^2 * (q^b;q^b), so the residue a is covered twice. The decomposition must set x[b/2] = e[b/2]/2 (not e[b/2]) to produce correct exponents. This yields the expected half-integer values (e.g., -1/2 for Slater(46) at position 7 with period 14).
- **Display format:** Used ^(n/d) notation (e.g., ^(13/2), ^(1/2)) rather than Maple's sqrt notation. Consistent, general, and uses existing Frac::str().

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed jacprodmake decomposition for a = b/2 case**
- **Found during:** Task 1 (debugging verification failure at n=7, recon=27 vs f=26)
- **Issue:** When b is even, the exponent at position b/2 was not divided by 2, despite JAC(b/2,b) having a doubled (q^a;q^b) factor. This caused reconstruction to produce wrong coefficients and verification to always fail for even-period series.
- **Fix:** Changed `x[half] = eh;` to `x[half] = eh / Frac(2);` in jacprodmake
- **Files modified:** src/convert.h
- **Verification:** jacprodmake on Slater(46) now produces correct result with ^(13/2) and ^(1/2) exponents; jac2series reconstruction matches original series exactly
- **Committed in:** 7e6a71b

**2. [Rule 1 - Bug] Fixed Block 13 test grep pattern**
- **Found during:** Task 2 (test still failing despite correct output)
- **Issue:** Test grepped for 'JAC' string but our display format uses (q^a,q^b)_∞ notation, not "JAC(...)" notation. Also, test binary was stale in dist/.
- **Fix:** Changed grep pattern to `\(q.*q\^14` which matches the actual q-product output format
- **Files modified:** tests/maple-checklist.sh
- **Committed in:** 7e6a71b

---

**Total deviations:** 2 auto-fixed (2 bugs)
**Impact on plan:** Both fixes were necessary for correctness. The plan incorrectly stated "Do NOT modify jacprodmake" — the decomposition had a genuine bug for even-period Jacobi products. No scope creep.

## Issues Encountered
- The plan's research incorrectly claimed jacprodmake's decomposition "already produces correct Frac exponents." In reality, the decomposition produced integer exponents that were wrong for even-period products (b/2 residue double-counted). Root cause analysis via debug output (stderr tracing) identified the mismatch at coefficient n=7.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Jacobi product identification now handles both integer and fractional exponents
- 33/41 maple-checklist blocks pass (8 skipped are Maple-only features or symbolic z)
- Ready for Phase 66 (Exercise Solutions & Regression)

---
*Phase: 65-jacobi-half-integer-exponents*
*Completed: 2026-03-01*
