# Phase 18: Timing - Context

**Gathered:** 2026-02-25
**Status:** Ready for planning

<domain>
## Phase Boundary

User sees elapsed time for each command evaluation. Time includes parse + eval. Format is readable.

</domain>

<decisions>
## Implementation Decisions

### Placement
- **After output, next line** — Elapsed time shown on the line after the command output (e.g. output, then `0.042s` on its own line)

### Format
- **Always seconds** — e.g. `0.042s`, `1.234s` (not ms, not switching by magnitude)

### Script mode
- **Interactive only** — Show timing when stdin is a TTY; do NOT show timing in script mode (piping, redirect)

### Claude's Discretion
- Scope: which commands get timed (blank lines, comments, set_trunc, help, etc.) — implementer chooses
</decisions>

<specifics>
## Specific Ideas

No specific requirements — standard timing patterns apply.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---
*Phase: 18-timing*
*Context gathered: 2026-02-25*
