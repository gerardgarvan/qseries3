# Project Research Summary

**Project:** Q-Series REPL v2.1 — Documentation Website with WebAssembly Playground
**Domain:** Mathematical software documentation site with live interactive computation
**Researched:** 2026-02-27
**Confidence:** HIGH

## Executive Summary

This milestone adds a documentation website with a live-in-browser playground to the existing C++20 q-series CLI REPL. The recommended stack is **Astro Starlight** for documentation (zero-JS by default, built-in search, dark mode, KaTeX math rendering) with the C++ REPL compiled to **WebAssembly via Emscripten** and embedded as a **React island**. Cloudflare Pages is the recommended host for its free tier, correct `.wasm` MIME type handling, and `_headers` support. The architecture is a monorepo with a `website/` subfolder — the Wasm build compiles the same `src/*.h` files as the native build, keeping everything in sync.

The critical technical decision is the I/O bridge between browser and Wasm. Research identified two viable approaches: (1) **Embind `evaluate(line) → string`** — simpler, no Asyncify overhead, no SharedArrayBuffer, but batch-mode only (loses interactive REPL feel); (2) **xterm-pty + Asyncify** — full terminal emulation with arrow keys, tab completion, and variable persistence, but ~70% Wasm size overhead and higher integration complexity. **Recommendation: start with Embind batch mode for MVP, design the architecture (Web Worker, component boundaries) to support an xterm-pty upgrade later.** The feature research confirms this — users want to try q-series in their browser, not necessarily have a pixel-perfect terminal clone on day one.

