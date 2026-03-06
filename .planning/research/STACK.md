# Technology Stack: Code Health Audit & Remediation

**Project:** qseries3 — C++20 single-binary q-series REPL  
**Researched:** 2026-03-06  
**Scope:** Stack additions for code health only — warnings, static analysis, coverage, sanitizers, tech-debt tooling.  
**Constraint:** Zero runtime dependencies. Dev tools (clang-tidy, cppcheck, lcov, bear) are acceptable; they do not ship with the binary.

---

## Executive Summary

For a **zero-dependency C++20** project built with GCC and Makefile, code health can be improved without adding runtime dependencies. Use **compiler flags** (extra warnings), **GCC sanitizers** (already in use), **cppcheck** (standalone), **clang-tidy** (optional, via Bear), and **gcov/lcov** for coverage. Do **not** add GTest, CMake, or any libraries — the project stays single-binary, single-TU.

**Recommendation:** Add `-Wshadow` to the build; adopt cppcheck in CI; add coverage and sanitizer Make targets; optionally add clang-tidy for deeper C++ analysis.

---

## Recommended Stack

### Compiler & Warning Flags

| Flag / Option | Version | Purpose | Why |
|---------------|---------|---------|-----|
| g++ | 13+ | Compiler | SPEC requires 13.3; C++20 full support |
| -std=c++20 | — | Language | Project standard |
| -Wall -Wextra -Wpedantic | — | Base warnings | Already in Makefile; keep |
| -Wshadow | — | Variable shadowing | Catches subtle bugs from name collisions; not in -Wall |
| -Wconversion | — | Implicit conversions | Optional; often noisy in math code; add later if feasible |

**Current Makefile:** `CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Wpedantic`  
**Add:** `-Wshadow` (low noise, high value).  
**Defer:** `-Wconversion` — qseries uses integer/size_t conversions; enable incrementally if desired.

**Note:** `.github/workflows/release.yml` uses `-Wall -Wextra` but not `-Wpedantic`. Align with Makefile for consistency.

---

### Static Analysis

| Tool | Version | Purpose | Integration | Zero-Dep? |
|------|---------|---------|-------------|-----------|
| **cppcheck** | 2.x | C++ static analysis | `cppcheck --enable=warning,style,performance src/` | Yes — standalone |
| **clang-tidy** | 18+ | C++ linter, checks | Needs `compile_commands.json` via Bear | Yes — dev tool only |
| GCC -fanalyzer | 13+ | Static analyzer | C-focused; weak C++ support | Yes — built-in |

**Recommendation:** Use **cppcheck** as primary — no `compile_commands.json`, works directly on `src/`. Add **clang-tidy** only if Bear/compiledb is available (Linux/Cygwin).

**What NOT to use:** GCC `-fanalyzer` — documented as C-focused; C++ support is limited in GCC 13/14.

---

### Test Coverage

| Tool | Version | Purpose | Integration | Zero-Dep? |
|------|---------|---------|-------------|-----------|
| **gcov** | (GCC) | Coverage instrumentation | `-fprofile-arcs -ftest-coverage -g -O0` | Yes — GCC built-in |
| **lcov** | 2.x | Report generation | `lcov --capture`, `genhtml` | Yes — dev tool |

**Workflow:**
1. Build with `--coverage` (or `-fprofile-arcs -ftest-coverage`) and `-g -O0`
2. Run acceptance tests (bash scripts)
3. `lcov --capture --directory . --output-file coverage.info`
4. `lcov --remove coverage.info '/usr/*'` — filter system headers
5. `genhtml coverage.info --output-directory coverage-report`

**Note:** Single-TU (`src/main.cpp`) means one `.gcno`/`.gcda` pair. Use `-O0` to reduce inlining and get accurate line coverage.

---

### Sanitizers

| Sanitizer | Flag | Purpose | Zero-Dep? |
|-----------|------|---------|-----------|
| AddressSanitizer | -fsanitize=address | Use-after-free, buffer overflows | Yes — libasan in GCC |
| UndefinedBehaviorSanitizer | -fsanitize=undefined | UB (overflow, null-deref) | Yes — libubsan in GCC |
| LeakSanitizer | -fsanitize=leak | Memory leaks | Yes — part of ASan |

**Current:** Makefile `debug` target already uses `-fsanitize=address,undefined`.

**Recommendation:** Keep as-is. LeakSanitizer can be added: `-fsanitize=address,undefined,leak`. ThreadSanitizer is irrelevant (single-threaded REPL).

**Restriction:** ASan, MSan, TSan are mutually exclusive. Do not combine them.

---

### Compilation Database (for clang-tidy)

| Tool | Version | Purpose | Platform |
|------|---------|---------|----------|
| **Bear** | 3.x | Intercepts `make` to generate `compile_commands.json` | Linux, Cygwin |
| **compiledb** | — | Alternative; parses make output | If Bear unavailable |

