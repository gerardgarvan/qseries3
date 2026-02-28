# Architecture: v4.0 Core Improvements Integration

**Project:** qseries REPL v4.0
**Researched:** 2026-02-28
**Confidence:** HIGH (based on direct source analysis of all 11 source files)

## Existing Architecture Snapshot

```
main.cpp  ─────────────────────────────────────────────┐
  │  entry point, --test unit tests, runRepl()         │
  ▼                                                    │
repl.h  (~1300 lines) ────────────────────────────────┐│
  │  Environment, EvalResult variant, dispatchBuiltin  ││
  │  format* functions, display(), runRepl() main loop ││
  │  readLineRaw(), handleTabCompletion(), history     ││
  │  RawModeGuard (raw terminal mode)                  ││
  │  ALL output via direct std::cout                   ││
  ▼                                                    ▼▼
parser.h ──► series.h ──► frac.h ──► bigint.h
               │
               ├── qfuncs.h   (etaq, theta, etaq_cache)
               ├── convert.h  (prodmake, etamake, jacprodmake)
               ├── linalg.h   (kernel, solve)
               └── relations.h (findhom, findnonhom)

main_wasm.cpp  (WASM binding, redirects cout to ostringstream)
```

### Critical Architectural Properties

| Property | Current State |
|----------|--------------|
| Output path | ~40 direct `std::cout <<` calls scattered in `repl.h` (display, dispatchBuiltin, format*) |
| Tab completion | `handleTabCompletion()` — prefix match from `getCompletionCandidates()` (help table + env vars) |
| History | `std::deque<std::string>`, maxHistory=100, in-memory only |
| Terminal mode | `RawModeGuard` RAII, cross-platform (termios/Win32), char-by-char read |
| TTY detection | `stdin_is_tty()` macro, guards interactive features |
| Timing | Basic: `chrono::steady_clock` around eval+display, printed when TTY |
| BigInt multiply | Schoolbook O(n²) in `operator*`, base 10^9, nested loop with carry |
| Series multiply | O(k₁·k₂) nested map iteration, early skip when e1+e2 >= trunc |
| Serialization | None — Environment is ephemeral |

---

## Feature Integration Analysis

### 1. ANSI Color Output

**Scope:** Modify output layer in `repl.h`; add thin `ansi.h` utility header.

**Problem:** Output is scattered across ~40 `std::cout <<` call sites in `repl.h` — the `display()` visitor, `dispatchBuiltin()` built-in handlers (jac2prod, coeffs, series, help, etc.), `runRepl()` (prompt, timing, errors), and the `format*()` functions. No single output bottleneck exists.

**Integration approach:**

```
NEW: src/ansi.h
  - Color constants: RESET, RED, GREEN, CYAN, YELLOW, BOLD, DIM
  - colorize(text, color) → wraps with ANSI codes
  - g_color_enabled global (checked by colorize)
  - Initialized from stdin_is_tty() in runRepl() startup

MODIFY: src/repl.h
  - #include "ansi.h"
  - Prompt "qseries> " → colorize(BOLD+GREEN)
  - Timing line → colorize(DIM)
  - Error messages → colorize(RED)
  - display() visitor: Series → default, product/eta → CYAN, errors → RED
  - format* functions: optional color parameter
  - Banner text → colorize

MODIFY: src/main_wasm.cpp
  - Set g_color_enabled = false before evaluate()
  - OR: strip ANSI from ostringstream output (less clean)
```

**Key decisions:**
- Global `g_color_enabled` boolean, not a config file. Set from `stdin_is_tty()` at startup; overridden by `color on`/`color off` commands.
- Colors go into the output *strings* (not a separate rendering layer). This is the simplest approach for a header-only project and matches how readline-free REPLs work.
- `ansi.h` has zero includes beyond `<string>`. Sits at same level as `bigint.h` (no dependencies).

**WASM impact:** `main_wasm.cpp` sets `g_color_enabled = false`. No ANSI codes leak into web output.

**Estimated touch points:** ~15 call sites in `repl.h` need wrapping, plus `ansi.h` creation (~40 lines).

