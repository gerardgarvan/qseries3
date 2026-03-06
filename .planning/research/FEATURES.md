# Feature Landscape: REPL UX for Maple Users

**Domain:** q-series REPL — ergonomics, error diagnostics, help, input convenience for mathematicians transitioning from Maple  
**Researched:** 2026-03-06  
**Confidence:** HIGH  

---

## Executive Summary

Mathematicians used to Maple's qseries package and worksheet model expect: (1) help that mirrors Maple's `?topic` / `help(topic)` with **calling sequence, synopsis, examples**; (2) **clear error messages** with location and context; (3) **input convenience** — tab completion, history, multi-line; (4) **ergonomic prompt/output layout** that separates input from result. qseries3 already has help, tab completion, history, multi-line, ANSI color, and "Did you mean" for unknown built-ins. Gaps: clearer parse errors (source snippet + caret), richer per-function help (examples, Maple-style structure), undefined-variable typo suggestions, and prompt/output layout polish. Table stakes for Maple-to-REPL transition: `help`/`help(func)`, tab completion, history, multi-line, semicolon suppress, clear runtime errors with function name. Differentiators: parse errors with caret, per-function examples in help, typo suggestions for variables.

---

## Maple User Expectations

### Help System

| Maple Pattern | What Users Expect | qseries3 Status |
|---------------|-------------------|-----------------|
| `?topic` or `help(topic)` | Open help for a topic | `help(func)` — present |
| Per-function pages | CALLING SEQUENCE, PARAMETERS, SYNOPSIS, EXAMPLES, SEE ALSO | Signature + one-line description only |
| `??topic` | Brief (calling sequence + params) | Not implemented |
| `???topic` | Examples only | Not implemented |
| Index / category list | Browse all commands | `help` lists flat built-in names |

