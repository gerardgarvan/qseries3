# Phase 108: Error Diagnostics — Research

**Researched:** 2026-03-06
**Domain:** C++ REPL error message presentation (parse/runtime diagnostics)
**Confidence:** HIGH

## Summary

This phase improves error presentation in a C++20 zero-dependency q-series REPL. The codebase already has `offsetToLineCol`, `runtimeErr`, ANSI color (`ansi::red()`), and script-mode line tracking. Changes are additive: add parse-error source snippet with caret, switch to Maple-style `Error, (in func) message`, introduce `expectArg` helper for argument errors, and refine script-mode line prefixes.

Primary recommendation: implement a `formatParseErrorWithSnippet` helper that parses line/col from existing parser messages, extracts the source line from input, and produces GCC/Clang-style output; change `runtimeErr` to Maple format; add `expectArg(N, name, expected, received)` and migrate incrementally.

<user_constraints>

## User Constraints (from CONTEXT.md)

### Locked Decisions
- **Parse Error Display:** Caret (`^`) under error column; ANSI red for parse errors; "Expected" stays in main message
- **Maple-Style Format:** `Error, (in funcname) message`; parse errors `Error, (in parser) line N, col M: expected X`; argument errors use "invalid input:" — `Error, (in etaq) invalid input: expects its 1st argument...`
- **Argument Error Wording:** Shared helper `expectArg(N, name, expected, received)`; ordinals 1st, 2nd, 3rd; migrate incrementally
- **Script-Mode:** `line N:` compact prefix; only when line known; omit or `line ?:` when unknown; skip redundant parse snippet prefix (parser already has line N, col M); stderr vs stdout: keep current (verify, only change if needed)

### Claude's Discretion
- Parse error layout (single vs multi-line, line numbers)
- "Received Y" format in argument errors
- Non-argument runtime error phrasing (invalid input: or not)
- stderr/stdout verification outcome

### Deferred Ideas (OUT OF SCOPE)
None

</user_constraints>

## Standard Stack

### Core (Existing)
| Component | Location | Purpose |
|-----------|----------|---------|
| offsetToLineCol | src/parser.h:27-32 | Convert byte offset → line, col (1-based) |
| runtimeErr | src/repl.h:49-51 | Builds "func: msg" (to become Maple format) |
| ansi::red/gold/reset | src/repl.h:119-144 | ANSI color for TTY |
| stdin_is_tty() | src/repl.h:31-38 | Script vs interactive mode |
| std::cerr | repl.h catch blocks | Error output (current: stderr) |
| inputLineNum | repl.h:2835, 2924 | Script-mode line counter |

### New Components (Phase Deliverables)
| Component | Purpose |
|-----------|---------|
| formatParseErrorWithSnippet | Source line + caret + message (GCC/Clang style) |
| expectArg(N, name, expected, received) | Argument error message builder |
| ordinal(N) | "1st", "2nd", "3rd", "4th", ... for expectArg |
| Maple-style runtimeErr | Return "Error, (in func) message" |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Parse e.what() for line/col | Custom exception type | Parsing is simpler; no ABI/exception changes |
| Inline ordinal logic | Lookup table 1..20 | Ordinal rule is trivial; 10-line helper sufficient |
| Centralized formatParseError | Per-throw formatting in parser | Single formatting point keeps layout consistent |

## Architecture Patterns

### Parse Error Flow
1. Parser/Tokenizer throws `std::runtime_error("parser: line N, col M: message")`.
2. REPL catch (repl.h:2941-2946) receives `e.what()` and has `trimmed` (current input).
3. If `e.what()` starts with `"parser: "`, call `formatParseErrorWithSnippet(trimmed, e.what())` to produce full display.
4. Output: source line, caret line, then `Error, (in parser) line N, col M: message` (Maple format).
5. Script mode: prepend `line inputLineNum: ` only when line known (script mode); for parse errors, parser message already has line/col.

### Runtime Error Flow
1. Built-ins call `runtimeErr(name, msg)` → currently returns `"name: msg"`.
2. Change to `"Error, (in name) msg"`.
3. Argument errors: use `expectArg(N, name, expected, received)` → `"invalid input: expects its Nth argument, name, to be expected, but received received"`.
4. Script mode: prepend `line inputLineNum: ` when known; use `line ?: ` when unknown (e.g. runtime errors without parse context).

### Recommended Structure
- **parser.h:** Add `formatParseErrorWithSnippet(input, message)` (or keep in repl if it needs ansi). Parser messages already include line/col; helper parses them.
- **repl.h:** Change `runtimeErr` signature/implementation; add `expectArg`, `ordinal`; update catch block to use formatParseError when applicable; ensure script-mode prefix logic.

