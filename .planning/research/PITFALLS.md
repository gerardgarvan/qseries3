# Pitfalls Research

**Domain:** Adding WebAssembly playground + Astro Starlight documentation site to existing C++20 CLI REPL
**Researched:** 2026-02-27
**Confidence:** HIGH (Emscripten, deployment) / MEDIUM (Astro Starlight integration specifics)

## Critical Pitfalls

### Pitfall 1: Terminal I/O Does Not Exist in the Browser

**What goes wrong:**
The existing REPL uses raw terminal I/O (termios on POSIX, `SetConsoleMode` / `ReadFile` on Windows) for character-by-character input, arrow key handling, tab completion, and history navigation. None of this exists in WebAssembly. Compiling the REPL as-is produces a binary that immediately fails — `tcgetattr` returns errors, `STDIN_FILENO` is a stub, and `stdin_is_tty()` returns false. The program falls back to `std::getline(std::cin, line)` which triggers the browser's `prompt()` popup — an unusable experience.

**Why it happens:**
Developers assume the existing REPL loop can be compiled straight to Wasm. The `#ifdef _WIN32` / `#ifdef __CYGWIN__` guards handle two platforms but not the browser. Emscripten provides a partial stdin/stdout emulation, but it's nowhere near a real terminal. The `RawModeGuard`, `readOneChar()`, `readLineRaw()` functions in `repl.h` (lines 48-98, 1073+) are all dead code in Wasm.

**How to avoid:**
Build a separate Wasm entry point that bypasses `runRepl()` entirely. Instead, expose an `evaluate(const char* input) -> const char*` function via Emscripten's `EXPORTED_FUNCTIONS` or embind. The JavaScript side (xterm.js or a textarea) handles all I/O: it collects user input, calls the C++ evaluate function, and displays the result. The C++ code becomes a pure function: string in, string out.

Concretely:
1. Create a `wasm_api.cpp` that includes `parser.h` and `repl.h`'s `Environment` / `evalStatement` but NOT `runRepl`
2. Expose `extern "C" { const char* wasm_eval(const char* input); }` 
3. Compile with `-sEXPORTED_FUNCTIONS=_wasm_eval -sEXPORTED_RUNTIME_METHODS=ccall,cwrap`
4. On the JS side: `const result = Module.ccall('wasm_eval', 'string', ['string'], [userInput]);`

**Warning signs:**
- `RawModeGuard` constructor runs but `active` stays false
- Browser shows `prompt()` dialogs
- Console errors about `tcgetattr` or `read(STDIN_FILENO)` failing
- Output appears in browser console instead of the page

**Phase to address:**
Wasm compilation phase (earliest Wasm work). This is the first thing to get right — nothing else works until I/O is solved.

---

### Pitfall 2: Long-Running Computations Freeze the Browser Tab

**What goes wrong:**
Functions like `etaq(q, 1, 500)`, `prodmake(f, 200)`, `findnonhomcombo(...)`, or deeply nested `sum()` calls can take seconds to minutes on native. In the browser, they block the main thread, freezing the UI completely. The tab becomes unresponsive, the browser may show a "page unresponsive" dialog, and the user cannot even cancel. Even `etaq(q, 1, 100)` — a common operation — takes noticeable time.

**Why it happens:**
WebAssembly runs on the main thread by default. JavaScript is single-threaded. The browser needs the main thread free every ~16ms to handle input events and rendering. A computation taking even 200ms causes visible jank; anything over 5 seconds triggers browser kill dialogs.

**How to avoid:**
Run the Wasm module in a Web Worker. The architecture should be:

```
Main Thread (UI)         Web Worker
  xterm.js / textarea  → postMessage(input)
  show "computing..."  ← postMessage(result)
  display result
```

This keeps the UI responsive. For cancellation of runaway computations, two options:
1. **Cooperative cancellation:** Add a `volatile` or atomic check variable in the C++ inner loops (Series multiplication, `etaq` loop, `prodmake` recurrence). Poll a SharedArrayBuffer flag periodically. Main thread sets the flag; worker code checks it.
2. **Worker termination:** Call `worker.terminate()` as a last resort. This destroys the worker — you lose the Environment state and must recreate it. Acceptable for an MVP playground.

For MVP, option 2 (terminate + restart) is simpler and sufficient. Add cooperative cancellation only if UX demands it.

