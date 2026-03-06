---
phase: 110-input-convenience
plan: 01
subsystem: repl
tags: [repl, tab-completion, levenshtein, typo-suggestions]

# Dependency graph
requires:
  - phase: 109-help-extensions
    provides: HelpEntry.sig, getHelpTable structure
provides:
  - formatUndefinedVariableMsg for undefined-variable typo suggestions
  - Signature hint on single-match tab completion for built-ins
  - Documented single (line, pos) buffer design
  - tests/acceptance-input-convenience.sh
affects: [repl, error-diagnostics]

# Tech tracking
tech-stack:
  added: []
  patterns: [formatUndefinedVariableMsg using getCompletionCandidates + levenshteinDistance, sig display in handleTabCompletion]

key-files:
  created: [tests/acceptance-input-convenience.sh]
  modified: [src/repl.h]

key-decisions:
  - "Use getCompletionCandidates(env) for undefined-var suggestions (not just getHelpTable) — suggests user vars like rr"
  - "Print sig on newline below prompt for single built-in match; user vars complete without sig"
  - "Levenshtein threshold d<=3, up to 2 suggestions — matches unknown built-in behavior"

patterns-established:
  - "formatUndefinedVariableMsg(env, name): centralized undefined-variable error formatting with typo suggestions"
  - "Single (line, pos) buffer: tab and history share one editing buffer; no second buffer"

# Metrics
duration: ~15min
completed: 2026-03-06
---

# Phase 110 Plan 01: Input Convenience Summary

**Typo suggestions for undefined variables via formatUndefinedVariableMsg, signature hint on tab completion for built-ins, and documented single (line, pos) buffer design.**

## Performance

- **Duration:** ~15 min
- **Tasks:** 3
- **Files modified:** 2 (repl.h, tests/acceptance-input-convenience.sh)

## Accomplishments
- formatUndefinedVariableMsg(env, name) using getCompletionCandidates and levenshteinDistance (d<=3, up to 2 suggestions)
- All 4 undefined-variable throw sites updated: evalExpr Var, jac2prod, jac2series, jac2eprod
- handleTabCompletion prints HelpEntry.sig on next line when single match is built-in
- Block comment above readLineRaw documents single (line, pos) buffer shared by tab and history
- acceptance-input-convenience.sh: undefined-var (RR→rr) and unknown built-in (etamke→etamake) tests

## Task Commits

1. **Task 1: formatUndefinedVariableMsg + all 4 throw sites** - `58b8610` (feat)
2. **Task 2: Signature hint in handleTabCompletion** - `b7036c6` (feat)
3. **Task 3: Document single-buffer + acceptance test** - `ba8c01c` (docs)

## Files Created/Modified
- `src/repl.h` - formatUndefinedVariableMsg, 4 throw-site updates, sig hint in handleTabCompletion, single-buffer block comment
- `tests/acceptance-input-convenience.sh` - New acceptance test for undefined-var and unknown-builtin suggestions

## Decisions Made
- Use getCompletionCandidates(env) not getHelpTable() for undefined-var suggestions so user variables (e.g. rr) are suggested
- jac2prod/jac2series use runtimeErr(name, formatUndefinedVariableMsg(...)); evalExpr and jac2eprod use plain formatUndefinedVariableMsg

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None. Build/run verification attempted; Windows PowerShell + Cygwin environment had PATH limitations for make/bash; dist/qseries.exe exists. User can run `make` and `bash tests/acceptance-input-convenience.sh` in their Cygwin shell.

## Self-Check

- [x] src/repl.h modified (formatUndefinedVariableMsg, handleTabCompletion, readLineRaw comment)
- [x] tests/acceptance-input-convenience.sh created
- [x] Commits 58b8610, b7036c6, ba8c01c exist

## Self-Check: PASSED

---
*Phase: 110-input-convenience*
*Plan: 01*
*Completed: 2026-03-06*
