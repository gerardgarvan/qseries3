# Phase 42: Garvan Tutorial Coverage - Research

**Researched:** 2026-02-27
**Domain:** C++ memoization, REPL builtins, version management
**Confidence:** HIGH

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **T_rn memoization**: Add cache to T_rn in convert.h using std::map with (r, n) pair as key. T_trunc is constant within a single top-level call. Pass cache by reference through recursion to avoid stale cache issues. Wrapper function T_rn_cached(r, n, T) creates the cache and calls the recursive helper.
- **min/max builtins**: Accept 2+ integer arguments: min(a, b), min(a, b, c), etc. Return int64_t (not Series). Add to help table and dispatch in repl.h. These are integer-only -- no series min/max.
- **Version bump**: Update banner, version() output, and --version flag to 2.0. Update MANUAL.md header and any version references. Update acceptance test expected values for version strings.
- **MANUAL.md updates**: Add min and max to a Utility Functions section or alongside Number Theory Helpers. Note T_rn performance improvement transparently. Keep manual style consistent with existing entries (signature, description, example).

### Claude's Discretion
- Exact cache data structure implementation
- Whether to expose T_rn improvement in help text or just let it be transparent
- MANUAL.md section placement for min/max

### Deferred Ideas (OUT OF SCOPE)
- Character-twisted Eisenstein series UE(k,p,T) -- significant new math, own phase
- User-defined functions/procedures -- major language feature, own milestone
- Double sums -- major language feature, own milestone
- Fractional exponents (q^(1/3)) -- fundamental Series change, own milestone
</user_constraints>

## Summary

This phase has four distinct work items: (1) adding memoization to T_rn to eliminate exponential recursive blowup, (2) adding min/max integer builtins, (3) bumping the version to 2.0, and (4) updating MANUAL.md. All items are straightforward C++ changes confined to existing files with well-understood patterns.

The T_rn memoization is the most impactful change. The current implementation (`convert.h:236-248`) uses naive recursion following the recurrence T(r,n) = -Σ_{k=1}^{⌊n/2⌋} qbin(q,k,r+2k,T) · T(r+2k, n-2k). For T(0,30,50), this creates an exponential call tree where the same (r,n) pairs are recomputed many times. The Maple original uses `option remember` (built-in memoization). Adding a `std::map<std::pair<int,int>, Series>` cache scoped to each top-level call will reduce complexity from exponential to O(n²) unique subproblems.

The min/max builtins and version bump are mechanical changes following existing patterns in repl.h and main.cpp.

**Primary recommendation:** Implement the wrapper+helper pattern for T_rn memoization first, verify with T(0,30,50), then add min/max and version bump.

## Standard Stack

### Core
| Component | Location | Purpose | Notes |
|-----------|----------|---------|-------|
| `std::map<std::pair<int,int>, Series>` | convert.h | T_rn memoization cache | Key is (r, n), value is computed Series |
| `std::pair<int,int>` | convert.h | Cache key type | Has built-in `operator<` for use as map key |
| `int64_t` | repl.h | min/max return type | Matches existing pattern for legendre/sigma |

### No External Dependencies
This is a zero-dependency C++20 project. All changes use standard library types already included.

## Architecture Patterns

### Pattern 1: Wrapper + Helper for Memoized Recursion
**What:** A public wrapper creates the cache and calls a private recursive helper that takes the cache by reference.
**When to use:** When adding memoization to a recursive function where the cache should be scoped to one top-level call.

Current T_rn signature:
```cpp
inline Series T_rn(int r, int n, int T_trunc);
```

New pattern:
```cpp
inline Series T_rn_impl(int r, int n, int T_trunc,
                         std::map<std::pair<int,int>, Series>& cache) {
    if (n == 0) return Series::one(T_trunc);
    if (n == 1) return Series::zero(T_trunc);
    auto key = std::make_pair(r, n);
    auto it = cache.find(key);
    if (it != cache.end()) return it->second;
    Series q_var = Series::q(T_trunc);
    Series sum = Series::zero(T_trunc);
    for (int k = 1; k <= n / 2; ++k) {
        auto qb = qbin(q_var, k, r + 2 * k, T_trunc);
        auto t_sub = T_rn_impl(r + 2 * k, n - 2 * k, T_trunc, cache);
        sum = (sum - (qb * t_sub).truncTo(T_trunc)).truncTo(T_trunc);
    }
    sum.trunc = T_trunc;
    cache[key] = sum;
    return sum;
}

inline Series T_rn(int r, int n, int T_trunc) {
    std::map<std::pair<int,int>, Series> cache;
    return T_rn_impl(r, n, T_trunc, cache);
}
```

