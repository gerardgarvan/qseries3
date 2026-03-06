---
phase: 64-fractional-power-infrastructure
plan: 01
subsystem: math
tags: [powFrac, binomial-series, bigpow, iroot, rational-exponent]

requires:
  - phase: 63-q-shift-arithmetic-fix
    provides: normalize_q_shift() for fractional q-shift absorption
provides:
  - BigInt::bigpow binary exponentiation
  - BigInt::iroot integer k-th root via binary search
  - Frac::rational_pow exact rational exponentiation
  - Series::powFrac fractional power via generalized binomial recurrence
  - REPL ^ operator dispatches to powFrac for non-integer exponents
affects: [65-jacobi-half-integer-exponents, 66-exercise-solutions]

tech-stack:
  added: []
  patterns: [logarithmic-differentiation coefficient recurrence]

key-files:
  created: [tests/acceptance-powfrac.sh]
  modified: [src/bigint.h, src/frac.h, src/series.h, src/repl.h]

key-decisions:
  - "O(T^2) coefficient recurrence via logarithmic differentiation, not O(T^3) binomial summation"
  - "REPL catch-fallback: try integer pow first, catch to powFrac with rational constant check"
  - "iroot uses binary search with bit-length upper bound estimation"

patterns-established:
  - "powFrac: extract leading q^k, factor out c0, normalize, recurrence, scale back"

duration: ~10min
completed: 2026-03-02
---

# Phase 64 Plan 01: Fractional Power Infrastructure Summary

**Series::powFrac via generalized binomial recurrence with BigInt iroot, Frac rational_pow, and REPL ^ dispatch**

## Performance

- **Duration:** ~10 min
- **Started:** 2026-03-01T19:36:34Z
- **Completed:** 2026-03-02T00:42:07Z
- **Tasks:** 5
- **Files modified:** 5

## Accomplishments
- Full fractional power pipeline: `(1-q)^(1/2)` produces correct generalized binomial coefficients
- Roundtrip verified: `((1-q)^(1/2))^2 = 1 - q` to full truncation
- Handles c0 != 1 (e.g., `(4-4*q)^(1/2) = 2 - q - ...`), negative exponents, leading q-power extraction
- All 10 acceptance tests + 31/41 maple-checklist blocks pass (no regressions)

## Task Commits

Each task was committed atomically:

1. **Task 1: BigInt::bigpow and BigInt::iroot** - `4ba9fd3` (feat)
2. **Task 2: Frac::rational_pow** - `b62a448` (feat)
3. **Task 3: Series::powFrac** - `03d2eb9` (feat)
4. **Task 4: REPL dispatch** - `2fd36f4` (feat)
5. **Task 5: Build, fix, tests** - `51b147a` (test)

## Files Created/Modified
- `src/bigint.h` - Added `bigpow` (binary exponentiation) and `iroot` (integer k-th root) free functions
- `src/frac.h` - Added `Frac::rational_pow(Frac alpha)` for exact rational exponentiation
- `src/series.h` - Added `Series::powFrac(Frac alpha)` with generalized binomial coefficient recurrence
- `src/repl.h` - Updated `BinOp::Pow` to fall back to `powFrac` when exponent is non-integer
- `tests/acceptance-powfrac.sh` - 6 acceptance tests covering all powFrac verification criteria

## Decisions Made
- Used O(T^2) coefficient recurrence via logarithmic differentiation instead of direct binomial summation
- REPL `^` tries integer path first, catches to `powFrac` — preserves existing integer behavior
- `iroot` uses bit-length heuristic for upper bound to avoid huge binary search space
- `bigpow` and `iroot` are free functions (not BigInt static methods) for consistency with `bigGcd`

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Fixed EvalResult vs EnvValue type mismatch in REPL dispatch**
- **Found during:** Task 5 (Build)
- **Issue:** Plan used `EnvValue` for eval return but `eval()` returns `EvalResult` (different variant type)
- **Fix:** Changed `EnvValue rv` to `EvalResult rv` in the catch block
- **Files modified:** src/repl.h
- **Verification:** Build succeeds, all tests pass
- **Committed in:** `51b147a` (Task 5 commit)

---

**Total deviations:** 1 auto-fixed (1 bug)
**Impact on plan:** Type correction necessary for compilation. No scope creep.

## Issues Encountered
None beyond the type mismatch above.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- powFrac infrastructure complete — Phase 65 (Jacobi half-integer exponents) can use `powFrac` for `jacprodmake` with fractional exponents
- Blocks 13-14 (Slater identities with fractional Jacobi exponents) are next targets

---
*Phase: 64-fractional-power-infrastructure*
*Completed: 2026-03-02*
