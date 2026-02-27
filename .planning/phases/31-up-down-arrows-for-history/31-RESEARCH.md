# Phase 31: Up-Down Arrows for History - Research

**Researched:** 2026-02-26
**Domain:** Terminal raw input / REPL history navigation
**Confidence:** HIGH

## Summary

This phase adds up/down arrow key support for navigating command history in the REPL's `readLineRaw()` function. The infrastructure is almost entirely in place: history is already stored as `std::deque<std::string>` in `runRepl()`, left/right arrow escape sequences are already parsed in `readLineRaw`, and `redrawLineRaw(line, pos)` already handles redrawing the prompt with cursor positioning. The only missing pieces are (a) passing history into `readLineRaw`, (b) handling ESC `[` A (up, code 65) and ESC `[` B (down, code 66), and (c) maintaining a history navigation index with a saved "current line" buffer.

This is a well-understood pattern implemented identically across readline, linenoise, and every custom REPL. The escape codes are the same ANSI sequences already being parsed. No new dependencies, no new terminal capabilities, no architectural changes — just extending the existing escape handler with two more cases and threading history through the function signature.

**Primary recommendation:** Modify `readLineRaw` to accept `const std::deque<std::string>& history`, add a local `historyIndex` and `savedLine`, handle ESC `[` 65 (up) and ESC `[` 66 (down) by swapping the line buffer and redrawing.

## Standard Stack

### Core

No new libraries. This phase uses only what's already in the codebase:

| Component | Location | Purpose | Already Exists |
|-----------|----------|---------|----------------|
| `std::deque<std::string>` | `runRepl()` line 1032 | History storage (oldest-first, max 100) | YES |
| `readLineRaw` | `repl.h` line 916 | Raw char-by-char input with escape handling | YES |
| `redrawLineRaw` | `repl.h` line 864 | Redraws prompt + line, positions cursor | YES |
| `RawModeGuard` | `repl.h` lines 48-99 | RAII raw terminal mode (termios/Win32) | YES |
| `readOneChar` | `repl.h` lines 66-98 | Single byte read from stdin | YES |

### Supporting

None needed.

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Passing `const deque&` by reference | Making history a member of Environment | Over-engineers; history is REPL-specific, not part of the math environment |
| Index-based navigation | Copying history into a vector | Unnecessary copy; deque supports random access via `operator[]` already |

## Architecture Patterns

### Pattern 1: History Index with Saved Line

**What:** When the user presses up for the first time, save the current in-progress line (the "working buffer"). Navigate through history using an index. When navigating back past the newest entry, restore the saved line.

**When to use:** Always — this is the universally expected behavior for REPL history.

**Key state variables (local to `readLineRaw`):**

```cpp
size_t histIdx = history.size();  // "past the end" = not navigating
std::string savedLine;            // stash current input when entering history
```

**Navigation logic:**

```cpp
// Up arrow (ESC [ A = 65)
if (c3 == 65) {
    if (!history.empty() && histIdx > 0) {
        if (histIdx == history.size()) savedLine = line;  // save current input
        --histIdx;
        line = history[histIdx];
        pos = line.size();
        redrawLineRaw(line, pos);
    }
}

// Down arrow (ESC [ B = 66)
if (c3 == 66) {
    if (histIdx < history.size()) {
        ++histIdx;
        if (histIdx == history.size())
            line = savedLine;  // restore saved input
        else
            line = history[histIdx];
        pos = line.size();
        redrawLineRaw(line, pos);
    }
}
```

### Pattern 2: Function Signature Change

**What:** Add `const std::deque<std::string>& history` parameter to `readLineRaw`.

**Current signature:** `inline std::optional<std::string> readLineRaw(Environment& env)`
**New signature:** `inline std::optional<std::string> readLineRaw(Environment& env, const std::deque<std::string>& history)`

**Call sites to update (all in `runRepl()`):**
1. Line 1042: `auto opt = readLineRaw(env);` → `auto opt = readLineRaw(env, history);`
2. Line 1064: `auto nextOpt = readLineRaw(env);` → `auto nextOpt = readLineRaw(env, history);`

### Anti-Patterns to Avoid

- **Modifying history from within readLineRaw:** History is append-only and managed by `runRepl()`. `readLineRaw` should only read it. Use `const&`.
- **Resetting histIdx on every keystroke:** Only reset when the user submits (Enter). During editing of a recalled line, the index should stay put so up/down continue working relative to current position.
- **Not saving the working line:** If user types partial input, presses up, then down, they expect their partial input back. Forgetting `savedLine` is the #1 beginner mistake.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| ANSI escape codes | Custom escape parser | Existing ESC `[` X pattern in readLineRaw | Already handles left/right; just add two more cases |
| Line redraw | Manual cursor movement | `redrawLineRaw(line, pos)` | Already handles \r, clear-to-EOL, cursor reposition |

**Key insight:** Everything needed already exists. This is purely additive — two new `else if` branches inside the existing ESC `[` handler.

## Common Pitfalls

### Pitfall 1: Off-by-One in History Index

