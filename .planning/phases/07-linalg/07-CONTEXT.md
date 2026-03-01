# Phase 7: linalg — Context

**Gathered:** 2025-02-24
**Status:** Ready for planning

**User note:** Implementation decisions left to standard practice; user deferred to implementation discretion.

<domain>
## Phase Boundary

Linear algebra over Q for relation finding. Per ROADMAP: Gaussian elimination over `vector<vector<Frac>>`; kernel computation (null space of rational matrices). Success: Gaussian elimination returns correct reduced form; kernel returns basis for null space. Downstream consumer: Phase 8 relations.h (findhom, findnonhom, etc.).
</domain>

<spec_locked>
## SPEC (Locked)

- **kernel(M):** `std::vector<std::vector<Frac>> kernel(std::vector<std::vector<Frac>>& M);`
- M is a vector of **rows**, each row is `vector<Frac>`
- Returns a **basis** for the kernel (null space)
- Standard row-reduction over exact rationals; choose nonzero pivot in each column
</spec_locked>

<implementation_discretion>
## Implementation Discretion

The following are delegated to the planner/implementer using **standard practice** — no user decisions required:

- **API shape:** Whether to expose a separate `gauss_elim` or only `kernel`; pass-by-value vs reference; in-place modification of M vs copy.
- **Kernel basis representation:** Normalization of basis vectors; ordering; handling of trivial kernel (empty vs explicit).
- **Edge cases:** Empty matrix, zero rows/columns, all-zero matrix, full-rank matrix — return empty or sensible defaults; avoid throws for valid inputs.
- **Pivot selection:** First nonzero vs partial pivoting; row swaps; zero-row handling.

**References:** SPEC §Layer 7; PITFALLS (kernel matrix size scaling); qseriesdoc (relations use kernel of transpose).
</implementation_discretion>

<deferred>
## Deferred Ideas

- Explicit `solve` for linear systems (not needed for relations; kernel suffices)
</deferred>

---
*Phase: 07-linalg*
*Context gathered: 2025-02-24*