The public `T_rn` signature is unchanged — all existing call sites (repl.h dispatch and unit tests) continue to work without modification.

### Pattern 2: Integer Builtin Dispatch (min/max)
**What:** Add entries to `dispatchBuiltin` in repl.h following the same pattern as `legendre`, `sigma`, etc.
**When to use:** For builtins that consume integer arguments and return int64_t.

The min/max builtins need to handle 2+ arguments (variadic). Current integer builtins like `legendre` take a fixed number of arguments. For min/max, iterate `args` and evaluate each via `evi()`, then fold with `std::min`/`std::max`.

```cpp
if (name == "min") {
    if (args.size() < 2)
        throw std::runtime_error(runtimeErr(name, "expects 2 or more arguments"));
    int64_t result = evi(0);
    for (size_t i = 1; i < args.size(); ++i)
        result = std::min(result, evi(i));
    return result;
}
```

Same pattern for `max` with `std::max`.

### Pattern 3: Version String Locations
**What:** All places where version "1.9" appears and needs updating to "2.0".
**Files to change:**

| File | Line(s) | Context |
|------|---------|---------|
| `src/main.cpp:697` | `std::cout << "qseries 1.9\n";` | --version flag handler |
| `src/repl.h:517` | `std::cout << "qseries 1.9\n";` | version() builtin |
| `src/repl.h:1187` | `q-series REPL (Maple-like), version 1.9` | Banner in runRepl() |
| `tests/acceptance-wins.sh:20` | `grep -q "qseries 1.9"` | --version test |
| `tests/acceptance-wins.sh:29` | `grep -q "qseries 1.9"` | version() builtin test |
| `MANUAL.md:143` | `qseries 1.9` | Version section example |
| `MANUAL.md:599` | `qseries 1.9` | version() example |

### Anti-Patterns to Avoid
- **Global/static cache for T_rn:** A static cache persisting across REPL invocations would break if set_trunc changes T, since cached results would have wrong truncation. The cache MUST be scoped to a single top-level T_rn call.
- **Adding min/max as Series operations:** These are integer-only. Don't try to make them work on Series (no meaningful definition).
- **Forgetting acceptance tests:** If the version string test isn't updated, the entire acceptance-wins.sh test suite will fail.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Map key for (r,n) | Custom hash struct | `std::pair<int,int>` with `std::map` | `std::pair` has built-in `operator<`; `std::map` doesn't need a hash function |
| Min/max of variadic args | Custom loop with initial value | `std::min(a, b)` in a fold | Standard library is correct and readable |

## Common Pitfalls

### Pitfall 1: Stale Cache Across Different Truncations
**What goes wrong:** If T_rn is called with T_trunc=50 and the cache is global/static, a later call with T_trunc=30 returns the T_trunc=50 result — wrong truncation.
**Why it happens:** T_trunc is implicit in the cached Series values but not in the (r,n) key.
**How to avoid:** Create the cache inside T_rn() as a local variable, pass by reference to the helper. Each top-level call gets a fresh cache.
**Warning signs:** Tests pass individually but fail when T_rn is called multiple times with different truncations in the same session.

### Pitfall 2: Cache Key Must Match Recursive Parameters
**What goes wrong:** Using only `n` as the cache key (forgetting `r`) would cause catastrophic wrong results because T_rn depends on both r and n.
**Why it happens:** The recurrence changes both parameters: T(r, n) → T(r+2k, n-2k).
**How to avoid:** Use `std::pair<int,int>(r, n)` as the key. Both parameters are essential.
**Warning signs:** T(0,30,50) produces wrong coefficients.

### Pitfall 3: min/max Must Evaluate All Arguments Before Folding
**What goes wrong:** Short-circuit evaluation or lazy evaluation could skip argument evaluation, hiding errors.
**Why it happens:** Unlikely in this codebase, but worth noting for completeness.
**How to avoid:** Use the `evi()` lambda in a simple loop. Each call evaluates eagerly.

### Pitfall 4: Version String Missed in One Location
**What goes wrong:** --version says "2.0" but version() says "1.9" (or vice versa), or banner still shows old version.
**Why it happens:** Version string is duplicated in 3 source files (main.cpp, repl.h x2) plus tests and manual.
**How to avoid:** Grep for "1.9" across all source, test, and doc files. Update ALL occurrences.
**Warning signs:** Acceptance test fails on version check.

