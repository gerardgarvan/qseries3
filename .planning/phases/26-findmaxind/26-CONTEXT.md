# Phase 26: findmaxind — Context

**Gathered:** 2026-02-26  
**Status:** Ready for planning

**Guiding principle:** Match Maple findmaxind; return maximal independent subset and indices.

<domain>
## Phase Boundary

findmaxind(L, topshift) — find maximal linearly independent subset of q-series in L. Uses RREF on transpose of coefficient matrix; pivot columns = maximal independent indices.
</domain>

<decisions>
## Implementation Decisions

### API
- **Signature:** findmaxind(L, topshift) — L = vector<Series>, topshift = int (usually 0)
- **Return:** struct FindmaxindResult { std::vector<Series> subset; std::vector<int> indices; } — indices 1-based to match Maple

### Algorithm
1. M = build_matrix(L, T+topshift)
2. MT = transpose(M)
3. Copy MT, run gauss_to_rref → pivot_cols
4. subset = [L[i] for i in pivot_cols]
5. indices = [i+1 for i in pivot_cols] (1-based for Maple compatibility)

### Placement
- relations.h (uses build_matrix, transpose; same domain as findhom)
- REPL: dispatch, display, help
</decisions>
