# Phase 21: Error Messages - Research

**Researched:** 2026-02-25
**Domain:** Parse/runtime diagnostics, error message design, zero-dependency C++ error handling
**Confidence:** HIGH

## Summary

Phase 21 improves parse and runtime error diagnostics to satisfy QOL-07. The current codebase uses plain `std::runtime_error` throughout: the Tokenizer has no position tracking, the Parser has no position in tokens, and the REPL prints only `error: ` + `e.what()`. No custom exception types exist.

**Parse errors** need position (line, column, or offset), expected token, and actual character/token. The Tokenizer already has `input` and `pos`; adding `line` and `col` (or computing them from offset) is straightforward. Each Token should carry `offset` (or `line, col`) for parser use.

**Runtime errors** need function name and brief context. Builtins in `dispatchBuiltin` already include the function name in the message text (e.g. `"etaq(k,T) or etaq(q,k,T)"`). Enhancing means standardizing format (e.g. `func(args): message`) and ensuring all call sites supply function/context.

**REPL display** should adopt a GCC/Clang-style format: `location: message` or `error: location: message`, with optional source snippet + caret when feasible.

**Backslash continuation**: Lines are concatenated with a space; the final string has no newlines. So "line" in the parsed input is always 1. For script mode, the REPL can track which logical input (command) triggered the error and report that as "line N" in the script.

**Primary recommendation:** Add position tracking to Tokenizer and Token; introduce a lightweight `ParseError` that carries position + message; standardize runtime error messages with `func(...): message`; and have the REPL format errors as `error: [line N:] col M: message` with optional snippet.

---

<user_constraints>
## User Constraints (from phase context)

No CONTEXT.md exists for Phase 21 — research options and recommend approaches. No locked user decisions.

**Project constraints (from .cursorrules):**
- C++20, zero external dependencies, single binary
- Parser in parser.h: Tokenizer + recursive descent; throws std::runtime_error("parser: ...")
- REPL in repl.h: catch (std::exception) prints "error: " + e.what()
- Must not add external libraries
</user_constraints>

---

## Standard Stack

### Core (Zero-Dependency Approach)

| Component | Purpose | Why |
|-----------|---------|-----|
| `std::runtime_error` | Base exception | Already used; `what()` returns string |
| Custom struct inheriting `std::runtime_error` | ParseError with position | Carries offset/line/col + message, still catchable as std::exception |
| `std::ostringstream` / `std::to_string` | Format error strings | Standard library only |
| Offset → line/column scan | Position conversion | O(n) one-pass over input; no extra state |

### Don't Use

| Avoid | Reason |
|-------|--------|
| GMP, Boost, fmt, spdlog | Zero-dependency constraint |
| External error/panic libraries | Not needed; simple string formatting suffices |

---

## Architecture Patterns

### 1. Parse Errors: Position Tracking

**Tokenizer changes:**
- Add `line` and `col` (or `offset`) to Tokenizer state; update on each character consumed.
- Store `size_t offset` (and optionally `int line`, `int col`) in each `Token`.
- On error, throw with position. Two options:
  - **A)** Throw `ParseError(offset, line, col, message)` — custom type.
  - **B)** Throw `std::runtime_error` with formatted string `"parser: line L, col C: message"` — minimal API change.

**Offset → line/column:**
```cpp
void offsetToLineCol(const std::string& s, size_t offset, int& line, int& col) {
    line = 1; col = 1;
    for (size_t i = 0; i < offset && i < s.size(); ++i) {
        if (s[i] == '\n') { ++line; col = 1; } else ++col;
    }
}
```
With backslash continuation, the concatenated input has no newlines (joined by space), so `line` is always 1 and `col` = offset+1. The function still supports future multi-line input.

**Token struct extension:**
```cpp
struct Token {
    Kind kind;
    std::string text;
    size_t offset = 0;  // or: int line=1, col=1
};
```