**Warning signs:**
- Browser shows "page unresponsive" warning
- `etaq(q, 1, 50)` works but `etaq(q, 1, 200)` hangs the tab
- Users cannot type while computation runs
- "Stop" button does nothing because main thread is blocked

**Phase to address:**
Wasm playground integration phase. The Web Worker architecture must be the foundation of the playground, not an afterthought.

---

### Pitfall 3: Emscripten Exception Handling Disabled By Default — Silent Aborts

**What goes wrong:**
The existing codebase uses `throw std::invalid_argument(...)` and `throw std::runtime_error(...)` extensively — in `bigint.h` (division by zero), `frac.h` (zero denominator), `series.h` (inverse of zero), `parser.h` (parse errors), and `repl.h` (runtime errors, ~85 catch sites). Emscripten disables C++ exception catching by default at `-O1` and above. When an exception is thrown, the program aborts instead of catching it. User input errors like `1/0` or `prodmake(0, 50)` crash the entire Wasm module instead of showing an error message.

**Why it happens:**
Emscripten's FAQ explains: exception handling has "relatively high overhead" in Wasm. The JavaScript-based implementation adds wrapper code at every potentially-throwing call site. The project has ~85 catch sites in `repl.h` alone — all silently broken without `-fexceptions`.

**How to avoid:**
Compile with `-fwasm-exceptions` (native Wasm exception handling, lower overhead than JavaScript-based `-fexceptions`). This requires browser support for the Wasm exceptions proposal — supported in Chrome 95+, Firefox 100+, Safari 15.2+. If targeting older browsers, use `-fexceptions` instead.

Alternative: wrap the top-level `wasm_eval` function in a single try-catch that returns error strings. This limits the exception overhead to one catch site that's always active, while individual `throw` sites still work correctly.

```cpp
extern "C" const char* wasm_eval(const char* input) {
    try {
        // ... evaluate input ...
        return result.c_str();
    } catch (const std::exception& e) {
        return format_error(e.what());
    }
}
```

Test with: `1/0`, `prodmake(0, 10)`, `etaq(q, 0, 10)`, `sum(q^n, n, 0, -1)`.

**Warning signs:**
- Module aborts (prints "Aborted()" to console) on invalid input
- `Module.ccall` throws a JavaScript error instead of returning an error string
- "exception catching is disabled, this exception cannot be caught" in console
- Works fine at `-O0`, crashes at `-O2`

**Phase to address:**
Wasm compilation phase. Must be in the initial Emscripten build flags and tested immediately.

---

### Pitfall 4: Wasm Binary Size Blowup — Multi-MB Downloads for a Playground

**What goes wrong:**
The complete qseries REPL compiled to Wasm with Asyncify and exception support can easily reach 2-5 MB (wasm) + 500KB-1MB (JS glue). Combined with xterm.js (~700KB), the playground page becomes 3-7 MB. On mobile or slow connections, users wait 10+ seconds before they can type their first command. This kills the "try it now" value proposition.

**Why it happens:**
- Asyncify alone inflates Wasm binary by ~70%
- `-fexceptions` (JS-based) adds wrapper code at every call site
- The REPL includes ~50 functions, many of which a playground user may never call
- `std::map`, `std::vector`, `std::string`, and other STL containers pull in substantial code
- Debug info or lack of optimization (`-O0`) makes it worse

**How to avoid:**
1. Compile with `-Os` or `-Oz` for size optimization, not `-O2`
2. Use `-fwasm-exceptions` instead of `-fexceptions` (smaller code)
3. Run `wasm-opt -Os` on the output (Binaryen post-processing)
4. Use `--closure 1` to minify the JS glue code
5. Serve `.wasm` files with gzip/brotli compression (60-75% reduction)
6. Consider lazy-loading the Wasm module — show documentation immediately, load playground only when user clicks "Try It"
7. Set a size budget: target <1.5 MB compressed for the complete playground

Measure early. After the first successful Emscripten build, check the file sizes. If over budget, investigate which functions contribute most and whether module splitting (`-sSPLIT_MODULE`) is worthwhile.

**Warning signs:**
- `.wasm` file exceeds 3 MB uncompressed
- JS glue file exceeds 500 KB
- Lighthouse performance score drops below 80 on the playground page
- Time to Interactive exceeds 5 seconds on a throttled connection

**Phase to address:**
Wasm compilation phase (initial build) and deployment phase (compression, lazy loading).