### Anti-Patterns to Avoid
- **Modifying exception type:** Keep `std::runtime_error`; avoid custom exception types for this phase.
- **Duplicating line:col in snippet:** CONTEXT says skip redundant prefix; parser message already has line N, col M.
- **Changing stderr without verification:** Verify current use of std::cerr and only change if requirements demand it.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Ordinal strings | Full i18n/plural rules | Small inline helper | 1st/2nd/3rd/4th rule is fixed; ~10 lines |
| ANSI color handling | Custom terminal lib | Existing ansi::* | Already in repl.h |
| Line/col from offset | Manual scan | offsetToLineCol | Already exists |

**Key insight:** The only "library" here is the existing parser/repl infrastructure. Ordinal and snippet formatting are small, deterministic helpers that do not warrant external dependencies.

## Common Pitfalls

### Pitfall 1: Column vs Display Width
**What goes wrong:** Tabs render as multiple columns; caret appears misaligned.
**Why it happens:** Caret position uses character count, not display width.
**How to avoid:** Use character count (1-based col from offsetToLineCol) for caret; col-1 spaces before `^`. Document that tabs = 1 char for alignment.
**Warning signs:** Carets misaligned in editors with tab stops ≠ 1.

### Pitfall 2: Parse Message Parsing Fragility
**What goes wrong:** formatParseErrorWithSnippet fails when parser message format varies.
**Why it happens:** "parser: line N, col M:" parsed via fixed pattern; new throw sites might use different wording.
**How to avoid:** Single pattern: `"parser: line "` + digits + `", col "` + digits + `": "` + rest. Centralize all parser throws to use this format.
**Warning signs:** Regex or split logic breaks on edge messages.

### Pitfall 3: Multi-Line Input (Continuations)
**What goes wrong:** Line N from parser refers to logical line in `trimmed`; `trimmed` can be multi-line (backslash continuation).
**Why it happens:** Parser receives full concatenated input; line numbers are 1-based within that.
**How to avoid:** Split `trimmed` by `\n`; line N → index N-1 in lines; use that line for snippet. Handle N > lines.size().
**Warning signs:** Snippet shows wrong line or crashes on continuation input.

### Pitfall 4: Script-Mode Line for Parse vs Runtime
**What goes wrong:** Parse errors have line from parser; runtime errors do not. Using inputLineNum for both can be misleading.
**Why it happens:** inputLineNum = logical REPL input line; parser line = line within current (possibly multi-line) input.
**How to avoid:** For parse errors: parser message already has "line N, col M"; CONTEXT says skip redundant snippet prefix. Script prefix "line inputLineNum: " applies to the error as a whole. For runtime: "line ?:" when unknown, or "line inputLineNum: " when the error is from the current statement.
**Warning signs:** Double "line 3" or wrong line attribution.

### Pitfall 5: WASM / Non-TTY Output
**What goes wrong:** main_wasm.cpp captures cout/cerr to string; ANSI codes may appear in output.
**Why it happens:** ansi::* emits codes when g_color true; WASM may not have TTY.
**How to avoid:** Existing ansi::init() checks stdin_is_tty(); WASM defines stdin_is_tty() false, so g_color should stay false. Verify no TTY checks bypassed.
**Warning signs:** Raw escape sequences in WASM or redirected output.

## Code Examples

### Parse Error with Snippet (Target Layout)
```
  x: = 1
      ^
Error, (in parser) line 1, col 5: expected :=
```

### Parsing Parser Message (C++)
```cpp
// Extract line, col from "parser: line N, col M: message"
// Return true if parse succeeded
bool parseParserMessage(const std::string& msg, int& line, int& col, std::string& rest) {
    const char* p = "parser: line ";
    if (msg.compare(0, 13, p) != 0) return false;
    size_t i = 13;
    line = 0;
    while (i < msg.size() && std::isdigit(static_cast<unsigned char>(msg[i])))
        line = line * 10 + (msg[i++] - '0');
    if (i + 6 > msg.size() || msg.compare(i, 6, ", col ") != 0) return false;
    i += 6;
    col = 0;
    while (i < msg.size() && std::isdigit(static_cast<unsigned char>(msg[i])))
        col = col * 10 + (msg[i++] - '0');
    if (i < msg.size() && msg[i] == ':') { ++i; if (i < msg.size() && msg[i] == ' ') ++i; }
    rest = (i < msg.size()) ? msg.substr(i) : "";
    return line > 0 && col > 0;
}
```

### Extracting Source Line from Input
```cpp
std::string getLineAt(const std::string& input, int lineNum) {
    int cur = 1;
    for (size_t i = 0; i < input.size(); ++i) {
        if (cur == lineNum) {
            size_t end = input.find('\n', i);
            if (end == std::string::npos) end = input.size();
            return input.substr(i, end - i);
        }
        if (input[i] == '\n') ++cur;
    }
    return "";
}
```