---

### 2. Smart Tab Completion

**Scope:** Modify `handleTabCompletion()` and `getCompletionCandidates()` in `repl.h`. No new files.

**Current mechanism:**
```cpp
handleTabCompletion(line, pos, env):
  1. Extract prefix (word before cursor)
  2. Match against getCompletionCandidates() = help table keys + env var names
  3. Single match → replace inline; multiple → list below prompt
```

**Smart completion additions:**

| Context | Detection | Completions |
|---------|-----------|-------------|
| After `(` or `,` in function call | Scan backwards from cursor for unmatched `(` and function name | Show argument signature hint from help table |
| Bare identifier | No `(` context, prefix matches | Functions + variables (current behavior) |
| After `:=` | LHS is assignment target | Only functions (not variables) |
| Inside list `[...]` | Unmatched `[` | Only variables (series names) |

**Integration:**
```
MODIFY: src/repl.h
  - handleTabCompletion(): add context analysis before matching
  - NEW helper: analyzeCompletionContext(line, pos) → enum {BARE, FUNC_ARG, ASSIGN_RHS, LIST}
  - For FUNC_ARG context: also return function name + arg position
  - Show inline hint: "etaq(k, T) — eta product" when pressing Tab inside etaq(|)
  - getCompletionCandidates(): add filter parameter for context type
```

**Key decision:** Context analysis is lightweight string scanning (find unmatched parens/brackets), NOT a full parse. Calling the parser on partial input would require error recovery that doesn't exist. The simple scan approach handles 95% of cases.

**No parser.h changes needed.** Context detection is syntactic (paren counting), not semantic.

---

### 3. Session Save/Load

**Scope:** Modify `repl.h` (new builtins + serialization logic). Possibly add string literal support to `parser.h`.

**What must be serialized:**

| Component | Type | Serialization |
|-----------|------|--------------|
| Variables | `map<string, EnvValue>` | Name + type tag + data |
| Series | `map<int,Frac>` + `int trunc` | `S name trunc k1 n1/d1 k2 n2/d2 ...` |
| JacFactor list | `vector<JacFactor>` | `J name a1 b1 e1 a2 b2 e2 ...` |
| Truncation | `int T` | `T value` |
| q variable | Series | Skipped (reconstructed from T) |

**File format:** Plain text, one entity per line. Human-readable and diffable.
```
# qseries session v1
T 50
S myvar 100 0 1/1 1 -1/1 2 -1/1 5 1/1
J myjac 0 5 1 1 5 -1
```

**Integration approach:**

```
MODIFY: src/parser.h
  - Add Token::Kind::STRING for "filename" literals (needed for save/load args)
  - Tokenizer: handle double-quoted strings
  - Parser: string literal → Expr node (new tag Expr::Tag::StrLit)

MODIFY: src/repl.h
  - dispatchBuiltin(): add "save" and "load" handlers
  - NEW: serializeEnvironment(env, ostream) — writes text format
  - NEW: deserializeEnvironment(env, istream) — reads text format, validates
  - save("file") → open file, serialize, close
  - load("file") → open file, deserialize into env, close
  - load merges into current env (doesn't clear first); user can restart to clear
  
  - #include <fstream> (new include in repl.h)
```

**Alternative (no parser change):** Use identifier-based filenames: `save(mysession)` and `load(mysession)`. This appends `.qsession` automatically. Avoids parser string literal work entirely. **Recommended approach** — simpler, and filenames-with-extensions is a secondary concern for a math REPL.

**BigInt serialization:** Use `BigInt::str()` (decimal). Deserialization via `BigInt(string)` constructor. Both already exist.

**WASM impact:** Save/load unavailable in WASM (no filesystem). Gate behind `#ifndef __EMSCRIPTEN__`.

---

### 4. Clear Screen

**Scope:** Modify `repl.h` only. Trivially small.

**Two entry points:**

| Trigger | Handler | Action |
|---------|---------|--------|
| `clear` command | Pre-parse check in `runRepl()` before calling `parse()` | Emit `\033[2J\033[H` |
| Ctrl-L keypress | `readLineRaw()` key handler (c == 12) | Emit escape + redraw prompt |

