# Technology Stack: v4.0 Core Improvements

**Project:** q-series REPL — ANSI color, session save/load, Karatsuba multiplication, benchmarking
**Researched:** 2026-02-28
**Overall confidence:** HIGH (all areas use zero-dependency C++ standard library patterns verified against official docs)

## Recommended Stack

No new dependencies. All four features are implementable within the existing zero-dependency C++20 constraint using standard library facilities and well-documented algorithms.

### Core Framework (unchanged)

| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| C++20 | g++ 13+ | Language standard | Already in use; `<chrono>`, `<fstream>`, `<sstream>` cover all new needs |
| g++ | 13+ | Compiler | `-std=c++20 -O2 -static` unchanged |

### New Headers Required (all standard library)

| Header | Purpose | Feature |
|--------|---------|---------|
| `<fstream>` | File I/O for session save/load | Session persistence |
| `<sstream>` | String stream parsing for session load | Session persistence |
| `<filesystem>` | (Optional) Path existence checks | Session persistence |
| `<chrono>` | Timing measurements | Benchmarking (already included) |

No new external libraries. No changes to build command.

---

## Feature 1: ANSI Color Output

### Approach: Direct ANSI SGR Escape Codes

**Confidence: HIGH** — Verified against Microsoft Learn documentation and POSIX terminal standards.

ANSI SGR (Select Graphic Rendition) escape codes are the only viable zero-dependency approach. The project already uses `\033[K` (erase line) and `\033[nD` (cursor back) in `redrawLineRaw()`, confirming the pattern works.

### Cross-Platform Strategy

**Unix/Cygwin:** ANSI codes work natively in all modern terminals. No setup required.

**Windows (native, non-Cygwin):** Must enable `ENABLE_VIRTUAL_TERMINAL_PROCESSING` (value `0x0004`) on the stdout handle via `SetConsoleMode()`. This is supported on Windows 10 build 14393+ (Anniversary Update, 2016) and all later versions including Windows 11.

**Emscripten/WASM:** Strip ANSI codes entirely — the web frontend handles its own styling.

### Recommended Color Scheme

Use only the 8 standard foreground colors (codes 30–37) for maximum compatibility. Avoid 256-color or RGB modes.

| Element | Color | Code | Rationale |
|---------|-------|------|-----------|
| Prompt `qseries>` | Cyan | `\033[36m` | Visually distinct from output, not distracting |
| Errors | Red | `\033[31m` | Universal error convention |
| Timing | Dark gray (bright black) | `\033[90m` | De-emphasize meta-info |
| Product results | Green | `\033[32m` | Highlight key results |
| Series output | Default | (none) | Most common output, keep neutral |
| Reset | — | `\033[0m` | After every colored segment |

### Implementation Pattern

```cpp
namespace ansi {
    inline bool enabled = false;

    inline void init() {
#ifdef __EMSCRIPTEN__
        enabled = false;
#elif defined(_WIN32) && !defined(__CYGWIN__)
        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD mode = 0;
        if (hOut != INVALID_HANDLE_VALUE && GetConsoleMode(hOut, &mode)) {
            mode |= 0x0004; // ENABLE_VIRTUAL_TERMINAL_PROCESSING
            enabled = SetConsoleMode(hOut, mode) != 0;
        }
#else
        enabled = isatty(STDOUT_FILENO);
#endif
    }

    inline const char* red()   { return enabled ? "\033[31m" : ""; }
    inline const char* green() { return enabled ? "\033[32m" : ""; }
    inline const char* cyan()  { return enabled ? "\033[36m" : ""; }
    inline const char* gray()  { return enabled ? "\033[90m" : ""; }
    inline const char* reset() { return enabled ? "\033[0m"  : ""; }
    inline const char* bold()  { return enabled ? "\033[1m"  : ""; }
}
```

### Key Details

1. **Call `ansi::init()` once** at REPL startup, before the banner.
2. **Guard all codes behind `enabled`** — returns empty string when off, so no conditional logic at call sites.
3. **Always emit `\033[0m` (reset)** after each colored segment to prevent color bleed.
4. **The existing `redrawLineRaw()` already uses raw escape codes** (`\033[K`, `\033[nD`) — these cursor-control codes are distinct from SGR color codes and should continue to work regardless of color enablement.
5. **Pipe detection:** On Unix, `isatty(STDOUT_FILENO)` returns false when output is piped, automatically suppressing colors. On Windows, `GetConsoleMode()` fails on pipe handles, achieving the same effect.
6. **User override:** Consider a `color(on/off)` REPL command to let users toggle at runtime. Store in Environment.