---

### Pitfall 5: Memory Growth Causes Performance Regression in Wasm

**What goes wrong:**
BigInt operations allocate `std::vector<uint32_t>` dynamically. Series operations create `std::map<int, Frac>` entries. Heavy computations like `etaq(q, 1, 500)` or `prodmake(f, 200)` allocate megabytes of temporaries. If compiled with `ALLOW_MEMORY_GROWTH=1`, every allocation that grows the heap causes a performance penalty — benchmarks show up to 2x slowdown. If compiled with a fixed heap, memory-intensive computations crash with OOM.

**Why it happens:**
Wasm linear memory cannot be shrunk, only grown. Growing memory may require copying the entire heap to a new location (depending on the browser engine). With `ALLOW_MEMORY_GROWTH=0`, the heap is fixed at compile time (default 16 MB), which is insufficient for large truncation values.

**How to avoid:**
Use `ALLOW_MEMORY_GROWTH=1` with a generous initial allocation:
```
-sINITIAL_MEMORY=64MB -sALLOW_MEMORY_GROWTH=1 -sMAXIMUM_MEMORY=256MB
```

This starts with 64 MB (enough for most operations) and grows only when needed, up to 256 MB. The initial allocation avoids frequent growth during normal use. The maximum prevents runaway allocations from consuming the user's entire browser memory.

For the playground, also limit the truncation value (`set_trunc`) to a reasonable maximum (e.g., 500) to prevent users from accidentally requesting enormous computations.

**Warning signs:**
- `etaq(q, 1, 200)` is 2-3x slower in the browser than native
- Browser tab memory usage spikes to 500+ MB
- "out of memory" errors in the console
- Performance degrades over multiple computations without page refresh

**Phase to address:**
Wasm compilation phase (memory flags) and playground UI phase (truncation limits, memory reset).

---

### Pitfall 6: COOP/COEP Headers Required for SharedArrayBuffer — Breaks CDN/Hosting

**What goes wrong:**
If the Web Worker approach uses `SharedArrayBuffer` (needed for cooperative cancellation via shared memory, or for Emscripten pthreads), the hosting server must serve specific HTTP headers: `Cross-Origin-Opener-Policy: same-origin` and `Cross-Origin-Embedder-Policy: require-corp`. Without these, `SharedArrayBuffer` is undefined and the playground crashes on load. Worse: these headers break loading of any cross-origin resources (Google Fonts, CDN scripts, analytics) unless those resources also have CORS headers.

**Why it happens:**
Browsers gate `SharedArrayBuffer` behind cross-origin isolation as a Spectre mitigation. This is enforced in Chrome 92+, Firefox 79+, Safari 15.2+. It's easy to develop locally (where restrictions may be relaxed) and only discover the issue in production.

**How to avoid:**
For MVP, **avoid SharedArrayBuffer entirely**:
- Use simple `postMessage` between main thread and Web Worker (no shared memory needed)
- For cancellation, use `worker.terminate()` + create new worker (crude but works)
- This eliminates the COOP/COEP requirement entirely

If cooperative cancellation is later needed:
- Use a `_headers` file for Cloudflare Pages or server config for Nginx/Apache
- Audit all cross-origin resources: Google Fonts, CDN links, analytics scripts
- Use `credentialless` COEP as a more permissive alternative: `Cross-Origin-Embedder-Policy: credentialless`

**Warning signs:**
- `SharedArrayBuffer is not defined` in browser console
- Playground works on localhost but not in production
- Google Fonts or CDN resources stop loading after adding headers
- `self.crossOriginIsolated` returns false in production

**Phase to address:**
Deployment phase. Design for no SharedArrayBuffer initially; add only if cooperative cancellation becomes a requirement.

---

### Pitfall 7: Astro Starlight Component Islands — Client Hydration Gotchas

**What goes wrong:**
Starlight is built on Astro, which defaults to zero client-side JavaScript. The Wasm playground requires a fully interactive client component (xterm.js or textarea + Wasm module). Developers embed the playground component but forget `client:load` or `client:visible` directives, resulting in a component that renders server-side HTML but never hydrates — the terminal appears but keyboard input doesn't work. Alternatively, using `client:load` on a heavy component (xterm.js + Wasm) delays page load for all documentation pages.

**Why it happens:**
Astro's "Islands Architecture" is opt-in interactivity. Starlight documentation pages are static by default. Interactive components must explicitly declare their hydration strategy. This is different from React/Next.js where everything is interactive by default.

