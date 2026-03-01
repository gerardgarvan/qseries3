# Plan 08-02: solve, findhomcombo, findnonhomcombo — Summary

**Completed:** 2025-02-25

## Outcome

- **linalg.h:** Added `solve(M, b)` — solves M·x = b; returns optional solution or nullopt if inconsistent
- **gauss_to_rref:** Extended with `maxCols` parameter so augmented [M|b] only pivots on M columns
- **relations.h:** Added findhomcombo, findnonhomcombo using solve(M^T, f_coeffs)
- **TEST-08 (Watson):** findnonhomcombo(T², [T, ξ], [1,7], 0) yields Watson modular equation coeffs
- All five relation functions (findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly) implemented

## Files Modified

- src/linalg.h (solve, gauss_to_rref maxCols)
- src/relations.h (findhomcombo, findnonhomcombo, enumerate_nlist_exponents)
- src/main.cpp (solve tests, TEST-08)

## Notes

- n_list monomial order: (0,0),(0,1),..,(0,n2),(1,0),.. — first index slow
- etaoption stubbed (display deferred to REPL phase)
