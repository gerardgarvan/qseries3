# Phase 16: Script mode - Context

**Gathered:** 2026-02-25
**Status:** Ready for planning

<domain>
## Phase Boundary

User can run `qseries < script.qs` to execute a file of commands non-interactively. Commands run sequentially; process exits when file ends. Script mode is detected when stdin is not a TTY.

</domain>

<decisions>
## Implementation Decisions

### Banner
- **Suppress in script mode** — When stdin is not a TTY, do not show the ASCII art banner. Output only commands and results.

### Claude's Discretion
- Invocation: stdin redirect (`qseries < file`) is the primary interface; file-argument form (`qseries file.qs`) is optional
- Command echo: Keep current behavior (echo `qseries> <line>` when not TTY) unless it conflicts with banner suppression
- Error handling: Continue vs exit-on-error — choose what fits scripting use cases

</decisions>

<specifics>
## Specific Ideas

No specific requirements — standard script-mode patterns apply.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---
*Phase: 16-script-mode*
*Context gathered: 2026-02-25*
