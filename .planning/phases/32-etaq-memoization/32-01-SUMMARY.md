---
plan: 32-01
status: complete
started: 2026-02-26
completed: 2026-02-26
---

# Plan 32-01 Summary: etaq memoization

## What was done
- Added `etaq_cache()` accessor returning `static std::map<std::pair<int,int>, Series>&`
- Added `clear_etaq_cache()` to clear the cache
- Modified `etaq()` to check if q is standard (single term q^1 with coefficient 1); if so, lookup cache on `{k, T}` before computing, and store result after computing
- Non-standard q (e.g. `etaq(q^2, k, T)`) bypasses cache entirely
- Wired `clear_etaq_cache()` to `set_trunc` handler (clears when truncation changes)
- Added `clear_cache()` REPL command with help table entry

## Files modified
- `src/qfuncs.h` — `etaq_cache()`, `clear_etaq_cache()`, modified `etaq()` with cache logic
- `src/repl.h` — `clear_etaq_cache()` call in `set_trunc`, `clear_cache` dispatch + help entry

## Verification
- Build: `g++ -std=c++20 -O2` — success
- Core acceptance tests: 10/10 pass
- acceptance-wins: 7/7 pass
- acceptance-optional-args: 5/5 pass
- acceptance-suppress-output: 4/4 pass
- Cache hit: two identical `etaq(1,50)` calls produce same output, second uses cache
- `clear_cache()` REPL command works
