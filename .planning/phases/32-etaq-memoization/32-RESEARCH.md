# Phase 32: etaq Memoization - Research

**Researched:** 2026-02-26
**Domain:** C++ memoization / caching for pure mathematical function
**Confidence:** HIGH

## Summary

`etaq(q, k, T)` computes `Π_{n=1}^{⌊T/k⌋-1} (1 - q^{kn})` via repeated Series multiplication. It is the most heavily-called primitive in the system: theta functions, tripleprod, quinprod, winquist, etamake, and direct REPL use all call it. The function is **pure** — for a given `(k, T)` with standard q, it always returns the same Series — making it an ideal memoization target.

The implementation is straightforward: add a `static std::map<std::pair<int,int>, Series>` cache inside `etaq`. Before computing, check if `q` is the standard q-series (single term with exponent 1, coefficient 1); if so, look up `(k, T)` in the cache and return a copy on hit. On miss, compute normally and store the result before returning. Provide a `clear_etaq_cache()` function for memory management.

**Primary recommendation:** Add a static cache keyed by `(k, T)` inside the existing `etaq` function with a fast O(1) standard-q detection check. No architectural changes, no new files, no new dependencies.

## Standard Stack

### Core

| Component | Version | Purpose | Why Standard |
|-----------|---------|---------|--------------|
| `std::map<std::pair<int,int>, Series>` | C++20 STL | Cache storage | Already used throughout codebase; ordered map matches project style |
| `static` local variable in `inline` function | C++17+ (guaranteed single instance) | Singleton cache lifetime | Zero-dependency, header-only compatible, no global init order issues |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| `std::map` | `std::unordered_map` with custom hash | Faster O(1) lookup vs O(log n), but `std::pair` needs a custom hasher; map size will be small (<100 entries typically), so O(log n) is negligible |
| Static local | Global variable | Global init order fiasco risk in header-only code; static local is safer |
| In-function cache | External cache manager class | Over-engineering for a single function; no other function needs caching currently |

## Architecture Patterns

### Pattern 1: In-Function Memoization with Standard-Q Guard

**What:** Cache results inside the function itself, gated on detecting that `q` is the standard q-series.

**When to use:** When the function is pure, called frequently with repeated arguments, and has a single canonical first argument (the standard q).

**Implementation:**

```cpp
inline Series etaq(const Series& q, int k, int T) {
    static std::map<std::pair<int,int>, Series> cache;

    bool standardQ = (q.c.size() == 1
                      && q.c.begin()->first == 1
                      && q.c.begin()->second == Frac(1));

    if (standardQ) {
        auto it = cache.find({k, T});
        if (it != cache.end()) return it->second;
    }

    // Original computation
    Series result = Series::one(T);
    for (int n = 1; k * n < T; ++n) {
        Series qkn = q.truncTo(T).pow(k * n);
        Series factor = Series::one(T) - qkn;
        result = (result * factor).truncTo(T);
    }
    result.trunc = T;

    if (standardQ) {
        cache[{k, T}] = result;
    }
    return result;
}
```

### Pattern 2: Cache Clear Function

**What:** A free function to clear the cache, exposed for REPL use or memory management.

```cpp
inline void clear_etaq_cache() {
    static auto& cache = /* needs access to the static inside etaq */;
    cache.clear();
}
```

**Problem:** The `static` local inside `etaq` isn't directly accessible from outside. Two solutions:

1. **Move cache to file scope** (still in the header, wrapped in an anonymous namespace or just `inline`):
```cpp
inline std::map<std::pair<int,int>, Series>& etaq_cache() {
    static std::map<std::pair<int,int>, Series> cache;
    return cache;
}
inline void clear_etaq_cache() { etaq_cache().clear(); }
```
Then `etaq` uses `etaq_cache()` instead of a local static.

2. **Use a flag pattern**: `etaq` checks a global flag and clears its local cache when set. Unnecessarily complex.

**Recommendation:** Use approach 1 — a named accessor function `etaq_cache()` returning a reference to the static map. This is clean, header-only compatible, and gives both `etaq` and `clear_etaq_cache` access.

### Anti-Patterns to Avoid