**How to avoid:**
1. Use `client:visible` (not `client:load`) for the playground component — this loads the Wasm module only when the user scrolls to the playground, keeping documentation pages fast
2. Create the playground as a standalone Astro component that imports xterm.js and the Wasm module
3. Use the playground component only on specific pages (e.g., `/playground/`) via MDX import, not globally
4. Test both: documentation pages WITHOUT the playground (should be fast, zero JS) and the playground page (should be interactive)

```astro
---
// In a documentation page with playground
import QseriesPlayground from '../../components/QseriesPlayground.astro';
---

<QseriesPlayground client:visible />
```

**Warning signs:**
- Playground renders but keyboard input is ignored
- Documentation pages load slowly (pulling in Wasm module on every page)
- `client:load` is used on the playground, adding 3+ MB to every page's JS bundle
- Console shows "hydration mismatch" warnings

**Phase to address:**
Documentation site setup phase. Decide the component strategy early; test hydration before building the full playground.

---

## Technical Debt Patterns

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Running Wasm on main thread (no Worker) | Simpler architecture, no message passing | UI freezes on any non-trivial computation | Never for production. Only for initial "does it compile?" validation |
| Using `worker.terminate()` instead of cooperative cancellation | No shared memory needed, no COOP/COEP headers | Loses environment state (variables, truncation) on cancel; must reinitialize | MVP — acceptable if cancel is rare. Add cooperative cancellation if users complain |
| Hardcoded truncation limit in browser (e.g., max T=200) | Prevents OOM and long-running computations | Limits what users can explore; different behavior from CLI | MVP — acceptable with clear UI messaging ("browser limited to T≤200") |
| Single `.wasm` file (no module splitting) | Simpler build, simpler deployment | Larger initial download, slower first load | Acceptable if compressed size stays under 1.5 MB |
| Skipping xterm.js, using a simple textarea | No terminal emulation dependency, smaller bundle | No syntax coloring, no real terminal feel, no cursor movement | MVP — can upgrade later without architectural changes |

## Integration Gotchas

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| Astro + Wasm module | Importing `.wasm` file in Astro's build pipeline — Vite doesn't handle raw Wasm well | Place `.wasm` in `public/` directory; load at runtime via `fetch()` in client-side JS |
| Starlight + custom pages | Adding playground as a Starlight documentation page — inherits sidebar/layout constraints | Create a custom Astro page at `/playground/` outside Starlight's content collection, or use Starlight overrides for layout-free pages |
| xterm.js + Emscripten | Assuming Emscripten's stdin/stdout hooks work with xterm.js directly | Use the `wasm_eval` API pattern: xterm.js captures input → calls Wasm function → displays result. Don't try to pipe stdin/stdout |
| GitHub Pages + Wasm | Assuming `.wasm` files serve correctly | GitHub Pages serves `.wasm` with wrong MIME type (`text/html`). Use Cloudflare Pages, Netlify, or Vercel instead — all handle `application/wasm` correctly |
| Emscripten + existing Makefile | Adding Wasm target to existing Makefile that uses `g++` | Create a separate `Makefile.wasm` or a build script. Emscripten uses `emcc`/`em++` with different flags (`-sEXPORTED_FUNCTIONS`, `-sASYNCIFY`, etc.). Keep native and Wasm builds completely separate |
| Web Worker + Wasm module | Loading the Wasm module on the main thread, then trying to move it to a Worker | Load and instantiate the Wasm module inside the Worker. The Worker script should `importScripts('qseries.js')` or use ES module import. The main thread never touches the Wasm module directly |

## Performance Traps

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| No Wasm compression | 3-5 MB download, 10+ second load on mobile | Serve with gzip/brotli. Most CDNs do this automatically. Verify with DevTools Network tab | Always — there's no reason not to compress |
| Loading Wasm eagerly on every page | All documentation pages become slow | Lazy-load: only load Wasm on the playground page, use `client:visible` for the component | As soon as you have more than 3 documentation pages |
| Static memoization caches grow unbounded in browser | `etaq` cache (`std::map<pair<int,int>, Series>`) grows without limit across multiple computations | Either clear caches periodically, expose `clear_cache()` to the playground, or set a cache size limit for Wasm builds | After 20+ sequential computations in one session |
| Allocating fresh `std::string` for every `wasm_eval` return | Memory leak — returned strings are never freed from the Wasm heap | Use a static return buffer, or have the JS side call `Module._free()` after reading the result | After hundreds of evaluations in one session |