### Caret Line
```cpp
std::string caretLine(int col) {
    return std::string(col > 0 ? static_cast<size_t>(col - 1) : 0, ' ') + "^";
}
```

### Ordinal Helper
```cpp
inline std::string ordinal(int n) {
    if (n <= 0) return std::to_string(n);
    int d = n % 10, t = (n / 10) % 10;
    if (t != 1 && d == 1) return std::to_string(n) + "st";
    if (t != 1 && d == 2) return std::to_string(n) + "nd";
    if (t != 1 && d == 3) return std::to_string(n) + "rd";
    return std::to_string(n) + "th";
}
```

### expectArg Helper
```cpp
inline std::string expectArg(int n, const std::string& name,
    const std::string& expected, const std::string& received) {
    return "invalid input: expects its " + ordinal(n) + " argument, " + name +
        ", to be " + expected + ", but received " + received;
}
```

### runtimeErr → Maple Format
```cpp
inline std::string runtimeErr(const std::string& func, const std::string& msg) {
    return func.empty() ? ("Error, " + msg) : ("Error, (in " + func + ") " + msg);
}
```
Note: Current callers pass func; top-level/unknown could pass "" — adjust so top-level gets a sensible placeholder if needed.

### REPL Catch Block (Concepts)
```cpp
} catch (const std::exception& e) {
    std::string text = e.what();
    bool isParse = (text.size() >= 8 && text.compare(0, 8, "parser: ") == 0);
    std::cerr << ansi::red();
    if (!stdin_is_tty() && inputLineNum > 0)
        std::cerr << "line " << inputLineNum << ": ";
    if (isParse) {
        int line = 0, col = 0; std::string rest;
        if (parseParserMessage(text, line, col, rest)) {
            std::string srcLine = getLineAt(trimmed, line);
            if (!srcLine.empty()) {
                std::cerr << "  " << srcLine << "\n  " << caretLine(col) << "\n";
            }
            std::cerr << "Error, (in parser) line " << line << ", col " << col << ": " << rest;
        } else {
            // Fallback: replace "parser: " with "Error, (in parser) "
            std::cerr << "Error, (in parser) " << text.substr(8);
        }
    } else {
        std::cerr << text;  // Already "Error, (in func) msg" from runtimeErr
    }
    std::cerr << ansi::reset() << std::endl;
}
```

## State of the Art

| Current | Target | When Changed |
|---------|--------|--------------|
| "parser: line N, col M: msg" | "Error, (in parser) line N, col M: msg" | Phase 108 |
| "func: msg" | "Error, (in func) msg" | Phase 108 |
| No source snippet | Source line + caret | Phase 108 |
| "expects N arguments" | "expects its Nth argument, name, to be X, but received Y" | Phase 108 (incremental) |
| "line N: " (script) | Same + "line ?:" when unknown | Phase 108 (optional refinement) |

**Deprecated/outdated:** None for this phase. Existing convert.h std::cerr diagnostic output (etamake, prodmake, etc.) is separate from user-facing parse/runtime errors; no change required unless CONTEXT expands scope.

## Open Questions

1. **"Received Y" format**
   - Known: Maple uses type names like "algebraic", "list"; we have Series, int, list, etc.
   - Open: Literal repr vs type name vs truncation for long values.
   - Recommendation: Use short type-style labels ("series", "integer", "list") for simplicity; literal for small values if space allows.

2. **Non-argument runtime errors**
   - Known: Many built-ins throw non-argument errors (e.g. "modulus cannot be zero").
   - Open: Prepend "invalid input:" or not per CONTEXT discretion.
   - Recommendation: Use "invalid input:" only for argument/type errors; keep other messages as-is (e.g. "Error, (in mod) modulus cannot be zero").

3. **stderr verification**
   - Known: Errors go to std::cerr in repl.h; convert.h uses std::cerr for internal diagnostics.
   - Open: Whether script-mode piping expects stderr separation.
   - Recommendation: Verify acceptance tests; keep stderr unless tests or CONTEXT require stdout.

## Sources

### Primary (HIGH confidence)
- CONTEXT.md (phase decisions)
- src/parser.h (offsetToLineCol, throw format)
- src/repl.h (runtimeErr, catch block, ansi, script mode)
- Maple Error Message Guide Overview (maplesoft.com)
- Clang Expressive Diagnostics (clang.llvm.org/diagnostics.html)

### Secondary (MEDIUM confidence)
- GCC Diagnostic Message Formatting Options
- Maple "invalid input" examples (expects its Nth argument...)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all components are in the codebase
- Architecture: HIGH — flow is clear from existing parser/repl
- Pitfalls: HIGH — common issues with column alignment, message parsing, multi-line input

**Research date:** 2026-03-06
**Valid until:** ~30 days (stable C++ REPL domain)

## RESEARCH COMPLETE
