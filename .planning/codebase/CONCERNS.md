# Codebase Concerns

**Analysis Date:** 2025-03-04

## Tech Debt

**repl.h size:**
- Issue: `src/repl.h` is ~2900 lines; aggregates many domain headers and built-in handlers
- Files: `src/repl.h`
- Impact: Hard to navigate; changes risk regressions; long compile times
- Fix approach: Split built-in dispatch into `repl_builtins.h` or domain-specific handlers; keep `repl.h` as thin orchestrator

**main.cpp dual role:**
- Issue: `src/main.cpp` contains both unit tests and production entry point
- Files: `src/main.cpp`
- Impact: Bloat in binary; test code shipped in release
- Fix approach: Extract unit tests to `tests/unit_main.cpp` or separate target; link or compile-test only in dev

**Header-only layout:**
- Issue: All logic in headers; single translation unit means full recompile on any change
- Files: All `src/*.h`
- Impact: Slow iteration; no incremental compilation
- Fix approach: Consider splitting into .cpp units for bigint, frac, series, qfuncs, convert, etc. (would require explicit instantiation or template tricks)

## Known Bugs

None explicitly documented as TODO/FIXME in `src/` (grep returned no matches).

**Rødseth test truncation:**
- Symptom: Comment in `acceptance.sh` notes "T=38 hits coefficient overflow" — Test 6 uses T=30 for etamake
- Files: `tests/acceptance.sh`
- Trigger: High truncation with rational coefficient growth
- Workaround: Use lower T in that test

## Security Considerations

**Input handling:**
- Risk: Parser receives arbitrary user input; long or malicious expressions could cause stack/memory issues
- Files: `src/parser.h`, `src/repl.h`
- Current mitigation: None explicit
- Recommendations: Consider input length limits, recursion depth limits, or timeouts for heavy computations

**No sandboxing:**
- Risk: REPL can consume unbounded CPU/memory (e.g., `findhom` with large degree)
- Files: `src/repl.h`, `src/relations.h`
- Current mitigation: None
- Recommendations: Document limits; optional resource caps for script mode

## Performance Bottlenecks

**Series multiplication:**
- Problem: O(T²) for dense series; hot path for everything
- Files: `src/series.h`
- Cause: Double loop over coefficient maps
- Improvement path: Karatsuba or FFT for dense case (SPEC says schoolbook is fine for typical T); optimize inner loop (skip e1+e2 >= trunc early)

**Eta memoization:**
- Problem: `etaq(k, T)` recomputed on each call
- Files: `src/qfuncs.h`
- Cause: No cache (SPEC stretch goal)
- Improvement path: Add static cache keyed by (k, T)

**findhom matrix size:**
- Problem: C(n+k-1, k-1) monomials; matrix can be large
- Files: `src/relations.h`
- Cause: Combinatorial blow-up
- Improvement path: Document limits; consider sparse/iterative methods for large k, n

## Fragile Areas

**Series::inverse():**
- Files: `src/series.h`
- Why fragile: Recurrence is error-prone; shift/unshift for non-zero leading exponent
- Safe modification: Add targeted unit tests; validate (f * f.inverse()).coeff(n) == δ_{n,0}
- Test coverage: Covered indirectly via prodmake, division

**prodmake / jacprodmake:**
- Files: `src/convert.h`
- Why fragile: Andrews algorithm and periodicity detection are subtle
- Safe modification: Rogers-Ramanujan is the canonical test; add more product/identity regression tests
- Test coverage: Good (acceptance.sh, maple-checklist.sh)

**Platform-specific terminal code:**
- Files: `src/repl.h` (termios vs windows.h, RawModeGuard, readOneChar)
- Why fragile: Conditional compilation; Cygwin vs native Windows vs Unix
- Safe modification: Test on all targets; consider abstraction layer
- Test coverage: acceptance-arrow-keys.sh, acceptance-history.sh (may skip if TTY unavailable)

## Scaling Limits

**Truncation T:**
- Current capacity: T~500 in Maple reference; REPL handles similar
- Limit: BigInt/Frac growth; memory for series
- Scaling path: Memoization; possible future GMP switch (breaks zero-dependency)

**findhom / findnonhom:**
- Current capacity: k=4, n=2 is fine; k=6, n=4 → 126 monomials
- Limit: Matrix size, kernel computation time
- Scaling path: Sparse representation; iterative/null space methods

## Dependencies at Risk

**None** — Zero external dependencies; no packages to sunset.

**Emscripten:**
- Risk: WASM build depends on emsdk; API changes over time
- Impact: Web demo may break
- Migration plan: Keep `main_wasm.cpp` minimal; track emsdk releases

## Missing Critical Features

**Eta memoization:**
- Problem: Listed in SPEC stretch goals; not implemented
- Blocks: Faster repeated etaq calls in scripts

**Euler product optimization for etaq(1,T):**
- Problem: SPEC suggests O(T) via pentagonal theorem
- Blocks: Faster partition function / eta product at k=1

## Test Coverage Gaps

**Unit tests for convert/relations:**
- What's not tested: Direct unit tests for prodmake, etamake, jacprodmake, findhom
- Files: `src/convert.h`, `src/relations.h`
- Risk: Regressions in conversion algorithms
- Priority: Medium — acceptance tests cover end-to-end

**Domain headers:**
- What's not tested: Dedicated scripts for eta_cusp, theta_ids, modforms, bailey, mock, crank, tcore, rr_ids
- Files: `src/eta_cusp.h`, `src/theta_ids.h`, etc.
- Risk: New features in domain code may regress
- Priority: Low — integration and feature scripts exist for many

**WASM:**
- What's not tested: Automated tests for WASM build
- Files: `tests/test_wasm.mjs`, `index.html`
- Risk: Web demo breaks without CI signal
- Priority: Medium

---

*Concerns audit: 2025-03-04*
