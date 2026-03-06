---
phase: 87-integration-testing
plan: 01
subsystem: testing
tags: [makefile, acceptance-tests, bash, regression, integration]
requires: []
provides:
  - acceptance-all Makefile target for full regression
  - BIN/QSERIES resolution in integration-tcore.sh and acceptance-bailey.sh
affects: [87-02 integration testing orchestration]

tech-stack:
  added: []
  patterns: [BIN fallback chain: dist/qseries.exe → dist/qseries → qseries.exe → qseries]

key-files:
  created: []
  modified: [Makefile, tests/integration-tcore.sh, tests/acceptance-bailey.sh]

key-decisions: []

patterns-established:
  - "BIN fallback: test scripts resolve binary via dist/qseries.exe first, then qseries.exe in repo root"

duration: ~3min
completed: 2026-03-02
---

# Phase 87 Plan 01: Full Regression Orchestration Summary

**acceptance-all Makefile target plus BIN/QSERIES fallback in integration-tcore.sh and acceptance-bailey.sh for dist/ binary resolution**

## Performance

- **Duration:** ~3 min
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments

- Added `acceptance-all` Makefile target that runs: acceptance-maple, run-all.sh, acceptance-modforms, acceptance-theta-ids
- integration-tcore.sh: BIN fallback chain (dist/qseries.exe, dist/qseries, qseries.exe, qseries); QSERIES="$BIN --no-banner"
- acceptance-bailey.sh: same BIN fallback chain; check() uses "$BIN" instead of hardcoded ./qseries.exe

## Task Commits

1. **Task 1: Add acceptance-all target and fix BIN in integration-tcore.sh** - `8e8240c` (feat)
2. **Task 2: Fix BIN resolution in acceptance-bailey.sh** - `a1344ee` (fix)

## Files Created/Modified

- `Makefile` - acceptance-all target, .PHONY update
- `tests/integration-tcore.sh` - BIN resolution before QSERIES
- `tests/acceptance-bailey.sh` - BIN fallback chain, check() uses "$BIN"

## Decisions Made

None - followed plan as specified.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None. Verification: `make acceptance-all` runs all four stages; integration-tcore.sh and acceptance-bailey.sh pass when binary is at dist/qseries.exe.

## Self-Check: PASSED

- Makefile contains acceptance-all target
- tests/integration-tcore.sh contains dist/qseries
- tests/acceptance-bailey.sh contains dist/qseries
- Commits 8e8240c, a1344ee exist
