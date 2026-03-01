# Phase 27: Suppress output (colon) — Context

**Gathered:** 2026-02-26  
**Status:** Ready for planning

<domain>
## Phase Boundary

User can suppress result printing by ending a statement with colon. Maple-style: `rr := sum(...):` stores the result without printing. Semicolon shows output; colon suppresses it. Applies to both assignments and expression statements.
</domain>

<decisions>
## Implementation Decisions

### Detection approach
- **REPL-layer only** — No parser or tokenizer changes. Before parsing: trim the line; if the last character is `:`, strip it and set `suppress_output = true`. Pass the trimmed string (without trailing colon) to `parse`.
- Parser receives normal input; the trailing colon never reaches it.
- **Caution:** Assignment uses `:=`. Only a *trailing* colon after the whole statement suppresses output (e.g. `x := 1:`). The `:=` in the middle is unchanged.

### Scope
- **Assignments:** `rr := sum(...):` → suppress
- **Expressions:** `etaq(1,50):` → suppress
- Both use the same rule: trailing `:` means suppress.

### Whitespace
- `x := 1 :` (space before colon) — trim first, then check back() == ':'. After trim, trailing spaces are gone, so back() is ':'. Strip colon. Valid.

### Display logic
- When `suppress_output` is true: evaluate as usual, **skip** `display(res, ...)` and timing output.
- `std::holds_alternative<std::monostate>(res)` still continues (no change).
</decisions>

<specifics>
## Specific Ideas

- Maple: trailing colon suppresses output; semicolon displays. Match that behavior.
- No multi-statement support (`a:=1: b:=2`) — single statement per line, optional trailing colon only.
</specifics>

<deferred>
## Deferred Ideas

- Multi-statement lines (`a:=1: b:=2`) — would require parser changes; out of scope for this phase.
- Colon/semicolon as expression separators — same.
</deferred>
