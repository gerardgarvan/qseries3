# Phase 8: relations — Context

**Gathered:** 2025-02-24
**Status:** Ready for research and planning

**User note:** Implementation decisions deferred to research; user does not have domain knowledge to decide.

<domain>
## Phase Boundary

Polynomial relation discovery between q-series. Per ROADMAP: findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly. Success: TEST-05 (findhom Gauss AGM), TEST-08 (Watson via findnonhomcombo), plus findnonhom, findhomcombo, findpoly per SPEC.
</domain>

<spec_locked>
## SPEC (Locked)

- **findhom(L, n, topshift):** Homogeneous relations of degree n among L; matrix = monomial q-expansions; kernel of transpose.
- **findnonhom(L, n, topshift):** Nonhomogeneous relations degree ≤ n; include degree-0 monomial (constant 1).
- **findhomcombo(f, L, n, topshift, etaoption):** Express f as homogeneous polynomial in L; solve f = Σ c_i·monomial_i. etaoption → etamake for display.
- **findnonhomcombo(f, L, n_list, topshift, etaoption):** Express f as poly in L; n_list = max degree per variable.
- **findpoly(x, y, deg1, deg2, check):** Polynomial relation P(X,Y)=0 between two series.
</spec_locked>

<implementation_discretion>
## Implementation Discretion (Research to Decide)

All gray areas delegated to researcher and planner:

- **Output format:** Representation (kernel vectors, structured polynomials, strings); display convention (X₁, X₂, etc.); variable mapping.
- **Truncation and topshift:** How T and topshift affect matrix size; defaults; relation to series truncation.
- **findhomcombo / findnonhomcombo:** Solve vs kernel; augment matrix approach; handling unique vs many solutions.
- **etaoption, n_list, edge cases:** etamake-on-fail behavior; n_list semantics; empty-result return value.

**References:** SPEC §Layer 7; qseriesdoc §4.1–4.4; Garvan findhom/findhomcombo/findnonhom/findnonhomcombo; PITFALLS (findhom matrix size C(n+k-1,k-1)).
</implementation_discretion>

<deferred>
## Deferred Ideas

- REPL display formatting for relations — Phase 10
</deferred>

---
*Phase: 08-relations*
*Context gathered: 2025-02-24*
