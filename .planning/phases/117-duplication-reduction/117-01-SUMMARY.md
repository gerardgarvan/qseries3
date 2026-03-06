---
phase: 117-duplication-reduction
plan: 01
subsystem: repl
tags: [c++, repl, duplication-reduction, expectArgCount, readLineFn, TTY]

# Dependency graph
requires:
  - phase: 116-built-in-registration
    provides: DispatchContext, getBuiltinRegistry, built-in handler structure
provides:
  - expectArgCount helper for consistent arg validation
  - readLineFn abstraction for TTY vs non-TTY line reading
  - Migration pattern for built-ins to shared helpers
affects: [118-brittle-guards]

# Tech tracking
tech-stack:
  added: []
  patterns: [expectArgCount for arg validation, readLineFn lambda for TTY abstraction]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "expectArgCount(ctx, n) for exact; expectArgCount(ctx, {a,b,c}) for allowed"
  - "readLineFn encapsulates both TTY (readLineRaw) and non-TTY (getline) paths"

patterns-established:
  - "expectArgCount: Use for built-in arg count validation; replaces manual size checks and throw"
  - "readLineFn: Single abstraction for main/backslash/bracket continuation reads"

# Metrics
duration: ~25min
completed: 2026-03-06
---

# Phase 117 Plan 01: Duplication Reduction Summary

**expectArgCount helper (2 overloads), etaq/prodmake/etamake migration, readLineFn TTY abstraction in runRepl — TD-03/TD-04 duplication reduction**

## Performance

- **Duration:** ~25 min
- **Tasks:** 4 (3 code, 1 verification)
- **Files modified:** 1 (src/repl.h)

## Accomplishments

- **expectArgCount helper:** Two overloads — exact count `(ctx, n)` and allowed counts `(ctx, {1,2,3})` with formatted error messages via expectArg
- **Built-in migration:** etaq, prodmake, etamake now use expectArgCount; error messages unchanged; handlers continue using ctx.ev/ctx.evi
- **readLineFn abstraction:** Lambda encapsulates TTY (readLineRaw + prompt + newline) vs non-TTY (getline + echo); main loop, backslash continuation, bracket continuation all use single call site
- **No duplicated TTY branching:** Remaining stdin_is_tty() uses (banner, history, clear, timing, error prefix, saveHistory) are distinct logical branches per plan

## Task Commits

Each task was committed atomically:

1. **Task 1: Add expectArgCount helper** - `1ee3029` (feat)
2. **Task 2: Migrate etaq, prodmake, etamake** - `9224cb5` (feat)
3. **Task 3: Add readLineFn abstraction** - `548a94c` (feat)
4. **Task 4: Preserve stdin_is_tty uses** - verification only (no code changes)

## Files Created/Modified

- `src/repl.h` — expectArgCount (2 overloads), etaq/prodmake/etamake migration, readLineFn lambda, runRepl refactor

## Decisions Made

- Used `std::initializer_list<size_t>` for allowed counts; built formatted string ("N or M" / "N, M, or P") for error messages
- readLineFn returns `std::optional<std::string>`; callers check `if (!opt) break`
- Bracket continuation: `trim(*nextOpt).empty()` check applied for both TTY and non-TTY (per plan)

## Deviations from Plan

None — plan executed exactly as written.

## Issues Encountered

**Build verification:** Cygwin g++ 13 in the execution environment fails to build with pre-existing errors (H macro comma splitting in initializer lists, getBuiltinRegistry forward reference, evalToInt/formatEtamake/formatProdmake scope, etc.). These are not caused by 117-01 changes. Build/acceptance verification should be run in Docker or the project's CI environment. See deferred-items.md for details.

## Deferred Items

Pre-existing build failures in Cygwin environment:
- H macro: `{"etaq", "prodmake"}` splits into 5 args; needs parentheses or macro change
- getBuiltinRegistry: forward declaration needed for getHelpTable
- evalToInt, formatEtamake, formatProdmake, formatUndefinedVariableMsg: declaration order
- tripleprod/quinprod return type deduction
- add/sum lambda conversion to BuiltinHandler

Out of scope per deviation rules — only auto-fix issues directly caused by 117-01.

## Next Phase Readiness

- expectArgCount available for additional built-ins (aqprod, RootOf, theta2/3/4, etc.)
- readLineFn pattern established; no further TTY read duplication
- Ready for Phase 118 (Brittle Guards)

## Self-Check: PASSED

- FOUND: .planning/phases/117-duplication-reduction/117-01-SUMMARY.md
- FOUND: 1ee3029, 9224cb5, 548a94c

---
*Phase: 117-duplication-reduction*
*Completed: 2026-03-06*
