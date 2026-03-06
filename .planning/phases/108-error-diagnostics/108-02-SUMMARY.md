---
phase: 108-error-diagnostics
plan: 02
subsystem: repl
tags: [parse-error, caret, maple-format, stderr, script-mode, wasm]

# Dependency graph
requires:
  - phase: 108-01
    provides: Maple format, runtimeErr, expectArg, ordinal
provides:
  - formatParseErrorWithSnippet (GCC/Clang-style source + caret)
  - Parse errors: source line, caret, Error (in parser) line N, col M
  - Script mode: line N: or line ?: prefix
  - WASM evaluate(): Maple format (no "error:" prefix)
  - acceptance-error-diagnostics.sh
affects: [108-error-diagnostics]

# Tech tracking
tech-stack:
  added: []
  patterns: [parse error snippet formatting, script-mode line prefix]

key-files:
  created: [tests/acceptance-error-diagnostics.sh]
  modified: [src/repl.h, src/main_wasm.cpp, tests/qol-acceptance.sh]

key-decisions:
  - "WASM Option B: strip parser: prefix, prepend Error (in parser) for consistency (no snippet in WASM)"

patterns-established:
  - "Parse error flow: catch parser: prefix → formatParseErrorWithSnippet(trimmed, e.what())"
  - "Script prefix: line N: when inputLineNum>0, line ?: when 0"

# Metrics
duration: 27min
completed: 2026-03-06
---

# Phase 108 Plan 02: Error Diagnostics (Parse Snippet) Summary

**Parse error formatting with GCC/Clang-style source snippet and caret; Maple format; script-mode line prefix; WASM consistency; Phase 108 acceptance tests.**

## Performance

- **Duration:** ~27 min
- **Tasks:** 3
- **Files modified:** 4 (3 modified, 1 created)

## Accomplishments
- Parse errors show source line + caret + `Error, (in parser) line N, col M: message`
- Script mode prepends `line N:` when known, `line ?:` when unknown
- WASM `evaluate()` outputs Maple format (parse: `Error, (in parser) ...`; runtime unchanged)
- `acceptance-error-diagnostics.sh` passes all 8 tests
- `qol-acceptance.sh` Phase 21 updated for Phase 108 format

## Task Commits

1. **Task 1: Parse error formatting helpers** - `11e79a3` (feat)
2. **Task 2: Update REPL catch block and script line prefix** - `0f1b1f9` (feat)
3. **Task 3: WASM error output and acceptance tests** - `55c226e` (feat)

## Files Created/Modified
- `src/repl.h` - parseParserMessage, getLineAt, caretLine, formatParseErrorWithSnippet; catch block uses formatParseErrorWithSnippet, script prefix
- `src/main_wasm.cpp` - parse errors: Error (in parser) + msg; remove error: prefix
- `tests/acceptance-error-diagnostics.sh` - Phase 108 parse/runtime/script tests
- `tests/qol-acceptance.sh` - Phase 21: parse/runtime grep patterns; etaq() for runtime test

## Decisions Made
- None beyond plan—WASM Option B used per plan (Maple format, no snippet).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] qol-acceptance Phase 21 runtime test used etaq(1)**
- **Found during:** Task 3 (acceptance tests)
- **Issue:** etaq(1) succeeds (returns series); test expected runtime error
- **Fix:** Changed to etaq() (0 args) to trigger expectArg runtime error
- **Files modified:** tests/qol-acceptance.sh
- **Committed in:** 55c226e (Task 3)

---

**Total deviations:** 1 auto-fixed (Rule 1)
**Impact:** Test correctness; no scope creep.

## Issues Encountered
- None

## Next Phase Readiness
- Phase 108 Error Diagnostics complete (plans 01 + 02)
- Ready for Phase 109 Help Extensions

## Self-Check: PASSED
- 108-02-SUMMARY.md exists
- Commits 11e79a3, 0f1b1f9, 55c226e exist

---
*Phase: 108-error-diagnostics*
*Plan: 02*
*Completed: 2026-03-06*
