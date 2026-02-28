# Project Research Summary

**Project:** qseries REPL v4.0 — Core Improvements
**Domain:** Zero-dependency C++20 mathematical REPL (q-series / partition theory)
**Researched:** 2026-02-28
**Confidence:** HIGH

## Executive Summary

The v4.0 milestone adds two categories of improvement to the existing qseries REPL: **visual polish** (ANSI color output, clear screen, smarter tab completion) and **computational performance** (Karatsuba multiplication, faster series operations, benchmarking infrastructure). All features are implementable within the zero-dependency C++20 constraint using only standard library facilities (`<chrono>`, `<fstream>`, `<sstream>`) and well-documented algorithms. No new external libraries, no build command changes, and no modifications to the mathematical core (frac.h, qfuncs.h, convert.h, relations.h, linalg.h) are required for the REPL enhancements. The performance work is confined to bigint.h and series.h. Session save/load adds persistence to repl.h.

The recommended approach is **bottom-up by layer**: start with the lowest-level performance optimization (Karatsuba in bigint.h), then series optimization (series.h), then the REPL presentation layer (ansi color, clear screen, tab completion, session persistence, benchmarking — all in repl.h). ANSI color and clear screen share a Windows VT processing prerequisite and should ship together. The performance features (Karatsuba + series optimization) are independent of the REPL features and can be developed in parallel. The benchmarking suite is both a deliverable and a validation tool for the Karatsuba threshold tuning.

The key risks are: (1) **Karatsuba intermediate overflow** — base 10^9 limb arithmetic requires signed 64-bit intermediates to handle the subtraction step `z1 = z_mid - z_high - z_low`, and wrong threshold selection can cause performance regressions on real workloads; (2) **ANSI codes leaking into non-TTY output** — the codebase has ~51 `std::cout` call sites that must all be gated on `stdin_is_tty()` or ANSI will contaminate script mode and piped output; (3) **ANSI codes breaking the line editor** — colorizing the prompt changes its byte length without changing display width, breaking cursor positioning in `redrawLineRaw()`.

## Key Findings

### Recommended Stack

No new dependencies. All v4.0 features use C++20 standard library headers already available or trivially addable: `<fstream>` and `<sstream>` for session persistence, `<chrono>` (already included) for benchmarking. The build command remains `g++ -std=c++20 -O2 -static -o qseries main.cpp`. An optional `bench_main.cpp` produces a separate benchmark binary for development-time performance testing.

**Core technologies:**
- **C++20 / g++ 13+**: Unchanged toolchain — all new features use standard library only
- **ANSI SGR escape codes**: Direct `\033[Nm` sequences for color; Windows requires one-time `SetConsoleMode()` call on stdout
- **Karatsuba algorithm**: Textbook divide-and-conquer multiplication (Knuth 4.3.3 / GMP), threshold ~32 limbs
- **`std::chrono::steady_clock`**: Monotonic clock for benchmarking (NOT `high_resolution_clock`, which is non-monotonic on libstdc++)

### Expected Features

**Must have (table stakes):**
- ANSI colored output — prompt (cyan/green), errors (red), timing (dim gray), results in default color
- Clear screen (`Ctrl+L` keypress + `clear` command) — universal REPL expectation
- Longest common prefix tab completion — current behavior (list-only, no prefix fill) is below readline standard
- Persistent history across sessions — `~/.qseries_history`, plain text, 1000 lines

**Should have (differentiators):**
- Auto-parentheses on function completion — append `(` after completing a known function name
- Argument signature hints — show `etaq(k) or etaq(k,T)` when Tab pressed inside empty parens
- `save("file")` / `load("file")` commands — save input history as replayable `.q` script
- Karatsuba multiplication — O(n^1.585) for large BigInt operands (significant at T=200+)
- Series inner-loop early break — trivial 3-line change for 2-4x speedup on high-truncation series
- Benchmarking suite — separate `bench_main.cpp` with `doNotOptimize` barriers and median reporting

**Defer (v2+):**
- Input syntax highlighting — requires per-keystroke tokenization + colored redraw; massive complexity
- Output syntax highlighting (coloring coefficients vs exponents) — unprecedented in math REPLs, distracting
- Full workspace serialization (save all variables as structured data) — requires Series→expression serialization; fragile
- Configurable color themes — overkill for 5-6 color uses
- Toom-Cook / FFT multiplication — over-engineered; Karatsuba is the right next step from schoolbook
- `NO_COLOR` env var support — nice-to-have, low priority (can add `--color=never` flag instead)

### Architecture Approach

All REPL enhancements (color, clear, completion, session, benchmarking) are confined to `repl.h` with a thin new `ansi.h` utility header. Performance work modifies only `bigint.h` (Karatsuba) and `series.h` (dense fast-path, inner-loop early break). No interface changes — `operator*` signatures, `eval()` signatures, and `Environment` structure are preserved. The existing `stdin_is_tty()` macro gates all interactive-only behavior. WASM compatibility is maintained by disabling color and session I/O behind `#ifndef __EMSCRIPTEN__`.