- **Caching when q is non-standard:** If a user calls `etaq(q^2, 3, 50)` via the 3-arg REPL form, the result differs from `etaq(q, 3, 50)`. The standard-q check prevents incorrect cache hits.
- **Cache invalidation on `set_trunc`:** Not needed. The cache key includes T, so different truncation levels are separate entries. Changing `env.T` just means future calls use a different T value, producing different cache keys.
- **LRU/size-limited cache:** Over-engineering. Typical REPL sessions use <50 distinct `(k, T)` pairs. Memory is bounded by `Σ min(T/k, T)` non-zero coefficients per entry.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Thread-safe cache | Mutex-protected map | Nothing (single-threaded REPL) | No threads exist; adding synchronization is pure overhead |
| Cache eviction policy | LRU, TTL, size limits | Unbounded map + manual clear | Session-scoped; entries are small; user can clear if needed |
| Hash function for `std::pair<int,int>` | Custom `std::hash` specialization | `std::map` with default `operator<` | Logarithmic lookup on <100 entries is fine |

## Common Pitfalls

### Pitfall 1: Caching Non-Standard Q Results

**What goes wrong:** If the standard-q check is missing or wrong, `etaq(q^2, 3, 50)` could return the cached result for `etaq(q, 3, 50)`.
**Why it happens:** The cache key is `(k, T)`, not `(q, k, T)`.
**How to avoid:** Check `q.c.size() == 1 && q.c.begin()->first == 1 && q.c.begin()->second == Frac(1)` before cache lookup.
**Warning signs:** Acceptance tests involving non-standard q arguments fail, or results differ from uncached version.

### Pitfall 2: Static Local in Inline Function Across TUs

**What goes wrong:** If the project ever compiles multiple .cpp files (not just main.cpp), a `static` local inside an `inline` function could theoretically have multiple instances (pre-C++17).
**Why it happens:** Older C++ standards didn't guarantee single instances for inline statics.
**How to avoid:** C++20 guarantees this. The project uses `-std=c++20`. Non-issue, but worth noting.
**Warning signs:** Different results in debug vs release builds.

### Pitfall 3: Returning Reference vs Copy

**What goes wrong:** If `etaq` returns a `const Series&` from the cache, and the caller modifies the result (e.g., `result.trunc = newT`), the cached entry is corrupted.
**Why it happens:** Series is mutable.
**How to avoid:** Return by value (copy). `Series` is already returned by value in the current code; keep it that way.
**Warning signs:** Results change after first computation; intermittent test failures.

### Pitfall 4: Forgetting to Cache Before Return

**What goes wrong:** The computation runs but the result isn't stored in the cache, defeating the purpose.
**Why it happens:** Adding `return result` before the cache store line.
**How to avoid:** Store in cache, then return. Or: `return cache[{k, T}] = result;`
**Warning signs:** No speedup observed; profiling shows etaq still O(T/k) on every call.

### Pitfall 5: Memory Growth with Large T

**What goes wrong:** `etaq(1, 500)` produces a Series with ~500 entries, each containing BigInt rationals. Many such entries could consume significant memory.
**Why it happens:** Large T values produce dense Series with many non-zero coefficients.
**How to avoid:** Provide `clear_etaq_cache()` for explicit cleanup. In practice, typical sessions use T ≤ 500 and ≤ 50 distinct k values, so total cache size is manageable (order of megabytes).
**Warning signs:** Process memory grows monotonically during long sessions.

## Code Examples

### Complete Implementation (Recommended)

```cpp
// Cache accessor — shared between etaq and clear_etaq_cache
inline std::map<std::pair<int,int>, Series>& etaq_cache() {
    static std::map<std::pair<int,int>, Series> cache;
    return cache;
}

inline void clear_etaq_cache() {
    etaq_cache().clear();
}

inline Series etaq(const Series& q, int k, int T) {
    // Fast check: is q the standard q-series (0 + 1·q¹)?
    bool standardQ = (q.c.size() == 1
                      && q.c.begin()->first == 1
                      && q.c.begin()->second == Frac(1));

    if (standardQ) {
        auto& cache = etaq_cache();
        auto it = cache.find({k, T});
        if (it != cache.end()) return it->second;
    }

    Series result = Series::one(T);
    for (int n = 1; k * n < T; ++n) {
        Series qkn = q.truncTo(T).pow(k * n);
        Series factor = Series::one(T) - qkn;
        result = (result * factor).truncTo(T);
    }
    result.trunc = T;

    if (standardQ) {
        etaq_cache()[{k, T}] = result;
    }
    return result;
}
```

