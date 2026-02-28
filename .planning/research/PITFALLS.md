# Domain Pitfalls — v4.0 Core Improvements

**Domain:** Adding ANSI color output, smart tab completion, session save/load, clear screen, Karatsuba multiplication, faster series multiplication, and benchmarking to an existing zero-dependency C++20 q-series REPL.
**Researched:** 2026-02-28
**Overall confidence:** HIGH (based on codebase inspection + verified community experience)

---

## Critical Pitfalls

Mistakes that cause regressions, silent data corruption, or cross-platform breakage.

---

### Pitfall 1: ANSI Escape Codes Contaminating Script Mode / Piped Output

**What goes wrong:** ANSI color codes (`\033[31m`, `\033[0m`, etc.) are emitted unconditionally via `std::cout`. When output is piped (`qseries < script.qs > out.txt`) or consumed by another program, the escape sequences appear as raw bytes in the output, breaking downstream parsers, diffs, and regression tests.

**Why it happens:** The codebase currently has ~51 `std::cout` call sites in `repl.h` that produce mathematical output (series, products, relations, errors). If color codes are added inline at these sites without gating on TTY, every output path leaks ANSI.

**Consequences:**
- Script mode (`stdin_is_tty() == false`) produces garbled output files
- Test harnesses comparing expected vs actual output fail on invisible characters
- WASM build (`__EMSCRIPTEN__`) receives meaningless escape bytes
- Piping `qseries < demo.qs | grep "theta"` matches ANSI fragments instead of math

**Prevention:**
1. Gate ALL color output on `stdin_is_tty()` (already exists in codebase, line 22-29 of `repl.h`)
2. Centralize color in a single output abstraction — never embed `\033[` literals at call sites
3. Add `--color=never` / `--color=always` CLI flags for override
4. Regression test: run acceptance scripts with `stdout` redirected to file, verify zero ANSI bytes (`grep -P '\x1b\[' out.txt` must be empty)

**Detection:** Run `qseries < demo.qs | xxd | grep '1b 5b'` — any match means ANSI leaked.

**Phase assignment:** ANSI color phase — must be the gating concern for the entire color implementation.

---

### Pitfall 2: ANSI Codes Breaking Line Editor Width Calculation

**What goes wrong:** The existing `redrawLineRaw()` (repl.h line 1028-1034) computes cursor position based on `line.size()`. If the prompt `"qseries> "` is colorized (e.g., green prompt), the ANSI escape bytes add to the byte count but not the visible width. Cursor positioning via `\033[<count>D` moves wrong number of columns. Left/right arrow, backspace, and tab completion all misalign.

**Why it happens:** `redrawLineRaw` uses `line.size()` for cursor math and `\033[K` for clear-to-end. These work because the current prompt is plain ASCII. Adding color to the prompt means the `\r` + prompt length calculation must exclude non-printing bytes.

**Consequences:**
- Cursor appears at wrong position after typing
- Backspace deletes wrong character visually (correct in buffer, wrong on screen)
- Tab completion redraws incorrectly — partial completions appear offset
- History navigation (up/down arrows) shows garbled lines

**Prevention:**
1. Keep the prompt string in the `redrawLineRaw` function as a constant and track its **display width** separately from its **byte length**
2. If prompt is colorized, wrap non-printing sequences in a helper that returns both raw bytes and display width
3. Never colorize user input text in the line editor — only the prompt and output
4. Test: type 20 characters, press Home, then type — cursor must land at position 0

**Detection:** Type a long line, press left arrow 5 times, type a character — if it appears in wrong position, width calculation is broken.

**Phase assignment:** ANSI color phase — must be addressed before colorizing the prompt.

---

### Pitfall 3: Karatsuba Intermediate Overflow at Base 10^9

**What goes wrong:** The existing `BigInt::operator*` (bigint.h line 203-220) uses `uint64_t` for `carry` and the accumulator `cur`. With base 10^9:
- Single digit product: up to `(10^9-1)^2 ≈ 10^18` — fits in `uint64_t` (max ~1.8×10^19)
- But the accumulator line `cur += d[i] * 1ULL * o.d[j]` also adds `r.d[i+j]` (up to 10^9-1) and `carry` (up to 10^9-1), totaling worst case ~10^18 + 2×10^9 — still fits