**Major components modified:**
1. **`bigint.h`** — Add `karatsubaMultiply()` static method with threshold dispatch; existing `addAbs`/`subAbs` reused for recombination
2. **`series.h`** — Inner-loop early break (trivial), optional dense multiply path for series with density >50%
3. **`repl.h`** — ANSI color wrapping (~15 call sites), `Ctrl+L` + `clear` command, smart tab completion (context analysis + LCP + auto-parens), history persistence, save/load commands, timing toggle, benchmark builtin
4. **`ansi.h` (new)** — ~40 lines: color constants, `enabled` flag, helper functions returning empty strings when disabled

### Critical Pitfalls

1. **ANSI codes leaking into script/piped output** — Gate ALL color on `stdin_is_tty()`. Never embed `\033[` literals at call sites; centralize in `ansi.h` helpers that return empty strings when disabled. Add regression test: `qseries < demo.qs | xxd | grep '1b 5b'` must be empty.

2. **ANSI codes breaking line editor cursor positioning** — Colorizing the prompt adds invisible bytes that break `redrawLineRaw()` cursor math. Track prompt **display width** separately from byte length. Never colorize user input text.

3. **Karatsuba intermediate overflow with base 10^9** — The recombination `z1 = z_mid - z_high - z_low` produces negative limbs that wrap in `uint32_t`. Use `int64_t` signed intermediates, or implement Karatsuba at the vector-split level (half the limbs, delegate sub-products to existing `operator*`). Cross-check: `karatsuba(a,b) == schoolbook(a,b)` for random inputs at all size boundaries.

4. **Wrong Karatsuba threshold causing regression** — Most BigInt values in the REPL are 1-4 limbs. Start threshold at ~32 limbs (~288 digits). Benchmark real workloads (`etaq(1,500)`, `prodmake(rr,200)`) not synthetic inputs. Reject if any acceptance test regresses.

5. **Series optimization breaking truncation invariant** — A dense-array fast path must handle negative exponents (Laurent series from `inverse()`) and enforce `result.trunc = min(a.trunc, b.trunc)`. Run Rogers-Ramanujan after any series.h change.

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: ANSI Color + Clear Screen
**Rationale:** Foundation for all visual improvements. Shares the Windows VT processing prerequisite with clear screen. Lowest complexity (~70 lines total). Transforms the REPL's visual impression immediately.
**Delivers:** Colored prompt, errors, timing; `Ctrl+L` and `clear` command; Windows VT enablement; `ansi.h` utility header.
**Addresses:** ANSI color (table stakes), clear screen (table stakes)
**Avoids:** Pitfalls 1 (leak to pipes), 2 (cursor miscalculation), 8 (Windows VT), 13 (color scheme legibility), 17 (error format), 18 (Unicode interaction)

### Phase 2: Karatsuba Multiplication
**Rationale:** Self-contained change in `bigint.h`. No upstream dependencies. Must land before series optimization (Phase 3) to get combined benefit. Must land before benchmarking (Phase 4) because the benchmark suite validates the threshold.
**Delivers:** O(n^1.585) BigInt multiply for operands ≥32 limbs; automatic speedup for Frac and Series at high truncation.
**Avoids:** Pitfalls 3 (overflow), 4 (threshold regression), 16 (not tested)

### Phase 3: Faster Series Operations
**Rationale:** Depends on Karatsuba being in place for combined benefit. Modifies only `series.h`. The inner-loop early break is trivial (3 lines) and delivers 2-4x on its own.
**Delivers:** Inner-loop early break; optional dense multiply path for dense series; optimized `inverse()` coefficient access.
**Avoids:** Pitfall 11 (truncation invariant breakage)

### Phase 4: Benchmarking Suite
**Rationale:** Validates Phases 2-3 performance gains. Provides infrastructure for future optimization work. Can be developed concurrently with Phases 2-3 but is most useful after they land.
**Delivers:** `bench_main.cpp` with `doNotOptimize` barriers, median reporting, benchmarks for BigInt multiply (3/20/100 limbs), series multiply (T=50/200), etaq, prodmake. Timing toggle (`timing on/off`) in REPL.
**Avoids:** Pitfalls 10 (benchmark noise), 15 (output interference)

### Phase 5: Smart Tab Completion
**Rationale:** Independent of performance work. Benefits from Phase 1 (dimmed hint text). Three sub-features of increasing complexity: LCP fill (LOW), auto-parens (LOW), arg hints (MEDIUM).
**Delivers:** Longest-common-prefix fill on first Tab; auto-parentheses on single function match; argument signature hints when Tab pressed inside empty parens.
**Avoids:** Pitfall 12 (new commands not registered in help table)

### Phase 6: Session Save/Load + History Persistence
**Rationale:** Largest REPL enhancement. Requires the most new code (~80 lines) and touches serialization, file I/O, and the REPL loop. Best done last to avoid churn from earlier phases modifying repl.h. History persistence is independent and can be split out.
**Delivers:** `~/.qseries_history` persistence (1000 lines); `save("file")` writes input history as replayable `.q` script; `load("file")` replays a script from within the REPL.
**Avoids:** Pitfalls 5 (format versioning), 6 (Series serialization), 7 (JacFactor serialization), 14 (unbounded growth)

