# Phase 43: Wasm Compile - Research

**Researched:** 2026-02-27
**Domain:** Emscripten / WebAssembly / C++ to Wasm compilation
**Confidence:** HIGH

## Summary

Compiling this C++20 codebase to WebAssembly via Emscripten is straightforward because the project has zero external dependencies and performs pure computation (no filesystem, no networking, no threads). The only porting friction comes from terminal I/O code (termios, SetConsoleMode, isatty) in `repl.h`, which must be guarded with `#ifndef __EMSCRIPTEN__` or bypassed entirely since the Wasm API receives input as strings rather than reading from stdin.

The `evaluate(expr) → string` API pattern is a solved problem with Embind. The implementation creates a persistent `Environment` object (stateful sessions), parses/evaluates the input using the existing `evalStmt` pipeline, captures `std::cout` output via `std::streambuf` redirection to `std::ostringstream`, and returns the captured string. Errors are caught and returned as `"error: ..."` prefixed strings instead of crashing. The existing codebase is ~4,000 lines producing a 643KB native binary; Wasm output should comfortably fit under the 1.5MB compressed target.

Despite the `.cursorrules` specifying C++20, the codebase only uses C++17 features (structured bindings, `if constexpr`, `std::variant`, `std::optional`). Emscripten has excellent C++17 support, so compilation compatibility is high confidence.

**Primary recommendation:** Create a separate `src/main_wasm.cpp` entry point that includes `repl.h`, instantiates a global `Environment`, and exposes `evaluate()` and `get_banner()` via Embind. Guard all terminal I/O code in `repl.h` with `#ifndef __EMSCRIPTEN__`. Add a `wasm` Makefile target using `em++ -Oz -lembind`.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **Return format:** Plain string — same text the CLI would print to stdout
- **Truncation default:** T=50 for Wasm session (safe for browser, user can change via set_trunc)
- **Error format:** Prefixed string — errors start with "error:" (e.g., "error: division by zero")
- **Crash recovery:** Terminate — Web Worker dies, session terminates
- **Wasm traps:** Add guards — check for potential traps before they happen (extra validation in C++)
- **REPL reuse:** Full REPL — redirect stdin/stdout to Wasm-compatible I/O (reuse as much of repl.h as possible)
- **Display format:** Exact same output as CLI
- **Banner:** Show "qseries v2.0" banner on first load
- **Testing:** Both Node.js test AND test HTML page for verification

