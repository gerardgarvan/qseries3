# Phase 104: Block 24 N/A rationale — Context

**Gathered:** 2026-03-06
**Status:** Ready for planning

---

## Phase Boundary

Document Block 24 as N/A; clarify scope without implementing collect. Phase 98 (v10.0) already added the explicit N/A rationale to maple-checklist.sh and maple_checklist.md. Phase 104 = verification that this documentation exists and meets success criteria. No implementation of collect.

---

## Implementation Decisions

### 1. Overlap with Phase 98

- **Phase 98** already: maple-checklist.sh Block 24 skip with rationale; maple_checklist.md Block 24 note referencing Phase 98 rationale.
- **Phase 104** = verification-first: confirm Block 24 has documented N/A rationale and explicit skip in test file. If both present and adequate, phase is verification-only (no edits). If missing or insufficient, add or strengthen documentation.

### 2. Documentation Locations

- **maple-checklist.sh** — Block 24 comment + skip message (lines 237–239). Rationale: "collect is Maple formatting-only; relation output is mathematically equivalent; no computational need."
- **maple_checklist.md** — Block 24 section with note referencing Phase 98 rationale.
- No other locations required unless verification finds gaps.

### 3. Scope Guardrail

- **No collect implementation** — Scope explicitly deferred. Do not implement Maple's collect().
- **Rationale:** collect is display/formatting; relation output is mathematically equivalent.

### 4. Verification Scope

- maple-checklist Block 24 shows SKIP (not FAIL).
- Skip message includes N/A rationale.
- maple_checklist.md Block 24 references rationale.

---

## Claude's Discretion

- Whether to add a Phase 104–specific note (e.g. in maple_checklist.md) or rely on Phase 98 references.
- Exact wording if strengthening documentation.

---

## Specific Ideas

- Current maple-checklist.sh skip: "Block 24: collect — N/A (Maple formatting-only; output equivalent)"
- maple_checklist.md Block 24: "N/A — collect() is Maple formatting-only; relation output is mathematically equivalent; no computational need (see Phase 98 rationale)."

---

## Deferred Ideas

- Implementing collect() — out of scope.
- Additional formatting options for relation output — future phase.

---

*Phase: 104-block-24-n-a-rationale*
*Context gathered: 2026-03-06*