### Platform Verification

| Platform | Status | Notes |
|----------|--------|-------|
| Linux terminal | Works natively | `isatty()` check only |
| macOS Terminal / iTerm2 | Works natively | Same as Linux |
| Cygwin / MSYS2 | Works natively | Uses Unix path (`__CYGWIN__` defined) |
| Windows Terminal | Requires `SetConsoleMode()` | Windows 10 1607+ |
| CMD.exe (Win10+) | Requires `SetConsoleMode()` | Same API |
| Older Windows (<10) | `SetConsoleMode()` fails gracefully | Falls back to no color |
| WASM/Emscripten | Disabled | Web UI handles styling |
| Piped output | Auto-disabled | `isatty()` / `GetConsoleMode()` detects |

---

## Feature 2: Session Save/Load

### Approach: Custom Human-Readable Text Format

**Confidence: HIGH** — Uses only `<fstream>`, `<sstream>`, and existing `str()` methods.

### Why Not JSON/TOML/etc.

Zero-dependency constraint eliminates all serialization libraries. A custom format is fine because:
- The data model is simple (name → Series or JacFactor list)
- We already have `str()` on BigInt, Frac, and Series
- Parsing is straightforward with existing BigInt/Frac constructors

### Session File Format (`.qsession`)

```
# qseries session — saved 2026-02-28 14:30:05
# truncation
T = 50

# variables
# VAR name type
# SERIES name trunc coeff_count
#   exp num den
#   exp num den
#   ...
# JAC name factor_count
#   a b exp_num exp_den
#   ...

SERIES f 50 3
  0 1 1
  1 -1 1
  2 1 1

SERIES g 50 2
  0 1 1
  5 -1 2

JAC h 2
  1 5 1 1
  4 5 1 1
```

### Format Specification

```
Line types:
  # ...              → comment (skip)
  T = <int>          → set truncation order
  SERIES <name> <trunc> <count>   → begin series block
    <exp> <num> <den>             → coefficient (one per line, indented)
  JAC <name> <count>              → begin Jacobi product block
    <a> <b> <exp_num> <exp_den>   → factor (one per line, indented)
  (blank line)       → skip
```

### Implementation Strategy

**Save (`save("filename")`):**
1. Open file with `std::ofstream`
2. Write header comment with timestamp
3. Write `T = <value>`
4. For each variable in `env.env`:
   - If `Series`: write `SERIES name trunc coeff_count`, then one `exp num den` per line
   - If `vector<JacFactor>`: write `JAC name factor_count`, then one `a b exp_num exp_den` per line
5. Use `BigInt::str()` for all integer values (handles arbitrary precision)

**Load (`load("filename")`):**
1. Open file with `std::ifstream`
2. Parse line by line with `std::getline` + `std::istringstream`
3. Reconstruct BigInt from string via `BigInt(const std::string&)` constructor (already exists)
4. Reconstruct Frac from num/den BigInts
5. Reconstruct Series by inserting coefficients into map
6. Insert into `env.env`

### Key Details

1. **Use `BigInt::str()` and `BigInt(const std::string&)`** — both already exist and handle arbitrary precision correctly, including negative numbers.
2. **Separate num/den with space, not "/"** — avoids ambiguity in parsing (the `/` in `Frac::str()` would require splitting on `/` which is fragile).
3. **Coefficients stored in reduced form** — Frac auto-reduces on construction, so saved values are already canonical.
4. **Overwrite vs. merge:** Load should clear existing environment or merge. Recommend merge (add/overwrite variables, preserve unlisted ones) with a `reset()` command for clearing.
5. **Error handling:** Report line number on parse errors. Skip malformed lines with a warning rather than aborting the entire load.
6. **File extension:** `.qsession` — distinctive, unlikely to conflict.

### Edge Cases

| Case | Handling |
|------|----------|
| Empty series (no coefficients) | `SERIES name trunc 0` — valid, creates `Series::zero(trunc)` |
| Negative exponents | Stored as-is (BigInt handles negative signs) |
| Very large coefficients | `BigInt::str()` handles arbitrary size |
| Overwriting existing variable | Silently replace on load (standard REPL semantics) |
| Missing file | Throw clear error with filename |
| Write permission denied | Catch and report `std::ofstream::fail()` |

