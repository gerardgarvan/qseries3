# Technology Stack

**Project:** Q-Series REPL (zero-dependency C++ mathematical software)
**Researched:** 2025-02-24
**Confidence:** HIGH

## Why Zero Dependencies?

The project mandates ZERO external libraries (no GMP, Boost, package managers). Rationale:
- **Portability** — Single binary runs anywhere with no install step
- **Reproducibility** — No version drift, no dependency hell
- **Binary size** — Target < 2MB; GMP adds significant bloat
- **Learning/control** — All arithmetic is explicit and auditable

This is a deliberate design choice, not a limitation. The q-series domain (partition theory, modular forms) operates on moderate-sized integers: coefficients rarely exceed ~100 digits. Roll-your-own BigInt and exact rationals are well within reach.

---

## Recommended Stack

### Core Compiler & Standard

| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| g++ (GCC) | 11+ (13.3 recommended) | Compiler | Full C++20 support (modules, coroutines, concepts). GCC 11+ implements all C++20 features used by q-series REPL; GCC 13.3 is specified in SPEC.md. |
| C++20 | `-std=c++20` | Language standard | Structured bindings, `std::gcd` (C++17), `std::lcm`, concepts, `[[nodiscard]]`, three-way comparison. Required by project. |

**GCC C++20 status:** GCC 11+ has solid C++20 support. C++20 is marked "experimental" in GCC docs through GCC 14, but in practice core features (concepts, coroutines, ranges) work. For this project, GCC 11+ is sufficient; GCC 13.3 is the project-specified version.

### Build Configuration

| Flag | Purpose | Why |
|------|---------|-----|
| `-std=c++20` | Enable C++20 | Project requirement |
| `-O2` | Optimization | Balances speed and compile time; `-O3` may increase binary size with little gain for this workload |
| `-static` | Static linking | Produce single binary with no runtime DLLs |
| `-Wall -Wextra` | Warnings | Catch bugs early |

**Build command:**
```bash
g++ -std=c++20 -O2 -static -Wall -Wextra -o qseries main.cpp
```

**Windows cross-compile:**
```bash
x86_64-w64-mingw32-g++ -std=c++20 -O2 -static -o qseries.exe main.cpp
```
Note: MinGW-w64 static linking may still pull in `libwinpthread` on some setups; test on target. For pure POSIX (Cygwin, Linux), `-static` produces a fully self-contained binary.

---

## BigInt Implementation Strategy

### Base Choice: 10^9

| Option | Pros | Cons | Verdict |
|--------|------|------|---------|
| Base 10^9 | Two digits fit in `uint64_t` (9+9 decimal digits); trivial decimal I/O; cp-algorithms reference | Slightly larger limb count than base 2^32 | **Use this** |
| Base 2^32 | Max limb utilization; bitwise ops | Awkward decimal conversion; more complex I/O | Skip |
| Base 10 | Simplest I/O | 10x more limbs; very slow | Skip |

**Implementation:** Store digits in `std::vector<uint32_t>`, least-significant first. Each digit in `[0, 999999999]`. Use `uint64_t` for intermediate products (e.g. `a[i] * b[j]` fits).

