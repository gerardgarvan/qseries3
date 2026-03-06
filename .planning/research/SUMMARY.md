# Project Research Summary

**Project:** qseries3
**Domain:** REPL UX for Maple users — ergonomics, error diagnostics, help, input convenience
**Researched:** 2026-03-06
**Confidence:** HIGH

## Executive Summary

The qseries3 REPL targets mathematicians transitioning from Maple. Research shows that Maple users expect familiar patterns: `help(topic)` with CALLING SEQUENCE and EXAMPLES, errors that name the function and point to the problem (column + caret), and input convenience (tab completion, history, multi-line). **No stack additions are required** — all enhancements extend the existing in-house infrastructure (`repl.h`, `parser.h`, `ansi::`). Do not add readline, ncurses, or any external line-edit libraries; zero-dependency constraint is non-negotiable.

**Recommended approach:** Implement improvements in this order: (1) error diagnostics with source snippet + caret; (2) help extensions (examples, Maple-style structure); (3) input convenience (argument hints in tab completion, typo suggestions for variables); (4) ergonomics polish. Critical risks: raw terminal mode leaving the shell broken on Ctrl+C (requires SIGINT handler), and script vs interactive path divergence (all TTY features must guard on `stdin_is_tty()`).

---

## Key Findings

### Recommended Stack

**No additions.** The current stack — termios/SetConsoleMode, custom `readLineRaw`, ANSI escape sequences, `getHelpTable`, `offsetToLineCol` — suffices for all planned REPL UX work. Extend `ansi::` with cyan/green/underline for diagnostics; extend `getHelpTable` with examples; extend `handleTabCompletion` for argument hints; extend continuation logic for bracket-aware multi-line.

**Core technologies (unchanged):**
- Raw terminal (termios / SetConsoleMode) — line editing, arrows, tab
- Custom readLineRaw — no readline; zero deps
- ANSI escapes — red, gold, dim; add cyan, green, underline for diagnostics
- Parser offsetToLineCol — enables source snippet + caret for parse errors

### Expected Features

**Must have (table stakes) — already present:**
- `help`, `help(func)` — signature + description ✓
- Tab completion (identifiers + built-ins) ✓
- History (up/down), multi-line (backslash), semicolon suppress ✓
- Runtime errors with function name ✓

**Should have for Maple parity (priority gaps):**
- **P1:** Parse errors with source snippet + caret — GCC/Clang style; high value, medium effort
- **P1:** Typo suggestions for undefined variable — "Did you mean: y?"; low effort, reuses levenshtein
- **P2:** Per-function help with 1–2 examples — e.g. Rogers-Ramanujan for `help(prodmake)`
- **P2:** Richer help structure — SYNOPSIS, SEE ALSO

**Defer (v2+):**
- `??func` (calling sequence only), `???func` (examples only)
- Help categories / index
- Delimiter auto-close, 2-D math input

### Architecture Approach

REPL UX changes integrate into the existing read→parse→eval→display pipeline. Error handling funnels through a single catch in `runRepl`; help and completion share `getHelpTable`. New work is all modifications: `formatParseError(input, offset, msg)` in the catch block, extended help table values, and completion logic. No new top-level components.

**Major components (modified, not new):**
1. **runRepl catch** — format parse errors with source line + caret; keep script "line N:" prefix
2. **getHelpTable** — add optional examples field; keep single source of truth for help + completion
3. **handleTabCompletion** — add function signature hints when completing built-ins
4. **readLineRaw / continuation loop** — bracket-aware multi-line; optional Ctrl+R history search

### Critical Pitfalls

1. **Raw terminal leaves shell broken on Ctrl+C** — Register SIGINT handler that restores termios before re-raising. RAII alone fails on crash/kill. Phase 28 or any raw-mode phase must include this.

2. **Script vs interactive path divergence** — Every TTY feature must guard on `stdin_is_tty()`. Tab, history, banner, timing: all can break or hang when piped. Add CI tests: `qseries < script.qs` must exit 0.

3. **Error format breaks Maple expectations** — Use `(in funcname) message`; use "expects its N-th argument, name, to be X, but received Y" for argument errors. Phase 21 must address before other UX polish.

4. **Tab completion and history fight over buffer** — Use single `(line, pos)` state. Tab must be cursor-position aware; history restores full line, `pos = line.size()`. Test: Tab at middle of word, Up/Down then Tab.