**Integration:**
```
MODIFY: src/repl.h
  - runRepl(): before parse(trimmed), check if trimmed == "clear" → emit escape, continue
  - readLineRaw(): add case c == 12 (Ctrl-L) → emit \033[2J\033[H, redrawLineRaw()
  - WASM: no-op (clear is meaningless without terminal)
```

**Cross-platform note:** ANSI `\033[2J\033[H` works on Cygwin, Linux, macOS Terminal, Windows Terminal, and modern cmd.exe with VT processing enabled. The existing `redrawLineRaw()` already uses `\033[K` (erase line), so ANSI escapes are established practice in this codebase.

---

### 5. Karatsuba Multiplication

**Scope:** Modify `bigint.h` only. Self-contained algorithm change.

**Current multiply (lines 203-220 of bigint.h):**
```cpp
BigInt operator*(const BigInt& o) const {
    // Schoolbook O(n²): nested loop over digits
    r.d.assign(d.size() + o.d.size(), 0);
    for (size_t i = 0; i < d.size(); ++i)
        for (size_t j = 0; j < o.d.size() || carry; ++j) ...
}
```

**Karatsuba integration:**
```
MODIFY: src/bigint.h
  - NEW: static BigInt karatsubaMultiply(const BigInt& a, const BigInt& b)
    - Base case: if min(a.d.size(), b.d.size()) <= KARATSUBA_THRESHOLD → schoolbook
    - Split each operand at midpoint m = max(a.d.size(), b.d.size()) / 2
    - Compute z0 = low(a) * low(b), z2 = high(a) * high(b)
    - z1 = (low(a)+high(a)) * (low(b)+high(b)) - z0 - z2
    - result = z2 * BASE^(2m) + z1 * BASE^m + z0
  - MODIFY: operator*() → dispatch: if both operands > threshold → karatsubaMultiply, else schoolbook
  - THRESHOLD: ~32 limbs (32 × 9 digits = 288-digit numbers). Below this, schoolbook wins.
```

**Implementation details:**
- "Shift by m" = insert m zero digits at front of `.d` vector (no actual multiplication).
- `addAbs` and `subAbs` already exist as static methods — reuse them for z1 computation.
- Need a helper: `BigInt::slice(start, len)` to extract limb ranges.
- All existing division, GCD, etc. are unaffected — they call `operator*` which now dispatches.

**Cascade impact:**
- Frac arithmetic uses BigInt multiply → automatically faster for large numerators/denominators.
- Series::operator* calls Frac multiply → automatically faster when individual coefficients are large (e.g., high-precision partition numbers).
- `bigGcd` calls `divmod` which calls `operator*` → GCD faster for large BigInts.

**No other files need modification.** The operator interface is unchanged.

---

### 6. Faster Series Operations

**Scope:** Modify `series.h` only. Algorithm-level optimization.

**Current bottleneck — Series::operator* (lines 133-151 of series.h):**
```cpp
for (const auto& [e1, c1] : c)
    for (const auto& [e2, c2] : o.c)
        if (e1 + e2 < t) → accumulate
```

**Optimization opportunities:**

| Optimization | Approach | Expected speedup | Complexity |
|--------------|----------|-------------------|------------|
| Inner loop early break | `std::map` is sorted; when `e1 + e2 >= t`, break inner loop (all subsequent e2 values are larger) | 2-4× for high-truncation series | Low — 3-line change |
| Dense multiplication path | Convert sparse map → dense `vector<Frac>` when density > 50%, multiply as array convolution, convert back | 2-5× for dense series (most q-series) | Medium — ~50 lines |
| Accumulation optimization | Accumulate into `vector<Frac>` (contiguous) instead of `map` lookups, build map at end | 1.5-2× from cache locality | Low-medium |

**Recommended phased approach:**

