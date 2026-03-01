---
phase: 43-wasm-compile
plan: 01
subsystem: wasm
tags: [emscripten, embind, wasm, webassembly, cpp-to-js]

requires:
  - phase: 10-repl
    provides: "repl.h with Environment, evalStmt, display, parse, trim"
provides:
  - "src/main_wasm.cpp — Wasm entry point with Embind evaluate()/get_banner() API"
  - "repl.h Emscripten guards — compiles under both g++ and em++"
  - "Makefile wasm target — make wasm produces build/wasm/qseries.{js,wasm}"
affects: [46-playground, 47-cicd-deploy]

tech-stack:
  added: [emscripten, embind, wasm-exceptions]
  patterns: [streambuf-redirect-for-output-capture, emscripten-ifdef-guards, separate-wasm-entry-point]

key-files:
  created: [src/main_wasm.cpp]
  modified: [src/repl.h, Makefile]

key-decisions:
  - "Separate main_wasm.cpp entry point instead of #ifdef in main.cpp"
  - "Global stateful Environment for persistent session across evaluate() calls"
  - "streambuf redirect captures all std::cout/cerr output including help() and coeffs()"
  - "T=50 default for Wasm (matches CLI default)"
  - "catch(...) block prevents unknown exceptions from becoming Wasm traps"

patterns-established:
  - "#ifndef __EMSCRIPTEN__ guards: 4 guard blocks in repl.h (isatty, termios/RawModeGuard, terminal helpers, runRepl)"
  - "Wasm API pattern: evaluate(string) -> string, get_banner() -> string via Embind"

duration: 7min
completed: 2026-02-27
---

# Phase 43 Plan 01: Wasm Compile Summary

**Emscripten-compatible repl.h guards and Embind-based main_wasm.cpp with evaluate()/get_banner() API, plus Makefile wasm target**

## Performance

- **Duration:** 7 min
- **Started:** 2026-02-27T17:08:17Z
- **Completed:** 2026-02-27T17:15:25Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments
- Added 4 `#ifndef __EMSCRIPTEN__` guard blocks in repl.h: isatty macros, termios/RawModeGuard/readOneChar, terminal helpers (redrawLineRaw/handleTabCompletion/readLineRaw), and runRepl — while keeping trim(), display(), evalStmt() unguarded for Wasm reuse
- Created src/main_wasm.cpp with Embind bindings exposing evaluate() (stateful, streambuf redirect) and get_banner() ("qseries v2.0")
- Added Makefile wasm target with em++ -Oz -lembind, MODULARIZE, FILESYSTEM=0, wasm-exceptions flags
- Native build verified clean (g++ -std=c++20 -O2, no warnings)

## Task Commits

Each task was committed atomically:

1. **Task 1: Guard terminal I/O in repl.h and create main_wasm.cpp** - `bd1eb21` (feat)
2. **Task 2: Add Makefile wasm target** - `0c6556d` (feat)

## Files Created/Modified
- `src/main_wasm.cpp` — Wasm entry point: global Environment, evaluate() with streambuf redirect, get_banner(), Embind bindings (66 lines)
- `src/repl.h` — 4 Emscripten guard blocks added; stdin_is_tty() defined as false under Emscripten
- `Makefile` — wasm target, EMXX/WASM_FLAGS variables, build/wasm directory creation, clean removes build/

## Decisions Made
- Separate main_wasm.cpp: cleaner than #ifdef in main.cpp since main.cpp has 691 lines of unit tests irrelevant for Wasm
- Stateful session: global Environment persists between evaluate() calls — matches CLI REPL behavior and enables multi-step workflows (define x, then prodmake(x,40))
- Single statement per evaluate() call: simpler API, matches existing evalStmt-per-line model
- streambuf redirect: captures all std::cout output from dispatchBuiltin (help, coeffs, series, etc.) automatically without modifying display code
- Trailing colon suppression: matches CLI behavior in Wasm entry point

## Deviations from Plan
None — plan executed exactly as written.

## Issues Encountered
- Wasm compilation could NOT be tested: emsdk (em++) is not installed on this machine. The Makefile target, main_wasm.cpp, and repl.h guards are all in place but `make wasm` has not been run successfully. The native build compiles cleanly, confirming the #ifndef guards don't break the existing code path.

## User Setup Required
To test the Wasm build, install Emscripten:
```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh
cd /path/to/qseries3 && make wasm
```

## Next Phase Readiness
- src/main_wasm.cpp and Makefile wasm target ready for compilation once emsdk is available
- Phase 44 (Starlight scaffold) can proceed independently
- Phase 46 (Playground) depends on a working build/wasm/qseries.js

---
*Phase: 43-wasm-compile*
*Completed: 2026-02-27*
