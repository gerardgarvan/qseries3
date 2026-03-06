# Technology Stack

**Analysis Date:** 2025-03-04

## Languages

**Primary:**
- C++20 — All core logic in `src/`. Single main compilation unit.

**Secondary:**
- Bash — Test scripts in `tests/` (acceptance-*.sh, maple-checklist.sh, run-all.sh)
- JavaScript/HTML — Web demo (`index.html`, `tests/test_wasm.mjs`) for WASM build

## Runtime

**Environment:**
- Native binary (Linux, macOS, Windows via Cygwin/MinGW)
- Emscripten — Optional WASM build for browser (`build/wasm/qseries.js`)

**Package Manager:**
- None — Zero external dependencies. No package.json, Cargo.toml, or requirements.txt.

**Lockfile:**
- Not applicable

## Frameworks

**Core:**
- None — Pure C++20 standard library (std::map, std::vector, std::string, etc.)

**Testing:**
- Bash scripts — Pipe REPL commands, grep output for expected patterns
- Embedded unit tests — `main.cpp` includes `runUnitTests()` invoked via `--test`

**Build/Dev:**
- Make — `Makefile` for build, test targets, WASM
- g++ — Primary compiler (gcc:14-bookworm in Docker)
- em++ (Emscripten) — WASM build (optional, requires emsdk)

## Key Dependencies

**Critical:**
- None — BigInt, Frac, Series, qfuncs, convert, linalg, relations all implemented from scratch in headers

**Infrastructure:**
- POSIX (termios, unistd) — Line editing, raw mode on Unix/Cygwin
- Windows API — Raw console mode on Windows (`windows.h`, `io.h`)
- Emscripten bindings — `emscripten/bind.h` for `main_wasm.cpp` only

## Configuration

**Environment:**
- `QSERIES_INSTALL_DIR` — Optional install target for `install.sh` (defaults to `$HOME/.local/bin`)
- No `.env` or config files required for runtime

**Build:**
- `Makefile` — CXX, CXXFLAGS, LDFLAGS
- `CXX ?= g++`, `CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -Wpedantic`
- `LDFLAGS ?=` (empty; use `make LDFLAGS=-static` for static linking)
- Docker: `Dockerfile` uses gcc:14-bookworm, copies `src/` only

## Platform Requirements

**Development:**
- g++ with C++20 support (gcc 10+ recommended, gcc 14 in CI)
- Bash (for tests)
- Optional: emsdk for WASM build

**Production:**
- Single static binary (~2MB target per SPEC)
- Deploy targets: Linux amd64, Darwin arm64 (per `.github/workflows/release.yml`)

---

*Stack analysis: 2025-03-04*
