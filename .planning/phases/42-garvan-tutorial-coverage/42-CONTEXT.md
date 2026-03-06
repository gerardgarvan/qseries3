# Phase 42: Garvan Tutorial Coverage - Context

**Gathered:** 2026-02-27
**Status:** Ready for planning

## Phase Boundary

Improve coverage of qseriesdoc.md tutorial examples by adding T_rn memoization and min/max builtins. Version bump to 2.0 and MANUAL.md update. No new language features (user-defined functions, double sums, complex arithmetic).

## Implementation Decisions

### T_rn memoization
- Add cache to T_rn in convert.h using std::map with (r, n) pair as key
- T_trunc is constant within a single top-level call
- Pass cache by reference through recursion to avoid stale cache issues
- Wrapper function T_rn_cached(r, n, T) creates the cache and calls the recursive helper

### min/max builtins
- Accept 2+ integer arguments: min(a, b), min(a, b, c), etc.
- Return int64_t (not Series)
- Add to help table and dispatch in repl.h
- These are integer-only -- no series min/max

### Version bump
- Update banner, version() output, and --version flag to 2.0
- Update MANUAL.md header and any version references
- Update acceptance test expected values for version strings

### MANUAL.md updates
- Add min and max to a Utility Functions section or alongside Number Theory Helpers
- Note T_rn performance improvement transparently
- Keep manual style consistent with existing entries (signature, description, example)

### Claude's Discretion
- Exact cache data structure implementation
- Whether to expose T_rn improvement in help text or just let it be transparent
- MANUAL.md section placement for min/max

## Specific Ideas

- T_rn memoization should match the option remember pattern from Maple proc definition
- The cache should be scoped to a single call, not persist across REPL invocations (avoids truncation mismatch bugs)

## Deferred Ideas

- Character-twisted Eisenstein series UE(k,p,T) -- significant new math, own phase
- User-defined functions/procedures -- major language feature, own milestone
- Double sums -- major language feature, own milestone
- Fractional exponents (q^(1/3)) -- fundamental Series change, own milestone

---

*Phase: 42-garvan-tutorial-coverage*
*Context gathered: 2026-02-27*
