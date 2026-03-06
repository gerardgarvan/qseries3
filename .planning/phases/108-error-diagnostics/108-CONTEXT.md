# Phase 108: Error Diagnostics — Context

**Gathered:** 2026-03-06
**Status:** Ready for planning

## Phase Boundary

Users get clear, actionable error messages that point to the problem location. This phase improves presentation and wording of parse and runtime errors — source snippet with caret, Maple-style format, argument error helper, script-mode line numbers — without adding new error types.

## Implementation Decisions

### Parse Error Display
- **Layout:** Claude's discretion — planner decides single vs multi-line context, line numbers
- **Position marker:** Caret (`^`) under the error column
- **ANSI color:** Yes — parse errors use red, consistent with runtime errors
- **"Expected" placement:** Keep current — in the main message; no structural change

### Maple-Style Format
- **"Error," prefix:** Yes — `Error, (in funcname) message`
- **Parse errors:** Use Maple pattern — `Error, (in parser) line 2, col 5: expected :=`
- **Argument/type errors:** Use "invalid input:" — `Error, (in etaq) invalid input: expects its 1st argument...`
- **Built-in vs library:** Same format — always `(in funcname)` from top-level call

### Argument Error Wording
- **Rollout:** Add shared helper `expectArg(N, name, expected, received)` and migrate incrementally
- **"Received Y" format:** Claude's discretion — literal vs type-based vs truncate
- **Ordinals:** Match Maple — use 1st, 2nd, 3rd, etc.
- **Non-argument runtime errors:** Claude's discretion — whether "invalid input:" applies or not

### Script-Mode Presentation
- **Line number format:** `line 3:` — compact prefix
- **Placement:** Only when line is known — omit or `line ?:` when unavailable (e.g. runtime errors)
- **Parse snippet prefix:** Skip redundant — parser message already has "line N, col M"
- **stderr vs stdout:** Keep current behavior — verify and only change if needed

## Claude's Discretion
- Parse error layout (lines of context)
- "Received Y" format in argument errors
- Non-argument runtime error phrasing (invalid input: or not)
- stderr/stdout verification outcome

## Specific Ideas
- GCC/Clang style for parse errors (caret under column)
- Maple Error Message Guide as reference for wording
- Existing `offsetToLineCol`, `runtimeErr` are integration points

## Deferred Ideas
None — discussion stayed within phase scope

---

*Phase: 108-error-diagnostics*
*Context gathered: 2026-03-06*
