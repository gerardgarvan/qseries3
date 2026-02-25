---
phase: 14-relations-demo
plan: 01
subsystem: demo
tags: [qseries, findhom, findhomcombo, findnonhomcombo, garvan-demo, relations]

# Dependency graph
requires:
  - phase: 13-product-conversion-demo
    provides: garvan-demo.sh pattern, run blocks, qseries binary
provides:
  - Relations section with findhom, findhomcombo, findnonhomcombo
  - DEMO-04 relation-finding examples (qseriesdoc §4)
affects: phase 15 (sifting/product identities demo)

# Tech tracking
tech-stack:
  added: []
  patterns: [run-block pattern, theta fallback for unsupported constructs]

key-files:
  created: []
  modified: [demo/garvan-demo.sh]

key-decisions:
  - "findhomcombo: theta-based fallback used; nested sum sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50) causes std::get variant error — REPL unsupported"

patterns-established:
  - "Relations demo: findhom (Gauss AGM), findhomcombo (theta fallback), findnonhomcombo (Watson) run blocks"

# Metrics
duration: ~8min
completed: 2026-02-25
---

# Phase 14 Plan 01: Relations Demo Summary

**Relations section in garvan-demo.sh with findhom (Gauss AGM), findhomcombo theta fallback, and findnonhomcombo (Watson) — DEMO-04 reproduces qseriesdoc §4**

## Performance

- **Duration:** ~8 min
- **Started:** 2026-02-25
- **Completed:** 2026-02-25
- **Tasks:** 1
- **Files modified:** 1

## Accomplishments

- Relations section with three run blocks: findhom, findhomcombo, findnonhomcombo
- findhom yields Gauss AGM relations (X₁²-X₃², X₂²-X₄², etc.)
- findhomcombo runs with theta-based fallback (nested sum unsupported)
- findnonhomcombo yields Watson modular equation (343, 49 coefficients)
- `cd dist && bash demo/garvan-demo.sh` produces expected output for all three blocks

## Task Commits

1. **Task 1: Replace Relations placeholder with three run blocks** - `71efad1` (feat)

**Plan metadata:** (pending final docs commit)

## Files Created/Modified

- `demo/garvan-demo.sh` — Relations section: findhom (Gauss AGM), findhomcombo (theta fallback), findnonhomcombo (Watson)

## Decisions Made

- Applied theta fallback for findhomcombo: original U_{5,6} example uses nested sum `sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)` which throws "std::get: wrong index for variant" — REPL does not support nested sum. Fallback: `findhomcombo(theta3(subs_q(q,2),100)^2, [theta3(q,100), theta4(q,100)], 2, 0)` (produces X₁²)

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] findhomcombo nested sum fails — applied theta fallback**
- **Found during:** Task 1 verification
- **Issue:** `sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)` causes parse/eval error (std::get variant)
- **Fix:** Replaced with theta-based example per plan fallback: `findhomcombo(theta3(subs_q(q,2),100)^2, [theta3(q,100), theta4(q,100)], 2, 0)`
- **Files modified:** demo/garvan-demo.sh
- **Verification:** Demo runs; findhomcombo outputs X₁²
- **Committed in:** 71efad1

---

**Total deviations:** 1 auto-fixed (blocking)
**Impact on plan:** Plan explicitly provided fallback for nested sum failure. Applied as specified.

## Issues Encountered

None — plan anticipated nested sum failure and documented theta fallback.

## User Setup Required

None.

## Next Phase Readiness

- Relations demo complete; DEMO-04 satisfied
- Phase 15 (Sifting and product identities) can proceed

## Self-Check

- [x] demo/garvan-demo.sh has three Relations run blocks
- [x] findhom outputs Gauss AGM relations
- [x] findhomcombo runs (theta fallback)
- [x] findnonhomcombo outputs Watson (343, 49)
- [x] Commit 71efad1 exists

---
*Phase: 14-relations-demo*
*Completed: 2026-02-25*