Karatsuba introduces intermediate **additions and subtractions of partial products** before carries are resolved. The recombination step computes `z1 = z_mid - z_high - z_low` where each term can be a full-width number. If partial products are stored in the same base-10^9 vector, digits can temporarily exceed 10^9 before carry propagation — and they can also go **negative** (the subtraction `z_mid - z_high - z_low` can produce negative limbs).

**Consequences:**
- Silent truncation of intermediate values → wrong multiplication results
- Negative limbs stored in `uint32_t` wrap to large positive values → catastrophically wrong
- The bug manifests only for specific operand sizes near the recursion boundary
- Frac reduction depends on correct BigInt multiplication; wrong GCD → unreduced fractions → exponential coefficient growth → OOM or wrong series

**Prevention:**
1. Use `int64_t` (signed) for intermediate Karatsuba limbs, not `uint32_t`
2. Defer carry propagation to a finalize step that runs after all additions/subtractions
3. Alternatively: implement Karatsuba on top of the existing `operator*` by splitting at the **vector level** (half the limbs), computing three sub-products with the existing multiply, and recombining — this avoids touching the carry logic entirely
4. **Exhaustive edge-case tests**: multiply numbers that are exactly 1, 2, 3, and 4 limbs long (each limb boundary). Test `(BASE-1) * (BASE-1)`, `BASE * BASE`, and `(BASE^k - 1) * (BASE^k - 1)` for k=1..4

**Detection:** Compare Karatsuba result against schoolbook result for random operands of 1–20 limbs. Any mismatch is a correctness bug.

**Phase assignment:** Karatsuba multiplication phase — blocking correctness concern.

---

### Pitfall 4: Wrong Karatsuba Recursion Threshold Causing Performance Regression

**What goes wrong:** Karatsuba's O(n^1.585) advantage only kicks in above a threshold number of limbs. Below it, the three recursive calls + extra additions are slower than the tight O(n²) loop. With base 10^9, each limb holds 9 decimal digits, so a 100-digit number is only ~12 limbs. Most BigInt values in this REPL are small (partition numbers, series coefficients) — often 1–4 limbs.

**Why it happens:** Developers set threshold too low (e.g., 2 limbs) or don't set one at all, making Karatsuba recurse all the way down to single digits.

**Consequences:**
- Series multiplication becomes **slower** than before because every Frac multiply pays Karatsuba overhead for tiny numbers
- `etaq(1, 500)` or Rogers-Ramanujan at T=200 regresses noticeably
- Benchmark shows Karatsuba "working" on large synthetic inputs but the REPL gets slower for real workloads

**Prevention:**
1. Start with threshold around 32 limbs (~288 decimal digits) and tune empirically
2. Run the existing acceptance tests with timing **before and after** — reject if any test regresses
3. Profile real workloads: `etaq(1,500)`, `prodmake(rr, 200)`, `findhom([theta3,theta4,...], 2, 0)` — these are the actual hot paths, not synthetic million-digit multiplies
4. Keep the schoolbook `operator*` intact; Karatsuba should be called from a dispatch that checks `d.size()` threshold

**Detection:** Time `etaq(1,200)` before and after. If it's slower, threshold is wrong.

**Phase assignment:** Karatsuba multiplication phase — performance validation step.

---

### Pitfall 5: Session Save/Load Format Not Versioned → Silent Data Loss on Format Change

**What goes wrong:** Session files are written in version 1 format. A future change adds a field (e.g., new variable types, function definitions). Old binary loads new file and silently ignores or misparses the new fields, or new binary loads old file and crashes on missing fields.

**Why it happens:** No version header in the session file format. The format is ad-hoc (e.g., plain text key=value or JSON-like) without a schema version.

**Consequences:**
- Users lose saved sessions after upgrading
- Corrupted state loaded silently — wrong truncation, wrong variable values
- `EnvValue` is `std::variant<Series, std::vector<JacFactor>>` — if a third type is added later, old files can't represent it

**Prevention:**
1. First line of session file: `# qseries-session v1` — always check on load
2. Use human-readable text format (not binary) — easier to debug, diff, version-control
3. Define explicit serialization for each `EnvValue` variant with a type tag
4. On version mismatch: warn user and skip (don't crash, don't silently load)
5. Test: save session in v1, modify format to v2, verify v2 binary loads v1 file gracefully

