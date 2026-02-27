# Phase 45: Documentation Content - Context

**Gathered:** 2026-02-27
**Status:** Ready for planning

<domain>
## Phase Boundary

Populate the Astro Starlight site (scaffolded in Phase 44) with actual documentation content. This includes: landing page feature highlights, MANUAL.md converted to 7 navigable HTML reference pages, and 4 Garvan tutorial pages with KaTeX math rendering. No playground integration (Phase 46). No CI/CD (Phase 47).

</domain>

<decisions>
## Implementation Decisions

### Landing Page Content
- **Feature cards:** 3-4 cards with icons highlighting key capabilities (exact arithmetic, prodmake, theta functions, relation finding)
- **Download link:** Points to Getting Started / Installation page (build from source)
- **Demo snippet:** Include a Rogers-Ramanujan example with REPL output as a static code block
- **Academic framing:** Prominent — mention Garvan, cite partition theory / modular forms prominently. This is a tool for researchers

### Manual Conversion
- **REPL examples:** Every function gets at least one REPL input/output example
- **Conversion approach:** Rewrite for web — restructure and enhance the existing MANUAL.md for web reading experience (not a direct copy)
- **Function signatures:** Formal typed syntax — e.g. `etaq(k: int, T?: int) → Series` showing types and optionality
- **Page organization:** Claude's Discretion — organize the 7 manual pages logically based on existing content

### Tutorial Content
- **Source material:** Follow Garvan's qseriesdoc.md tutorial structure directly, adapted to our REPL syntax
- **Math depth:** Claude's Discretion — balance theorem statements with computation demonstrations
- **Style:** Reference-style — show capabilities, less hand-holding (not step-by-step walkthrough)
- **Prerequisites:** Include a prerequisites section on the tutorial overview page mentioning basic q-series / partition theory knowledge
- **Tutorial pages:** Rogers-Ramanujan, Theta Functions, Partition Identities, Modular Equations (matching scaffold)

### Math Rendering
- **Integration:** Claude's Discretion — choose the best KaTeX integration approach for Starlight
- **Density:** Moderate — key formulas in KaTeX, explanations in prose
- **Layout:** Sequential — math formula first, then REPL code block below showing verification

</decisions>

<specifics>
## Specific Ideas

- Landing page should feel academic/research-oriented, not startup/commercial
- Garvan's qseriesdoc.md (in the repo root) is the primary reference for tutorial content
- The existing MANUAL.md (in the repo root) contains all 50+ built-in function documentation
- Rogers-Ramanujan is the showcase example — it should appear on the landing page AND the tutorial

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 45-doc-content*
*Context gathered: 2026-02-27*
