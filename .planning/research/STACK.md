# Technology Stack: qseries3 Gap Closure & Improvements

**Project:** qseries REPL — v11.1 Gap Closure & Improvements  
**Researched:** 2026-03-06  
**Scope:** C++ standard/library usage, build system, tooling. What to add vs avoid for closing Maple qseries gaps. Zero-deps constraint enforced.

---

## Executive Summary

The qseries3 codebase achieves 40/41 maple-checklist blocks with a **zero-dependency** stack: C++20, single-source compilation, in-house BigInt/Frac/Series. No GMP, Boost, or package managers. For v11.1 gap closure (phases 85–86, 97–99, remaining Maple functions), **no stack additions are required**. Use existing C++20 standard library and in-place algorithms. Avoid any new external libraries or build-system complexity.

**Recommendation:** Stay on C++20; keep single-file compilation; add no new libraries. Remaining gaps are algorithmic (provemodfuncid, findids, findlincombomodp, collect formatter) and do not demand new stack components.

---

## Current Stack (What Exists)

| Component | Technology | Version / Notes |
|-----------|------------|-----------------|
| Language | C++ | C++20 |
| Compiler | g++ | gcc 13.3+ (Cygwin, Linux), gcc 14 (Docker), MinGW for cross-compile |
| Build | Makefile | Single TU: `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic` |
| Arbitrary precision | BigInt | In-house, base 10⁹, header-only |
| Rationals | Frac | In-house over BigInt |
| Algebraic | Omega3 | Q(ω), ω² = -ω-1, in-house |
| Data structures | std::* | map, vector, set, deque, optional, variant |
| I/O | iostream, termios | POSIX; Windows: io.h, windows.h, SetConsoleMode |
| Wasm | Emscripten | em++ for web build; optional |
| Docker | gcc:14-bookworm | Multi-stage; scratch base image |
| LDFLAGS | -static | Optional for static binary |

**Standard library usage:** string, vector, map, set, algorithm, iostream, sstream, fstream, chrono, variant, optional, stdexcept, cmath, cstdint, climits, cctype, cstdlib, functional, tuple, numeric. No third-party headers beyond POSIX/Windows and emscripten for Wasm.

---

## Stack Gaps for Gap Closure (What Could Be Needed)

### Phases 85–86: Theta IDs Prover, RR Identity Search

| Feature | Stack Implication | Verdict |
|---------|-------------------|---------|
| provemodfuncid (Gamma_1(N)) | Sturm bound, cusp orders, q-series comparison | Use existing eta_cusp, theta_ids, Series, Frac. No new libs. |
| findids(type, T) search | Iteration over eta products, checkid | Uses convert.h, eta_cusp.h, rr_ids.h. Pure C++. |
| RRG, RRH, RRGstar, RRHstar | JAC products, aqprod | Already in rr_ids.h; gap is completion, not stack. |
| checkid | prodmake / etamake / jacprodmake | Existing convert.h. No change. |

**Conclusion:** No stack changes. All algorithms reduce to existing Series, Frac, BigInt, and header modules.

### Phases 97–99: Block 25, Block 24, findlincombomodp

| Feature | Stack Implication | Verdict |
|---------|-------------------|---------|
| Block 25 (findpoly on theta quotients) | q-shift alignment | Series::addAligned already implemented. No stack change. |
| Block 24 (collect-style formatter) | String formatting, term grouping | std::string, std::map. No new lib. |
| findlincombomodp(f, L, p, T) | Linear algebra over F_p | Extend linalg.h; use existing modp, F_p kernel. No new lib. |

**Conclusion:** No stack changes. Extend existing linalg, relations, repl display logic.

### modforms Package Parity (EISENq, makebasisM, etc.)

| Feature | Stack Implication | Verdict |
|---------|-------------------|---------|
| EISENq(k, q, T) | Eisenstein series | sigma, divisor sum, q-series; qfuncs.h, modforms.h. Done. |
| makebasisM, makeALTbasisM | E4, E6, Delta_12 | modforms.h. Done. |
| Phiq, dilly | q-dilation, partition GF | qfuncs.h patterns. No new stack. |

---

## C++ Standard and Library Usage

### What to Use (Stay With)

| Feature | When | Why |
|---------|------|-----|
| C++20 | Always | Project standard; good compiler support (gcc 10+). |
| std::map, std::vector, std::set | Sparse/dense structures | Ordered iteration for Series; no unordered_map needed. |
| std::variant, std::optional | EnvValue, AST, optional results | Type-safe unions; no boost::variant. |
| std::chrono | Timing | REPL per-command timing. |
| std::string, std::ostringstream | Display, formatting | No fmt, spdlog. |
| std::algorithm (sort, transform, etc.) | Kernels, RREF | Sufficient for linalg. |
| <cmath> | sqrt, floor, etc. | Machine ints; no extended precision from cmath. |

### What NOT to Add

| Avoid | Reason | Use Instead |
|-------|--------|-------------|
| C++23 | Unnecessary; some targets may not support | C++20 |
| std::mdspan, std::expected | Nice-to-have; not required for current algos | Existing patterns |
| <execution> parallelism | Algorithms are sequential; parallelism adds complexity | Sequential |
| std::format (C++20) | Limited adoption on older gcc; ostringstream works | ostringstream, manual formatting |
| std::ranges (full) | Partial use is fine; full refactor not justified | Existing loops, algorithms |

### Potential C++20 Usage (If Not Already)

