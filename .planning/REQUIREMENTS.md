# Requirements: qseries3 — v11.4 Tech Debt

**Defined:** 2026-03-06
**Core Value:** Rogers-Ramanujan must work; prodmake recovers product form with denominators at exponents ≡ ±1 (mod 5)
**Milestone:** v11.4 — Tech debt remediation (from TECH_DEBT.md)

## v11.4 Requirements

Requirements for milestone v11.4. Each maps to roadmap phases. Source: TECH_DEBT.md.

### Built-in Registration

- [ ] **TD-01**: Built-ins registered in a data structure (name → handler); dispatch uses lookup, not if-chain
- [ ] **TD-02**: Single source of truth for built-in names and arity; getHelpTable derives from registration table

### Duplication Reduction

- [ ] **TD-03**: expectArg/ev/evi helpers — built-ins use shared arg-check and evaluation helpers
- [ ] **TD-04**: TTY branching centralized — runRepl uses abstraction for stdin_is_tty() branches where feasible

### Brittle Guards

- [ ] **TD-05**: Guard env.at("q") — clear error if q unset before dispatch; no implicit throw
- [ ] **TD-06**: Named constants for magic numbers — pow limit, maxHistory, maxContinuations, Levenshtein threshold

## v11.4 Deferred (Future Milestone)

| Feature | Reason |
|---------|--------|
| Split repl.h into smaller headers | High risk; registration table reduces hotspot impact first |
| Refactor mock.h mockqs_term to table | Lower priority; mock theta is self-contained |
| formatProdmake/formatEtamake unification | Minor duplication; defer |
| parseParserMessage robustness | Parser output format stable; low ROI |
| Platform #ifdef abstraction | Cross-platform; high risk for little gain |

## Out of Scope

| Feature | Reason |
|---------|--------|
| Reimplement in another language | C++20, zero-deps is non-negotiable |
| Add external dependencies | Zero-deps constraint |
| Coverage/tidy tooling | v11.3 deferred; not this milestone |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| TD-01 | Phase 116 | Pending |
| TD-02 | Phase 116 | Pending |
| TD-03 | Phase 117 | Pending |
| TD-04 | Phase 117 | Pending |
| TD-05 | Phase 118 | Pending |
| TD-06 | Phase 118 | Pending |

**Coverage:**
- v11.4 requirements: 6 total
- Mapped to phases: 6
- Unmapped: 0

---
*Requirements defined: 2026-03-06*
*Last updated: 2026-03-06 after v11.4 milestone start*
