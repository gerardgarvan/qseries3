# Phase 19: Multi-line input - Context

**Gathered:** 2026-02-25
**Status:** Ready for planning

<domain>
## Phase Boundary

User can split long expressions across lines using backslash continuation. Backslash at end of line continues on next line. REPL prompts for continuation until expression is complete. Backslash-newline treated as whitespace.

</domain>

<decisions>
## Implementation Decisions

### Continuation prompt
- **Implementer chooses** — Use whatever works best long-term (e.g. `  ... ` or `  > ` to distinguish continuation from main prompt)

### Claude's Discretion
- Max lines limit, script mode behavior, backslash handling (only at EOL vs elsewhere) — implementer chooses
</decisions>

<specifics>
## Specific Ideas

No specific requirements — standard backslash-continuation patterns apply.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---
*Phase: 19-multi-line-input*
*Context gathered: 2026-02-25*
