---
phase: 12-rogers-ramanujan-demo
plan: 01
subsystem: demo
tags: [repl, series, prodmake, maple, unicode, qseriesdoc]

# Dependency graph
requires:
  - phase: 11-demo-artifact
    provides: garvan-demo.sh, Makefile demo target
provides:
  - series(f,T) honors T; O-term Unicode O(q⁵⁰); formatProdmake Maple-style
affects: [12-rogers-ramanujan-demo]

# Tech tracking
tech-stack:
  added: []
  patterns: [formatProdmake mapleStyle, Series::str expToUnicode O-term]

key-files:
  created: []
  modified: [src/series.h, src/repl.h, demo/garvan-demo.sh]

key-decisions:
  - "formatProdmake mapleStyle=true by default; n=1 → (1-q), n>1 → (-q^n+1)"
  - "series(f) and series(f,T) cap display at min(T,100) terms"

patterns-established:
  - "Series O-term uses expToUnicode for exponents: O(q⁵⁰) not O(q^50)"

# Metrics
duration: ~15min
completed: 2026-02-25
---

# Phase 12 Plan 01: Rogers-Ramanujan Demo Summary

**Rogers-Ramanujan demo output aligned with qseriesdoc §3.1: 50-term series with Unicode O(q⁵⁰), prodmake Maple-style factors (1-q)(-q⁴+1)(-q⁶+1)..., and explicit series(rr,50) in demo block**

## Performance

- **Duration:** ~15 min
- **Tasks:** 3
- **Files modified:** 3

## Accomplishments
- series(f,T) displays up to T terms (cap 100); series(f) uses env.T
- Series::str O-term uses Unicode exponent via expToUnicode (O(q⁵⁰))
- formatProdmake supports Maple factor style: (1-q) for n=1, (-q^n+1) for n>1
- Rogers-Ramanujan demo block emits both Output (1) and (2) per qseriesdoc §3.1

## Task Commits

1. **Task 1: Fix series() built-in and Series::str O-term** - `2dd2eba` (feat)
2. **Task 2: Add Maple-style factor form to formatProdmake** - `ebe4bc6` (feat)
3. **Task 3: Update Rogers-Ramanujan demo block** - `b2969b0` (feat)

## Files Created/Modified
- `src/series.h` - O-term uses expToUnicode(trunc)
- `src/repl.h` - series() honors T; formatProdmake(a, mapleStyle)
- `demo/garvan-demo.sh` - adds series(rr, 50) between assign and prodmake

## Decisions Made
- formatProdmake mapleStyle default true to match qseriesdoc Output (2)
- series display cap 100 terms for safety

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered
- git commit -m fails with "option trailer requires a value" when run from shell; worked via commit script
- Build requires Cygwin bash -lc for proper PATH (make/g++)

## Next Phase Readiness
- Rogers-Ramanujan demo matches qseriesdoc §3.1 Output (1) and (2)
- Ready for Phase 13 (product conversion demo §3.2–3.4)

## Self-Check: PASSED

- FOUND: .planning/phases/12-rogers-ramanujan-demo/12-01-SUMMARY.md
- FOUND: 2dd2eba, ebe4bc6, b2969b0 (task commits)

---
*Phase: 12-rogers-ramanujan-demo*
*Completed: 2026-02-25*
