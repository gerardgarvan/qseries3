---
phase: 39-update-manual
plan: 01
status: COMPLETED
duration: ~8min
---

## What Was Done

Updated MANUAL.md with all 10 new v1.8 built-in functions and fixed version references across the project.

### MANUAL.md additions

**Series Construction:**
- `eisenstein(k, T)` — normalized Eisenstein series E_{2k}(q)

**Series Inspection:**
- `coeff(f, n)` — single coefficient extraction
- `dissect(f, m, T)` — m-dissection
- `qdiff(f)` — q-derivative

**Number Theory Helpers (6 new entries):**
- `divisors(n)`, `euler_phi(n)`, `jacobi(a, n)`, `kronecker(a, n)`, `mobius(n)`, `partition(n)`

### Version updates

- MANUAL.md: `qseries 1.3` → `qseries 1.9` (2 occurrences)
- `src/repl.h`: banner `version 1.7` → `version 1.9`, version() output `1.7` → `1.9`
- `src/main.cpp`: `--version` flag output `1.3` → `1.9`
- `tests/acceptance-wins.sh`: expected version strings updated to `1.9`

## Files Modified

- `MANUAL.md` — 10 new function entries, 2 version fixes
- `src/repl.h` — version bump (banner + version())
- `src/main.cpp` — version bump (--version flag)
- `tests/acceptance-wins.sh` — version expectations updated

## Verification

- Build: clean under `-Wall -Wextra -Wpedantic` (zero warnings)
- All 10 acceptance tests pass
- All 7 quick-win tests pass (including version checks)
