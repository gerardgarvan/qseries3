# Plan 07-01 Summary

**Completed:** 2025-02-24
**Phase:** 07-linalg

## Deliverables

- **linalg.h:** `kernel(M)` returns basis for null space; internal `gauss_to_rref` reduces to RREF
- **main.cpp:** linalg test group with 4 cases and M*v=0 verification

## Verification

- 2×3 [[1,1,0],[2,2,0]] → kernel dim 2, M*v=0 for both
- Full rank [[1,0],[0,1]] → kernel empty
- All-zero 2×3 → kernel = 3 canonical basis vectors
- Single row [[1,2,3]] → kernel dim 2, M*v=0