5. **Multi-line EOF hang in script mode** — Trailing backslash on last line must not block. Cap continuations; treat EOF as end of input. Test: `printf 'x := 1\\' | qseries` must exit.

---

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: Error Diagnostics
**Rationale:** Errors are cross-cutting; improving them first helps debug all later changes.
**Delivers:** `formatParseError(input, offset, msg)` with source line + caret; Maple-style runtime messages via `runtimeErr`.
**Addresses:** Parse errors with column + snippet (P1), error format (PITFALLS #3)
**Avoids:** Parser throws without location; generic messages

### Phase 2: Help Extensions
**Rationale:** getHelpTable is shared by help + tab completion; extend once.
**Delivers:** Per-function examples (1–2 per key built-in); optional richer structure (SYNOPSIS, SEE ALSO).
**Addresses:** Per-function help with examples (P2), Maple CALLING SEQUENCE + EXAMPLES parity
**Uses:** Existing getHelpTable; qseriesdoc.md / Garvan examples as content source

### Phase 3: Input Convenience
**Rationale:** Depends on getHelpTable for completion hints; no parser changes.
**Delivers:** Argument hints in tab completion (signature from getHelpTable); typo suggestions for undefined variable.
**Addresses:** Typo suggestions for variables (P1), Maple discoverability
**Avoids:** Tab + history buffer conflict (single line/pos)

### Phase 4: Ergonomics Polish
**Rationale:** Cosmetic; depends on nothing.
**Delivers:** Bracket-aware multi-line continuation; optional Ctrl+R history search; continuation prompt consistency.
**Addresses:** Maple multi-line expectations; input convenience
**Avoids:** Multi-line EOF hang (script-mode guards)

### Phase Ordering Rationale

- **Error diagnostics first** — PITFALLS recommends Phase 21 early; every subsequent phase benefits from better failure reporting.
- **Help before input** — Tab completion argument hints consume `getHelpTable`; extend help content before completion logic.
- **Ergonomics last** — Mostly cosmetic; no architectural dependencies.
- **Raw-mode phases** — Must include SIGINT handler in the same phase that introduces/enhances raw mode.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 1 (Error diagnostics):** Verify parser exception payload (offset vs line/col) for `formatParseError`; confirm script-mode line-number handling.
- **Phase 4 (Multi-line):** Bracket-balance rules for edge cases (strings, nested); max continuation cap.

Phases with standard patterns (skip research-phase):
- **Phase 2 (Help):** getHelpTable extension is straightforward; content curation from qseriesdoc.md.
- **Phase 3 (Input):** Typo suggestions reuse existing levenshtein + built-in set; completion hints are data-driven.

---

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | Explicit "no additions"; STACK.md verified against codebase |
| Features | HIGH | Maple docs and prodmake.html verified; feature matrix from FEATURES.md |
| Architecture | HIGH | ARCHITECTURE.md maps components; integration points clear |
| Pitfalls | HIGH | Maple Error Message Guide, Python termios post-mortems, REPL community reports |

**Overall confidence:** HIGH

### Gaps to Address

- **`?topic` syntax:** Research recommends keeping `help(topic)`; adding `?topic` would require parser changes. Document clearly for Maple users.
- **NO_COLOR handling:** ANSI extension (cyan, green, underline) must respect existing `ansi::init()` NO_COLOR check.
- **Emscripten:** REPL UX enhancements target TTY; WASM path uses `evaluate()` and captures cout/cerr. Ensure error format is useful in both.

---

## Sources

### Primary (HIGH confidence)
- Maple Help, Error Message Guide Overview — maplesoft.com
- qseries prodmake.html — qseries.org/fgarvan/qmaple/qseries/functions/prodmake.html
- src/repl.h, src/parser.h — RawModeGuard, readLineRaw, getHelpTable, offsetToLineCol
- .planning/research/STACK.md, FEATURES.md, ARCHITECTURE.md, PITFALLS.md

### Secondary (MEDIUM confidence)
- Python termios Ctrl+C / raw mode — cpython#128330
- OhMyREPL error display patterns — color, reverse stack
- Phase research files — 20-tab-completion, 21-error-messages, 31-up-down-arrows

---
*Research completed: 2026-03-06*
*Ready for roadmap: yes*
