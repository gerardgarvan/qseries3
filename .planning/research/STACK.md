# Technology Stack: REPL UX Improvements

**Project:** qseries REPL — REPL ergonomics, error diagnostics, help/docs, tab completion, multi-line, history  
**Researched:** 2026-03-06  
**Scope:** Stack additions or changes for NEW REPL UX capabilities. Zero-external-dependency constraint enforced.  
**Target users:** Mathematicians accustomed to Maple.

---

## Executive Summary

The qseries3 REPL already implements a **zero-dependency** line editor: raw termios (Unix/Cygwin) or Windows `SetConsoleMode`, custom `readLineRaw`, ANSI escape sequences, tab completion, history, help, multi-line backslash continuation, and colored error output. For **new** REPL UX improvements (better diagnostics, richer help, enhanced completion, improved ergonomics), **no stack additions are required**. Extend the existing in-house infrastructure. Do **not** add readline, libedit, ncurses, linenoise, or replxx — they violate the zero-deps constraint.

**Recommendation:** Stay with the current stack. Add no new libraries. Implement all REPL UX enhancements by extending `repl.h`, `parser.h`, and the `ansi::` namespace.

---

## Current Stack (What Exists)

| Component | Technology | Location | Notes |
|-----------|------------|----------|-------|
| **Raw terminal mode** | termios (Unix/Cygwin) or Windows `SetConsoleMode` | `repl.h` RawModeGuard | POSIX / Win32 API only |
| **Char-by-char input** | `read()`, `ReadFile()` | `readLineRaw`, `readOneChar` | No readline |
| **Line editor** | Custom in `readLineRaw` | `repl.h` ~2650–2715 | Arrow keys, backspace, tab |
| **History** | `std::deque<std::string>` | `runRepl`, `loadHistory`, `saveHistory` | Up/down navigation |
| **Tab completion** | `handleTabCompletion`, `getCompletionCandidates` | `repl.h` ~2592–2650 | Built-ins + env vars |
| **ANSI styling** | Hardcoded `\033[...m` | `ansi::` namespace | gold, red, dim, bold, reset |
| **Help** | `getHelpTable()`, `help`, `help(func)` | `repl.h` ~2112–2136 | Per-function sig + desc |
| **Multi-line** | Backslash continuation | `runRepl` loop | `  > ` secondary prompt |
| **Error output** | `ansi::red()`, `e.what()` | catch block ~2941–2946 | Prefix "error: " |
| **Parser diagnostics** | `offsetToLineCol`, `kindToExpected` | `parser.h` | line/col in messages |

**Standard library:** `string`, `vector`, `map`, `set`, `deque`, `optional`, `chrono`, `algorithm`, `sstream`, `fstream`.  
**Platform headers:** `<termios.h>`, `<unistd.h>` (Unix/Cygwin); `<windows.h>` (native Win32).  
**Build:** `g++ -std=c++20 -O2` with optional `-static`. Single TU, no `-lreadline` or other libs.

---

## Stack Additions for New REPL UX Features

### Verdict: **None**

All planned REPL UX improvements can be implemented using the existing stack. No new libraries, no new dependencies.

| New Capability | Stack Implication | Implementation Path |
|----------------|-------------------|---------------------|
| **Better error diagnostics** | Extend `ansi::`, use `offsetToLineCol` | Add `ansi::cyan()`, caret/underline; format "line X, col Y" with source line and pointer |
| **Richer help/docs** | Extend `getHelpTable` | Add examples, usage strings; keep `std::map<std::string, std::pair<std::string, std::string>>` or extend value type |
| **Argument hints in completion** | Extend `handleTabCompletion` | Add optional sig suffix when completing functions; use `getHelpTable()` |
| **Improved tab completion** | Same candidates + UX tweaks | Substring/prefix completion, grouping; all via `std::set`, `std::vector` |
| **Syntax highlighting** | Extend `redrawLineRaw` or post-echo | Inline ANSI escapes; no lexer library — use parser/tokenizer or simple heuristics |
| **Bracket-aware multi-line** | Extend continuation logic | Count `(`, `[`, `{` vs `)`, `]`, `}`; optional `  > ` until balanced |
| **Inline hints (e.g. arg count)** | Extend `readLineRaw` / display | Echo hint string after tab; use existing ANSI |
| **History search (Ctrl+R)** | Extend `readLineRaw` escape handler | Add case for Ctrl+R; iterate `history`; no lib needed |

---

## What NOT to Add (Zero-Deps)

| Library | Why Not | Use Instead |
|---------|---------|-------------|
| **readline** | External dependency; GPL/libreadline licensing | Existing `readLineRaw` |
| **libedit** | External dependency | Existing `readLineRaw` |
| **ncurses** | External dependency; overkill for line input | termios + ANSI |
| **linenoise** | External dependency (even if small) | Existing raw mode + completion |
| **linenoise-ng** | Same | Same |
| **cpp-linenoise** | Header-only but still external | Extend in-house |
| **replxx** | External; UTF-8/highlighting nice but not zero-deps | Extend in-house |
| **bestline** | External | Extend in-house |

