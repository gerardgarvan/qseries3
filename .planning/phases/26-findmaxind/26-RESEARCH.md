# Phase 26: findmaxind — Research

**Researched:** 2026-02-26  
**Domain:** Find maximal linearly independent subset of q-series  
**Confidence:** HIGH

## Summary

`findmaxind(L, T)` returns a maximal independent subset of the q-series in list L, plus the list of indices. Uses linear algebra (RREF on transpose of coefficient matrix); pivot columns = maximal independent indices.

---

## Maple Reference

**Source:** [qseries.org/.../findmaxind.html](https://qseries.org/fgarvan/qmaple/qseries/functions/findmaxind.html)

| Item | Detail |
|------|--------|
| **Call** | findmaxind(XFL, T) |
| **Params** | XFL — list of q-series; T — nonnegative int (used in findhom) |
| **Return** | [P, NXFL] where P = maximal independent subset, NXFL = list of indices |

**Example:** Y := findmaxind(GL1, 0); Y[2] → [1, 2, 3, 4, 6, 7, 8] (indices 1-based; index 5 omitted = dependent)

**Algorithm:** Maximal linearly independent subset = pivot columns of RREF(M^T) where M has rows = series, cols = q-power coefficients. build_matrix(L, numCols) gives M. Transpose → columns = series. gauss_to_rref returns pivot column indices = maximal independent indices.

---

## Dependencies

| Dependency | Location | Purpose |
|------------|----------|---------|
| build_matrix | relations.h | M with rows = series coeffs |
| transpose | relations.h | M^T for column = series |
| gauss_to_rref | linalg.h | pivot column indices (need non-const or copy) |

gauss_to_rref modifies its argument; we pass a copy of M^T.

---

## API Design

**C++:** `findmaxind(L, topshift)` → struct with `std::vector<Series> subset` and `std::vector<int> indices` (0-based or 1-based; Maple uses 1-based).

**REPL:** findmaxind(L, topshift) — L = list of series, topshift = T parameter (usually 0).

**Return:** Pair (subset, indices) or struct FindmaxindResult { vector<Series> subset; vector<int> indices; }.