**What to include in parse errors (GCC/Clang style):**
- Location: line, column (or offset for single-line).
- Expected: e.g. `expected ':='`, `expected identifier`, `expected primary expression`.
- Actual: when useful, e.g. `unexpected character 'x'`, `unexpected token ')'`.

### 2. Runtime Errors: Function Name and Context

**Current state:** Most builtins throw strings like `"etaq(k,T) or etaq(q,k,T)"` or `"undefined variable: x"`. Some lack function name (e.g. `evalToInt`, `getSeriesFromEnv`).

**Pattern:** Use consistent format: `function(args): message` for builtin errors; `message` for generic eval errors.

Example improvements:
- `"etaq: expected etaq(k,T) or etaq(q,k,T), got N arguments"`
- `"undefined variable: x"` (already good)
- `"evalToInt: expected integer, variable 'k' not in sum scope"` (add context)

**Implementation:** No new exception type required. Standardize message strings at throw sites. Optionally introduce a helper:
```cpp
inline std::string runtimeErr(const std::string& func, const std::string& msg) {
    return func.empty() ? msg : (func + ": " + msg);
}
```

### 3. REPL Display Format

**GCC/Clang style:** `file:line:column: severity: message`

For REPL (no file):
- Interactive: `error: col 12: parser: expected ':='`
- Script mode: `error: line 5, col 12: parser: expected ':='`

**Optional enhancement:** Print source line and caret:
```
error: line 1, col 7: unexpected character 'x'
  foo x := 1
        ^
```

**Implementation:** Catch `std::exception`, inspect `e.what()`. If it contains structured info (e.g. prefix `"parse:L:C:"`), parse and format. Alternatively, catch `ParseError` specifically and format; otherwise use `e.what()` as-is for runtime errors.

### 4. Custom Exception Type (Optional but Recommended)

**ParseError:**
```cpp
struct ParseError : std::runtime_error {
    int line = 1, col = 1;
    std::string expected;
    ParseError(int line_, int col_, const std::string& msg, const std::string& exp = "")
        : std::runtime_error(buildWhat(line_, col_, msg, exp)), line(line_), col(col_), expected(exp) {}
private:
    static std::string buildWhat(int L, int C, const std::string& msg, const std::string& exp);
};
```
REPL can `catch (const ParseError& e)` and format with line/col; fallback to `catch (const std::exception& e)` for others.

### 5. Backslash Continuation and Line Numbers

**Current behavior:** Lines joined with `" "`; result has no `\n`. So within one logical input, line is always 1.

**Script mode:** REPL reads one logical command per iteration. Add `size_t inputLineNum` (incremented each iteration) and pass to error handler. Display as `line N` when not TTY.

**Multi-line continuation:** If continuation preserved newlines (Phase 19 doesn’t), offset→line/col would give the correct line within the chunk. Current design: no newlines in concatenated string, so column-only for parse errors is sufficient.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Pretty-print diagnostics | Full diagnostic engine | Simple format string + optional snippet | Scope is REPL; Clang-style engine is overkill |
| Parsing error messages | Regex/DSL | Structured exception or prefix convention | Keep it simple |
| Line/column from offset | External lib | One-pass scan over string | Trivial, no deps |

---

## Common Pitfalls

### Pitfall 1: Forgetting to Update Position in Tokenizer

**What goes wrong:** Tokenizer advances `pos` but not `line`/`col`; tokens have wrong position.

**Prevention:** Update `line` and `col` (or `offset`) in every path that consumes input (including `skipWhitespace`, `skipComment`, and each character match). Assign to Token before returning.

### Pitfall 2: Parser Loses Position

**What goes wrong:** Parser uses tokenized vector; if errors are thrown from `expect()` or `parsePrimary()`, the "current" token has position, but we need to pass it into the exception.

**Prevention:** Ensure `peek()` or `consume()` gives the token that caused the error; store that token’s position in the thrown error.