**Rationale:** SPEC and `.cursorrules` mandate "ZERO external dependencies." Vendoring a single-file header (e.g. cpp-linenoise) would add external code and maintenance burden. The existing custom readline already provides: raw mode, history, tab completion, arrow keys. Extending it is simpler than integrating and maintaining a forked/vendored library.

---

## Integration Points for UX Improvements

| Area | File | Function/Block | Extend How |
|------|------|----------------|------------|
| **Error display** | `repl.h` | catch block ~2941 | Add `formatDiagnostic(input, offset, msg)`; print source line + caret |
| **ANSI colors** | `repl.h` | `ansi::` namespace | Add `cyan()`, `yellow()`, `green()` for diagnostics/hints |
| **Parser errors** | `parser.h` | `offsetToLineCol`, throws | Add optional "show source line" helper; keep same exception flow |
| **Tab completion** | `repl.h` | `handleTabCompletion` | Add sig hint from `getHelpTable()`; optionally group by type |
| **Help** | `repl.h` | `getHelpTable`, `help` | Extend value to `(sig, desc, examples)`; add `help(topic, subtopic)` if needed |
| **Multi-line** | `repl.h` | Continuation loop ~2855 | Add bracket balance check; auto-continue when unbalanced |
| **History** | `repl.h` | `readLineRaw` | Add Ctrl+R handling; reuse `history` deque |
| **Line redraw** | `repl.h` | `redrawLineRaw` | Add optional syntax-highlight pass before output |

---

## ANSI Escape Sequences (Extend In-House)

The project already uses a minimal set. For richer diagnostics and hints:

| Sequence | Code | Use |
|----------|------|-----|
| Red | `\033[31m` | Errors (existing) |
| Yellow/Gold | `\033[33m` | Prompt (existing) |
| Dim | `\033[2m` | Timing (existing) |
| Bold | `\033[1m` | (existing) |
| Cyan | `\033[36m` | **Add** — file/line references, diagnostics |
| Green | `\033[32m` | **Add** — success, hints |
| Underline | `\033[4m` | **Add** — error underline/caret |

All are standard ECMA-48 / ISO 6429. No library needed. Respect `NO_COLOR` (existing check in `ansi::init()`).

---

## Maple UX Parity (Target Users)

Mathematicians used to Maple expect:

| Maple Feature | qseries Status | Stack Impact |
|---------------|----------------|--------------|
| `?topic` or `help(topic)` | `help`, `help(func)` | Done; extend with examples |
| `??topic` (calling sequence only) | — | Add optional `help(func, "sig")`; no new deps |
| Multi-line input | Backslash continuation | Done; add bracket-aware continuation |
| Command history | Up/down arrows | Done |
| Tab completion | Identifiers + built-ins | Done; add argument hints |
| Colored errors | Red "error:" prefix | Done; add source-line + caret |
| `readline(terminal)` in scripts | N/A for qseries | — |

No additional libraries needed to match these patterns.

---

## Alternatives Considered

| Instead of | Could Use | Verdict |
|------------|-----------|---------|
| Extending in-house readLineRaw | Vendoring cpp-linenoise | **No** — adds external code; existing impl sufficient |
| Hardcoded ANSI | curses/ncurses | **No** — external dep; ANSI is enough |
| Plain text errors | Structured diagnostics (JSON, LSP) | **Defer** — overkill for REPL; simple formatted text suffices |
| In-memory help | External doc files | **Optional** — `getHelpTable` is fine; could load `.md` later without new libs |

---

## Recommended Approach

1. **Keep** termios/SetConsoleMode + custom `readLineRaw` — no readline/libedit.
2. **Extend** `ansi::` with cyan, green, underline for diagnostics.
3. **Add** `formatParseError(input, offset, msg)` in `repl.h` to print source line + caret.
4. **Extend** `getHelpTable` value type if examples are added.
5. **Extend** `handleTabCompletion` to show function signatures as hints.
6. **Add** bracket-balance check to continuation loop for smarter multi-line.
7. **Optionally** add Ctrl+R history search in `readLineRaw`.

All of the above use only C++20 standard library and existing platform APIs.

---

## Phase-Specific Stack Notes

| Phase Topic | Stack Impact | Action |
|-------------|--------------|--------|
| Error diagnostics | None | Extend `ansi::`, add format helper |
| Help/docs | None | Extend `getHelpTable` structure |
| Tab completion | None | Extend `handleTabCompletion` |
| Multi-line | None | Extend continuation logic |
| History search | None | Extend `readLineRaw` escape handler |
| Syntax highlighting | None | Optional; inline ANSI in `redrawLineRaw` |

---

## Sources

- SPEC.md — Zero external libraries, single static binary
- .cursorrules — ZERO external dependencies
- src/repl.h — RawModeGuard, readLineRaw, handleTabCompletion, ansi::, getHelpTable
- src/parser.h — offsetToLineCol, kindToExpected
- .planning/phases/20-tab-completion/20-RESEARCH.md — Raw termios, zero readline
- .planning/phases/31-up-down-arrows-for-history/31-RESEARCH.md — History pattern
- Maple help: ?topic, help(topic) — maplesoft.com/support/help
- ANSI escape codes — ECMA-48, ISO 6429; NO_COLOR convention
