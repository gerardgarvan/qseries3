---
phase: 28-arrow-key-line-navigation
plan: 01
subsystem: repl
tags: [readline, terminal, tty, arrow-keys, tab-completion]

# Dependency graph
requires:
  - phase: 20-tab-completion
    provides: readLineRaw, handleTabCompletion, RawModeGuard
provides:
  - Cursor-aware line editing in readLineRaw (TTY only)
  - Left/right arrow navigation within input line
  - Tab completion at cursor position
affects: [repl, script-mode]

# Tech tracking
tech-stack:
  added: []
  patterns: [redrawLineRaw for full line redraw + cursor reposition, ESC sequence parsing]

key-files:
  created: [tests/acceptance-arrow-keys.sh]
  modified: [src/repl.h, Makefile]

key-decisions:
  - "redrawLineRaw defined before handleTabCompletion so both readLineRaw and handleTabCompletion can call it"
  - "Acceptance test uses script to simulate TTY; skips gracefully if script unavailable; tries both DEL and BS for backspace"

patterns-established:
  - "ESC [ D / ESC [ C parsing for arrow keys; consume and ignore other ESC [...] sequences"
  - "Full line redraw on cursor move, insert, delete for correct display"

# Metrics
duration: ~20min
completed: 2026-02-26
---

# Phase 28 Plan 01: Arrow-key Line Navigation Summary

**Left/right arrow-key line navigation in readLineRaw (TTY only) with cursor-aware editing and Tab completion**

## Performance

- **Duration:** ~20 min
- **Started:** 2026-02-26T18:58:00Z (est)
- **Completed:** 2026-02-26T19:18:00Z (est)
- **Tasks:** 3
- **Files modified:** 3

## Accomplishments

- Cursor position (`pos`) tracked in readLineRaw; typing and backspace operate at cursor
- ESC [ D (left) and ESC [ C (right) escape sequences parsed; cursor moves within line bounds
- redrawLineRaw helper redraws prompt + line and repositions cursor via "\033[N D"
- handleTabCompletion made cursor-aware: finds word at cursor, completes, updates line and pos
- Script mode (non-TTY) unchanged — getline path used, no arrow handling
- acceptance-arrow-keys.sh verifies arrow-left + insert and arrow-left + backspace via script/PTY

## Task Commits

Each task was committed atomically:

1. **Task 1: Add cursor position and escape-sequence handling** - `f8b59c1` (feat)
2. **Task 2: Make handleTabCompletion cursor-aware** - `389f6bc` (feat)
3. **Task 3: Add acceptance test for arrow-key navigation** - `674f827` (test)

## Files Created/Modified

- `src/repl.h` - redrawLineRaw, cursor pos, ESC parsing, cursor-aware backspace/insert/Tab
- `tests/acceptance-arrow-keys.sh` - TTY-simulated tests for arrow + insert and arrow + backspace
- `Makefile` - acceptance-arrow-keys target

## Decisions Made

- Defined redrawLineRaw as inline function before handleTabCompletion so both callers share it
- Acceptance test uses `printf ... | script -q -c "$BIN"` so qseries receives input via PTY (TTY)
- Backspace test tries both 127 (DEL) and 8 (BS) since terminals differ; run_script wrapper adds timeout

## Deviations from Plan

None - plan executed as written. Minor implementation note: backspace test tries both 127 and 8 for portability; plan specified 127.

## Issues Encountered

- Git commit -m fails with "option trailer requires a value" (global config); used gsd-tools commit successfully
- g++/make not in PATH when running from PowerShell; used Cygwin bash -l -c for builds and tests

## Self-Check

- [x] src/repl.h contains pos, redrawLineRaw, handleTabCompletion(line, pos, env)
- [x] tests/acceptance-arrow-keys.sh exists
- [x] Commits f8b59c1, 389f6bc, 674f827 exist

## Next Phase Readiness

- REPL-OPTS-02 satisfied: arrow keys move cursor; editing works at cursor
- Phase 29 (optional-arg audit) can proceed

---
*Phase: 28-arrow-key-line-navigation*
*Completed: 2026-02-26*