**Detection:** Save a session, hex-edit the version number, load it — should produce a clear error message.

**Phase assignment:** Session save/load phase — must be the first thing defined before any serialization code.

---

## Moderate Pitfalls

Mistakes that cause user confusion, subtle bugs, or maintenance burden.

---

### Pitfall 6: Session Serialization of Arbitrary-Precision Rationals

**What goes wrong:** `Series` stores `std::map<int, Frac>` where each `Frac` contains two `BigInt`s. Serializing these requires exact round-trip fidelity. Using `Frac::str()` (which outputs `"3/2"` or `"7"`) and parsing back works, but edge cases lurk:
- Negative exponents in the map (e.g., after `inverse()` of `q*(1-q)`)
- Very large coefficients (100+ digit numerators from relation finding)
- The truncation value must be preserved exactly

**Why it happens:** Developers serialize using `Series::str()` which is a display format with `O(q^N)` suffix and limited term count (`maxTerms=30`). This loses coefficients beyond term 30 and conflates display with data.

**Consequences:**
- Reloaded series has fewer terms than original
- Truncation information lost → subsequent arithmetic produces wrong results
- Negative-exponent terms (from Laurent series) may be omitted or misparsed

**Prevention:**
1. Serialize as structured data, not display strings: `{trunc: 50, coeffs: {0: "1", 1: "-1", 2: "-1", ...}}`
2. Emit ALL coefficients, not just first 30
3. Include explicit truncation value
4. Round-trip test: serialize → deserialize → compare every coefficient

**Detection:** Save a series with 100 terms, reload, check `coeff(99)` matches.

**Phase assignment:** Session save/load phase.

---

### Pitfall 7: Jacobi Product Variables Not Serializable

**What goes wrong:** `EnvValue = std::variant<Series, std::vector<JacFactor>>`. If session save only handles `Series`, users who ran `jp := jacprodmake(rr, 40)` lose `jp` on reload. The `JacFactor` type (a tuple of `int, int, Frac`) needs its own serialization path.

**Why it happens:** `JacFactor` is a less common type, easy to forget. The `jac2prod()` display function exists but produces display-only output like `1/(q,q^5)(q^4,q^5)`, not a parseable format.

**Consequences:**
- Jacobi product variables silently dropped on save
- User re-runs expensive `jacprodmake` computations after reload

**Prevention:**
1. Enumerate all variants in `EnvValue` and require serialization for each
2. Serialize `JacFactor` as structured `{a: 0, b: 5, exp: 1}` triples
3. Compile-time check: if `EnvValue` gains a new alternative, the save function must handle it (use `std::visit` exhaustively)

**Detection:** Save session with a Jacobi product variable, reload, access it — must not throw "undefined variable."

**Phase assignment:** Session save/load phase.

---

### Pitfall 8: Windows ANSI Requires Explicit Console Mode Activation

**What goes wrong:** On Windows, ANSI escape codes are NOT processed by default. The codebase already uses `SetConsoleMode` for input (repl.h line 82-94, `RawModeGuard`), but that's for `STD_INPUT_HANDLE`. ANSI output requires `ENABLE_VIRTUAL_TERMINAL_PROCESSING` on `STD_OUTPUT_HANDLE`, which is a separate call.

**Why it happens:** Unix developers assume ANSI works everywhere. Windows 10+ supports it, but only after explicit opt-in via `SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)`.

**Consequences:**
- On Windows, users see raw `←[31m` text instead of colored output
- Older Windows 10 builds (pre-19042) may fail the `SetConsoleMode` call entirely
- The existing `RawModeGuard` destructor restores input mode; a separate guard is needed for output mode

**Prevention:**
1. On startup (before any output), call `SetConsoleMode` on `STD_OUTPUT_HANDLE` with `ENABLE_VIRTUAL_TERMINAL_PROCESSING`
2. Check return value — if it fails, fall back to no-color mode
3. Create a separate `AnsiOutputGuard` or integrate into existing startup
4. Test on Windows Terminal, PowerShell, cmd.exe, and Cygwin terminal (which uses its own ANSI handling)

**Detection:** Run on Windows cmd.exe without the `SetConsoleMode` call — ANSI codes will appear as literal text.

**Phase assignment:** ANSI color phase — Windows platform concern.

---

### Pitfall 9: Clear Screen Inconsistency Across Platforms

