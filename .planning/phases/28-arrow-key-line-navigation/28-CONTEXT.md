# Phase 28: Arrow-key line navigation — Context

**Gathered:** 2026-02-26  
**Status:** Ready for planning

<domain>
## Phase Boundary

User can move the cursor left and right within the input line using arrow keys (TTY only). Typing, backspace, and Tab completion continue to work at the cursor position. Script mode (non-TTY) is unchanged. History (if/when up-down is added) should work together with line editing.
</domain>

<decisions>
## Implementation Decisions

### Scope (from ROADMAP)
- **Left/right arrow keys** — Move cursor within current line
- **Cursor-aware editing** — Typing and backspace operate at cursor position (not just end-of-line)
- **TTY check** — Arrow handling only when stdin is TTY
- **Script mode** — When not TTY, use plain getline; no arrow handling
- **Tab completion** — Must remain functional; completion works at cursor

### Platform
- **Cygwin/Unix** — Arrow keys send escape sequences (e.g. ESC [ D / C for left/right)
- **Windows** — May send different sequences; `readLineRaw` already has `#ifdef` for termios vs SetConsoleMode
- Use `readOneChar()` to detect escape sequences; parse ESC [ X style

### Claude's Discretion
- **Home/End keys** — Include if trivial (same escape-sequence family); otherwise defer
- **Delete key (forward delete)** — ROADMAP says "editing works at cursor"; include if straightforward
- **History up/down** — ROADMAP success criterion #4 says "history navigation and line editing remain functional together." Current code has no up/down history recall. Planner decides: add up/down in same phase or note as future enhancement
</decisions>

<specifics>
## Specific Ideas

- `readLineRaw` in repl.h already has raw mode, `readOneChar`, Tab completion, backspace. Extend with: cursor position (`size_t pos`), left/right arrow handling (detect ESC [ D / ESC [ C etc.), redraw line when cursor moves.
- Backspace currently does `line.pop_back()` — must change to delete at `pos-1` when pos > 0
</specifics>

<deferred>
## Deferred Ideas

- Full readline replacement (external library) — zero-dependency constraint
- Vi/Emacs key bindings — out of scope
- Multi-line editing within a single input — backslash continuation handles that separately
</deferred>