**Usage:**
```bash
bear -- make clean all
# Creates compile_commands.json in project root
clang-tidy -p . src/main.cpp
```

---

## Installation

```bash
# Cygwin / Linux — development tools (optional)
# cppcheck
apt-get install cppcheck        # Debian/Ubuntu
# or: Cygwin Setup → cppcheck

# lcov (for coverage)
apt-get install lcov

# Bear (for clang-tidy)
apt-get install bear

# clang-tidy (optional)
apt-get install clang-tidy
```

No `npm`, `pip`, or package-manager deps for the **binary**. All tools are build-time only.

---

## Integration Points

| Concern | Where | Action |
|---------|-------|--------|
| Warnings | Makefile `CXXFLAGS` | Add `-Wshadow` |
| Release CI | `.github/workflows/release.yml` | Add `-Wpedantic` for parity |
| Static analysis | Makefile, CI | Add `make cppcheck` target |
| Coverage | Makefile | Add `make coverage` target; run acceptance tests, then lcov |
| Sanitizers | Makefile `debug` | Already present; optional: add `-fsanitize=leak` |
| clang-tidy | Manual / CI | `bear -- make` then `clang-tidy -p . src/main.cpp` |

---

## What NOT to Add

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| GTest, Catch2 | Adds test framework dep; project uses bash acceptance tests | Keep bash scripts; coverage via gcov on instrumented binary |
| CMake | Project uses Makefile; adds build complexity | Stay with Makefile |
| Valgrind | External; higher overhead | Sanitizers (ASan, UBSan) are compiler-integrated |
| GCC -fanalyzer | Weak C++ support | cppcheck, clang-tidy |
| -Wconversion (initially) | Often very noisy in math/integer code | Add later, with suppressions if needed |
| Readline, Boost, GMP | Runtime dependencies | Out of scope — already excluded |

---

## Makefile Additions (Suggested)

```makefile
# Stricter warnings (code health)
CXXFLAGS_STRICT = $(CXXFLAGS) -Wshadow

# Static analysis
cppcheck:
	cppcheck --enable=warning,style,performance --inline-suppr -I src src/

# Coverage build
coverage: CXXFLAGS_COV = -std=c++20 -g -O0 -Wall -Wextra -Wpedantic -fprofile-arcs -ftest-coverage
coverage: LDFLAGS_COV = -fprofile-arcs -ftest-coverage -lgcov
coverage: clean
	$(CXX) $(CXXFLAGS_COV) $(LDFLAGS_COV) -o qseries_coverage src/main.cpp
	@echo "Run: ./qseries_coverage < tests/input; then make coverage-report"

coverage-report:
	lcov --capture --directory . --output-file coverage.info
	lcov --remove coverage.info '/usr/*' '*/tests/*' --output-file coverage.filtered.info
	genhtml coverage.filtered.info --output-directory coverage-report
	@echo "Open coverage-report/index.html"
```

---

## Alternatives Considered

| Instead of | Alternative | Verdict |
|------------|-------------|---------|
| cppcheck | clang-tidy | Use cppcheck first (no compile DB); clang-tidy if Bear available |
| gcov | llvm-cov | llvm-cov requires Clang; project uses GCC — stick with gcov |
| Sanitizers | Valgrind | Sanitizers are faster and zero-dependency; keep sanitizers |
| -Werror | — | Optional for CI; project may want warnings-as-errors in a later phase |

---

## Version Compatibility

| Tool | Min Version | Notes |
|------|-------------|-------|
| GCC | 13.3 | Per SPEC; gcov, sanitizers included |
| cppcheck | 2.0+ | 2.12+ preferred for C++20 |
| clang-tidy | 18+ | Match C++20; 17 may work |
| lcov | 2.0+ | genhtml for HTML reports |
| Bear | 3.0+ | compile_commands.json generation |

---

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Warnings | HIGH | GCC docs, Stack Overflow consensus |
| Sanitizers | HIGH | GCC built-in; project already uses them |
| cppcheck | HIGH | Standalone, well-documented |
| clang-tidy | MEDIUM | Needs Bear; GCC flags may need filtering |
| Coverage | HIGH | gcov/lcov standard for GCC projects |

---

## Sources

- GCC Warning Options: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html
- GCC Static Analyzer: https://gcc.gnu.org/onlinedocs/gcc/Static-Analyzer-Options.html — C-focused, weak C++
- clang-tidy: https://clang.llvm.org/extra/clang-tidy/
- cppcheck manual: https://cppcheck.sourceforge.io/manual.html
- Bear: https://github.com/rizsotto/Bear — compile_commands.json from make
- gcov/lcov with Makefile: Stack Overflow, svnscha.de, smhk.net (2024)
- Sanitizers: cppcheatsheet.com, GCC docs — libasan/libubsan in GCC

---

*Stack research for: Code health audit and remediation — zero-dependency C++20 project*