**What goes wrong:** `\033[2J\033[H` clears the visible screen on Unix terminals and Windows Terminal (with VT processing enabled). But behavior differs:
- `\033[2J` clears screen but may not reset cursor to top on all terminals
- `\033[3J` also clears scrollback buffer — may surprise users who want to scroll up
- On Cygwin, terminal behavior depends on the underlying terminal emulator (mintty vs Windows console)

**Why it happens:** "Clear screen" has at least 3 different user expectations: clear visible area, clear scrollback, or reset terminal state.

**Consequences:**
- Users on some terminals see blank screen but cursor at bottom
- WASM build (`__EMSCRIPTEN__`) has no terminal to clear — command is meaningless
- Script mode: clear screen in a script does nothing useful and may insert ANSI into piped output

**Prevention:**
1. Use `\033[2J\033[H` (clear + cursor home) as the standard sequence
2. Gate on `stdin_is_tty()` — in script mode, `clear` is a no-op
3. Gate on `__EMSCRIPTEN__` — in WASM, `clear` is a no-op (or sends a message to the web frontend)
4. Document what `clear` does: "clears the visible terminal area"

**Detection:** Run `clear` in script mode — should produce no output.

**Phase assignment:** Clear screen phase.

---

### Pitfall 10: Benchmark Noise Hiding Real Performance Changes

**What goes wrong:** Benchmarks show 15% variance between runs due to OS scheduling, CPU frequency scaling, background processes, and cache effects. A real 5% improvement or regression is invisible within the noise.

**Why it happens:** Using a single `steady_clock::now()` before/after measurement (as the REPL already does at line 1280-1290) is fine for user-visible timing, but insufficient for benchmarking algorithm changes.

**Consequences:**
- Karatsuba appears faster/slower depending on system load at test time
- Performance regressions ship because benchmark was run once and looked okay
- Series multiplication optimization appears to help but actually doesn't (or vice versa)

**Prevention:**
1. Run each benchmark N times (N ≥ 5, ideally 11+) and report median, not mean
2. Use warm-up iterations (discard first 1-2 runs) to fill caches
3. Disable CPU frequency scaling during benchmarks (if possible), or accept and document variance
4. Compare before/after using statistical significance (e.g., are the medians separable given the variance?)
5. Benchmark specific hot paths: `etaq(1, 500)`, `prodmake(rr, 100)`, `findhom(L, 3, 0)` — these are the user-visible operations
6. Use `std::chrono::steady_clock`, not `high_resolution_clock` (which may be an alias for `system_clock` on some platforms)

**Detection:** Run the same benchmark 10 times — if min and max differ by >20%, the measurement methodology is too noisy.

**Phase assignment:** Benchmarking phase.

---

### Pitfall 11: Faster Series Multiplication Breaking Truncation Invariant

**What goes wrong:** The current `Series::operator*` (series.h line 133-151) maintains the critical invariant: `result.trunc = min(a.trunc, b.trunc)` and skips terms where `e1 + e2 >= trunc`. A "faster" implementation (e.g., converting to dense vector, using FFT-style polynomial multiply, or batching coefficient additions) might:
- Forget to enforce the truncation cutoff
- Use a dense array indexed 0..N but forget that series can have negative exponents (Laurent series from `inverse()`)
- Accumulate into Frac without calling `reduce()`, causing BigInt growth

**Why it happens:** Optimization focuses on speed and changes data structures, breaking invariants that were implicit in the original `std::map` iteration.

**Consequences:**
- Series with extra terms beyond truncation → wrong results in subsequent operations
- Dense-array approach crashes on negative exponents
- Unreduced Frac intermediates → exponential memory growth → OOM on moderate computations

**Prevention:**
1. Keep the `std::map<int, Frac>` representation — it handles sparse series and negative exponents naturally. Optimize within this representation first (e.g., skip zero coefficients, batch additions)
2. If converting to dense, handle the `minExp()` offset explicitly and enforce `trunc` cutoff
3. Run the full acceptance test suite after any series multiplication change — Rogers-Ramanujan is the ultimate correctness test
4. Profile first: the bottleneck may be in Frac arithmetic (BigInt multiply/GCD), not in Series iteration

**Detection:** Compute `etaq(1, 100)` and compare every coefficient against known Euler pentagonal values.

**Phase assignment:** Faster series multiplication phase.