The primary risks are all in the Emscripten compilation phase: terminal I/O dead code must be bypassed (the existing `readLineRaw` with termios/Win32 console APIs won't compile), C++ exception handling is disabled by default at `-O1+` (causing silent aborts on `1/0` or parse errors), and the binary can balloon to 3-5 MB without size optimization. All three risks have known, well-documented mitigations. A Web Worker is non-negotiable for the playground — without it, any computation over ~200ms freezes the browser tab.

## Key Findings

### Recommended Stack

The website introduces an entirely new technology layer (JavaScript/Node.js ecosystem) alongside the existing C++20 codebase. No changes to the existing C++ source are needed for the basic build — conditional compilation (`#ifdef __EMSCRIPTEN__`) isolates Wasm-specific code.

**Core technologies:**
- **Astro Starlight** (^5.7 / ^0.37): Documentation framework — zero-JS pages by default, built-in Pagefind search, dark mode, sidebar nav, MDX support. No custom theme work needed.
- **Emscripten emsdk** (^4.0): C++ → Wasm compiler — only viable C++20 → Wasm toolchain. Supports `std::map`, `std::vector`, all STL features used by the codebase.
- **React** (^19.0): UI framework for the playground island only — Astro renders everything else as static HTML.
- **@xterm/xterm** (^5.5): Terminal emulator component for the playground UI. Industry standard (powers VS Code terminal).
- **Cloudflare Pages**: Static hosting — free tier, correct `application/wasm` MIME type, global CDN, `_headers` file support.

**Key rejections:** No GMP/Boost (violates zero-dependency constraint), no Asyncify for MVP (70% size overhead), no server-side execution (client-side Wasm eliminates hosting costs), no Monaco/CodeMirror (overkill for math expressions), no Tailwind (Starlight's built-in styles suffice).

### Expected Features

**Must have (table stakes):**
- Responsive layout, dark mode, full-text search, sidebar navigation (all Starlight defaults)
- Math rendering via KaTeX (remark-math + rehype-katex) — q-series docs are meaningless without rendered Σ, Π, (a;q)_n
- Playground with code input, Run button, output panel, error display
- Wasm binary compiled from the existing C++ source
- 5-8 preloaded examples sourced from `garvan-demo.sh`
- Loading indicator while Wasm downloads (~2-4 MB payload)

**Should have (differentiators):**
- Shareable permalinks (encode playground code in URL hash — no server needed)
- Inline "Try It" buttons in documentation pages (Codapi pattern)
- Terminal mode upgrade (xterm.js + xterm-pty for full interactive REPL)
- Guided tutorial walkthrough from `qseriesdoc.md`

**Defer (v2+):**
- LocalStorage draft persistence, embeddable widget, multiple output formats
- User accounts, server-side execution, collaborative editing, Jupyter integration

**Competitive advantage:** Client-side Wasm is rare among math playgrounds (Rust/Go/Sage all use server-side). Integrated docs + playground is uncommon. No math software project has a modern Starlight-quality docs site.

### Architecture Approach

Monorepo with `website/` subfolder containing an independent Astro project. The Emscripten build reads the same `src/*.h` + `src/main.cpp` as the native `g++` build. A thin `wasm_entry.cpp` (or `#ifdef __EMSCRIPTEN__` block) exposes an `evaluate()` function via Embind. The playground is a React island (`client:only="react"`) on a dedicated `/playground/` page — documentation pages remain static HTML with zero JS. The Wasm module runs inside a **Web Worker** to prevent UI freezing; communication is via `postMessage`.

**Major components:**
1. **Emscripten build pipeline** — `em++` with `-fwasm-exceptions`, `-O2`/`-Os`, `MODULARIZE=1`, `ALLOW_MEMORY_GROWTH=1` → produces `qseries.wasm` + `qseries.js`
2. **Astro Starlight site** — MDX documentation pages (manual, reference, tutorial), Pagefind search, KaTeX math
3. **QSeriesPlayground React component** — xterm.js terminal UI, Web Worker message passing, loading/computing states
4. **Web Worker** — loads and instantiates Wasm module, receives input via `postMessage`, returns results
5. **CI/CD pipeline** — GitHub Actions: emsdk setup → `make wasm` → `npm run build` → deploy to Cloudflare Pages

### Critical Pitfalls

1. **Terminal I/O dead code in Wasm** — `readLineRaw()`, `RawModeGuard`, termios/Win32 APIs are dead code under Emscripten. **Avoid:** Create a separate Wasm entry point that bypasses `runRepl()` and exposes `evaluate(string) → string`. Use `#ifdef __EMSCRIPTEN__` guards.

2. **Long computations freeze browser** — `etaq(q,1,200)` or `findnonhomcombo()` can take seconds, blocking the main thread. **Avoid:** Run Wasm in a Web Worker from day one. Use `worker.terminate()` for cancellation (loses state but acceptable for MVP).

3. **C++ exceptions silently disabled** — Emscripten disables exception catching at `-O1+`. The codebase has ~85 catch sites in `repl.h`. Without `-fwasm-exceptions`, `1/0` aborts the module instead of showing an error. **Avoid:** Compile with `-fwasm-exceptions` (native Wasm exceptions, supported in Chrome 95+/Firefox 100+/Safari 15.2+).

4. **Wasm binary size blowup** — Full REPL + STL + exception support = 2-5 MB uncompressed. **Avoid:** Use `-Os` or `-O2`, `-fwasm-exceptions` (not `-fexceptions`), `--closure 1`, gzip/brotli compression (60-75% reduction). Budget: <1.5 MB compressed.

5. **GitHub Pages serves wrong MIME for `.wasm`** — Returns `text/html` instead of `application/wasm`. **Avoid:** Use Cloudflare Pages (correct MIME by default) or configure `_headers` file.

## Implications for Roadmap

Based on combined research, here is the suggested phase structure. The Wasm compilation is the critical path — everything playground-related depends on it. Documentation content is independent and can be parallelized.

### Phase 1: Wasm Compilation & API
**Rationale:** Critical path — nothing playground-related works without a functioning Wasm binary. This is also where the highest-risk pitfalls live (I/O dead code, exceptions, binary size).
**Delivers:** `qseries.wasm` + `qseries.js` that can evaluate q-series expressions from JavaScript.
**Tasks:** Create `wasm_entry.cpp` with Embind `evaluate()`, configure Emscripten flags (`-fwasm-exceptions`, `-Os`, `MODULARIZE`, `ALLOW_MEMORY_GROWTH`), verify error handling (`1/0`, parse errors), measure binary size.
**Addresses:** Playground table stakes (client-side execution)
**Avoids:** Pitfalls 1 (terminal I/O), 3 (exceptions), 4 (binary size), 5 (memory growth)

### Phase 2: Astro Starlight Scaffold
**Rationale:** Independent of Wasm — can be done in parallel with Phase 1. Establishes the website structure that all content and the playground will live in.
**Delivers:** `website/` directory with Astro Starlight, React integration, sidebar nav, placeholder pages.
**Tasks:** `npm create astro`, add Starlight + React integrations, configure sidebar, set up KaTeX (remark-math + rehype-katex), create placeholder pages for manual/reference/tutorial/playground.
**Addresses:** Site table stakes (responsive, dark mode, search, navigation)
**Avoids:** Pitfall 7 (hydration gotchas — test `client:only` early)

### Phase 3: Playground Component & Web Worker
**Rationale:** Depends on Phase 1 (Wasm binary) and Phase 2 (Astro site). This is the core interactive feature.
**Delivers:** Working playground page with xterm.js terminal, Web Worker execution, loading states, error display, cancel button.
**Tasks:** Build `QSeriesPlayground.tsx` React component, set up Web Worker to load Wasm, wire xterm.js for input/output, add loading indicator, add "Reset" button, test with preloaded examples.
**Addresses:** Playground table stakes (code editor, output panel, error display, loading indicator)
**Avoids:** Pitfall 2 (main-thread blocking), Pitfall 6 (COOP/COEP — avoided by using `postMessage` not SharedArrayBuffer)

### Phase 4: Documentation Content
**Rationale:** Independent of playground — can be done in parallel with Phase 3. Content already exists in MANUAL.md and qseriesdoc.md.
**Delivers:** Complete documentation pages: manual (split into sections), function reference, tutorial with KaTeX math and code examples.
**Tasks:** Convert MANUAL.md → MDX sections, convert qseriesdoc.md → tutorial pages, add KaTeX formulas, verify rendering in dark/light modes, add landing page with feature highlights and download links.
**Addresses:** Must-have features (math rendering, manual pages, semantic URLs)

### Phase 5: CI/CD & Deployment
**Rationale:** Depends on Phases 1-4 working locally. Automates the build-and-deploy pipeline.
**Delivers:** Automated deployment: push to main → build Wasm → build site → deploy to Cloudflare Pages.
**Tasks:** Create GitHub Actions workflow (`deploy-website.yml`), set up emsdk in CI (`mymindstorm/setup-emsdk`), add Wrangler for Cloudflare deployment, configure cache headers for `.wasm` files, verify MIME types in production.
**Addresses:** Deployment requirements
**Avoids:** Pitfall 5 (wrong MIME — Cloudflare handles correctly), compression (Cloudflare automatic gzip/brotli)

### Phase 6: Polish & Differentiators
**Rationale:** After core site is live and validated. These features elevate the site from functional to impressive.
**Delivers:** Preloaded example gallery, shareable permalinks, inline "Try It" buttons in docs, mobile polish.
**Tasks:** Build example dropdown from garvan-demo.sh, implement URL hash encoding for permalinks, add "Run" buttons to doc code blocks, test on mobile devices, add "browser limited to T≤N" messaging.
**Addresses:** Differentiator features (permalinks, inline Try It, example gallery)

### Phase Ordering Rationale

- **Phase 1 first** because it's the highest-risk, highest-dependency item. If Emscripten compilation fails or produces an unusable binary, the entire playground is blocked.
- **Phase 2 parallel with Phase 1** because the Astro site has zero dependency on the Wasm binary. Documentation pages are static HTML.
- **Phase 3 after Phase 1+2** because the playground component needs both the Wasm binary (to run) and the Astro site (to live in).
- **Phase 4 parallel with Phase 3** because content conversion is pure Markdown/MDX work, independent of the playground.
- **Phase 5 after Phase 3+4** because CI/CD should automate what already works locally. Deploying broken builds wastes time.
- **Phase 6 last** because differentiators are valuable but not blocking. The site is useful without them.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 1 (Wasm Compilation):** Emscripten flag interactions are complex. Need to validate: does `-fwasm-exceptions` + `-Os` + `MODULARIZE` work together? What's the actual binary size? Does `std::map` performance degrade in Wasm?
- **Phase 3 (Playground):** Web Worker + Wasm module loading has nuances (ES module Workers, `importScripts` vs dynamic import). xterm.js configuration for non-PTY usage needs investigation.

Phases with standard patterns (skip research-phase):
- **Phase 2 (Astro Scaffold):** Starlight is extremely well-documented. `npm create astro` + add integrations is routine.
- **Phase 4 (Content):** Markdown → MDX conversion is mechanical. KaTeX syntax is well-documented.
- **Phase 5 (CI/CD):** GitHub Actions + Cloudflare Pages deployment is a standard pattern with many examples.

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Astro Starlight, Emscripten, xterm.js are all mature, well-documented technologies. Cloudflare Pages free tier is well-understood. |
| Features | HIGH | Feature landscape is well-mapped by comparing Rust/Go/Julia/Sage playgrounds. MVP scope is clear. |
| Architecture | HIGH | Monorepo structure is straightforward. Emscripten Embind and Web Worker patterns are proven. xterm-pty has a working Vim demo as proof. |
| Pitfalls | HIGH | All 7 pitfalls have documented mitigations with specific Emscripten flags or architectural decisions. Recovery costs are LOW-MEDIUM for all. |

**Overall confidence:** HIGH

### Gaps to Address

- **KaTeX + Starlight compatibility:** Research mentions "known Starlight compatibility issues with rehype-katex." May need MathJax fallback. Validate during Phase 2 scaffold by rendering test formulas.
- **Actual Wasm binary size:** Estimates range 1-5 MB. Won't know until the first Emscripten build completes. If over budget, `-Os`, `--closure 1`, and module splitting are available mitigations.
- **xterm.js without xterm-pty:** The MVP uses Embind (not xterm-pty), but xterm.js is still recommended for the terminal UI. Need to validate that xterm.js works well as a "dumb terminal" (capturing line input, displaying output) without the full PTY layer.
- **Emscripten C++20 support breadth:** Confirmed working for `-std=c++20` with basic STL, but the codebase uses advanced features (structured bindings, `if constexpr`, fold expressions). Validate with actual compilation.

## Sources

### Primary (HIGH confidence)
- [Astro Starlight docs](https://starlight.astro.build/) — project structure, sidebar config, custom pages, component overrides
- [Emscripten Embind](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html) — C++/JS bridge API
- [Emscripten Exceptions](https://emscripten.org/docs/porting/exceptions.html) — `-fexceptions` vs `-fwasm-exceptions` analysis
- [Emscripten Asyncify](https://emscripten.org/docs/porting/asyncify.html) — size overhead, blocking I/O in Wasm
- [xterm.js](https://xtermjs.org/) — terminal emulation, themes, addon system
- [xterm-pty](https://github.com/mame/xterm-pty) — PTY bridge for Emscripten programs (Vim demo proves feasibility)
- [Cloudflare Pages](https://developers.cloudflare.com/pages/) — hosting limits, `_headers` configuration, MIME types

### Secondary (MEDIUM confidence)
- [coi-serviceworker](https://github.com/gzuidhof/coi-serviceworker) — SharedArrayBuffer on static hosts (needed only if upgrading to PROXY_TO_PTHREAD)
- [web.dev COOP/COEP](https://web.dev/articles/coop-coep) — cross-origin isolation requirements
- [Emscripten Memory Settings](https://emscripten.org/docs/tools_reference/settings_reference.html) — `ALLOW_MEMORY_GROWTH` performance characteristics
- [mymindstorm/setup-emsdk](https://github.com/mymindstorm/setup-emsdk) — GitHub Action for Emscripten in CI

### Tertiary (LOW confidence)
- [ALLOW_MEMORY_GROWTH performance](https://emscripten-discuss.narkive.com/t2ZA8bo5) — 2x slowdown claim from community benchmarks (needs validation with our specific workload)
- [GitHub Pages WASM MIME issue](https://stackoverflow.com/questions/79381719) — confirmed broken, but GitHub may fix this

---
*Research completed: 2026-02-27*
*Ready for roadmap: yes*