### Pitfall 5: Help Table Must Include min/max
**What goes wrong:** Functions work but `help()` doesn't list them, and `help(min)` says "unknown function".
**Why it happens:** Forgetting to add entries to the `getHelpTable()` static table in repl.h.
**How to avoid:** Add help entries at the same time as the dispatch code.

## Code Examples

### T_rn Memoization — Performance Analysis
The recurrence T(r, n) = -Σ_{k=1}^{⌊n/2⌋} qbin(q,k,r+2k) · T(r+2k, n-2k) for T(0, 30, 50):
- Without memoization: each call branches up to n/2 ways, and the same (r,n) is visited exponentially many times. For n=30, this is intractable.
- With memoization: the unique (r,n) pairs have r ∈ {0, 2, 4, ..., 30} and n ∈ {0, 1, 2, ..., 30} with r+n ≤ 30+initial_r. About O(n²/2) ≈ 450 unique subproblems. Each computes at most n/2 series multiplications. Total: ~O(n³) series ops, which is fast.

### min/max Help Table Entries
Following the existing style in `getHelpTable()`:
```cpp
{"min", {"min(a, b, ...)", "minimum of 2 or more integers"}},
{"max", {"max(a, b, ...)", "maximum of 2 or more integers"}},
```

### MANUAL.md Entry for min/max
Place alongside existing Number Theory Helpers or in a new "Utility Functions" subsection:
```markdown
#### `min(a, b, ...)` — Minimum

Returns the minimum of two or more integer arguments.

\`\`\`
qseries> min(3, 1, 4, 1, 5)
1
\`\`\`

#### `max(a, b, ...)` — Maximum

Returns the maximum of two or more integer arguments.

\`\`\`
qseries> max(3, 1, 4, 1, 5)
5
\`\`\`
```

**Recommendation for placement:** Add these in the existing "Number Theory Helpers" section (before `sigma` which is alphabetically after them, or at the end of the section). Alternatively, add a new "### Integer Utilities" subsection right after "Number Theory Helpers" since min/max aren't number-theoretic per se but are integer operations. Either is fine; the key is consistency with the existing manual style.

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Naive T_rn recursion | Memoized T_rn with per-call cache | Phase 42 | T(0,30,50) goes from intractable to <2s |
| Version 1.9 | Version 2.0 | Phase 42 | Milestone version bump |

## Open Questions

1. **Should clear_cache() also mention T_rn?**
   - What we know: T_rn cache is per-call (local variable), so there's nothing to clear between invocations. etaq cache is global/static and is what clear_cache() resets.
   - What's unclear: Should help text for clear_cache mention that T_rn cache is automatic?
   - Recommendation: No change needed. T_rn cache is transparent. Don't mention it in clear_cache help.

2. **Should T_rn help text mention memoization?**
   - What we know: The CONTEXT.md leaves this to Claude's discretion.
   - Recommendation: Keep it transparent. The help text for T already says `T(r,n) or T(r,n,T) — finite q-product T_{r,n}`. Users don't need to know about the internal optimization. If anyone notices the speed improvement, that's a bonus.

3. **MANUAL.md section for min/max**
   - Recommendation: Add a "### Integer Utilities" subsection immediately after "Number Theory Helpers" containing min and max. This avoids cluttering the number theory section with non-number-theoretic functions while keeping them in the same general area of the manual.

## Sources

### Primary (HIGH confidence)
- Source code inspection: `src/convert.h` (T_rn at lines 236-248), `src/repl.h` (dispatchBuiltin, help table, version strings, banner), `src/main.cpp` (--version flag)
- `qseriesdoc.md` (lines 264-278): Maple T(r,j) proc with `option remember`, establishing the memoization precedent
- `qseriesdoc.md` (line 326): `min(a,b,c)` usage in dixson function, establishing need for min builtin
- `qseriesdoc.md` (lines 662-697): UE/Eisenstein section establishing the tutorial §4.2 context (deferred)
- `tests/acceptance-wins.sh` (lines 20, 29): Version string tests that need updating

### Secondary (MEDIUM confidence)
- C++20 standard library: `std::map`, `std::pair`, `std::min`, `std::max` — well-known APIs

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all standard C++ library types already used in the project
- Architecture: HIGH — wrapper+helper memoization is a textbook pattern; all changes are to existing well-understood files
- Pitfalls: HIGH — identified from direct source code analysis and the CONTEXT.md constraints

**Research date:** 2026-02-27
**Valid until:** indefinite (stable C++20 project with no external dependencies)