```
MODIFY: src/series.h

Phase A: Inner loop early break (trivial)
  - In operator*, after `if (exp >= t) continue;` change to `if (exp >= t) break;`
  - Works because map iterates in ascending key order

Phase B: Dense fast path
  - NEW: denseMul(vector<Frac>&, vector<Frac>&, int trunc) → vector<Frac>
  - operator*(): if both operands have minExp >= 0 and density > 50%:
    1. Convert maps → dense vectors (O(n))
    2. Call denseMul (O(n²) but cache-friendly, no map overhead)
    3. Convert result → map, strip zeros (O(n))
  - Density = c.size() / (maxExp - minExp + 1)

Phase C: inverse() optimization
  - Current inverse() calls coeff(j) and coeff(n-j) in inner loop — O(log n) map lookups each
  - Pre-extract coefficients into dense vector for O(1) access
```

**No interface changes.** `operator*`, `inverse()` signatures are unchanged. All callers (qfuncs, convert, relations) benefit automatically.

---

### 7. Benchmarking

**Scope:** Modify `repl.h` only. Extend existing timing infrastructure.

**Current timing (lines 1280-1291 of repl.h):**
```cpp
auto t0 = chrono::steady_clock::now();
StmtPtr stmt = parse(trimmed);
EvalResult res = evalStmt(stmt.get(), env);
// ... display ...
auto t1 = chrono::steady_clock::now();
if (stdin_is_tty()) cout << secs << "s" << endl;
```

**New capabilities:**

| Feature | Syntax | Implementation |
|---------|--------|----------------|
| Timing toggle | `timing on` / `timing off` | Add `bool showTiming` to Environment, check in runRepl display section |
| Benchmark | `benchmark(expr, N)` | New builtin: parse expr, run N times, report min/avg/max/total |
| Benchmark command | `benchmark N { expr }` | Alternative syntax: cleaner but needs parser support |

**Recommended approach (no parser changes):**

```
MODIFY: src/repl.h
  - Environment: add `bool showTiming = true;`
  - runRepl(): check env.showTiming before printing time
  - runRepl(): handle "timing on" / "timing off" as special commands (like "clear")
  - dispatchBuiltin(): add "benchmark" handler:
    - benchmark(expr, N): re-evaluate the expression N times
    - Capture min/avg/max times
    - Return DisplayOnly (output printed directly)
  - Help table: add benchmark entry
```

**Benchmark implementation detail:** The `benchmark` builtin receives `args[0]` as an AST node. It can call `eval(args[0], env, sumIndices)` repeatedly in a loop without re-parsing, measuring each iteration. This is possible because `eval` takes an `Expr*`, not a string.

---

## Component Dependency Map

```
                     ansi.h (NEW)
                       │
                       ▼
bigint.h ──► frac.h ──► series.h ──► qfuncs.h ──► convert.h
  │(Karatsuba)           │(dense mul)               │
  │                      │                           ├──► linalg.h
  │                      │                           │
  │                      │                    relations.h
  │                      │                           │
  │                      ▼                           ▼
  │                 parser.h  ◄───────────────── repl.h
  │                  │(optional: string lits)     │(ANSI color)
  │                  │                            │(smart completion)
  │                  │                            │(session save/load)
  │                  │                            │(clear screen)
  │                  │                            │(benchmark)
  │                  │                            │(timing toggle)
  ▼                  ▼                            ▼
              main.cpp                    main_wasm.cpp
```

## New vs Modified Files

| File | Status | Changes |
|------|--------|---------|
| `src/ansi.h` | **NEW** | ANSI color constants, colorize(), g_color_enabled (~40 lines) |
| `src/bigint.h` | **MODIFY** | Add Karatsuba multiply, threshold dispatch, slice helper |
| `src/series.h` | **MODIFY** | Inner-loop early break, optional dense multiply path |
| `src/repl.h` | **MODIFY** | Color wrapping, smart completion, session save/load, clear, benchmark, timing toggle |
| `src/parser.h` | **MODIFY** (optional) | String literal support if save/load uses quoted filenames |
| `src/main_wasm.cpp` | **MODIFY** | Set g_color_enabled = false |
| `src/main.cpp` | **MODIFY** | Unit tests for Karatsuba, series perf |