### Standard-Q Detection (Explained)

The standard q-series `Series::q(T)` has exactly one entry in its coefficient map: `{1: Frac(1)}`. The check:

```cpp
bool standardQ = (q.c.size() == 1
                  && q.c.begin()->first == 1
                  && q.c.begin()->second == Frac(1));
```

This is O(1) — map size check + iterator dereference + Frac equality (which compares two BigInts). Negligible cost compared to the Series multiplication loop it guards.

### Cache Wiring in REPL (set_trunc handler)

Optionally clear the cache when truncation changes, to avoid accumulating stale large-T entries:

```cpp
if (name == "set_trunc") {
    // ... existing code ...
    env.T = N;
    env.env["q"] = Series::q(N);
    clear_etaq_cache();  // optional: free memory from old-T entries
    return std::monostate{};
}
```

This is optional because the cache is correct regardless (keyed by T), but it prevents memory accumulation.

## Call Site Analysis

| Call Site | File | How Q is Passed | Caching Applies? |
|-----------|------|-----------------|------------------|
| `dispatchBuiltin` etaq(k) | repl.h:270-271 | `env.q` = `Series::q(T)` | YES — standard q |
| `dispatchBuiltin` etaq(k,T) | repl.h:273-276 | `env.q` = `Series::q(T)` | YES — standard q |
| `dispatchBuiltin` etaq(q,k,T) | repl.h:278-279 | User-provided `ev(0)` | MAYBE — depends on user input |
| `etamake` loop | convert.h:69 | `Series::q(T)` local | YES — standard q |
| `main.cpp` tests | main.cpp:274+ | `Series::q(T)` local | YES — standard q |

All internal call sites pass standard q. Only the 3-arg REPL form (`etaq(q,k,T)`) could pass non-standard q, and the standard-q check handles this correctly.

## Verification Strategy

1. **Correctness:** All existing acceptance tests must pass unchanged (PERF-03). The cache is transparent — same inputs, same outputs.
2. **Cache hit verification:** Run `etaq(1,50)` twice in the REPL; second call should be near-instant. Can verify by timing or by adding a temporary debug print on cache hit.
3. **Non-standard q bypass:** Call `etaq(q^2, 1, 50)` — must compute fresh, not return cached `etaq(q, 1, 50)`.
4. **Memory:** After many computations, `clear_etaq_cache()` (if exposed as REPL command) should free memory.

## Open Questions

1. **Should `clear_etaq_cache` be exposed as a REPL command?**
   - What we know: It's useful for memory management in long sessions.
   - What's unclear: Whether users will run long enough sessions to need it.
   - Recommendation: Implement the C++ function. Optionally wire it to a REPL command like `clear_cache` in the same phase or defer to a later phase.

2. **Should we cache larger-T results and truncate for smaller-T requests?**
   - What we know: If `etaq(1, 100)` is cached and `etaq(1, 50)` is requested, we could return `cached.truncTo(50)` instead of recomputing.
   - What's unclear: Whether this optimization matters in practice (different-T calls for the same k are uncommon).
   - Recommendation: Start with exact `(k, T)` match only. Add truncation-based lookup later if profiling shows it helps.

## Sources

### Primary (HIGH confidence)
- **Source code analysis:** `src/qfuncs.h`, `src/repl.h`, `src/convert.h`, `src/series.h` — direct inspection of all call sites, function signatures, and data flow
- **C++20 standard:** `inline` function with `static` local variable guarantees single instance across translation units (P0386R2, standardized in C++17)

### Secondary (MEDIUM confidence)
- **Project architecture:** `.cursorrules`, `CLAUDE.md`, `SPEC.md` — confirms zero-dependency, header-only, single-binary constraints

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — uses only `std::map` and `std::pair`, already in project
- Architecture: HIGH — single function modification, no structural changes
- Pitfalls: HIGH — enumerated from direct code analysis, all verifiable

**Research date:** 2026-02-26
**Valid until:** Indefinite (pure C++ caching pattern; no external dependencies to go stale)