**Reference:** [cp-algorithms Big Integer](https://cp-algorithms.com/algebra/big-integer.html) — canonical base-10^9 approach.

### Multiplication: Schoolbook Only

| Algorithm | Complexity | Crossover | Verdict |
|-----------|------------|-----------|---------|
| Schoolbook | O(n²) | — | **Use this** |
| Karatsuba | O(n^1.585) | ~320–640 bits (~100–200 decimal digits) | Skip for q-series |

**Rationale:** SPEC.md states "our numbers rarely exceed ~100 digits." Karatsuba's crossover is typically 100+ digits; implementation overhead and constant factors often make schoolbook faster at this scale. Stack Overflow benchmarks show schoolbook beating poorly optimized Karatsuba on 500-digit numbers. **Do not add Karatsuba** until profiling proves it necessary.

### Division: Long Division with Binary-Search Quotient

**Algorithm:** Standard long division in base 10^9. For each quotient digit, binary-search in `[0, BASE-1]` for the largest `k` where `k * divisor <= remainder`. This is the trickiest BigInt operation; get borrowing and normalization right.

**Reference:** [janmr.com Multiple-Precision Long Division](https://janmr.com/blog/2014/04/basic-multiple-precision-long-division/), GMP Basecase Division docs. SPEC.md explicitly recommends binary-search quotient digit selection.

---

## Rational Arithmetic (Frac)

### Strategy: BigInt-Based, GCD on Every Operation

| Option | Use? | Reason |
|--------|------|--------|
| `std::ratio` | No | Compile-time only; cannot hold runtime BigInt values |
| Boost.Rational | No | External dependency; ruled out |
| GMP `mpq_class` | No | External dependency; ruled out |
| **Roll your own** | **Yes** | `struct Frac { BigInt num, den; };` with `reduce()` after every constructor and arithmetic op |

**Critical:** Call `reduce()` after every Frac operation. `reduce()` must: (1) ensure `den > 0` (flip signs if needed), (2) compute `g = gcd(|num|, den)` and divide both by `g`, (3) normalize zero as `0/1`. Forgetting reduce causes **exponential BigInt growth** — a common bug.

**GCD:** Use Euclidean algorithm with `BigInt::divmod`. C++17 `std::gcd` only works with built-in integers; implement `bigGcd(BigInt a, BigInt b)` yourself.

---

## Header-Only Architecture

| Pattern | Purpose |
|---------|---------|
| All code in `.h` files | Single `main.cpp` includes headers; one compilation unit |
| `inline` on all functions | Avoid ODR violations when definitions appear in headers |
| Structs with methods, no virtual | No vtable; zero abstraction overhead |
| `#include` order matters | `bigint.h` → `frac.h` → `series.h` → ... (dependency order) |

**Why header-only:** Simplifies build (no `.cpp` objects to link), keeps binary small, matches SPEC.md layout. Use `inline` so the linker can merge duplicate definitions.

---

## REPL & Parser: Roll Your Own

**No external parser libraries** — use recursive-descent. The expression language is small (assignments, arithmetic, function calls, `sum`). Recursive-descent is ~200–400 lines and gives full control. Libraries like Boost.Spirit or flex/bison add dependencies and complexity; avoid them.

**Line editing:** Simple `std::getline` with a history buffer is sufficient. Do not add readline (external dep). Arrow-key editing is a stretch goal; basic history (up/down) can be implemented with a `std::vector<std::string>` and index.

---

## What NOT to Use

| Avoid | Why | Use Instead |
|-------|-----|-------------|
| GMP, libgmp | External dependency; violates zero-deps constraint | Roll BigInt + Frac |
| Boost (any) | External dependency; large | std + roll your own |
| Karatsuba/FFT for multiplication | Premature; numbers too small | Schoolbook |
| `float` / `double` in math pipeline | Breaks exactness; q-series require exact rationals | Frac only |
| Base 10 or base 2^64 for BigInt | Base 10 too slow; 2^64 overflows in digit product | Base 10^9 |
| Separate .cpp files | Adds link step; header-only is simpler | All inline in headers |

---

## Binary Size Target (< 2MB)

With `-static` and only the standard library:
- BigInt + Frac + Series + qfuncs + convert + linalg + relations + parser + repl: typically 500KB–1.5MB
- Static libstdc++ adds ~1–2MB depending on what’s used
- **Total < 2MB is achievable** if you avoid pulling in heavy STL features (e.g. regex, iostreams abuse). Prefer `printf`/`puts` for simple output if size becomes an issue.

---

## Version Compatibility

| Component | Minimum | Recommended | Notes |
|-----------|---------|-------------|-------|
| GCC | 11 | 13.3 | C++20 features used: concepts, `[[nodiscard]]`, structured bindings |
| C++ standard | C++20 | C++20 | Non-negotiable |
| Standard library | libstdc++ (GCC) | — | No libc++ required |

---

## Sources

| Source | Confidence | Topics |
|--------|------------|--------|
| [cp-algorithms Big Integer](https://cp-algorithms.com/algebra/big-integer.html) | HIGH | Base 10^9, schoolbook algorithms |
| [GCC C++ Standards Support](https://www.gnu.org/software/gcc/projects/cxx-status.html) | HIGH | C++20 feature availability |
| [GCC Link Options (-static)](https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html) | HIGH | Static linking |
| [janmr.com Multiple-Precision Long Division](https://janmr.com/blog/2014/04/basic-multiple-precision-long-division/) | MEDIUM | Division algorithm |
| [Stack Overflow: BigInteger performance](https://stackoverflow.com/questions/23967334/biginteger-numbers-implementation-and-performance/23967901) | MEDIUM | Schoolbook vs Karatsuba |
| SPEC.md, .cursorrules (workspace) | HIGH | Project constraints, BigInt/Frac design |
| WebSearch: C++ BigInt without GMP, GCC static, MinGW | MEDIUM | Ecosystem patterns |

---

*Stack research for: Zero-dependency C++ q-series mathematical REPL*
*Researched: 2025-02-24*