**What goes wrong:** Using 0-based index where history.size() means "at the newest entry" instead of "past all entries" (or vice versa).
**Why it happens:** Confusion between "pointing at current history entry" vs "past the end = editing new line".
**How to avoid:** Convention: `histIdx == history.size()` means "not in history, editing current line". `histIdx == 0` means "at oldest entry". Always check bounds before decrementing/incrementing.
**Warning signs:** Pressing up when there's one history entry does nothing, or pressing down at newest entry clears the line unexpectedly.

### Pitfall 2: Not Saving Current Line Before First Up

**What goes wrong:** User types "foo", presses up (gets previous command), presses down — "foo" is gone, line is empty.
**Why it happens:** Forgot to stash the current line before overwriting it with history.
**How to avoid:** `if (histIdx == history.size()) savedLine = line;` before the first decrement.
**Warning signs:** Partial input lost when navigating history and returning.

### Pitfall 3: Cursor Position After History Recall

**What goes wrong:** Cursor ends up at position 0 instead of end-of-line after recalling a history entry.
**Why it happens:** Forgot to set `pos = line.size()` after replacing line content.
**How to avoid:** Always set `pos = line.size()` after `line = history[histIdx]` or `line = savedLine`.

### Pitfall 4: History Navigation During Continuation Lines

**What goes wrong:** Pressing up during a backslash-continuation prompt (`  > `) navigates history, corrupting multi-line input.
**Why it happens:** Continuation also calls `readLineRaw`, which now accepts history.
**How to avoid:** This is actually fine — continuation lines are independent input events. History will show previous full commands. The continuation call at line 1064 can pass history or an empty deque. Passing history is acceptable since each continuation line is a separate readLineRaw invocation.
**Recommendation:** Pass history to continuation lines too. It's useful and matches shell behavior (bash allows up/down during continuation).

### Pitfall 5: Empty History

**What goes wrong:** Up arrow when history is empty causes underflow or crash.
**Why it happens:** Decrementing `histIdx` when it's 0 and history is empty.
**How to avoid:** Guard: `if (!history.empty() && histIdx > 0)`. When history is empty, `histIdx` starts at 0 (= `history.size()`), and the guard prevents any action.

## Code Examples

### Complete readLineRaw Escape Handler (After Change)

```cpp
if (c == 27) {  // ESC
    int c2 = readOneChar();
    if (c2 == '[') {
        int c3 = readOneChar();
        if (c3 == 65) {  // up arrow
            if (!history.empty() && histIdx > 0) {
                if (histIdx == history.size()) savedLine = line;
                --histIdx;
                line = history[histIdx];
                pos = line.size();
                redrawLineRaw(line, pos);
            }
        } else if (c3 == 66) {  // down arrow
            if (histIdx < history.size()) {
                ++histIdx;
                line = (histIdx == history.size()) ? savedLine : history[histIdx];
                pos = line.size();
                redrawLineRaw(line, pos);
            }
        } else if (c3 == 68) {  // left
            if (pos > 0) { --pos; redrawLineRaw(line, pos); }
        } else if (c3 == 67) {  // right
            if (pos < line.size()) { ++pos; redrawLineRaw(line, pos); }
        }
    }
    continue;
}
```

### Acceptance Test Pattern (Following Phase 28 Conventions)

```bash
# Up = ESC [ A, Down = ESC [ B
UP=$'\033[A'
DOWN=$'\033[B'

# Test: type "1+1", enter, then up arrow + enter → should re-evaluate "1+1"
out=$(printf '1+1\n%s\n\n' "$UP" | run_script script -q -c "$BIN" /dev/null 2>/dev/null)
# Output should contain "2" twice (once for original, once for recalled)
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| No history navigation | Up/down arrow navigation | This phase | Users can recall and re-execute previous commands |

**Note:** The project deliberately avoids readline/linenoise as external dependencies. Hand-rolling this specific feature is justified because (a) the escape parsing infrastructure already exists, and (b) the full readline feature set is not needed — just basic history recall.

## Open Questions

1. **Should typing into a recalled history entry reset the history index?**
   - What we know: Bash/zsh do NOT reset — you can edit a recalled line and still press up/down to navigate further.
   - Recommendation: Don't reset on typing. Keep histIdx stable. Only reset to `history.size()` on Enter (which happens naturally since readLineRaw returns and a new call starts fresh).

2. **Should continuation lines support history?**
   - What we know: Passing `history` to continuation readLineRaw calls is harmless and matches shell behavior.
   - Recommendation: Pass history to continuation calls. Low risk, potentially useful.

## Sources

### Primary (HIGH confidence)
- `src/repl.h` — Direct code inspection of readLineRaw, runRepl, history deque, redrawLineRaw, ESC sequence handling
- Phase 28 summary — Documents ESC `[` D/C pattern, redrawLineRaw design, acceptance test approach with `script`

### Secondary (MEDIUM confidence)
- ANSI escape code standard — ESC `[` A (up), ESC `[` B (down) are universal VT100/xterm codes. Same codes used on Cygwin, Linux, macOS, Windows Terminal.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — no new dependencies, all infrastructure exists
- Architecture: HIGH — pattern is universally understood, code changes are minimal and additive
- Pitfalls: HIGH — all pitfalls identified from direct code analysis and readline implementation experience

**Research date:** 2026-02-26
**Valid until:** Indefinite — ANSI escape codes and history navigation patterns are stable
