# Phase 43: Wasm compile - Context

**Gathered:** 2026-02-27
**Status:** Ready for planning

<domain>
## Phase Boundary

Compile the existing C++20 REPL to WebAssembly via Emscripten so JavaScript can call `evaluate(expr)` and get string results back. This phase delivers a working .wasm binary with an API — not the website or playground UI.

</domain>

<decisions>
## Implementation Decisions

### API Surface
- **State:** Claude's Discretion — stateful (variables persist) vs stateless is implementation choice
- **Return format:** Plain string — same text the CLI would print to stdout
- **Truncation default:** T=50 for Wasm session (safe for browser, user can change via set_trunc)
- **Multi-line input:** Claude's Discretion — single vs multi-statement per call

### Error Reporting
- **Error format:** Prefixed string — errors start with "error:" (e.g., "error: division by zero")
- **Crash recovery:** Terminate — Web Worker dies, playground shows "session crashed, click to restart"
- **Timeout:** Claude's Discretion — may or may not implement built-in timeout
- **Wasm traps:** Add guards — check for potential traps before they happen (extra validation in C++)

### I/O Bridge
- **Entry point:** Claude's Discretion — separate main_wasm.cpp or #ifdef guards in main.cpp
- **REPL reuse:** Full REPL — redirect stdin/stdout to Wasm-compatible I/O (reuse as much of repl.h as possible)
- **Display format:** Exact same output as CLI — users see identical output in browser as in terminal
- **Banner:** Show "qseries v2.0" banner on first load

### Build Setup
- **Build trigger:** Claude's Discretion — Makefile target or separate script
- **Emscripten install:** Claude's Discretion — system-wide or project-local
- **Output location:** Claude's Discretion — website/public/wasm/ or build/wasm/
- **Testing:** Both Node.js test AND test HTML page for verification

### Claude's Discretion
- Whether evaluate() maintains stateful sessions (variables persisting between calls)
- Single vs multi-statement input handling
- Timeout implementation (if any)
- Entry point approach (separate file vs #ifdef)
- Build system integration (Makefile target vs script)
- Emscripten installation approach
- Wasm output file location

</decisions>

<specifics>
## Specific Ideas

- User wants the exact same output format as CLI — identical experience
- Banner should show on first load ("qseries v2.0")
- Default T=50 (lower than CLI's T=100) to keep browser computations reasonable
- Errors should be prefixed with "error:" for easy parsing by playground JS
- On unrecoverable crash, session terminates (Web Worker dies) — no attempt at recovery

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 43-wasm-compile*
*Context gathered: 2026-02-27*
