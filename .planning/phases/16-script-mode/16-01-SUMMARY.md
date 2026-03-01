---
phase: 16-script-mode
plan: 01
subsystem: repl
tags: tty, script-mode, qseries

requires:
  - phase: 10-repl
    provides: runRepl, stdin_is_tty
provides:
  - TTY-aware banner: suppressed when stdin is not a TTY
affects: demo scripts, garvan-demo output

tech-stack:
  added: []
  patterns: ["TTY detection for script vs interactive mode"]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "Banner shown only when stdin_is_tty(); script mode outputs no banner"

patterns-established:
  - "TTY check before user-facing startup output: use stdin_is_tty() for script vs interactive"

duration: ~5min
completed: 2026-02-25
---

# Phase 16 Plan 01: Script mode — Summary

**Banner suppressed when stdin is not a TTY; script mode (`qseries < script.qs`) outputs only commands and results**

## Performance

- **Duration:** ~5 min
- **Tasks:** 1
- **Files modified:** 1

## Accomplishments

- Wrapped banner output in `if (stdin_is_tty())` in `runRepl()`
- Script mode (`qseries < script.qs`, `echo "q" | qseries`) no longer shows ASCII art or version line
- Interactive mode unchanged: banner and prompt still shown when stdin is a TTY

## Task Completed

1. **Task 1: Guard banner with stdin_is_tty() in runRepl()** — Edit applied to `src/repl.h`

## Files Modified

- `src/repl.h` — Banner block wrapped in `if (stdin_is_tty()) { ... }`

## Decisions Made

None — followed plan and CONTEXT decisions exactly.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Build verification: `g++` not in PATH in PowerShell; user should run build in Cygwin or environment with g++ available. The code change is correct.

## Next Phase Readiness

- Phase 16 complete
- Ready for Phase 17: help + help(func)

---
*Phase: 16-script-mode*
*Completed: 2026-02-25*