---

## Feature 3: Karatsuba Multiplication

### Approach: Hybrid Schoolbook/Karatsuba in `BigInt::operator*`

**Confidence: HIGH** — Algorithm is textbook (Knuth 4.3.3); threshold requires empirical tuning.

### Current Multiplication: O(n²) Schoolbook

The existing `BigInt::operator*` (bigint.h lines 203–220) is a standard schoolbook multiplication using base 10^9 limbs with uint64_t intermediates for carry. This is optimal for small operands but becomes the bottleneck for large coefficients.

### When Karatsuba Matters for This Project

In q-series computation, BigInt size growth depends on:
- **Truncation order T:** Higher T → more terms → bigger coefficients from accumulation
- **Series operations:** Multiplication and `inverse()` produce the largest coefficients
- **Typical sizes:** At T=50, coefficients are usually 1-3 limbs (≤27 digits). At T=200, coefficients can reach 10-20+ limbs. At T=500+, some coefficients may exceed 50 limbs.

**Bottom line:** Karatsuba won't help at default T=50 but becomes significant at T=200+ where partition-function-scale coefficients appear.

### Algorithm (from GMP / Knuth)

Given two BigInts with limb vectors `x` and `y`, each of length `n` (pad shorter to match):

```
Split at m = n/2:
  x = x1 * B^m + x0       (x1 = high limbs, x0 = low limbs)
  y = y1 * B^m + y0

Three sub-products:
  z2 = x1 * y1
  z0 = x0 * y0
  z1 = (x0 + x1) * (y0 + y1) - z2 - z0

Result:
  x * y = z2 * B^(2m) + z1 * B^m + z0
```

Where B = 10^9 (our base) and "B^m" means shifting by m limb positions.

### Implementation Recommendations

```cpp
// In BigInt struct, replace operator* body:

static constexpr size_t KARATSUBA_THRESHOLD = 32; // tune empirically

static void mulSchoolbook(const uint32_t* a, size_t na,
                          const uint32_t* b, size_t nb,
                          uint32_t* out) {
    // existing O(n²) code, operating on raw arrays
}

static void mulKaratsuba(const uint32_t* a, size_t na,
                         const uint32_t* b, size_t nb,
                         uint32_t* out, uint32_t* scratch) {
    size_t n = std::max(na, nb);
    if (n < KARATSUBA_THRESHOLD) {
        mulSchoolbook(a, na, b, nb, out);
        return;
    }
    size_t m = n / 2;
    // split, recurse, combine
    // ...
}
```

### Threshold Recommendations

Based on GMP's empirical data across architectures:

| Limb type | GMP typical threshold | Our recommendation |
|-----------|----------------------|-------------------|
| 64-bit limbs | 20–73 limbs | N/A |
| 32-bit limbs (our case, base 10^9) | ~40–80 limbs | **Start at 32, tune empirically** |

**Why 32 as starting point:** Our limbs are 32-bit values in base 10^9. GMP's MIPS32 (also 32-bit limbs) uses threshold 20, but GMP's addition routines are hand-tuned assembly. Our C++ `addAbs`/`subAbs` have higher overhead, pushing the threshold up. Start at 32, measure, adjust.

### Critical Implementation Details

1. **Scratch buffer:** Karatsuba needs temporary storage for intermediate sums. Pre-allocate a scratch buffer of ~4n limbs and pass it through recursion to avoid heap allocations in the hot path.

2. **Asymmetric operands:** When `na` and `nb` differ significantly (e.g., 5 limbs × 50 limbs), split only the longer operand and use schoolbook for the chunks. This is more complex but avoids padding waste. For v4.0, a simpler approach (pad shorter to length of longer) is acceptable.

3. **In-place addition with carry for recombination:** The `z2 * B^(2m) + z1 * B^m + z0` step is just array shifts and additions. Use a helper that adds a limb array at an offset into the result.

4. **Sign of z1 cross-term:** `(x0+x1)*(y0+y1) - z2 - z0` can be negative temporarily during subtraction. Compute `|x0 - x1| * |y0 - y1|` instead and track sign, as GMP recommends — this avoids unsigned underflow.

5. **Squaring optimization:** When `a == b` (same pointer or equal values), the formula simplifies to 3 squares instead of 3 multiplies, and the middle term `(x1-x0)²` is always positive. Detect this case and use the simpler formula.