**Files NOT modified:** `frac.h`, `qfuncs.h`, `convert.h`, `linalg.h`, `relations.h`.

---

## Recommended Build Order

The order is driven by two principles: (1) lower layers before upper layers, (2) independent features can be parallelized.

```
Phase 1: Foundation (no dependencies between these)
  ├── 1a. ansi.h + ANSI color in repl.h     [independent]
  ├── 1b. Karatsuba in bigint.h              [independent]
  └── 1c. Clear screen in repl.h             [independent, trivial]

Phase 2: Series optimization (depends on 1b for full benefit)
  └── 2a. Series fast-path in series.h

Phase 3: REPL enhancements (depends on 1a for colored output)
  ├── 3a. Smart tab completion in repl.h     [independent]
  ├── 3b. Benchmarking/timing in repl.h      [independent]
  └── 3c. Session save/load in repl.h        [independent]
```

**Rationale:**
- **ansi.h first** because it's the thinnest layer and subsequent features (benchmarking, smart completion) benefit from colored output.
- **Karatsuba first** because it's self-contained in bigint.h and the series optimization in Phase 2 gets the combined benefit of both Karatsuba + dense multiply.
- **Clear screen** is trivial (5 lines), do it early to build momentum.
- **Series optimization** in Phase 2 because it benefits from Karatsuba already being in place — benchmarking the improvement requires both layers.
- **Phase 3 features** are all repl.h modifications but touch different parts of the file (completion = readLineRaw area, benchmarking = eval/display area, session = new serialization section). They can be implemented in any order.

---

## Data Flow Changes

### Current Output Flow
```
eval() → EvalResult → display() → std::cout
                                    ↑ direct, unformatted
dispatchBuiltin() → std::cout (for DisplayOnly results)
runRepl() → std::cout (prompt, timing, errors)
```

### Post-v4.0 Output Flow
```
eval() → EvalResult → display() → colorize() → std::cout
                                    ↑ color-wrapped
dispatchBuiltin() → colorize() → std::cout
runRepl() → colorize() → std::cout
             ↑ prompt=GREEN, timing=DIM, error=RED
```

### Current Multiply Flow
```
Series::operator* → Frac::operator* → BigInt::operator* (schoolbook)
```

### Post-v4.0 Multiply Flow
```
Series::operator* ─┬─ sparse path (map iteration, sorted early-break)
                    └─ dense path (vector convolution when density > 50%)
                         ↓
                    Frac::operator* → BigInt::operator* ─┬─ schoolbook (< 32 limbs)
                                                          └─ Karatsuba (≥ 32 limbs)
```

### New Session Data Flow
```
save: Environment.env → serializeEnvironment() → ofstream → .qsession file
load: .qsession file → ifstream → deserializeEnvironment() → Environment.env
```

---

## Architectural Invariants Preserved

| Constraint | How v4.0 Respects It |
|------------|---------------------|
| Zero external dependencies | All new code uses only C++ standard library |
| Header-only | ansi.h is header-only with `inline` functions |
| Single binary | No new compilation units (ansi.h included via repl.h) |
| Exact arithmetic | Karatsuba produces identical results to schoolbook |
| WASM compatibility | Color disabled, save/load gated with `#ifndef __EMSCRIPTEN__` |
| Operator interface | Series `+ - * /` signatures unchanged |
| Auto-reduce | Frac::reduce() call pattern unchanged |

---

## Risk Areas

| Risk | Severity | Mitigation |
|------|----------|------------|
| repl.h grows beyond maintainability (~1300 → ~1600 lines) | Medium | Extract serialization into `session.h` if it exceeds 100 lines |
| Karatsuba correctness for base 10^9 | Medium | Extensive unit tests: schoolbook vs Karatsuba on random inputs |
| Dense series path changing numerical results | Low | Dense and sparse paths must produce bit-identical results; test with Rogers-Ramanujan |
| ANSI codes leaking to non-TTY output | Medium | Guard every colorize() call with g_color_enabled; add acceptance test piping through `cat` |
| Session file format versioning | Low | Include version header `# qseries session v1`; reject unknown versions |
