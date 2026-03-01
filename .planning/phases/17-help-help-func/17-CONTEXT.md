# Phase 17: help + help(func) - Context

**Gathered:** 2026-02-25
**Status:** Ready for planning

<domain>
## Phase Boundary

User can access built-in documentation. `help` shows general usage and list of built-ins. `help(func)` shows per-function docs. `help(unknown)` gives clear feedback.

</domain>

<decisions>
## Implementation Decisions

### General help layout
- **Single flat list** — All built-ins in one list (not grouped by category)
- **Detail per built-in** — Claude's discretion (name only, one-liner, or signature — choose what fits)

### help(func) content
- **Signature + one-line description** — e.g. `etaq(k,T) — eta product Π(1-q^{kn})`

### Unknown function response
- **Short message** — e.g. `unknown function: nonexistent`

### Coverage
- **All built-ins** — Document every function the REPL exposes (aqprod, etaq, theta2/3/4, prodmake, etamake, jacprodmake, qfactor, sift, findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly, subs_q, T, series, coeffs, set_trunc, sum, add, jac2prod, etc.)

### Claude's Discretion
- Detail level in `help` flat list (names, one-liners, or signatures)
</decisions>

<specifics>
## Specific Ideas

No specific references — follow qseriesdoc and REPL function signatures for descriptions.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope.

</deferred>

---
*Phase: 17-help-help-func*
*Context gathered: 2026-02-25*
