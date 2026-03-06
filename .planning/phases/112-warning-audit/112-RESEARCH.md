# Phase 112: Warning Audit — Research

**Researched:** 2026-03-06
**Domain:** C++ compiler warnings, unused parameters, -Wshadow
**Confidence:** HIGH (project codebase inspection; existing research PITFALLS.md, STACK.md)

---

## Summary

Phase 112 cleans the build to zero warnings with `-Wshadow` enabled. Three unused-parameter sites are known (tcore.h:149, eta_cusp.h:51, eta_cusp.h:99). The project already uses both `[[maybe_unused]]` and `(void)x`; PITFALLS.md recommends fix-before-suppress and no global `-Wno-*`. For each site: use `[[maybe_unused]]` when the parameter is part of the public API and semantically meaningful; prefer omitting the name when the parameter is truly vestigial.

**Primary recommendation:** Fix all three sites with `[[maybe_unused]]`; add `-Wshadow` to Makefile; assess shadow warnings locally (build environment unavailable during research); run acceptance suite to verify behavior unchanged.

---

## Standard Stack

### Compiler Flags (Current → Target)
| Flag | Current | Target | Purpose |
|------|---------|--------|---------|
| -std=c++20 | ✅ | ✅ | Language |
| -O2 | ✅ | ✅ | Optimization |
| -Wall -Wextra -Wpedantic | ✅ | ✅ | Base warnings |
| -Wshadow | ❌ | ✅ | Variable shadowing (not in -Wall) |

### Unused-Parameter Handling (per PITFALLS.md)
| Method | When to Use | Example |
|--------|-------------|---------|
| Omit name | You control signature; param vestigial | `void f(int, int c)` |
| `/* param */` | You control; keep name in comment | `void f(int /*n*/)` |
| `[[maybe_unused]]` | Conditionally unused; API requires param | `void f([[maybe_unused]] int n)` |
| `(void)x` | Must match external API; suppress in body | `(void)n;` |
| Pragmas | Last resort; use push/pop | See PITFALLS Pitfall 1 |

**Policy:** Fix before suppress. Never use global `-Wno-unused-parameter`.

---

## Exact Locations and Recommended Fixes

### 1. tcore.h:149 — `tcore_tcores(int p, int n, ...)`

**File:** `src/tcore.h`  
**Line:** 149  
**Function:** `tcore_tcores(int p, int n, const std::vector<std::vector<int64_t>>& all_partitions)`  
**Unused param:** `n`

**Context:** Lists p-cores of n by filtering `all_partitions`. The implementation iterates over `all_partitions` and keeps those passing `tcore_istcore(ptn, p)`. The parameter `n` is semantically part of the API (p-cores of n) but the brute-force implementation does not use it—caller passes `n` and `enumerate_partitions(n)` is done at call site (repl.h:2002–2003).

**Recommended fix:** `[[maybe_unused]]` on `n` — the API signature should remain `(p, n, all_partitions)` for clarity; `n` documents intent even though implementation doesn't use it.

```cpp
inline std::vector<std::vector<int64_t>> tcore_tcores(int p, [[maybe_unused]] int n,
    const std::vector<std::vector<int64_t>>& all_partitions) {
```

---

### 2. eta_cusp.h:51 — `cuspord(..., int cusp_num, int cusp_den)`

**File:** `src/eta_cusp.h`  
**Line:** 51  
**Function:** `cuspord(const std::vector<int>& GP, int cusp_num, int cusp_den)`  
**Unused param:** `cusp_num`

**Context:** Ligozat order at cusp. Cusp is (num, den); (1,0) = infinity. The formula uses only `cusp_den` (for gcd and infinity check). Call site: `cuspord(GP, cusp[0], cusp[1])` (repl.h:1666). API keeps (num, den) for consistency with cusp representation.

**Recommended fix:** `[[maybe_unused]]` on `cusp_num` — API documents cusp as (d, c); numerator not needed by Ligozat formula.

```cpp
inline Frac cuspord(const std::vector<int>& GP, [[maybe_unused]] int cusp_num, int cusp_den) {
```

---

### 3. eta_cusp.h:99 — `fanwidth(int cusp_num, int cusp_den, int N)`

**File:** `src/eta_cusp.h`  
**Line:** 99  
**Function:** `fanwidth(int cusp_num, int cusp_den, int N)`  
**Unused param:** `cusp_num`

**Context:** fanwidth(cusp d/c, N) = N / gcd(N, c²). Formula uses only `cusp_den` (c). Call site: `fanwidth(d, c, N)` (eta_cusp.h:120). Same rationale as cuspord.

**Recommended fix:** `[[maybe_unused]]` on `cusp_num`.

```cpp
inline int fanwidth([[maybe_unused]] int cusp_num, int cusp_den, int N) {
```

---

## -Wshadow Impact Assessment

**Status:** Build could not be executed during research (g++ not in PATH in PowerShell/Cygwin session). Impact must be assessed locally.

