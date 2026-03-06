# Requirements: qseries3 — v11.2 Improve User Experience

**Defined:** 2026-03-06  
**Core Value:** Rogers-Ramanujan must work; prodmake recovers product form with denominators at exponents ≡ ±1 (mod 5)  
**Milestone:** v11.2 — REPL UX for Maple users

## v11.2 Requirements

Requirements for milestone v11.2. Each maps to roadmap phases.

### Error Diagnostics

- [ ] **ERR-01**: Parse errors display source line with caret at error position (GCC/Clang style)
- [ ] **ERR-02**: Parse errors use `offsetToLineCol` and show line:col when available
- [ ] **ERR-03**: Runtime errors follow Maple format: `(in funcname) message`
- [ ] **ERR-04**: Argument errors use "expects its N-th argument, name, to be X, but received Y"
- [ ] **ERR-05**: Script mode prepends line number to error output

### Help Extensions

- [ ] **HELP-01**: Per-function help includes 1–2 examples for key built-ins (e.g. prodmake, etamake)
- [ ] **HELP-02**: Help structure includes SYNOPSIS and optionally SEE ALSO
- [ ] **HELP-03**: Examples sourced from qseriesdoc.md / Garvan tutorial where applicable

### Input Convenience

- [ ] **INPUT-01**: Typo suggestions for undefined variable ("Did you mean: x?")
- [ ] **INPUT-02**: Tab completion shows function signature hint when completing built-ins
- [ ] **INPUT-03**: Single (line, pos) buffer for tab and history (no cursor/history conflict)

### Ergonomics

- [ ] **ERGO-01**: Bracket-aware multi-line continuation (continue when parens/brackets unclosed)
- [ ] **ERGO-02**: Trailing backslash in script mode does not hang (EOF handled)
- [ ] **ERGO-03**: SIGINT handler restores termios before re-raise (raw mode safe on Ctrl+C)
- [ ] **ERGO-04**: All TTY features guarded with `stdin_is_tty()` in script mode

## v2 Requirements (Deferred)

- **HELP-04**: `?topic` shortcut (parser change)
- **HELP-05**: Help search by substring
- **ERGO-05**: Ctrl+R history search
- **ERGO-06**: Syntax highlighting

## Out of Scope

| Feature | Reason |
|---------|--------|
| readline / linenoise / replxx | Zero-dependency constraint; extend in-house readLineRaw |
| ncurses / full-screen UI | REPL is line-based; no full-screen mode |
| LaTeX output | Plain text sufficient; Maple users expect readable ASCII |
| 2-D math input | Complexity; defer to future |
| `??func` / `???func` variants | Maple parity; defer |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| ERR-01 | 108 | Pending |
| ERR-02 | 108 | Pending |
| ERR-03 | 108 | Pending |
| ERR-04 | 108 | Pending |
| ERR-05 | 108 | Pending |
| HELP-01 | 109 | Pending |
| HELP-02 | 109 | Pending |
| HELP-03 | 109 | Pending |
| INPUT-01 | 110 | Pending |
| INPUT-02 | 110 | Pending |
| INPUT-03 | 110 | Pending |
| ERGO-01 | 111 | Pending |
| ERGO-02 | 111 | Pending |
| ERGO-03 | 111 | Pending |
| ERGO-04 | 111 | Pending |

**Coverage:**
- v11.2 requirements: 14 total
- Mapped to phases: 14
- Unmapped: 0 ✓

---
*Requirements defined: 2026-03-06*
*Last updated: 2026-03-06 after v11.2 research synthesis*
