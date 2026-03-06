---
phase: 111-ergonomics-polish
plan: 02
subsystem: repl
tags: [posix, termios, sigaction, signal-handling, cygwin]

# Dependency graph
requires: []
provides:
  - SIGINT handler that restores termios before re-raising (ERGO-03)
  - Terminal usable after Ctrl+C during raw mode (POSIX/CYGWIN)
affects: [111-01]

# Tech tracking
tech-stack:
  added: []
  patterns: [async-signal-safe handler pattern, sigaction for SIGINT]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "Use sys/signal.h on Cygwin for struct sigaction (C++ header resolution)"

patterns-established:
  - "SIGINT handler: tcsetattr, signal(SIG_DFL), raise(SIGINT) — async-signal-safe only"

# Metrics
duration: ~15min
completed: 2026-03-06
---

# Phase 111 Plan 02: SIGINT Handler with termios Restore Summary

**SIGINT handler on POSIX/CYGWIN that restores original termios before re-raising, so Ctrl+C during REPL raw mode leaves the terminal usable**

## Performance

- **Duration:** ~15 min
- **Started:** 2026-03-06T18:45:00Z
- **Completed:** 2026-03-06T18:55:00Z
- **Tasks:** 1
- **Files modified:** 1

## Accomplishments

- ERGO-03: Ctrl+C during raw mode (readLineRaw) restores terminal; shell prompt accepts input normally after
- `g_repl_orig_termios` and `repl_sigint_handler` added in repl.h (POSIX/CYGWIN only)
- sigaction installed in runRepl when stdin_is_tty() and tcgetattr succeeds
- Handler uses async-signal-safe calls only: tcsetattr, signal, raise

## Task Commits

Each task was committed atomically:

1. **Task 1: SIGINT handler with termios restore (ERGO-03)** - `09c984c` (feat)

## Files Created/Modified

- `src/repl.h` - Added g_repl_orig_termios, repl_sigint_handler, sigaction setup in runRepl; includes csignal and sys/signal.h

## Decisions Made

- Use `<sys/signal.h>` for struct sigaction on Cygwin — standard `<signal.h>`/`<csignal>` caused C++ to treat sigaction as an incomplete local type
- Added `_POSIX_C_SOURCE 200809L` before POSIX headers for compatibility

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Cygwin C++ struct sigaction resolution**
- **Found during:** Task 1 (build verification)
- **Issue:** With `<csignal>` and `<signal.h>` only, g++ on Cygwin reported "struct sigaction has incomplete type" — C++ name resolution created a local incomplete type
- **Fix:** Include `<sys/signal.h>` for struct sigaction definition on Cygwin; added `_POSIX_C_SOURCE 200809L`
- **Files modified:** src/repl.h
- **Verification:** make -j succeeds; dist/qseries.exe builds
- **Committed in:** 09c984c (Task 1 commit)

---

**Total deviations:** 1 auto-fixed (blocking)
**Impact on plan:** Necessary for Cygwin build. Plan scope unchanged.

## Issues Encountered

None beyond the Cygwin header fix documented in deviations.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- ERGO-03 complete; terminal usable after Ctrl+C during raw mode
- Plan 111-01 (bracket continuation, EOF guard, TTY audit) can proceed independently

## Self-Check: PASSED

- FOUND: .planning/phases/111-ergonomics-polish/111-02-SUMMARY.md
- FOUND: commit 09c984c

---
*Phase: 111-ergonomics-polish*
*Completed: 2026-03-06*
