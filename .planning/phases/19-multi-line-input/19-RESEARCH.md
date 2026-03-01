# Phase 19: Multi-line input — Research

**Domain:** Backslash continuation in REPL read loop.

## Summary

Implement continuation in runRepl() before parse: if line ends with backslash (after rstrip), strip it, read next line, append with space, repeat until complete. Use continuation prompt (e.g. `  > `) when reading more. Parser receives combined string; no tokenizer changes if backslash-newline becomes space.

---

## Current Flow

- runRepl: getline → trim → parse → evalStmt → display
- Parser: Tokenizer(s) consumes string; skipWhitespace skips spaces/newlines
- Backslash is not special in Tokenizer today — would be parsed as unexpected if present

## Implementation

1. **REPL read loop:** Before parse, loop: if trimmed line ends with `\`, remove trailing `\`, prompt for continuation (`  > `), getline, append ` ` + next, repeat. Max lines (e.g. 100) to avoid infinite loop.
2. **Backslash-newline:** Replacing `\`+newline with space means the concatenated string has a space where the continuation was. Parser sees "foo bar" not "foo\nbar".
3. **Script mode:** getline naturally returns next line from pipe; loop works the same.
4. **Continuation prompt:** Use `  > ` (implementer choice per CONTEXT).

---
*Phase: 19-multi-line-input*
