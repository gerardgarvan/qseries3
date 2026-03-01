---
phase: 36-expose-nt-helpers
plan: 01
status: COMPLETED
duration: ~5min
---

## What Was Done

Added REPL bindings for 3 internal number theory helper functions that already existed in `src/qfuncs.h`:

1. **`divisors(n)`** — returns sorted list of positive divisors, displayed as `[1, 2, 3, ...]`
2. **`mobius(n)`** — Möbius function μ(n): returns 0 if squared factor, (-1)^k otherwise
3. **`euler_phi(n)`** — Euler's totient φ(n)

## Files Modified

- `src/repl.h` — 3 help table entries + 3 dispatch handlers
- `tests/acceptance-wins.sh` — Fixed pre-existing version string mismatch (1.3 → 1.7)

## Verification

- Build: clean under `-Wall -Wextra -Wpedantic` (zero warnings)
- Smoke test: all 3 functions produce correct results
  - `divisors(12)` → `[1, 2, 3, 4, 6, 12]`
  - `mobius(6)` → 1, `mobius(4)` → 0, `mobius(30)` → -1
  - `euler_phi(12)` → 4, `euler_phi(100)` → 40
- `help()` lists all 3 new functions
- All 10 acceptance tests pass
- All 7 quick-win tests pass

## Deviation

Fixed pre-existing test in `acceptance-wins.sh` that expected version `1.3` instead of `1.7` (from the version bump in Phase 35). Not caused by this phase.