| Feature | Benefit | Risk |
|---------|---------|------|
| `[[nodiscard]]` | Catch ignored return values | Low; add sparingly |
| `constexpr` where applicable | Compile-time checks | Low; limited gain for REPL |
| Structured bindings | Cleaner code | Already used |
| `std::span` | Bounds-checked views | Optional; vector works fine |

---

## Build System

### Current: Makefile

- Single TU: `src/main.cpp` includes all headers.
- Targets: `all`, `clean`, `test`, `acceptance*`, `bench`, `wasm`, `docker-build`.
- CXX override: `make CXX=x86_64-w64-mingw32-g++` for Windows cross-compile.
- LDFLAGS: Optional `-static` for static linking.

### What to Keep

| Practice | Reason |
|----------|--------|
| Single main.cpp | Simplicity; no link-step; static binary trivial |
| Makefile (not CMake/meson) | Zero deps; Make is universal |
| CXX ?= g++ | Cygwin/Linux default; MinGW override when needed |
| -Wall -Wextra -Wpedantic | Zero warnings policy |

### What NOT to Add

| Avoid | Reason | Use Instead |
|-------|--------|-------------|
| CMake, Meson | Adds build dependency; overkill for single binary | Makefile |
| Conan, vcpkg | Package managers; project is zero-deps | Header-only, in-house |
| Separate .o/.a | Increases complexity; single TU is fast enough | Single TU |
| Precompiled headers | Marginal gain; added complexity | Current build |
| Ninja | Optional; Make suffices | Make |

### Debug Build (Already Present)

```
debug: g++ -std=c++20 -g -O0 -fsanitize=address,undefined -o qseries_debug src/main.cpp
```

Keep this. Sanitizers are part of the toolchain, not external deps.

---

## Tooling

### Compiler

| Tool | Recommendation | Notes |
|------|----------------|-------|
| g++ 10+ | Minimum | C++20 support |
| g++ 13+ | Preferred | Better diagnostics, C++20 completeness |
| Clang | Optional | `CXX=clang++` should work; test if needed |
| MSVC | Not primary | Windows via MinGW cross-compile |

### Static Analysis

| Tool | Verdict | Reason |
|------|---------|--------|
| -Wall -Wextra -Wpedantic | Use | Zero warnings |
| -fsanitize=address,undefined | Use (debug) | Catches memory/UB |
| cppcheck, clang-tidy | Optional | No install required; CI could add later |
| Coverity, SonarQube | Skip | Heavy; not justified for this scope |

### Testing

| Tool | Current | Recommendation |
|------|---------|----------------|
| Shell scripts | tests/acceptance*.sh, maple-checklist.sh | Keep; no framework needed |
| Unit tests | In main.cpp (runUnitTests) | Keep |
| make acceptance-all | Full regression | Keep as gate |

---

## What Must NOT Be Added (Zero-Deps)

| Category | Do Not Add | Rationale |
|----------|------------|-----------|
| **Arithmetic** | GMP, MPFR, NTL, Flint, PARI | BigInt + Frac + Omega3 are in-house |
| **Rational/arbitrary precision** | Boost.Multiprecision, libgmp | SPEC: zero external libs |
| **Build** | CMake, Conan, vcpkg, npm | Makefile + single TU |
| **Parsing** | lex/yacc, ANTLR, re2c | Recursive descent in parser.h |
| **Logging** | spdlog, fmt, log4cxx | iostream, custom formatting |
| **JSON/XML** | RapidJSON, pugixml | Session format is simple text |
| **Terminal** | readline, libedit, ncurses | Raw termios + custom readLineRaw |
| **Networking** | libcurl, Boost.Asio | Not applicable |
| **Cryptography** | OpenSSL, libsodium | Not applicable |

The project explicitly forbids: "No GMP, no Boost, no package managers." All bignum and exact arithmetic are built from scratch.

---

## Summary: Add vs Avoid

### Add (Minimal, Optional)

| Item | Purpose | Priority |
|------|---------|----------|
| `[[nodiscard]]` on critical returns | Safety | Low |
| cppcheck in CI (if zero install cost) | Static analysis | Low |
| Clarify CXXFLAGS in Makefile (e.g., LDFLAGS doc) | Build clarity | Low |

### Avoid (Non-Negotiable)

| Item | Reason |
|------|--------|
| Any external library | Zero-deps constraint |
| CMake, Meson, Conan, vcpkg | Build simplicity |
| C++23 requirement | Portability; C++20 sufficient |
| Parallelism (std::execution, threads) | Not needed for current workloads |
| fmt, spdlog, GMP, Boost | Explicitly excluded |

### No Change Needed

| Area | Status |
|------|--------|
| C++ standard | C++20 |
| Compiler | g++ |
| Build system | Makefile |
| Standard library | Current set is sufficient |
| Headers | Header-only in-house modules |

---

## Phase-Specific Stack Notes

| Phase | Stack Impact | Action |
|-------|--------------|--------|
| 85 Theta IDs Prover | None | Use theta_ids, eta_cusp, Series |
| 86 RR Identity Search | None | Use rr_ids, convert, qfuncs |
| 97 Block 25 | None | addAligned already in Series |
| 98 Block 24 | None | String/display in repl.h |
| 99 findlincombomodp | None | Extend linalg.h F_p solve |

---

## Sources

- SPEC.md — Zero-deps, single static binary, no GMP/Boost
- .cursorrules — ZERO external dependencies
- CLAUDE.md — Build: g++ -std=c++20 -O2 -static
- .planning/ROADMAP.md — Phases 85–102
- gaps/wprogmodforms.txt — modforms package structure
- gaps/misc.txt — EISENq, dilly, misc functions