---

### Pitfall 12: Tab Completion Breaking with New Completable Names

**What goes wrong:** The existing `getCompletionCandidates` (repl.h line 1036-1043) returns help table keys + environment variable names. Adding new built-in commands (like `save`, `load`, `clear`, `benchmark`) requires adding them to the help table AND ensuring the tab completion picks them up. If they're implemented as special-cased strings in the REPL loop (not in the help table), tab completion won't find them.

**Why it happens:** New commands are added in the REPL dispatch (`runRepl`) as string comparisons, bypassing the `getHelpTable()` registry.

**Consequences:**
- User types `sa<TAB>` and nothing happens — `save` isn't in the candidate set
- Inconsistency: some commands are completable, others aren't
- Help system and completion diverge

**Prevention:**
1. Add ALL new commands to `getHelpTable()` — it's the single source of truth
2. Tab completion already uses `getHelpTable()` — no extra work needed IF commands are registered there
3. Test: for every new command, verify `<prefix><TAB>` completes it

**Detection:** Type first 2 letters of every new command + TAB — each must complete or show candidates.

**Phase assignment:** Every phase that adds a new command.

---

## Minor Pitfalls

Mistakes that cause minor UX issues or technical debt.

---

### Pitfall 13: Color Scheme Illegible on Light/Dark Terminal Backgrounds

**What goes wrong:** Choosing specific ANSI colors (e.g., dark blue for errors) that are invisible on dark terminal backgrounds, or bright yellow that's invisible on light backgrounds.

**Prevention:**
1. Use bold/bright variants for important text (errors, warnings)
2. Stick to colors visible on both light and dark: red for errors, cyan/green for prompts, default for output
3. Provide `--color=never` for users with problematic terminals
4. Test on at least: dark terminal, light terminal, Windows Terminal, Cygwin mintty

**Phase assignment:** ANSI color phase.

---

### Pitfall 14: Session File Grows Without Bound

**What goes wrong:** If session auto-saves on every command (or save captures full history + all variables), the session file grows large. A series at T=500 has 500 coefficients, each a rational number. Twenty such variables = 10,000 Frac values = potentially megabytes of text.

**Prevention:**
1. Session save is explicit (user types `save("file")`), not automatic
2. Warn if session file exceeds a threshold (e.g., 1 MB)
3. Consider saving only variable names and the commands that created them (replay approach) — more compact but requires re-execution on load

**Phase assignment:** Session save/load phase.

---

### Pitfall 15: Benchmark Output Interfering with Normal REPL Output

**What goes wrong:** Benchmark results printed to `stdout` intermix with series output, making both hard to read. Or benchmark mode changes global state (truncation, variables) that persists after benchmarking.

**Prevention:**
1. Benchmark output goes to `stderr` or is clearly delimited
2. Benchmark runs in an isolated `Environment` copy — no side effects on user state
3. Format: `benchmark: etaq(1,200) — median 0.342s (5 runs, min 0.331s, max 0.358s)`

**Phase assignment:** Benchmarking phase.

---

### Pitfall 16: Karatsuba Not Tested Against Existing BigInt Test Suite

**What goes wrong:** Karatsuba is tested with new unit tests but the existing BigInt test suite in `main.cpp` (lines 25-99) still uses the old multiply path (because test operands are small). The Karatsuba code path is never exercised by existing acceptance tests.

**Prevention:**
1. Add large-operand multiplication tests that exceed the Karatsuba threshold
2. Run a randomized cross-check: for 1000 random (a, b) pairs of varying sizes, verify `karatsuba(a, b) == schoolbook(a, b)`
3. Ensure the existing `--test` flag exercises Karatsuba for at least some cases

**Detection:** Code coverage — if Karatsuba function has 0% coverage after `--test`, it's not being tested.

**Phase assignment:** Karatsuba multiplication phase.

---

### Pitfall 17: ANSI Codes in Error Messages Breaking Error Line Detection in Script Mode

**What goes wrong:** Currently errors go to `std::cerr` (repl.h line 1293-1296) with format `"error: line N: message"`. If error messages are colorized (red), the script-mode line number detection `"error: line "` won't match because ANSI codes are interspersed.

**Prevention:**
1. Gate error colorization on `stdin_is_tty()` (same as all other ANSI gating)
2. Never embed ANSI in the error message string itself — color the output at the print site
3. Regression test: parse error output in script mode to verify `"error: line N:"` format is intact

