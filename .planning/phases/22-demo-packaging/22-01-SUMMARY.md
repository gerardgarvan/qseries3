---
phase: 22-demo-packaging
plan: 01
subsystem: packaging
tags: [make, bash, distribution, qseries-demo]

# Dependency graph
requires: []
provides:
  - qseries-demo/ distributable folder (binary + garvan-demo.sh + README.md)
  - make package-demo target
  - DIR-based binary resolution in garvan-demo.sh
affects: [demo, distribution, QOL-08]

# Tech tracking
tech-stack:
  added: []
  patterns: [BASH_SOURCE-based script directory resolution, flat distribution layout]

key-files:
  created: [demo/README-dist.md]
  modified: [demo/garvan-demo.sh, Makefile]

key-decisions:
  - "Unified garvan-demo.sh with DIR-based BIN resolution for both flat (qseries-demo/) and nested (dist/demo/) layouts"
  - "demo/README-dist.md as distribution README template, copied as qseries-demo/README.md by package-demo"

patterns-established:
  - "DIR-based binary resolution: use dirname BASH_SOURCE for CWD-independent script execution"

# Metrics
duration: 8min
completed: 2026-02-25
---

# Phase 22 Plan 01: Demo Packaging Summary

**make package-demo produces qseries-demo/ with binary, garvan-demo.sh, and README.md; demo script uses DIR-based resolution so it runs regardless of CWD**

## Performance

- **Duration:** ~8 min
- **Tasks:** 3
- **Files modified:** 3

## Accomplishments

- DIR-based binary resolution in garvan-demo.sh so script works in flat (qseries-demo/) and nested (dist/demo/) layouts
- demo/README-dist.md distribution template with Contents, Requirements (Bash on Windows), How to run, REPL usage, optional rebuild
- package-demo target copies binary, garvan-demo.sh, and README-dist.md as README.md into qseries-demo/
- make demo and cd qseries-demo && bash garvan-demo.sh both succeed

## Task Commits

Each task was committed atomically:

1. **Task 1: DIR-based binary resolution in garvan-demo.sh** - `25840a5` (feat)
2. **Task 2: Create demo/README-dist.md** - `1732656` (feat)
3. **Task 3: Extend Makefile package-demo** - `77dcd13` (feat)

## Files Created/Modified

- `demo/garvan-demo.sh` - BIN resolution from dirname BASH_SOURCE, fallback to $DIR/../qseries.exe
- `demo/README-dist.md` - Distribution README template
- `Makefile` - package-demo adds cp garvan-demo.sh and cp README-dist.md README.md

## Decisions Made

None - followed plan as specified.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Git commit fails with "option trailer requires a value" when run from shell (global config); used gsd-tools commit which succeeds.
- make and git not in PowerShell PATH; used Cygwin bash for all builds and commits.

## Next Phase Readiness

- QOL-08 satisfied: users can run make package-demo, zip qseries-demo/, and share a self-contained demo folder
- make demo unchanged; nested layout still works

## Self-Check: PASSED

- SUMMARY.md exists
- demo/README-dist.md exists
- Commits 25840a5, 1732656, 77dcd13 present in git log

---
*Phase: 22-demo-packaging*
*Completed: 2026-02-25*
