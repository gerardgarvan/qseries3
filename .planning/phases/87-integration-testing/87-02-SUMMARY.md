---
phase: 87-integration-testing
plan: 02
subsystem: testing
tags: [integration-tests, eta, theta-ids, modforms, acceptance-all]

# Dependency graph
requires:
  - phase: 87-01
    provides: acceptance-all target, BIN resolution pattern
provides:
  - Cross-package integration script (ETA + theta IDs + modforms)
  - integration-eta-theta-modforms Makefile target wired into acceptance-all
affects: [regression-testing, v6.0]

# Tech tracking
tech-stack:
  added: []
  patterns: [BIN fallback chain, check() helper, run() for multi-line input]

key-files:
  created: [tests/integration-eta-theta-modforms.sh]
  modified: [Makefile]

key-decisions:
  - "Use theta3(100) not theta3 in etamake — theta3 is a function"

patterns-established:
  - "Cross-package integration: chain DELTA12, provemodfuncGAMMA0id, jacprodmake→jac2eprod→provemodfuncid in one script"

# Metrics
duration: ~5min
completed: 2026-03-02
---

# Phase 87 Plan 02: Integration ETA-Theta-Modforms Summary

**Cross-package integration script chaining ETA identity prover, modular forms basis, and theta IDs (jac2eprod, provemodfuncid) with Makefile target in acceptance-all**

## Performance

- **Duration:** ~5 min
- **Tasks:** 2 completed
- **Files created:** 1 (tests/integration-eta-theta-modforms.sh)
- **Files modified:** 1 (Makefile)

## Accomplishments

- Created `tests/integration-eta-theta-modforms.sh` with BIN resolution and 5 cross-package tests
- Test 1: ETA + modforms chain — DELTA12 = q*etaq^24, provemodfuncGAMMA0id eta identity
- Test 2: Theta IDs chain — rr→jacprodmake→jac2eprod yields GETA/EETA, provemodfuncid Rogers-Ramanujan form
- Test 3: ETA roundtrip — etamake(theta3(100),100) yields eta product
- Added `integration-eta-theta-modforms` Makefile target and wired into `acceptance-all`

## Task Commits

1. **Task 1: Create integration-eta-theta-modforms.sh** — `a1ac9e2` (feat)
2. **Task 2: Add Makefile target and wire into acceptance-all** — `00c32c3` (feat)

## Files Created/Modified

- `tests/integration-eta-theta-modforms.sh` — New script with BIN resolution, check()/run() helpers, 5 cross-package tests
- `Makefile` — New `integration-eta-theta-modforms` target, added to `acceptance-all` as 5th stage

## Decisions Made

- Use `theta3(100)` (function call) in etamake — theta3 is a built-in function, not a variable
- Pattern for Test 3: match η or `[[` to accept either Unicode eta or internal list format

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Test 3 used theta3 as variable**
- **Found during:** Task 1 (script verification)
- **Issue:** Plan said `etamake(theta3, 100)` but theta3 is undefined — it must be called as `theta3(100)` or `theta3(q,100)`
- **Fix:** Changed to `etamake(theta3(100), 100)` matching acceptance.sh and maple-checklist
- **Files modified:** tests/integration-eta-theta-modforms.sh
- **Verification:** All 5 tests pass
- **Committed in:** a1ac9e2 (Task 1)

---

**Total deviations:** 1 auto-fixed (1 bug)
**Impact on plan:** Fix required for correctness. No scope creep.

## Issues Encountered

None — plan executed with one inline fix for Test 3.

## User Setup Required

None — no external service configuration required.

## Next Phase Readiness

- Cross-package integration tests run as part of `make acceptance-all`
- Full regression (maple-checklist, run-all, modforms, theta-ids, integration-eta-theta-modforms) passes
- Ready for further integration testing or phase 87 completion

## Self-Check: PASSED

- `tests/integration-eta-theta-modforms.sh` exists
- `bash tests/integration-eta-theta-modforms.sh` exits 0
- `make integration-eta-theta-modforms` runs script
- `make acceptance-all` includes and runs integration script
- Commits a1ac9e2 and 00c32c3 present in git log

---
*Phase: 87-integration-testing*
*Completed: 2026-03-02*