## Security Mistakes

| Mistake | Risk | Prevention |
|---------|------|------------|
| No computation timeout in Wasm | User enters `sum(q^(n^2), n, 0, 999999)` — tab hangs forever, possible DOS of their browser | Implement timeout via Worker termination. Kill worker after 30 seconds of computation |
| No input length limit | User pastes a 10 MB expression — parser allocates enormous AST | Limit input to 10,000 characters in the JS layer before sending to Wasm |
| Exposing raw Emscripten Module to page | `Module.ccall` can call any exported function; crafted input could probe memory | Only expose the `wasm_eval` wrapper. Don't export internal functions. Use `-sEXPORTED_FUNCTIONS=_wasm_eval` exclusively |
| Serving Wasm over HTTP (not HTTPS) | Modern browsers restrict Wasm loading over insecure connections; also no Service Worker support | Always serve from HTTPS. Static site hosts (Cloudflare, Netlify, Vercel) provide this by default |

## UX Pitfalls

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| No loading indicator while Wasm downloads | User sees an empty/broken playground for 3-10 seconds | Show a spinner or "Loading playground..." message. Hide the input area until Wasm is ready |
| No "computing..." feedback during evaluation | User types `etaq(q,1,100)`, nothing happens for 2 seconds, user types it again | Show a "Computing..." indicator immediately on submit. Disable input until result returns |
| Different behavior between CLI and browser | CLI supports `set_trunc(1000)` but browser limits to 200 — user is confused | Clearly document browser limitations. Show a message: "Browser playground limited to T≤200. Download the CLI for larger computations." |
| No way to clear state / reset environment | User's variables accumulate, `q` gets overwritten, they're stuck | Add a "Reset" button that terminates the Worker and creates a fresh one |
| Playground input doesn't support multi-line / backslash continuation | CLI supports `\` continuation but the playground textarea doesn't | Either support multi-line input (textarea with Shift+Enter) or document that the playground is single-expression only |
| Output formatting differs between CLI and browser | CLI shows aligned columns; browser textarea has different font/width | Use a monospace font for output. Match the CLI's output formatting. Test with the longest expected output (e.g., `coeffs(f, 0, 50)`) |

## "Looks Done But Isn't" Checklist

- [ ] **Wasm eval:** Works for basic expressions but crashes on error input — verify `1/0`, `prodmake(0,10)`, `parse("(((")`
- [ ] **Web Worker:** Computes correctly but has no timeout — verify with `sum(q^n, n, 0, 1000000)` that it doesn't hang forever
- [ ] **Playground UI:** Terminal renders but doesn't handle paste — verify Ctrl+V works in xterm.js / textarea
- [ ] **Compression:** Wasm is served but not compressed — check Content-Encoding header in DevTools
- [ ] **Mobile:** Works on desktop but playground is unusable on mobile — test on actual phone/tablet (virtual keyboard, small screen)
- [ ] **MIME types:** Works on dev server but `.wasm` 404s or wrong MIME in production — verify with `curl -I https://site/qseries.wasm`
- [ ] **Memory cleanup:** Individual evaluations work but 50 sequential evaluations leak memory — check Wasm heap size after extended use
- [ ] **Error messages:** C++ exceptions produce error strings but they're raw `what()` text — wrap in user-friendly messages
- [ ] **Variable persistence:** Variables set in one evaluation are available in the next — verify `x := etaq(q,1,50)` then `prodmake(x, 40)` works
- [ ] **Cache behavior:** Browser caches old `.wasm` file after deployment — verify cache-busting (content hash in filename or proper Cache-Control headers)

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Main-thread Wasm (no Worker) | MEDIUM | Refactor to Worker-based architecture. The `wasm_eval` API doesn't change — only where it runs changes. 1-2 day refactor |
| Wrong MIME type on hosting | LOW | Switch hosting provider or add `_headers` file. No code changes needed |
| Binary size too large | MEDIUM | Add `-Os`, `--closure 1`, compression. May need module splitting if still too large. 1 day |
| Exceptions disabled | LOW | Add `-fwasm-exceptions` flag. Test all error paths. Half-day fix |
| Memory leak from string returns | LOW | Add `Module._free()` call in JS after reading result, or use static buffer. 1-2 hours |
| xterm.js hydration failure | LOW | Add correct `client:visible` directive. 30-minute fix |
| COOP/COEP breaks CDN resources | MEDIUM | Audit all cross-origin resources, add CORS headers, or switch to `credentialless` COEP. 1 day |

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Terminal I/O doesn't exist in browser | Wasm API design (earliest) | `wasm_eval("1+1")` returns `"2"` without any stdin/stdout |
| Long computations freeze browser | Playground architecture (Web Worker setup) | `etaq(q,1,200)` completes without freezing UI; "Stop" button works |
| Exceptions disabled by default | Wasm compilation (build flags) | `wasm_eval("1/0")` returns error string, doesn't abort module |
| Binary size blowup | Wasm compilation + deployment | Compressed `.wasm` + JS < 1.5 MB total |
| Memory growth performance | Wasm compilation (memory flags) | `etaq(q,1,100)` runs within 2x of native speed |
| COOP/COEP header requirements | Deployment (hosting config) | `self.crossOriginIsolated` matches expectation; no console errors |
| Astro component hydration | Documentation site setup | Playground page is interactive; doc pages have zero JS overhead |
| GitHub Pages wrong MIME | Deployment (hosting choice) | `curl -I` returns `application/wasm` for `.wasm` files |
| Unbounded memoization cache | Playground UX polish | "Reset" button clears state; memory doesn't grow indefinitely |
| No computation timeout | Playground UX (cancel mechanism) | 30-second timeout terminates worker; user can continue |