**Procedure:**
1. Add `-Wshadow` to CXXFLAGS in Makefile.
2. Run `make clean && make 2>&1`.
3. Record any `-Wshadow` warnings (variable declaration shadows a previous local or parameter).
4. Fix each by renaming the inner variable or the outer one as appropriate.

**Typical sources of -Wshadow:**
- Loop variables: `for (int i = ...)` inside a scope that already has `i`
- Lambda captures shadowing outer variables
- Function parameters shadowing globals or outer scope
- Nested blocks reusing names (e.g., `int n` in inner block when outer has `n`)

**Codebase note:** Project uses many `for (int n = ...)`, `for (int i = ...)`, `for (int j = ...)` in nested loops. Risk of shadowing exists where outer and inner loops use the same loop variable name; per STACK.md and PITFALLS.md, -Wshadow is low-noise and high-value—fix any reported rather than disabling.

---

## Acceptance Test List

Run these after applying fixes to ensure behavior is unchanged:

| Target | Purpose |
|--------|---------|
| `make test` | Rogers-Ramanujan + prodmake (core math) |
| `make acceptance` | All 9 SPEC acceptance tests |
| `make acceptance-eta-cusp` | cuspord, gammacheck, etaprodtoqseries, vp |
| `make acceptance-theta-ids` | getacuspord, theta ID cusp theory |
| `make acceptance-all` | Full regression (maple-checklist, run-all, exercises, factor, modforms, theta-ids, integration) |

**Critical for this phase:**
- `acceptance-eta-cusp` — exercises `cuspord` and `fanwidth` (tests/acceptance-eta-cusp.sh:28)
- `acceptance` — exercises full REPL; tcore_tcores used in `tcores` builtin (repl.h:2003)
- Any test invoking `tcores`, `cuspord`, or code paths through `fanwidth`

**Minimal post-fix check:**
```bash
make clean && make 2>&1 | tee build.log
# Expect: zero lines of warning output
make acceptance
make acceptance-eta-cusp
```

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Unused param | `-Wno-unused-parameter` | `[[maybe_unused]]` or omit name | Global suppression hides real issues |
| Pragma scope | Bare `#pragma GCC diagnostic ignored` | `#pragma push` / `ignored` / `pop` | Unbounded pragma leaks to rest of file |
| Shadow fix | Rename only inner var blindly | Rename to avoid confusion | Preserve readability; prefer outer→inner clarity |

---

## Common Pitfalls

### Pitfall 1: Suppress Instead of Fix
**What goes wrong:** Adding `-Wno-unused-parameter` or `(void)x` everywhere instead of choosing the right fix per site.  
**Prevention:** Per PITFALLS.md—omit name when you control API and param is vestigial; use `[[maybe_unused]]` when API requires the param for documentation/consistency.

### Pitfall 2: Pragmas Without Push/Pop
**What goes wrong:** `#pragma GCC diagnostic ignored "-Wunused-parameter"` in a header without pop leaks to all includers.  
**Prevention:** Always use push before and pop after; prefer fixing the warning over pragmas.

### Pitfall 3: -Wshadow Fix That Reduces Readability
**What goes wrong:** Renaming loop vars to `ii`, `jj` everywhere just to silence -Wshadow.  
**Prevention:** Prefer renaming the inner variable to a more specific name (e.g., `row` vs `col`) when both are `i`; avoid generic `i2`-style names.

---

## Code Examples (Existing Project Patterns)

### [[maybe_unused]] (repl.h:486)
```cpp
inline void formatHelpEntry([[maybe_unused]] const std::string& name, const HelpEntry& e) {
```

### (void)x (rr_ids.h:101, 107)
```cpp
(void)n;
```

### Omit name (alternative when param truly vestigial)
```cpp
void callback(int /*unused*/, int used);
```

---

## State of the Art

| Approach | Recommendation | Rationale |
|----------|----------------|-----------|
| `[[maybe_unused]]` | Use for API-preserving unused params | C++17 standard; documents intent; no runtime cost |
| `(void)x` | Use when param must exist (e.g. callback) | Traditional; works in C and C++ |
| Omit parameter name | Use when param is vestigial and you control API | Cleanest; no attribute noise |
| `-Wno-*` globally | Never | Hides real bugs project-wide |

---

## Sources

### Primary (HIGH confidence)
- `.planning/research/PITFALLS.md` — fix vs suppress, pragma push/pop, unused-parameter guidance
- `.planning/research/STACK.md` — -Wshadow recommendation, CXXFLAGS
- `src/tcore.h`, `src/eta_cusp.h`, `src/repl.h` — call sites and signatures

### Secondary (MEDIUM confidence)
- Project grep for existing `[[maybe_unused]]` and `(void)x` usage

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — project research docs and codebase
- Exact locations / recommended fixes: HIGH — direct inspection
- -Wshadow impact: LOW — build not run; requires local verification

**Research date:** 2026-03-06  
**Valid until:** 30 days (stable domain)