### Pitfall 3: Error Message Too Long for IDE/CLI

**What goes wrong:** Long messages are hard to read in terminals and IDE popups.

**Prevention:** Keep primary message to 1–2 sentences; put details (e.g. expected alternatives) in a second line or note if needed. Follow "short and actionable" guidance from diagnostics literature.

### Pitfall 4: Runtime Errors Without Function Name

**What goes wrong:** User sees "expected integer" with no indication of which builtin or expression caused it.

**Prevention:** For builtin dispatch, always include function name. For eval/evalToInt, include "in sum/add" or "in func(args)" when context is available.

### Pitfall 5: Breaking Existing Tests

**What goes wrong:** Tests expect exact `e.what()` strings; new format breaks assertions.

**Prevention:** Update tests to match new format, or check for substring/keywords rather than full string equality.

---

## Code Examples

### Offset to Line/Column

```cpp
inline void offsetToLineCol(const std::string& s, size_t offset, int& line, int& col) {
    line = 1; col = 1;
    for (size_t i = 0; i < offset && i < s.size(); ++i) {
        if (s[i] == '\n') { ++line; col = 1; } else ++col;
    }
}
```

### Parse Error with Position (Tokenizer)

```cpp
// In Tokenizer::next(), when throwing:
int line = 1, col = 1;
offsetToLineCol(input, pos, line, col);
throw std::runtime_error("parser: line " + std::to_string(line) + ", col " 
    + std::to_string(col) + ": expected := ");
```

### REPL Error Display

```cpp
} catch (const std::exception& e) {
    std::string loc;
    if (inputLineNum > 0) loc = "line " + std::to_string(inputLineNum) + ": ";
    std::cerr << "error: " << loc << e.what() << std::endl;
}
```

### Optional: Source Snippet + Caret

```cpp
if (!input.empty() && col > 0 && col <= (int)input.size()) {
    std::cerr << "  " << input << std::endl;
    std::cerr << "  " << std::string(col - 1, ' ') << "^" << std::endl;
}
```

---

## State of the Art

| Old Approach | Current Practice | Notes |
|--------------|------------------|-------|
| "parse error" with no location | file:line:col: message | GCC, Clang, Rust, modern compilers |
| Long prose messages | Short, actionable, 1–2 sentences | IDE-friendly (Clang, Flow, Elm) |
| Caret under error | Caret + optional range | Clang; helps multi-symbol lines |
| Generic runtime errors | Function + context | e.g. `func(args): expected X, got Y` |

---

## Open Questions

1. **Custom ParseError vs formatted string**
   - Custom type: REPL can format differently (e.g. snippet) and extend later.
   - Formatted string: Simpler, no new types; REPL just prints `e.what()`.
   - **Recommendation:** Custom `ParseError` if we want snippet/caret; otherwise formatted `std::runtime_error` is enough.

2. **Script mode: "line" semantics**
   - "Line" = Nth logical command (each full input after continuation).
   - Matches user mental model for script files.
   - **Recommendation:** Use command index as line number in script mode.

3. **Color in terminal**
   - Out of scope for this phase; can be added later if desired.

---

## Sources

### Primary (HIGH confidence)
- Clang diagnostics: https://clang.llvm.org/diagnostics — format, caret, ranges
- GCC diagnostic guidelines: https://gcc.gnu.org/onlinedocs/gccint/Guidelines-for-Diagnostics.html
- Codebase: `src/parser.h`, `src/repl.h` — current error handling

### Secondary (MEDIUM confidence)
- "Writing Good Compiler Error Messages" (Caleb Mer) — short, actionable, IDE-first

### Tertiary
- Rust/Elm error style — philosophy only, not implementation

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — no external deps, std-only
- Architecture: HIGH — patterns match existing parser/REPL structure
- Pitfalls: HIGH — derived from codebase and common compiler practices

**Research date:** 2026-02-25
**Valid until:** ~30 days (stable domain)