**Phase assignment:** ANSI color phase.

---

### Pitfall 18: Unicode Superscript Interaction with ANSI Color Reset

**What goes wrong:** The codebase uses UTF-8 Unicode superscripts (`⁰¹²³⁴⁵⁶⁷⁸⁹`) and subscripts (`₁₂₃₄₅`) for mathematical display (series.h `expToUnicode`, repl.h line 959). If an ANSI color reset (`\033[0m`) is placed mid-expression, it could split a multi-byte UTF-8 sequence, producing garbled output.

**Prevention:**
1. Apply color at the expression level (before the whole expression, reset after), not character-by-character
2. Never insert ANSI sequences between bytes of a multi-byte UTF-8 character
3. Test with series containing exponents 10+ (multi-character superscripts like `¹⁰`)

**Phase assignment:** ANSI color phase.

---

## Phase-Specific Warnings Summary

| Phase Topic | Likely Pitfall | Severity | Mitigation |
|---|---|---|---|
| ANSI color output | Codes leak into script mode / piped output | **CRITICAL** | Gate all color on `stdin_is_tty()`; add `--color` flag |
| ANSI color output | Line editor cursor miscalculation | **CRITICAL** | Track display width separately from byte length |
| ANSI color output | Windows needs explicit VT processing | **MODERATE** | `SetConsoleMode` on `STD_OUTPUT_HANDLE` |
| ANSI color output | Colors illegible on some backgrounds | MINOR | Use bold/bright; test light+dark |
| ANSI color output | Unicode superscript split by color reset | MINOR | Color at expression level, not char level |
| ANSI color output | Error format broken by color in script mode | MINOR | Gate error color on TTY check |
| Karatsuba multiplication | Intermediate overflow / negative limbs | **CRITICAL** | Use signed int64 intermediates; implement at vector-split level |
| Karatsuba multiplication | Wrong threshold → performance regression | **CRITICAL** | Benchmark real workloads before/after; threshold ~32 limbs |
| Karatsuba multiplication | Not tested by existing test suite | MINOR | Add cross-check tests + large-operand tests |
| Session save/load | No format versioning → future breakage | **CRITICAL** | Version header on line 1; graceful degradation on mismatch |
| Session save/load | Series serialized as display string | **MODERATE** | Use structured format with ALL coefficients |
| Session save/load | Jacobi product variables dropped | **MODERATE** | Exhaustive `std::visit` serialization |
| Session save/load | File grows without bound | MINOR | Explicit save only; warn on large files |
| Clear screen | Inconsistent across platforms | **MODERATE** | `\033[2J\033[H`; no-op in script/WASM mode |
| Faster series multiplication | Truncation invariant broken | **CRITICAL** | Keep `std::map`; full acceptance test after change |
| Benchmarking | Noise hides real changes | **MODERATE** | Multiple runs, report median, warm-up iterations |
| Benchmarking | Output interferes with REPL | MINOR | Isolated environment; clear output format |
| Tab completion (new commands) | New commands not registered | **MODERATE** | Add to `getHelpTable()` — single source of truth |

## Sources

- Codebase inspection: `src/bigint.h`, `src/series.h`, `src/repl.h`, `src/main.cpp` (direct reading)
- ANSI + piped output: unix.stackexchange.com/questions/10823 (HIGH confidence — canonical Unix behavior)
- Windows ANSI: learn.microsoft.com/en-us/windows/console/setconsolemode (HIGH — Microsoft official docs)
- ANSI width bugs: github.com/willmcgugan/rich/issues/1670, github.com/block/goose/issues/7463 (MEDIUM — real-world confirmed)
- Karatsuba overflow: stackoverflow.com/questions/47873434, stackoverflow.com/questions/31338166 (HIGH — multiple independent reports)
- Karatsuba threshold: gmplib.org/manual/Karatsuba-Multiplication, swtch.com/math/big/calibrate.html (HIGH — GMP is authoritative)
- Benchmark clocks: sandordargo.com/blog/2025/12/10/clocks-part-4-high_resolution_clock (MEDIUM — recent analysis, matches cppreference)
- Session versioning: studyplan.dev/pro-cpp/binary-serialization/q/versioning-serialized-game-save-files (MEDIUM)
