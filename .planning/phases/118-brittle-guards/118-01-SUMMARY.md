---
phase: 118-brittle-guards
plan: 01
subsystem: repl
tags: [repl, constants, error-handling, td-05, td-06]

# Dependency graph
requires: []
provides:
  - kPowExponentLimit, kMaxHistory, kMaxContinuations, kMaxBracketContinuations, kLevenshteinSuggestionThreshold
  - q-guard in dispatchBuiltin with clear "q not set" error
affects: [repl, eval, dispatch]

# Tech tracking
tech-stack:
  added: []
  patterns: [named constants for magic numbers, explicit guard before env lookup]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "Use file-scope static constexpr for all five constants (kPowExponentLimit as int64_t, others size_t/int)"
  - "Guard uses env.env.find('q') == env.env.end() before at('q') — clear runtime_error"
  - "Pow error message uses std::to_string(kPowExponentLimit) for dynamic limit display"

patterns-established:
  - "Named constants for REPL configuration limits (Phase 118 TD-06)"
  - "Explicit q-guard in dispatchBuiltin before any built-in (Phase 118 TD-05)"

# Metrics
duration: 15min
completed: 2026-03-06
---

# Phase 118 Plan 01: Brittle Guards Summary

**Named constants for pow limit, history, continuations, and Levenshtein threshold; q-guard in dispatchBuiltin with clear "q not set" error instead of std::out_of_range**

## Performance

- **Duration:** ~15 min
- **Started:** 2026-03-06
- **Completed:** 2026-03-06
- **Tasks:** 3
- **Files modified:** 1 (src/repl.h)

## Accomplishments
- Five named constants (kPowExponentLimit, kMaxHistory, kMaxContinuations, kMaxBracketContinuations, kLevenshteinSuggestionThreshold) added at file scope
- q-guard in dispatchBuiltin: `if (env.env.find("q") == env.env.end()) throw std::runtime_error("q not set");`
- All six magic-number locations replaced with constants (pow exponent, maxHistory, maxContinuations, maxBracketContinuations, Levenshtein in dispatchBuiltin and formatUndefinedVariableMsg)

## Task Commits

Each logical task implemented; combined into one atomic commit due to cohesive changes:

1. **Task 1: Add named constants** — `e62d8c2` (feat)
2. **Task 2: Guard q in dispatchBuiltin** — `e62d8c2` (feat)
3. **Task 3: Replace magic numbers at 6 locations** — `e62d8c2` (feat)

**Commit:** `e62d8c2` feat(118-01): brittle guards - named constants and q-guard in dispatchBuiltin

## Files Created/Modified
- `src/repl.h` — Added constants block, q-guard, replaced 6 magic numbers

## Decisions Made
- Used `std::to_string(kPowExponentLimit)` in pow error message so limit stays in sync with constant
- Kept local variables (maxHistory, maxContinuations, maxBracketContinuations) in runRepl using the constants for readability

## Deviations from Plan

None — plan executed exactly as written.

## Issues Encountered

**Build verification:** Executor environment (Cygwin via PowerShell) produced compilation errors (e.g., H macro arity, getBuiltinRegistry scope) that appear pre-existing and unrelated to 118-01 changes. The 118-01 edits are confined to: constants block, q-guard, and six in-place literal→constant replacements. User should run `make` and acceptance tests in their environment to verify.

## Verification Commands

```bash
# q unset
printf 'prodmake(q,40)\n' | ./dist/qseries.exe 2>&1 | grep -q "q not set"

# q set
printf 'q:=Series::q(50); prodmake(q,40)\n' | ./dist/qseries.exe 2>&1

# pow limit
printf 'q:=Series::q(50); pow(1+q,10001)\n' | ./dist/qseries.exe 2>&1 | grep -q "exponent"
```

## Next Phase Readiness
- TD-05 and TD-06 addressed; Phase 118 Plan 01 complete
- Ready for next Phase 118 plan or phase completion

## Self-Check

- **Commit e62d8c2:** `git log --oneline | grep e62d8c2` — FOUND
- **Commit 5602124:** `git log --oneline | grep 5602124` — FOUND
- **118-01-SUMMARY.md:** `.planning/phases/118-brittle-guards/118-01-SUMMARY.md` — FOUND
- **Acceptance tests:** Executor Cygwin env lacks full PATH (grep); user should run `make` and `./tests/acceptance.sh` locally

---
*Phase: 118-brittle-guards*
*Completed: 2026-03-06*