### Phase Ordering Rationale

- **Performance before presentation:** Karatsuba and series optimization are lower-layer changes with zero risk of conflicting with REPL-layer work. Landing them first means the benchmark suite (Phase 4) can validate them, and all subsequent testing runs faster.
- **ANSI first among REPL features:** It's the thinnest REPL change and establishes the `ansi.h` utility used by later phases (dimmed hint text in tab completion, colored benchmark output).
- **Tab completion before session:** Tab completion modifies `handleTabCompletion()` and `readLineRaw()`. Session save/load adds new builtins and serialization. Keeping them in separate phases reduces merge conflicts in the large `repl.h` file.
- **Benchmarking as a tool phase:** The suite is needed to validate Karatsuba threshold and series optimization, but is also a deliverable. Positioning it after performance work means it can immediately report meaningful results.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 2 (Karatsuba):** Threshold tuning is empirical — needs benchmark-driven experimentation. The signed-intermediate approach for the subtraction step requires careful implementation.
- **Phase 3 (Series optimization):** Dense fast-path density threshold (50%?) needs empirical validation. Laurent series (negative exponents) handling in dense path needs design.

Phases with standard patterns (skip research-phase):
- **Phase 1 (ANSI color):** Well-documented; project already uses ANSI escapes for cursor control. Pattern is established.
- **Phase 4 (Benchmarking):** Standard `steady_clock` + compiler barrier pattern. `doNotOptimize` template is textbook.
- **Phase 5 (Tab completion):** Readline conventions are well-documented; existing `handleTabCompletion()` provides clear extension points.
- **Phase 6 (Session save/load):** Plain text I/O with `fstream`. The replay-based approach (save history, not variables) is straightforward.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Zero new dependencies; all standard library. Verified against official docs. |
| Features | HIGH | Conventions established by IPython, Julia, PARI/GP. Feature classification (table stakes vs differentiator) grounded in real REPL precedents. |
| Architecture | HIGH | Based on direct source analysis of all 11 project files. Touch points identified by line number. |
| Pitfalls | HIGH | Critical pitfalls verified against community reports (GMP Karatsuba docs, ANSI leak issues, readline width bugs). |

**Overall confidence:** HIGH

### Gaps to Address

- **Karatsuba threshold value:** Research recommends ~32 limbs but this is a starting estimate from GMP's data on 32-bit limbs. Actual optimal threshold depends on this codebase's `addAbs`/`subAbs` overhead and must be measured empirically with the benchmark suite.
- **Dense series multiplication density cutoff:** The 50% density threshold for switching from sparse (map) to dense (vector) multiplication is an educated guess. Needs profiling on real q-series workloads (etaq products, theta functions) to find the actual crossover.
- **Session save scope:** Research recommends Level 1 only (save input history as replayable script) over Level 2 (full workspace serialization). If users demand variable persistence, the structured text format from STACK.md provides a viable design, but it requires handling all `EnvValue` variants including future additions.
- **`NO_COLOR` environment variable:** The no-color.org convention is becoming standard. Whether to support it in v4.0 or defer is a prioritization decision.

## Sources

### Primary (HIGH confidence)
- [Microsoft Learn: Console Virtual Terminal Sequences](https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences) — ANSI/VT processing on Windows
- [Microsoft Learn: SetConsoleMode](https://learn.microsoft.com/en-us/windows/console/setconsolemode) — Windows API for enabling VT
- [GMP Manual: Karatsuba Multiplication](https://gmplib.org/manual/Karatsuba-Multiplication) — Algorithm reference and threshold guidance
- [GMP Tuneup Tables](https://gmplib.org/devel/thres/) — Empirical thresholds across architectures
- [IPython documentation](https://ipython.readthedocs.io/en/stable/) — REPL color, history, and save conventions
- [Julia REPL docs](https://docs.julialang.org/en/v1/stdlib/REPL) — Tab completion and color conventions
- [PARI/GP defaults](https://pari.math.u-bordeaux.fr/dochtml/ref/GP_defaults.html) — Math REPL color and history conventions
- Direct codebase analysis of all 11 source files (bigint.h, frac.h, series.h, qfuncs.h, convert.h, linalg.h, relations.h, parser.h, repl.h, main.cpp, main_wasm.cpp)

### Secondary (MEDIUM confidence)
- [Sandor Dargo: Clocks in C++](https://www.sandordargo.com/blog/2025/12/10/clocks-part-4-high_resolution_clock) — steady_clock vs high_resolution_clock analysis
- [Stack Overflow: DoNotOptimize patterns](https://stackoverflow.com/questions/40122141) — Compiler barrier techniques for benchmarking
- [no-color.org](https://no-color.org/) — Convention for disabling terminal colors
- [OhMyREPL.jl](https://kristofferc.github.io/OhMyREPL.jl/latest/) — Julia REPL enhancement patterns

---
*Research completed: 2026-02-28*
*Ready for roadmap: yes*
