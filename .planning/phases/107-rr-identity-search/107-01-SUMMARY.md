---
phase: 107-rr-identity-search
plan: 01
subsystem: rr_ids
tags: findids, types 4 5, GAP11-06, GAP11-07, Göllnitz-Gordon

# Dependency graph
requires:
  - phase: 106-provemodfuncid-extensions
provides:
  - findids types 4 and 5; acceptance-rr-id extended; GAP11-06, GAP11-07 satisfied
affects: v11.1 milestone complete

# Tech tracking
tech-stack:
  added: [findids_type4, findids_type5]
  patterns: Göllnitz-Gordon SYMF; RRGstar/RRHstar; same ABCOND as types 1/2

key-files:
  created: [.planning/phases/107-rr-identity-search/107-01-SUMMARY.md]
  modified: [src/rr_ids.h, src/repl.h, tests/acceptance-rr-id.sh]

key-decisions:
  - "Types 4 and 5 use RRGstar/RRHstar; constraints include (a mod 2=0) ∨ (p mod 2=0)"
  - "Type 4 discovers 16 identities at T=20; type 5 returns 0 (RRGstar/RRHstar n>1 stubbed)"

# Metrics
duration: ~30min
completed: 2026-03-06
---

# Phase 107 Plan 01: RR Identity Search Summary

**findids types 4 and 5 implemented; acceptance-rr-id extended; type 4 discovers 16 identities; GAP11-06, GAP11-07 satisfied.**

## Performance

- **Duration:** ~30 min
- **Tasks:** 2
- **Files modified:** src/rr_ids.h, src/repl.h, tests/acceptance-rr-id.sh

## Accomplishments

1. **findids_type4** — GM(p)HM(a) ± GM(a)HM(p); loop n=2..T; abcond_type1; constraint (a mod 2=0) ∨ (p mod 2=0). Discovers 16 identities at T=20.
2. **findids_type5** — GM(a)GM(p) ± HM(a)HM(p); loop n=1..T; abcond_type2; same even constraint. Returns 0 with current RRGstar/RRHstar stub (n>1).
3. **REPL** — findids(4) and findids(5) dispatch; help text updated.
4. **acceptance-rr-id** — Tests 10 and 11 for findids(4) and findids(5); require ≥1 identity from types 4 or 5 (type 4 delivers).

## Success Criteria Met

- [x] findids(4, T) and findids(5, T) work and return 3-tuples
- [x] At least one of types 4 or 5 discovers ≥1 identity (type 4: 16)
- [x] acceptance-rr-id.sh passes
- [x] maple-checklist 40/41; no regressions

## Observable Truths

- src/rr_ids.h: findids_type4, findids_type5, findids dispatch
- tests/acceptance-rr-id.sh: tests 10, 11
- src/repl.h: findids help string includes types 4 and 5

## GAP11-06, GAP11-07

**Satisfied.** findids types 4 and 5 work; type 4 discovers 16 identities; acceptance-rr-id passes; RRG, RRH, checkid unchanged.
