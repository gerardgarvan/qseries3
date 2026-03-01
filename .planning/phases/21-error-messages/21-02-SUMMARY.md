---
phase: 21-error-messages
plan: 02
subsystem: repl
tags: [repl, error-messages, script-mode, C++]

# Dependency graph
requires:
  - phase: 21-01
    provides: parse error position (line, col) in e.what()
provides:
  - runtimeErr helper for built-in error prefixing
  - ev/evi wrappers in dispatchBuiltin rethrow with func name
  - script-mode "line N:" prefix in REPL error display
affects: []

# Tech tracking
tech-stack:
  added: []
  patterns: [runtimeErr(func, msg) for built-in errors, inputLineNum for script line tracking]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "Line number = Nth non-empty command (blank lines don't increment)"
  - "Line prefix only when !stdin_is_tty() (script/pipe mode)"

patterns-established:
  - "Built-in errors: func: message via runtimeErr"
  - "Script errors: error: line N: message"

# Metrics
duration: 15min
completed: 2026-02-25
---

# Phase 21 Plan 02: Runtime Error Standardization Summary

**Standardized built-in error messages with function name prefix and added script-mode line number to REPL error display (QOL-07)**

## Performance

- **Duration:** ~15 min
- **Tasks:** 2
- **Files modified:** 1 (src/repl.h)
- **Commits:** 1 (344b389)

## Accomplishments

- Built-in errors show function name prefix (e.g. `etaq: expected etaq(k,T) or etaq(q,k,T), got 1 arguments`)
- Script mode errors include line number: `error: line 2: parser: line 1, col 5: expected :=`
- runtimeErr helper and ev/evi wrappers already present; Task 2 added inputLineNum and catch-block formatting
- All 10 acceptance tests pass

## Task Commits

1. **Task 1: Add runtimeErr helper and standardize built-in error messages** — No code changes (already implemented in codebase: runtimeErr, ev/evi wrappers, dispatchBuiltin uses runtimeErr for all throws)
2. **Task 2: Add script-mode line number and format error display** — `344b389` (feat)

## Files Created/Modified

- `src/repl.h` — Added inputLineNum tracking, increment per non-empty command; prepend "line N: " in catch block when !stdin_is_tty()

## Decisions Made

- Line number = Nth non-empty command; blank lines don't increment
- Line prefix only when stdin is not TTY (script/pipe mode)

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None

## Verification

- `etaq(1)` → `error: line 1: etaq: expected etaq(k,T) or etaq(q,k,T), got 1 arguments` ✓
- `prodmake(x, 50)` with x undefined → `error: line 1: prodmake: undefined variable: x` ✓
- Script with parse error on line 2 → `error: line 2: parser: line 1, col 5: expected :=` ✓
- tests/acceptance.sh: 10 passed ✓

## Self-Check

- [x] src/repl.h modified
- [x] Commit 344b389 exists
