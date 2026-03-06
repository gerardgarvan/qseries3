---
phase: 106-provemodfuncid-extensions
plan: 01
subsystem: theta_ids, REPL
tags: provemodfuncidBATCH, GAP11-04, GAP11-05, theta_aids

# Dependency graph
requires:
  - phase: 105-findlincombomodp
provides:
  - provemodfuncidBATCH; theta_aids regression verified; GAP11-04, GAP11-05 satisfied
affects: Phase 107 (RR Identity Search)

# Tech tracking
tech-stack:
  added: [provemodfuncidBATCH]
  patterns: Batch API mirroring provemodfuncGAMMA0idBATCH

key-files:
  created: [.planning/phases/106-provemodfuncid-extensions/106-01-SUMMARY.md]
  modified: [src/theta_ids.h, src/repl.h, tests/acceptance-theta-ids.sh]

key-decisions:
  - "provemodfuncidBATCH loops over jacids, calls provemodfuncid per identity"
  - "REPL dispatch reuses provemodfuncid term parser; output format id[i] sturm_bound=... proven=..."

# Metrics
duration: ~20min
completed: 2026-03-06
---

# Phase 106 Plan 01: provemodfuncid extensions Summary

**provemodfuncidBATCH implemented; REPL dispatch and help added; acceptance-theta-ids Test 9 passes; GAP11-04, GAP11-05 satisfied.**

## Performance

- **Duration:** ~20 min
- **Tasks:** 3
- **Files modified:** src/theta_ids.h, src/repl.h, tests/acceptance-theta-ids.sh

## Accomplishments

1. **provemodfuncidBATCH** in theta_ids.h — iterates over jacids, calls provemodfuncid; returns `vector<ProveModfuncIdResult>`.
2. **REPL dispatch** — provemodfuncidBATCH(jacids, N); parses outer list of jacid, inner term format same as provemodfuncid; prints `id[i] sturm_bound=... proven=...` per identity.
3. **Help entry** — provemodfuncidBATCH(jacids, N) — batch prove multiple Jacobi theta identities on Gamma_1(N).
4. **Test 9** — provemodfuncidBATCH with 2 identities (trivial RR + constant); pattern `id[2].*proven=1`.

## Success Criteria Met

- [x] provemodfuncidBATCH(jacids, N) in theta_ids.h
- [x] REPL accepts provemodfuncidBATCH; prints id[i] per identity
- [x] acceptance-theta-ids.sh 9/9 pass
- [x] provemodfuncid single-identity unchanged; tests 7 and 8 pass

## Observable Truths

- src/theta_ids.h: provemodfuncidBATCH
- src/repl.h: help entry, dispatch block
- tests/acceptance-theta-ids.sh: Test 9

## GAP11-04, GAP11-05

**Satisfied.** provemodfuncidBATCH callable; 2 theta_aids identities (trivial RR, constant) verified in Test 9; acceptance-theta-ids passes.
