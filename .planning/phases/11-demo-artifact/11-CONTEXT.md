# Phase 11: Demo artifact — Context

**Gathered:** 2026-02-25
**Status:** Ready for planning

<domain>
## Phase Boundary

Create a runnable demo artifact that executes qseries commands. User can pipe commands to qseries and get output. Structure supports Phases 12–15 appending Garvan examples. This phase establishes the artifact; content (Rogers-Ramanujan, product conversion, etc.) comes in later phases.
</domain>

<decisions>
## Implementation Decisions

### Artifact format
- **Primary:** Shell script (`.sh`) that pipes commands to qseries binary — matches existing `tests/acceptance.sh` pattern
- Single growing file, not multiple files
- Optionally a brief companion `demo/README.md` with intro and copy-paste commands

### Location and structure
- **Location:** `demo/garvan-demo.sh` (or `demo/demo.sh`)
- Section headers via comments: `# === Rogers-Ramanujan (qseriesdoc §3.1) ===` so Phases 12–15 can append blocks
- Use `printf '%s\n' "cmd1" "cmd2" | ./qseries` for multi-command sequences (variable state persists)

### Execution model
- Fully automated — run `bash demo/garvan-demo.sh`; output streams to stdout
- No interactive prompts; demo is non-interactive

### Output expectations
- Demo shows output only; does not assert expected output (that's `tests/acceptance.sh`)
- For human demonstration, not verification

### Extensibility
- Phases 12–15 append new sections to the same script
- Each section: comment header + `printf ... | qseries` block
</decisions>

<specifics>
## Specific Ideas

- Follow qseriesdoc structure: prodmake (§3.1), qfactor/etamake/jacprodmake (§3.2–3.4), relations (§4), sift/product identities (§5, §6)
- Script should locate qseries binary (./qseries or ./qseries.exe) like acceptance.sh
</specifics>

<deferred>
## Deferred Ideas

- Demo-as-test (asserting output) — acceptance.sh already covers that
- Interactive demo / walkthrough mode — out of scope
- Separate .qs script format — script-only for v1.1
</deferred>

---
*Phase: 11-demo-artifact*
*Context gathered: 2026-02-25*
