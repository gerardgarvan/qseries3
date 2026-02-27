# Phase 10: repl — Context

**Gathered:** 2025-02-25
**Status:** Ready for research and planning

<domain>
## Phase Boundary

Full REPL with variable environment; evaluation of parsed expressions; built-in dispatch; series/coeffs display; set_trunc; REPL loop with history. All 9 SPEC acceptance tests must pass when run through the REPL.
</domain>

<decisions>
## Locked Decisions

### Area 1: Prompt and output format
- **Prompt:** `qseries> ` (exact)
- **Series display, coeffs, relations:** Follow Maple/Garvan (qseriesdoc) conventions. Use existing `Series::str()` for series; coeffs and relation output match Garvan package style as documented in qseriesdoc.

### Area 2: Line input and history
- **Line editing:** Plain `std::getline` (no termios; don't over-engineer)
- **History:** Last 100 commands, per SPEC
- **Multi-line:** Follow Maple conventions if practical; implementation discretion
- **Empty lines:** Do nothing (ignore)

### Area 3: Error display and recovery (implementation discretion)
- **Parse errors:** Include position/column when practical; otherwise short message
- **Eval errors:** Include context (e.g. function name) when available
- **Recovery:** REPL always continues to next prompt; never exit on user input error
- **Exception text:** Show message to user; may wrap for clarity

### Area 4: Startup banner
- **Banner:** ASCII kangaroo + short description of program (q-series REPL, Maple-like)
- **Version:** Use fixed string (e.g. "1.0") in banner
- **Timing, help:** Implementation discretion (SPEC mentions timing; help/help(func) deferred per REPL-08)
</decisions>

<spec_locked>
## SPEC (Phase 10)

- Variable environment: `std::map<std::string, Series>`
- `q` pre-defined as `Series::q(T)` with default truncation
- Default truncation 50; `set_trunc(N)` to change
- Built-in dispatch for aqprod, etaq, prodmake, etamake, jacprodmake, sift, findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly, series, coeffs, sum/add, etc.
- All 9 acceptance tests must pass via REPL
</spec_locked>

<deferred>
## Deferred Ideas

- help / help(func) — REPL-08
- Script mode — REPL-07
- Multi-line backslash: include if practical; else defer
</deferred>

---
*Phase: 10-repl*
*Context gathered: 2025-02-25*
