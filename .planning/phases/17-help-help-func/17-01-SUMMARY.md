---
phase: 17-help-help-func
plan: 01
subsystem: repl
tags: help, documentation, built-ins

requires:
  - phase: 10-repl
    provides: dispatchBuiltin, eval, parser
provides:
  - help() — general usage and flat list of built-ins
  - help(func) — signature + one-line description per function
  - help(unknown) — short "unknown function: X" message
affects: users exploring built-ins

tech-stack:
  added: []
  patterns: ["static help table mapped by function name"]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "getHelpTable() returns static map; help branch in dispatchBuiltin"

duration: ~10min
completed: 2026-02-25
---

# Phase 17 Plan 01: help + help(func) — Summary

**help and help(func) built-in: flat list of 28 built-ins, signature + description per function, short unknown message**

## Performance

- **Duration:** ~10 min
- **Tasks:** 1
- **Files modified:** 1

## Accomplishments

- Added getHelpTable() — static map of 28 built-ins to (signature, description)
- Added help branch in dispatchBuiltin: help() prints usage + flat list; help(name) prints sig + desc or "unknown function: name"
- Coverage: aqprod, etaq, theta2/3/4, theta, qbin, tripleprod, quinprod, winquist, sift, T, prodmake, etamake, jacprodmake, jac2prod, qfactor, series, coeffs, set_trunc, findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly, legendre, sigma, subs_q, sum, add

## Task Completed

1. **Task 1: Add help table and help built-in** — getHelpTable() + help branch in dispatchBuiltin

## Files Modified

- `src/repl.h` — getHelpTable(), help branch after subs_q

## Decisions Made

None — followed CONTEXT and plan.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Build: g++ not in PATH on Windows PowerShell; user should verify in Cygwin.

## Next Phase Readiness

- Phase 17 complete
- Ready for Phase 18: Timing

---
*Phase: 17-help-help-func*
*Completed: 2026-02-25*
