---
phase: 96-block-4-parity
plan: 01
subsystem: testing
tags: [maple-checklist, factor, cyclotomic, t8, qseriesdoc]

# Dependency graph
requires:
  - phase: 95-factor-builtin
    provides: factor(expr) built-in with cyclotomic Φ output
provides:
  - Block 4 real test (factor(t8) cyclotomic) in maple-checklist.sh
  - maple-checklist parity with qseriesdoc Output (4)
affects: []

# Tech tracking
tech-stack:
  added: []
  patterns: [run/grep pattern for cyclotomic verification: Φ|Cyclotomic, q⁶|q^6]

key-files:
  created: []
  modified: [tests/maple-checklist.sh]

key-decisions:
  - "Block 4 pass criteria: output must contain Φ or Cyclotomic and q⁶ or q^6 per acceptance-factor.sh pattern"

patterns-established: []

# Metrics
duration: ~5min
completed: 2026-03-03
---

# Phase 96 Plan 01: Block 4 Parity Summary

**Replace maple-checklist Block 4 skip with real factor(t8) cyclotomic test; Block 4 and Block 5 both pass.**

## Performance

- **Duration:** ~5 min
- **Tasks:** 1 completed
- **Files modified:** 1 (tests/maple-checklist.sh)

## Accomplishments

- Block 4 now runs real test: `set_trunc(64)`, `t8 := T(8,8)`, `factor(t8)`
- Pass when output contains Φ/Cyclotomic and q⁶ per qseriesdoc Output (4)
- Block 5 (qfactor(t8, 20)) unchanged and still passes
- SKIP count reduced by 1

## Task Commits

1. **Task 1: Replace Block 4 skip with factor(t8) cyclotomic test** - `faa389e` (feat)

## Files Created/Modified

- `tests/maple-checklist.sh` - Replaced lines 48–49 (skip) with OUT4 capture and grep-based pass/fail for cyclotomic form

## Decisions Made

None - followed plan as specified.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None

## Verification

```text
PASS: Block 4: factor(t8) cyclotomic
PASS: Block 5: qfactor(t8)
```

## Self-Check: PASSED

- [x] tests/maple-checklist.sh modified and contains Block 4 factor(t8) test
- [x] Commit faa389e exists

---
*Phase: 96-block-4-parity*
*Completed: 2026-03-03*