### Claude's Discretion
- Whether evaluate() maintains stateful sessions (variables persisting between calls)
- Single vs multi-statement input handling
- Timeout implementation (if any)
- Entry point approach (separate file vs #ifdef)
- Build system integration (Makefile target vs script)
- Emscripten installation approach
- Wasm output file location

### Deferred Ideas (OUT OF SCOPE)
None — discussion stayed within phase scope
</user_constraints>

## Standard Stack

### Core
| Tool | Version | Purpose | Why Standard |
|------|---------|---------|--------------|
| Emscripten (emsdk) | latest (4.0.x) | C++ to Wasm compiler | Only production-grade C++ → Wasm toolchain |
| Embind (`-lembind`) | included with emsdk | C++/JS binding | Automatic std::string conversion, type-safe, zero-config for simple functions |
| em++ | included with emsdk | C++ frontend | Drop-in replacement for g++, supports `-std=c++20` flag |

### Supporting
| Tool | Version | Purpose | When to Use |
|------|---------|---------|-------------|
| Node.js | 18+ | Testing Wasm in CI | Run evaluate() tests without browser |
| gzip/brotli | system | Size verification | Verify <1.5MB compressed output |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Embind | ccall/cwrap | ccall requires manual string memory management; Embind handles std::string automatically |
| Embind | wasm-bindgen | wasm-bindgen is Rust-only |
| `-lembind` | `--bind` | Both work; `-lembind` is the syntax used in current official docs (v5.0.x) |

### Installation

```bash
# Clone emsdk (one-time)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
./emsdk install latest
./emsdk activate latest
source ./emsdk_env.sh  # or emsdk_env.bat on Windows
```

## Architecture Patterns

### Recommended File Structure
```
src/
├── bigint.h           # existing — no changes needed
├── frac.h             # existing — no changes needed
├── series.h           # existing — no changes needed
├── qfuncs.h           # existing — no changes needed
├── convert.h          # existing — no changes needed
├── linalg.h           # existing — no changes needed
├── relations.h        # existing — no changes needed
├── parser.h           # existing — no changes needed
├── repl.h             # existing — add #ifndef __EMSCRIPTEN__ guards around terminal I/O
├── main.cpp           # existing — no changes needed (native CLI entry)
└── main_wasm.cpp      # NEW — Wasm entry point with Embind bindings
build/
└── wasm/
    ├── qseries.js     # Emscripten-generated JS glue
    └── qseries.wasm   # Compiled WebAssembly binary
tests/
├── test_wasm.mjs      # Node.js test script
└── test_wasm.html     # Browser test page
```

### Pattern 1: Separate Wasm Entry Point
**What:** A `main_wasm.cpp` file that includes the existing headers and exposes the Embind API without modifying the native CLI entry point.
**Why:** Avoids `#ifdef __EMSCRIPTEN__` clutter in `main.cpp`. The Wasm entry doesn't need `main()`, unit tests, `--version`, or `--test` flags — it only needs the evaluate/banner API.

```cpp
// src/main_wasm.cpp
#include <emscripten/bind.h>
#include <sstream>
#include "repl.h"

static Environment g_env;

std::string evaluate(std::string input) {
    std::ostringstream oss;
    auto* old_cout = std::cout.rdbuf(oss.rdbuf());
    auto* old_cerr = std::cerr.rdbuf(oss.rdbuf());

    try {
        std::string trimmed = trim(input);
        if (trimmed.empty()) {
            std::cout.rdbuf(old_cout);
            std::cerr.rdbuf(old_cerr);
            return "";
        }

        bool suppress = false;
        if (trimmed.back() == ':') {
            trimmed.pop_back();
            trimmed = trim(trimmed);
            suppress = true;
        }

        StmtPtr stmt = parse(trimmed);
        EvalResult res = evalStmt(stmt.get(), g_env);

        if (!suppress && !std::holds_alternative<std::monostate>(res)
                      && !std::holds_alternative<DisplayOnly>(res)) {
            display(res, g_env, g_env.T);
        }
    } catch (const std::exception& e) {
        oss << "error: " << e.what() << std::endl;
    }

    std::cout.rdbuf(old_cout);
    std::cerr.rdbuf(old_cerr);

    std::string result = oss.str();
    // Trim trailing newline for clean API
    while (!result.empty() && result.back() == '\n')
        result.pop_back();
    return result;
}

std::string get_banner() {
    return "qseries v2.0";
}

EMSCRIPTEN_BINDINGS(qseries) {
    emscripten::function("evaluate", &evaluate);
    emscripten::function("get_banner", &get_banner);
}
```

### Pattern 2: stdout Capture via streambuf Redirection
**What:** Temporarily redirect `std::cout.rdbuf()` to an `std::ostringstream` during evaluate() calls, capturing all output that would normally go to the terminal.
**Why:** This reuses the existing `display()` function and all `std::cout` calls in `dispatchBuiltin` (help, coeffs, series, version, etc.) without modifying them. All 48 `std::cout` call sites in `repl.h` are captured automatically.
**Critical detail:** Must also redirect `std::cerr` to the same stringstream so error messages from catch blocks are captured too. Must restore both in a RAII-safe way (including exception paths).

### Pattern 3: Terminal I/O Guard
**What:** Wrap all termios/SetConsoleMode/isatty code in `#ifndef __EMSCRIPTEN__` blocks.
**Where in repl.h:** Lines 19-25 (isatty includes/macros), 38-99 (RawModeGuard + readOneChar), 1073+ (readLineRaw). These ~80 lines need guarding.
**Why:** These APIs don't exist in Wasm. The Wasm entry point never calls `runRepl()` or `readLineRaw()`, but the compiler still needs to parse them when including `repl.h`.

```cpp
// In repl.h — guard the terminal-specific sections
#ifndef __EMSCRIPTEN__
#ifdef _WIN32
#include <io.h>
#define stdin_is_tty() _isatty(_fileno(stdin))
#else
#include <unistd.h>
#define stdin_is_tty() isatty(STDIN_FILENO)
#endif
// ... RawModeGuard, readOneChar, readLineRaw ...
#endif // __EMSCRIPTEN__
```

### Anti-Patterns to Avoid
- **Modifying display() to return strings:** Would require changing the return types and all call sites in dispatchBuiltin. The streambuf redirect is simpler and captures everything.
- **Using ccall/cwrap instead of Embind:** Requires manual memory management for strings (malloc, UTF8ToString, free). Embind handles std::string conversion automatically.
- **Putting Wasm code in main.cpp behind #ifdefs:** Creates maintenance burden. A separate file is cleaner.
- **Using SINGLE_FILE=1 in production:** Embeds Wasm as base64 in JS, increasing total size by ~33%. Use separate .wasm file.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| C++ → JS string conversion | Manual pointer/UTF8ToString | Embind std::string binding | Automatic memory management, no leaks |
| JS module loading | Custom script loader | Emscripten MODULARIZE | Handles async Wasm compilation, Node + browser |
| Exception → error string | setjmp/longjmp | C++ try/catch with `-fwasm-exceptions` | Native Wasm exceptions are smaller and faster than JS-based |

## Common Pitfalls

### Pitfall 1: Forgetting to Guard Terminal I/O
**What goes wrong:** Compilation fails with "termios.h: No such file or directory" or "isatty undeclared"
**Why it happens:** `repl.h` includes `<termios.h>` and uses `isatty()` unconditionally
**How to avoid:** Add `#ifndef __EMSCRIPTEN__` guards around ALL terminal I/O code blocks: the includes (lines 19-25, 38-43), RawModeGuard struct (lines 47-98), readOneChar (lines 66-98), and readLineRaw function (lines 1073-1131). Also guard runRepl() since it calls readLineRaw.
**Warning signs:** Compile errors mentioning termios, unistd, SetConsoleMode, isatty, STDIN_FILENO

### Pitfall 2: Not Restoring cout/cerr on Exception
**What goes wrong:** After an exception in evaluate(), subsequent calls write to the old stringstream or nowhere
**Why it happens:** If streambuf redirect isn't restored in both success and exception paths
**How to avoid:** Use RAII guard for streambuf restoration, or ensure restore code runs in both try and catch blocks. The evaluate() function shown above handles this correctly.
**Warning signs:** Missing output after an error, or output appearing in wrong calls

### Pitfall 3: Wrong Exception Handling Flag
**What goes wrong:** With no exception flag, `throw` becomes `abort()` — the entire Wasm module crashes instead of returning an error string
**Why it happens:** Emscripten defaults to no exception support for size optimization
**How to avoid:** Use `-fwasm-exceptions` (preferred — native Wasm exceptions, smaller code, better performance) or `-fexceptions` (broader browser support, larger binary). Use `-fwasm-exceptions` since all modern browsers support it.
**Warning signs:** Module.evaluate() causes "RuntimeError: unreachable" instead of returning "error: ..."

### Pitfall 4: Missing MODULARIZE Flag
**What goes wrong:** The generated JS file executes immediately, polluting global scope, and can't be imported as ES module for Node testing
**Why it happens:** Default Emscripten output is a self-executing script
**How to avoid:** Always compile with `-s MODULARIZE=1 -s EXPORT_NAME=createQSeries`. This wraps output in a factory function.
**Warning signs:** Global variable pollution, "Module is not a function" errors in Node tests

### Pitfall 5: Wasm Traps from Unguarded Division
**What goes wrong:** Division by zero in BigInt causes a Wasm trap (RuntimeError: unreachable) instead of a C++ exception
**Why it happens:** Wasm has trap semantics for integer division by zero, and some compilers optimize the C++ exception throw into a trap
**How to avoid:** The existing code already throws `std::invalid_argument` on BigInt division by zero. With `-fwasm-exceptions`, this should propagate correctly. Test specifically: `evaluate("1/0")` must return "error: ..." not crash.
**Warning signs:** "RuntimeError: unreachable" or "RuntimeError: divide by zero"

### Pitfall 6: Binary Size Bloat from Filesystem Support
**What goes wrong:** Wasm binary is much larger than expected (>2MB uncompressed)
**Why it happens:** Emscripten includes filesystem emulation by default
**How to avoid:** Use `-s FILESYSTEM=0` since this project does no file I/O. Also use `-Oz` for aggressive size optimization.
**Warning signs:** Wasm file >1MB uncompressed

### Pitfall 7: DisplayOnly and help() Output Not Captured
**What goes wrong:** `help()` returns empty string, `coeffs()` returns empty string
**Why it happens:** These built-ins write directly to `std::cout` inside `dispatchBuiltin` and return `DisplayOnly{}`, which `display()` ignores. If stdout isn't redirected during eval, their output is lost.
**How to avoid:** The streambuf redirect must wrap the ENTIRE evaluate flow (parse + evalStmt + display), not just the display() call. The pattern shown in the Architecture section handles this correctly.
**Warning signs:** help() and coeffs() return empty strings while other expressions work

## Code Examples

### Embind Binding (verified from official Emscripten docs)
```cpp
// Source: https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html
#include <emscripten/bind.h>
using namespace emscripten;

std::string exclaim(std::string message) {
    return message + "!";
}

EMSCRIPTEN_BINDINGS(my_module) {
    function("exclaim", &exclaim);
}
// Compile: em++ -lembind -o output.js input.cpp
// JS: Module.exclaim("hello") → "hello!"
```

### Makefile Target for Wasm Build
```makefile
# Wasm build (requires emsdk activated)
EMXX ?= em++
WASM_FLAGS = -std=c++20 -Oz -lembind \
    -s MODULARIZE=1 \
    -s EXPORT_NAME=createQSeries \
    -s FILESYSTEM=0 \
    -s ALLOW_MEMORY_GROWTH=1 \
    -s ENVIRONMENT=web,worker,node \
    -fwasm-exceptions

wasm: build/wasm/qseries.js

build/wasm:
	mkdir -p build/wasm

build/wasm/qseries.js: src/main_wasm.cpp | build/wasm
	$(EMXX) $(WASM_FLAGS) -o build/wasm/qseries.js src/main_wasm.cpp
```

### Node.js Test Script
```javascript
// tests/test_wasm.mjs
import createQSeries from '../build/wasm/qseries.js';

const Module = await createQSeries();

// Test 1: Basic arithmetic
const r1 = Module.evaluate("1+1");
console.assert(r1.includes("2"), `Expected "2", got "${r1}"`);

// Test 2: Series
const r2 = Module.evaluate("etaq(q, 1, 50)");
console.assert(!r2.startsWith("error:"), `Unexpected error: ${r2}`);

// Test 3: Error handling (not abort)
const r3 = Module.evaluate("1/0");
console.assert(r3.startsWith("error:"), `Expected error prefix, got "${r3}"`);

// Test 4: Stateful session
Module.evaluate("x := etaq(q,1,50)");
const r4 = Module.evaluate("prodmake(x,40)");
console.assert(!r4.startsWith("error:"), `prodmake failed: ${r4}`);

// Test 5: Banner
console.assert(Module.get_banner() === "qseries v2.0");

console.log("All Wasm tests passed");
```

### Test HTML Page
```html
<!DOCTYPE html>
<html>
<head><title>qseries Wasm Test</title></head>
<body>
<h1>qseries Wasm Test</h1>
<pre id="output"></pre>
<script type="module">
import createQSeries from './qseries.js';
const log = s => document.getElementById('output').textContent += s + '\n';
try {
    const M = await createQSeries();
    log('Banner: ' + M.get_banner());
    log('1+1 = ' + M.evaluate('1+1'));
    log('etaq: ' + M.evaluate('etaq(q,1,20)'));
    log('error test: ' + M.evaluate('1/0'));
    M.evaluate('x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)');
    log('prodmake: ' + M.evaluate('prodmake(x,40)'));
    log('ALL TESTS PASSED');
} catch(e) { log('FATAL: ' + e); }
</script>
</body>
</html>
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `--bind` flag for Embind | `-lembind` linker flag | Emscripten 4.x | Both still work; `-lembind` is current docs syntax |
| `-fexceptions` (JS-based) | `-fwasm-exceptions` (native) | Wasm exceptions proposal standardized ~2023 | 30-50% smaller exception handling code, better performance |
| `Module.print` callback for stdout | Still current | N/A | No change; streambuf redirect in C++ is cleaner for our use case |
| `--bind` generates legacy global | `-s MODULARIZE=1` | Long-standing | Modular output is standard practice now |

## Discretion Recommendations

### Stateful Sessions: YES — variables should persist
**Recommendation:** Use a global `Environment` that persists between evaluate() calls. This matches how the CLI REPL works and enables the multi-step workflows users expect (define x, then use x in prodmake).
**Rationale:** All CONTEXT.md examples assume statefulness (e.g., "x := ...; prodmake(x,40)"). Stateless would break the core use case.

### Entry Point: Separate file (`src/main_wasm.cpp`)
**Recommendation:** Create a separate Wasm entry point rather than #ifdef guards in main.cpp.
**Rationale:** main.cpp contains unit tests (691 lines of `runUnitTests`) and CLI argument handling that are irrelevant for Wasm. A clean 60-line main_wasm.cpp is easier to maintain.

### Multi-statement: Single statement per evaluate() call
**Recommendation:** Each evaluate() call processes one statement. Multi-line input with `\` continuation is not needed since the caller controls statement boundaries.
**Rationale:** Simpler API, easier error attribution, matches the existing evalStmt-per-line model.

### Timeout: Not implemented in Phase 43
**Recommendation:** Defer timeout to the playground phase. The Web Worker can implement a timeout externally by terminating the worker after N seconds.
**Rationale:** C++ doesn't have portable async timeout without threads. The Worker-based approach is simpler and more reliable.

### Build: Makefile target
**Recommendation:** Add a `wasm` target to the existing Makefile.
**Rationale:** Consistent with existing build system. Users already run `make` for native builds.

### Output location: `build/wasm/`
**Recommendation:** Output to `build/wasm/qseries.js` and `build/wasm/qseries.wasm`.
**Rationale:** Separates build artifacts from source. The `build/` directory is a standard convention and already exists conceptually alongside `dist/`.

## Open Questions

1. **Emscripten + C++20 flag compatibility**
   - What we know: The codebase uses C++17 features (structured bindings, if constexpr, std::variant). Passing `-std=c++20` to em++ should work since Emscripten's clang backend supports C++20 syntax.
   - What's unclear: Whether any edge case in the ~4,000 lines triggers an Emscripten-specific C++20 incompatibility.
   - Recommendation: Try `-std=c++20` first. Fall back to `-std=c++17` if needed (the code should compile fine either way).

2. **Exact Wasm binary size**
   - What we know: Native binary is 643KB. With `-Oz` and no filesystem, Wasm should be comparable or smaller. Gzip typically achieves 40-60% compression on Wasm.
   - What's unclear: Exact size until we compile. Exception handling adds some overhead.
   - Recommendation: Compile and measure. If over budget, try `-fwasm-exceptions` vs `-fexceptions` (former is smaller), or `-s MINIMAL_RUNTIME`.

3. **help() function stdout writes**
   - What we know: The `help()` built-in writes directly to `std::cout` inside `dispatchBuiltin` and returns `DisplayOnly{}`. The streambuf redirect captures this.
   - What's unclear: Whether any built-in function uses `printf()` or `write()` instead of `std::cout` (these would bypass the C++ streambuf redirect).
   - Recommendation: Audit all output paths in repl.h. Current audit shows all 48 stdout writes use `std::cout`, so streambuf redirect should capture everything.

## Sources

### Primary (HIGH confidence)
- Emscripten Embind docs (v5.0.3-git): https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html — Binding syntax, std::string support, EMSCRIPTEN_BINDINGS macro, compilation flags
- Emscripten Exception docs: https://emscripten.org/docs/porting/exceptions.html — `-fexceptions` vs `-fwasm-exceptions`, browser support, performance tradeoffs
- Emscripten Optimization docs: https://emscripten.org/docs/optimizing/Optimizing-Code.html — `-Oz`, FILESYSTEM=0, binary size reduction
- Emscripten MODULARIZE docs: https://emscripten.org/docs/compiling/Modularized-Output.html — Module factory pattern, EXPORT_NAME, ES6 module support
- Source code audit: `src/repl.h` (1293 lines), `src/main.cpp` (706 lines) — Identified all 48 std::cout sites, 3 std::cerr sites, terminal I/O dependencies

### Secondary (MEDIUM confidence)
- Emscripten emsdk releases: https://github.com/emscripten-core/emscripten/releases — Latest stable is 4.0.23
- WebAssembly/WASI termios issue #161: https://github.com/WebAssembly/WASI/issues/161 — Confirmed termios not available in Wasm
- StackOverflow: stdout capture via rdbuf redirect — Standard C++ pattern, works in Emscripten context

### Tertiary (LOW confidence)
- StackOverflow C++20 Emscripten issue (2022, may be outdated): std::span not available was the specific issue; our code doesn't use std::span

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — Emscripten + Embind is the only option for C++ → Wasm, well-documented
- Architecture: HIGH — Pattern is straightforward (separate entry + streambuf redirect + Embind), verified against source code
- Pitfalls: HIGH — Identified from direct source code audit and official Emscripten docs
- Binary size: MEDIUM — Estimate based on native binary size and typical Wasm overhead ratios; exact size unknown until compiled

**Research date:** 2026-02-27
**Valid until:** 2026-03-27 (Emscripten is stable; patterns unlikely to change)
