# Phase 44: Astro Starlight Scaffold - Context

**Gathered:** 2026-02-27
**Status:** Ready for planning

<domain>
## Phase Boundary

Scaffold an Astro Starlight documentation site in `website/`. The site must have sidebar navigation, dark mode, responsive layout, Pagefind search, and placeholder pages for all documentation sections. No actual content — Phase 45 handles that. No playground integration — Phase 46 handles that.

</domain>

<decisions>
## Implementation Decisions

### Site Structure & Navigation
- Claude's Discretion — organize sidebar into logical groups based on the project's documentation needs
- Expected sections: Getting Started, Manual/Reference, Tutorial, Playground
- Sidebar should reflect the structure a mathematician/researcher would expect from a CAS documentation site

### Visual Identity
- Claude's Discretion — use Starlight's built-in theming with a color scheme appropriate for a math/computation tool
- Dark mode as default (dev tool aesthetic, as specified in milestone)
- No custom logo required for MVP — text-based site title is fine
- Monospace font emphasis for code/math content areas

### Page Layout
- Claude's Discretion — standard Starlight landing page with hero section
- Hero should convey: what qseries does, that it's free/open, and link to playground
- Placeholder pages should have realistic titles and brief "content coming in Phase 45" notes
- Playground page placeholder should indicate "interactive REPL coming in Phase 46"

### Site Metadata
- Claude's Discretion — title "qseries", appropriate tagline about q-series computation
- Standard SEO meta tags
- GitHub repository link in header/footer

</decisions>

<specifics>
## Specific Ideas

No specific requirements — open to standard Starlight approaches. The milestone description calls for a "modern dev-tool aesthetic" which Starlight provides out of the box with dark mode.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 44-starlight-scaffold*
*Context gathered: 2026-02-27*
