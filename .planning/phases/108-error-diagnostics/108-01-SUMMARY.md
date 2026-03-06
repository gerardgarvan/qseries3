---
phase: 108-error-diagnostics
plan: 01
subsystem: repl
tags: [error-handling, maple-format, expectArg, ordinal]

# Dependency graph
requires: []
provides:
  - Maple-style runtime error format: Error, (in func) message
  - ordinal(n) helper for 1st, 2nd, 3rd, 4th, etc.
  - expectArg(n, name, expected, received) for argument errors
  - etaq, prodmake, etamake migrated to expectArg
affects: [108-02]

# Tech tracking
tech-stack:
  added: []
  patterns: [Maple-style errors, expectArg for built-in argument validation]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "Use ordinal(1) with 'arguments' for arity errors (e.g. expectArg(1, 'arguments', '1, 2, or 3', N))"
  - "Keep runtimeErr for non-argument errors (e.g. modulus cannot be zero); expectArg only for argument/arity"

patterns-established:
  - "Maple format: Error, (in func) message"
  - "Argument errors: invalid input: expects its Nth argument, name, to be X, but received Y"

# Metrics
duration: ~15min
completed: 2026-03-06
---

# Phase 108 Plan 01: Maple Format + expectArg Summary

**Maple-style runtime errors and expectArg helper; etaq, prodmake, etamake migrated to use expectArg for arity/argument errors**

## Performance

- **Duration:** ~15 min
- **Tasks:** 2
- **Files modified:** 1 (src/repl.h)

## Accomplishments

- runtimeErr now returns `Error, (in func) message` (Maple format)
- ordinal(n) helper returns "1st", "2nd", "3rd", "4th", etc. (11th/12th/13th = "th")
- expectArg(n, name, expected, received) produces `invalid input: expects its Nth argument, name, to be expected, but received received`
- etaq, prodmake, etamake use expectArg for their arity errors

## Task Commits

Each task was committed atomically:

1. **Task 1: Maple format + ordinal + expectArg helpers** - `8efbbff` (feat)
2. **Task 2: Migrate etaq, prodmake, etamake to expectArg** - `d67436d` (feat)

## Files Created/Modified

- `src/repl.h` - runtimeErr Maple format; ordinal, expectArg helpers; etaq, prodmake, etamake argument errors

## Decisions Made

- Use expectArg(1, "arguments", "1, 2, or 3", std::to_string(args.size()) + " arguments") for etaq arity to fit the helper's signature while conveying arity info
- Non-argument errors (e.g. modulus cannot be zero) remain with runtimeErr only — no "invalid input:" prefix per CONTEXT discretion

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Build verification (`make`, `echo 'etaq()' | ./qseries`) could not be run in this session: Cygwin toolchain (g++, make) not available in PowerShell. User should verify in their Cygwin environment:
  - `make` (or `g++ -std=c++20 -O2 -o dist/qseries.exe src/main.cpp`)
  - `echo 'etaq()' | ./dist/qseries.exe` — should show `Error, (in etaq) invalid input: expects its 1st argument...`
  - `echo 'prodmake(x,50)' | ./dist/qseries.exe` (x undefined) — should show `Error, (in prodmake)...`

## Next Phase Readiness

- Plan 108-02 can proceed (parse error snippet, caret, etc.)
- All runtime throw sites now produce Maple format via runtimeErr; ev/evi wrappers rethrow with runtimeErr so expectArg messages flow through correctly

## Self-Check

- [x] src/repl.h contains runtimeErr Maple format
- [x] src/repl.h contains ordinal and expectArg
- [x] etaq, prodmake, etamake use expectArg
- [x] Commits 8efbbff, d67436d exist

---
*Phase: 108-error-diagnostics*
*Completed: 2026-03-06*