6. **Do NOT change the `operator*` signature.** Keep it `BigInt operator*(const BigInt& o) const`. The Karatsuba logic is internal.

### Complexity Impact

| Operation | Current | With Karatsuba |
|-----------|---------|----------------|
| n-limb × n-limb multiply | O(n²) | O(n^1.585) |
| Frac multiply (via BigInt) | O(n²) | O(n^1.585) |
| Series multiply (T terms) | O(T² × M(n)) | O(T² × M(n)) where M(n) improves |
| Series inverse | O(T × M(n)) | O(T × M(n)) where M(n) improves |

The T² factor in series multiplication is structural (it's the number of coefficient pairs). Karatsuba only helps the individual BigInt multiplications within each coefficient multiply. This means:
- **Low T (≤50):** No measurable impact (coefficients small)
- **High T (200+):** Significant speedup on heavy computations
- **Very high T (500+):** Major speedup, potentially 2–3x on operations like etaq products

---

## Feature 4: Benchmarking

### Approach: `std::chrono::steady_clock` + DoNotOptimize Barrier

**Confidence: HIGH** — Uses only `<chrono>` (already included) plus a portable compiler barrier.

### Current State

The REPL already times every command (repl.h lines 1280–1291) using `std::chrono::steady_clock`. This is correct — `steady_clock` is monotonic and won't be affected by system clock adjustments, unlike `high_resolution_clock` which may alias to `system_clock` on some platforms (libstdc++).

### Benchmarking Infrastructure Design

Two levels of benchmarking:

**Level 1: REPL Command Timing (already exists)**
The existing `auto t0 = steady_clock::now()` / `auto t1 = steady_clock::now()` around `evalStmt` + `display` is sufficient for user-facing timing. No changes needed.

**Level 2: Internal Micro-Benchmarking (new)**
For development-time performance testing of BigInt operations, series multiplication, etc.

```cpp
namespace bench {

// Compiler barrier: prevent dead-code elimination
template<typename T>
inline void doNotOptimize(T&& val) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : "+r,m"(val) : : "memory");
#else
    // MSVC: volatile read forces materialization
    static volatile auto sink = val;
    (void)sink;
#endif
}

inline void clobberMemory() {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" ::: "memory");
#else
    _ReadWriteBarrier();
#endif
}

struct Result {
    double min_ns;
    double median_ns;
    double mean_ns;
    int iterations;
};

template<typename Func>
Result measure(Func&& fn, int iterations = 100) {
    std::vector<double> times;
    times.reserve(iterations);
    for (int i = 0; i < iterations; ++i) {
        auto t0 = std::chrono::steady_clock::now();
        fn();
        clobberMemory();
        auto t1 = std::chrono::steady_clock::now();
        double ns = std::chrono::duration<double, std::nano>(t1 - t0).count();
        times.push_back(ns);
    }
    std::sort(times.begin(), times.end());
    double sum = 0;
    for (double t : times) sum += t;
    return {
        times[0],
        times[times.size() / 2],
        sum / times.size(),
        iterations
    };
}

} // namespace bench
```

### Benchmark Suite Recommendations

Create a `bench_main.cpp` (separate from `main.cpp`, excluded from production build):

| Benchmark | What it measures | Expected scale |
|-----------|-----------------|----------------|
| `bigint_mul_small` | 3-limb × 3-limb multiply | ~50 ns |
| `bigint_mul_medium` | 20-limb × 20-limb multiply | ~2 μs |
| `bigint_mul_large` | 100-limb × 100-limb multiply | ~50 μs (schoolbook), ~20 μs (Karatsuba) |
| `frac_add` | Frac addition (includes GCD) | ~200 ns |
| `series_mul_T50` | Series multiply at T=50 | ~5 ms |
| `series_mul_T200` | Series multiply at T=200 | ~200 ms |
| `etaq_T100` | etaq(1, 100) construction | ~50 ms |
| `prodmake_T50` | prodmake on Rogers-Ramanujan at T=50 | ~100 ms |

### Build for Benchmarking

```bash
# Production build (unchanged)
g++ -std=c++20 -O2 -static -o qseries main.cpp

# Benchmark build
g++ -std=c++20 -O2 -static -o qbench bench_main.cpp
```

### Key Details

1. **Use `steady_clock`, not `high_resolution_clock`:** On libstdc++, `high_resolution_clock` is `system_clock` (not monotonic). On libc++ and MSVC it's `steady_clock`. Using `steady_clock` directly is portable and correct.

2. **DoNotOptimize barrier:** Essential for micro-benchmarks. Without it, the compiler may eliminate the computation entirely if the result is unused. The `asm volatile("" : "+r,m"(val) : : "memory")` pattern works on GCC and Clang. For MSVC, use `volatile` sink.

3. **Warm-up iterations:** Discard the first few iterations (or rely on median) to avoid cold-cache artifacts.

4. **Report median, not mean:** Outliers from OS scheduling skew the mean. Median is more representative.

5. **Iteration count:** For operations under 1μs, run 10000+ iterations. For operations over 1ms, 10-100 iterations suffice.

6. **Karatsuba threshold tuning:** The benchmark suite is how you find the optimal `KARATSUBA_THRESHOLD`. Run `bigint_mul` at sizes 10, 20, 30, 40, 50, 60, 80, 100 limbs with schoolbook vs. Karatsuba and find the crossover.

---

## Alternatives Considered

| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| Color output | ANSI SGR codes | Windows Console API (SetConsoleTextAttribute) | Legacy API, doesn't support all SGR features, adds code duplication |
| Color library | None (direct codes) | fmt library, rang | Violates zero-dependency constraint |
| Session format | Custom text | JSON, TOML, Protocol Buffers | Require parsing libraries; our data model is simple enough for a custom format |
| Session format | Line-based text | Single-line-per-variable | Series with many coefficients would create absurdly long lines |
| Multiplication | Karatsuba (recursive) | Toom-Cook 3-way, FFT/NTT | Over-engineered for our use case; Karatsuba is the right next step from schoolbook |
| Benchmarking | std::chrono + barriers | Google Benchmark, Catch2 Benchmark | External dependencies; our needs are simple |
| Benchmarking clock | steady_clock | high_resolution_clock | Not portable (may be non-monotonic on libstdc++) |

---

## Installation / Build Changes

```bash
# No changes to build command
g++ -std=c++20 -O2 -static -o qseries main.cpp

# Optional: benchmark binary (development only)
g++ -std=c++20 -O2 -static -o qbench bench_main.cpp
```

No new packages. No dependency manager changes. No new system requirements beyond what's already needed (g++ 13+, C++20 support).

---

## Risk Assessment

| Risk | Severity | Mitigation |
|------|----------|------------|
| ANSI codes don't work on old Windows | Low | `SetConsoleMode()` fails gracefully; colors auto-disable |
| Karatsuba threshold wrong | Low | Benchmark suite finds empirical optimum; defaulting to schoolbook is always correct |
| Session file corruption | Medium | Validate on load; skip malformed lines with warnings |
| Karatsuba bugs (off-by-one in split/combine) | Medium | Fuzz test: verify `karatsuba(a,b) == schoolbook(a,b)` for random inputs |
| `DoNotOptimize` not portable to MSVC | Low | MSVC fallback uses `volatile`; project primarily targets GCC anyway |

---

## Sources

- [Microsoft Learn: Console Virtual Terminal Sequences](https://learn.microsoft.com/en-us/windows/console/console-virtual-terminal-sequences) — ENABLE_VIRTUAL_TERMINAL_PROCESSING docs (HIGH confidence)
- [Microsoft Learn: SetConsoleMode](https://learn.microsoft.com/en-us/windows/console/setconsolemode) — Windows API reference (HIGH confidence)
- [GMP Manual: Karatsuba Multiplication](https://gmplib.org/manual/Karatsuba-Multiplication) — Algorithm and threshold discussion (HIGH confidence)
- [GMP Tuneup Tables](https://gmplib.org/devel/thres/) — Empirical thresholds across architectures (HIGH confidence)
- [Knuth TAOCP Vol 2, Section 4.3.3](https://en.wikipedia.org/wiki/Karatsuba_algorithm) — Original Karatsuba description (HIGH confidence)
- [Stack Overflow: DoNotOptimize patterns](https://stackoverflow.com/questions/40122141/preventing-compiler-optimizations-while-benchmarking) — Compiler barrier techniques (MEDIUM confidence)
- [Sandor Dargo: Clocks in C++](https://www.sandordargo.com/blog/2025/12/10/clocks-part-4-high_resolution_clock) — steady_clock vs high_resolution_clock analysis (MEDIUM confidence)