**Source:** [Maple Help](https://www.maplesoft.com/support/help/Maple/view.aspx?path=help), [qseries prodmake.html](https://qseries.org/fgarvan/qmaple/qseries/functions/prodmake.html)

### Error Messages

Maple errors include function name and brief context. Compiler-style diagnostics (GCC/Clang) are the modern standard: `location: message`, optional source snippet + caret.

### Input Convenience

| Feature | Maple | Mathematica | qseries3 |
|---------|-------|-------------|----------|
| Tab completion | Prefix match only | Partial, middle-of-word, abbreviation | Prefix match (identifiers + built-ins) ✓ |
| History | Up/down | Up/down | Up/down ✓ |
| Multi-line | Semicolon-separated or continuation | Natural multi-line | Backslash continuation ✓ |
| Delimiter matching | Limited | Auto-close brackets | None |
| Command templates | Keyboard only | Mouse + keyboard | None |

**Source:** Maple 2025 interface updates; Mathematica vs Maple ease-of-use comparison (LOW confidence for detailed feature matrix).

---

## Feature Landscape

### Table Stakes (Users Expect These)

Features users assume exist. Missing = product feels incomplete.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| **help** and **help(func)** | Maple `?topic` / `help(topic)` is standard | LOW | ✓ Present; general help + per-function signature + one-liner |
| **Tab completion** | Maple has autocomplete; mathematicians rely on it | LOW | ✓ Present; identifiers and built-in names |
| **Command history (up/down)** | Standard REPL behavior | LOW | ✓ Present; 100 commands |
| **Multi-line input** | Long expressions (sum, findhom) are common | LOW | ✓ Present; backslash continuation |
| **Semicolon suppress output** | Maple uses `;` vs `:` for display vs suppress | LOW | ✓ Present |
| **Clear runtime errors** | Function name + context (e.g. `etaq: expected ...`) | LOW | ✓ Present; runtimeErr helper, script line number |
| **Arrow-key line navigation** | Edit within line without retyping | LOW | ✓ Present (TTY) |
| **Per-function help with signature** | Maple pages show CALLING SEQUENCE | LOW | ✓ Present; signature + one-line description |

### Differentiators (Competitive Advantage)

Features that set the product apart. Not required, but valuable.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| **Parse errors with source snippet + caret** | Pinpoints exact column; GCC/Clang style | MEDIUM | Parser has offset; needs snippet + caret in REPL catch |
| **Per-function help with examples** | Maple prodmake.html has EXAMPLES; speeds learning | MEDIUM | Extend help table: optional third field (examples) or link to doc |
| **Typo suggestions for undefined variable** | "Did you mean: x?" when `y` is undefined | LOW | Already done for unknown built-in; extend to evalExpr Var branch |
| **Richer help structure** | Maple: CALLING SEQUENCE, SYNOPSIS, EXAMPLES | MEDIUM | Multi-line help text per function; `help(prodmake)` could show 3–5 lines |
| **Color-gated output** | Prompt, errors, timing, help names visually distinct | LOW | ✓ Present; ANSI gold/red/dim |

### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| **Full worksheet/document mode** | Maple has worksheet; Mathematica has notebook | Requires GUI, cell model, rich display; conflicts with single-binary REPL | Keep REPL; optional script mode (✓ present) for batch work |
| **2-D Math input** | Maple worksheet uses 2-D notation | Needs WYSIWYG editor, symbol palette; not feasible in terminal REPL | 1-D syntax; document notation in help and MANUAL |
| **External browser for help** | Maple can open help in browser | Adds dependency, breaks offline/single-binary | Inline `help(func)` in terminal |
| **AI-powered completion** | Mathematica has context-aware suggestions | Out of scope; external API; no training on q-series | Tab completion on known identifiers + built-ins |
| **Delimiter auto-close** | Mathematica auto-closes `[` with `]` | Can confuse users; cursor placement edge cases | Manual typing; document in quick-start |
| **`?func` syntax** | Maple uses `?` for help | Parser already uses `#` for comment; `?` could conflict | Keep `help(func)`; document in help |

---

## Feature Dependencies

```
help(func) with examples
    └── requires ──> help table (exists) + richer content (examples text)
                          └── depends on ──> qseriesdoc.md / Garvan examples

Typo suggestions for undefined variable
    └── requires ──> getHelpTable() or built-in name set (exists)
    └── requires ──> levenshteinDistance (exists, used for unknown built-in)

Parse errors with snippet + caret
    └── requires ──> Token.offset / line, col (parser has offsetToLineCol)
    └── requires ──> REPL catch block formats error (currently prints e.what())
```

### Dependency Notes

- **Per-function examples require help table extension:** Add optional `examples` string per entry; or keep help table minimal and reference qseriesdoc.md / website.
- **Typo suggestions for variables:** Reuse existing `levenshteinDistance` and built-in set; add variable name set from `env.env` for "undefined variable: x. Did you mean: y?"
- **Source snippet + caret:** Parser throws `"parser: line L, col C: message"`; REPL can parse that or use a custom ParseError. Snippet needs the raw input line; REPL has it in the catch scope.

---

## MVP Definition (REPL UX Milestone)

### Launch With (v1 for this milestone)

- [x] help, help(func) — DONE
- [x] Tab completion — DONE
- [x] History, multi-line, semicolon suppress — DONE
- [x] Runtime errors with function name — DONE
- [x] Typo suggestions for unknown built-in — DONE
- [ ] **Parse errors with column + optional snippet/caret** — HIGH value, MEDIUM effort
- [ ] **Typo suggestions for undefined variable** — LOW effort, reuses existing logic

### Add After Validation (v1.x)

- [ ] **Per-function help with 1–2 examples** — e.g. `help(prodmake)` shows Rogers-Ramanujan example
- [ ] **Richer help structure** — SYNOPSIS + SEE ALSO for key functions (prodmake, etamake, etc.)

### Future Consideration (v2+)

- [ ] `??func` brief help (signature only)
- [ ] `???func` examples only
- [ ] Help categories (e.g. "Conversion: prodmake, etamake, jacprodmake")

---

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| Parse errors with snippet + caret | HIGH | MEDIUM | P1 |
| Typo suggestions for undefined variable | MEDIUM | LOW | P1 |
| Per-function help with examples | HIGH | MEDIUM | P2 |
| Richer help structure (SYNOPSIS, SEE ALSO) | MEDIUM | MEDIUM | P2 |
| `??func` / `???func` | LOW | LOW | P3 |

**Priority key:** P1 = must have for this milestone; P2 = should have; P3 = nice to have.

---

## Competitor Feature Analysis

| Feature | Maple | Mathematica | qseries3 Approach |
|---------|-------|-------------|-------------------|
| Help invocation | `?topic`, `help(topic)` | `?symbol` | `help`, `help(func)` |
| Per-function content | CALLING SEQUENCE, SYNOPSIS, EXAMPLES | Full doc + examples | Signature + one-liner; add examples |
| Error format | Function + message | Full traceback | `func: message`; add location + snippet |
| Tab completion | Prefix match | Partial, abbreviation | Prefix match (sufficient for REPL) |
| Typo suggestions | Limited | "Did you mean" for symbols | Present for unknown built-in; add for variable |

---

## What Makes UX "Best" for Maple-to-REPL Transition

1. **Minimal syntax change** — `help(func)` mirrors Maple; `:=` for assignment; function call `name(args)` same.
2. **Familiar help structure** — When `help(prodmake)` shows CALLING SEQUENCE + SYNOPSIS + 1–2 EXAMPLES, Maple users feel at home.
3. **Errors that point to the problem** — Column number + caret under the bad character; function name in runtime errors.
4. **Fast recovery from typos** — "Did you mean: prodmake?" for `prodmakee`; extend to "undefined variable x, did you mean: y?" when `env` has a close match.
5. **No surprises** — Single binary, no package install, no worksheet complexity. REPL is a focused tool.
6. **Script mode for worksheet-like workflow** — `qseries < script.qs` lets users prepare commands in a file, akin to Maple cells.

---

## Sources

- Maple Help: https://www.maplesoft.com/support/help/Maple/view.aspx?path=help
- Maple qseries prodmake: https://qseries.org/fgarvan/qmaple/qseries/functions/prodmake.html
- qseries functions index: https://qseries.org/fgarvan/qmaple/qseries/functions/
- Maple 2025 Interface: https://www.maplesoft.com/support/help/maple/view.aspx?path=updates%2FMaple2025%2FInterface
- Phase 21 Research (error messages): .planning/phases/21-error-messages/21-RESEARCH.md
- Codebase: src/repl.h (help, runtimeErr, typo suggestions, tab completion), src/parser.h (offsetToLineCol)