## Sources

- [Emscripten Portability Guidelines](https://emscripten.org/docs/porting/guidelines/portability_guidelines.html) — code patterns that won't compile or run slowly in Wasm
- [Emscripten C++ Exceptions Support](https://emscripten.org/docs/porting/exceptions.html) — `-fexceptions` vs `-fwasm-exceptions`, overhead analysis
- [Emscripten Asyncify](https://emscripten.org/docs/porting/asyncify.html) — ~70% code size increase, runtime overhead analysis
- [Emscripten Interacting with Code](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html) — `ccall`, `cwrap`, `EXPORTED_FUNCTIONS`
- [Emscripten Memory Settings](https://emscripten.org/docs/tools_reference/settings_reference.html) — `ALLOW_MEMORY_GROWTH`, `INITIAL_MEMORY`, `MAXIMUM_MEMORY`
- [Emscripten Module Splitting](https://emscripten.org/docs/optimizing/Module-Splitting.html) — lazy loading of Wasm code
- [Emscripten Pthreads / SharedArrayBuffer](https://emscripten.org/docs/porting/pthreads.html) — COOP/COEP header requirements
- [xterm-pty](https://xterm-pty.netlify.app/) — bridge for xterm.js + Emscripten programs
- [wasm-webterm](https://github.com/cryptool-org/wasm-webterm) — alternative xterm.js + Wasm integration
- [web.dev: COOP and COEP](https://web.dev/articles/coop-coep) — cross-origin isolation requirements
- [Starlight Customization Guide](https://starlight.astro.build/guides/customization) — component overrides, CSS layers
- [Starlight CSS & Styling](https://starlight.astro.build/guides/css-and-tailwind) — cascade layer gotchas
- [Starlight Overrides Reference](https://starlight.astro.build/reference/overrides/) — component override complexity warnings
- [GitHub Pages WASM MIME issue](https://stackoverflow.com/questions/79381719/github-pages-page-wrong-mime-type) — confirmed `text/html` served for `.wasm`
- [Cloudflare Pages Headers](https://developers.cloudflare.com/pages/configuration/headers) — `_headers` file for MIME type configuration
- [Emscripten Deploying Pages](https://emscripten.org/docs/compiling/Deploying-Pages.html) — gzip compression for Wasm (60-75% reduction)
- [StackOverflow: Cancel Wasm in Worker](https://stackoverflow.com/questions/57365381/how-to-cancel-a-wasm-process-from-within-a-webworker) — `worker.terminate()` vs cooperative cancellation
- [ALLOW_MEMORY_GROWTH performance](https://emscripten-discuss.narkive.com/t2ZA8bo5) — 2x slowdown benchmark

---
*Pitfalls research for: C++20 REPL → WebAssembly playground + Astro Starlight documentation site*
*Researched: 2026-02-27*
