# Phase 7: linalg — Research

**Researched:** 2025-02-25
**Domain:** Linear algebra over Q (Gaussian elimination, kernel computation)
**Confidence:** HIGH

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **kernel(M):** `std::vector<std::vector<Frac>> kernel(std::vector<std::vector<Frac>>& M);`
- M is a vector of **rows**, each row is `vector<Frac>`
- Returns a **basis** for the kernel (null space)
- Standard row-reduction over exact rationals; choose nonzero pivot in each column

### Claude's Discretion
- API shape: Whether to expose a separate `gauss_elim` or only `kernel`; pass-by-value vs reference; in-place modification of M vs copy
- Kernel basis representation: Normalization of basis vectors; ordering; handling of trivial kernel (empty vs explicit)
- Edge cases: Empty matrix, zero rows/columns, all-zero matrix, full-rank matrix — return empty or sensible defaults; avoid throws for valid inputs
- Pivot selection: First nonzero vs partial pivoting; row swaps; zero-row handling

### Deferred Ideas (OUT OF SCOPE)
- Explicit `solve` for linear systems (not needed for relations; kernel suffices)
</user_constraints>

---

## Summary

Phase 7 implements linear algebra over Q for relation finding. The sole required interface is `kernel(M)`, which returns a basis for the null space (right kernel) of matrix M. The downstream consumer (Phase 8 relations.h) uses this for findhom, findnonhom, etc. Per qseriesdoc §4.1, findhom builds a matrix of monomial q-expansions (rows = monomials, cols = q-powers) and computes the **kernel of the transpose** to get relations. Relations.h will handle transposition; linalg provides `kernel(M)` for a matrix given as rows.

Implementation reduces to: (1) Gaussian elimination to RREF over exact rationals, (2) kernel extraction from RREF by identifying free columns and back-substitution. Exact arithmetic avoids numerical pivoting heuristics; first nonzero pivot per column suffices. PITFALLS warn that findhom matrices can reach C(n+k-1,k-1) monomials (e.g. 126 for k=6, n=4), so kernel must handle matrices of that size without fragility.

**Primary recommendation:** Implement `kernel(M)` by copying M, reducing to RREF (first nonzero pivot per column, zero rows sink to bottom), then for each free column build one kernel vector via back-substitution. Do not modify the caller's M.

---

## Dependencies

| Dependency | Layer | Purpose |
|------------|-------|---------|
| frac.h | Layer 2 | Frac for exact rational arithmetic, row operations |
| bigint.h | Layer 1 | Via Frac; BigInt for numerators/denominators |

**No Series, no convert.** linalg is independent of q-series logic. All operations use `Frac`: `+`, `-`, `*`, `/`, `isZero()`.

---

## Gaussian Elimination

### Algorithm (RREF over Q)

1. **Input:** Matrix `A` (rows × cols), stored as `vector<vector<Frac>>` with rows as outer vector.
2. **Work on a copy** — do not mutate the caller's matrix.
3. **Pivot columns:** Scan columns left-to-right. For each column `c`:
   - Find the first row `r >= last_pivot_row` with `A[r][c] != 0`.
   - If none, column is free; continue to next column.
   - Otherwise: swap row `r` to pivot position, scale pivot to 1, eliminate above and below.
4. **Row operations:** For each pivot `(r,c)`:
   - Scale row `r` by `1/A[r][c]` so pivot becomes 1.
   - For all other rows `i`: subtract `A[i][c] * row_r` from row `i`.
   - Zero rows sink to bottom by swapping (or by processing pivots top-to-bottom so they stay ordered).
5. **Output:** RREF matrix; list of pivot column indices.

### Pivot Selection

- **First nonzero per column** is sufficient for exact rationals. No partial pivoting needed (no numerical instability).
- Row swaps are acceptable and standard.
- Track `pivot_cols`: `vector<int>` of column indices that have pivots.

### Rationale

Exact arithmetic preserves correctness. Scaling by `1/pivot` and elimination produce exact Frac results. RREF is unique for a given matrix over a field (Q).

---

## Kernel Algorithm

### From RREF to Kernel Basis

1. Let `ncols = M[0].size()` (or 0 if empty).
2. **Pivot vs free columns:** Columns not in `pivot_cols` are free.
3. **For each free column `j`:**
   - Construct kernel vector `v` of length `ncols`, initially zero.
   - Set `v[j] = 1`.
   - For each pivot column `c` (in order), back-substitute: `v[c] = -Σ_{free cols k > c} A[row_of_pivot(c)][k] * v[k]` — but since we set one free var at a time, simplify: for the free column `j`, the pivot variables are determined by the RREF row containing that pivot. Standard procedure:
     - For each pivot row `i` with pivot in column `piv[i]`, the equation is: `x[piv[i]] + Σ_{j free} A[i][j]*x[j] = 0`. With `x[j]=1` for our chosen free `j` and `x[k]=0` for other free `k`, we get `x[piv[i]] = -A[i][j]`.
   - Fill `v` accordingly: `v[j]=1`, for each pivot column `c` set `v[c]` from the corresponding row.
4. Append `v` to the basis.
5. **Return** `vector<vector<Frac>>` of kernel basis vectors.

### Number of Kernel Vectors

- `nullity = ncols - rank`. Each free column yields one basis vector.
- Full rank → no free columns → return `{}`.
- All-zero matrix → all columns free → return `ncols` vectors (standard basis).

---

## API

### Required (Locked)

```cpp
std::vector<std::vector<Frac>> kernel(std::vector<std::vector<Frac>>& M);
```

- `M`: matrix as rows; `M[i]` is row `i`; all rows must have same length (caller's responsibility).
- Returns: basis for the null space (right kernel), i.e. `{v : M*v = 0}`. Each vector has length `ncols`.

### Recommended

- **Pass `M` by const reference** if we don't need to modify it. The SPEC shows `& M`; if we copy internally, `const &` is cleaner. Clarify: SPEC says `kernel(std::vector<std::vector<Frac>>& M)` — use that signature; implementation copies M internally and never modifies the argument.
- **Do not expose `gauss_elim`** for Phase 7 unless the planner chooses to. `kernel` alone suffices for relations. If needed later, `gauss_elim` can be added without breaking API.
- **Return type:** `vector<vector<Frac>>`. Each inner vector is one kernel basis vector of length `ncols`.

---

## Edge Cases

| Case | Behavior |
|------|----------|
| Empty matrix `{}` | Return `{}` (no columns → no kernel vectors of length 0 needed; empty basis). |
| Zero rows `{ {}, {} }` | Treat as 0×0; return `{}`. |
| Zero columns (rows exist but `M[0].size()==0`) | Return `{}`. |
| All-zero matrix | `ncols` free columns → return `ncols` canonical basis vectors `e_0, ..., e_{n-1}`. |
| Full rank | No free columns → return `{}`. |
| Ragged rows | Undefined; caller must ensure rectangular. Avoid crashing: can check row lengths and return `{}` or document as UB. |
| Single row/column | Handle normally; e.g. 1×3 with one relation → one pivot, two free → two kernel vectors. |

### All-Zero Matrix Detail

For an `m × n` all-zero matrix, RREF is all zeros. Every column is free. Kernel = all of Q^n. A basis is `{e_0, e_1, ..., e_{n-1}}`. Return these `n` vectors.

### Empty Matrix

`M = {}` → no rows. Convention: no columns either. Return `{}`. If `M = {{}}` (one empty row), then `ncols = 0`, return `{}`.

---

## Testing

### Unit Tests (Independent of relations)

1. **2×3 matrix with one relation:** e.g. `[[1,1,0],[2,2,0]]` → RREF has pivot in col 0; cols 1,2 free. Kernel has dimension 2. Verify two linearly independent vectors.
2. **Full rank:** `[[1,0],[0,1]]` → kernel = `{}`.
3. **All-zero 2×3:** kernel has 3 vectors = canonical basis.
4. **Single relation:** `[[1,2,3]]` — one row, three columns. One pivot, two free. Kernel dimension 2.
5. **Rational coefficients:** Use non-integer Fracs to ensure no accidental int overflow.

### Sanity Check

- For each returned vector `v`, verify `M * v = 0` (matrix-vector product).
- Verify vectors are linearly independent (e.g. form a matrix and check rank = number of vectors).

### Relations Downstream

Phase 8 will test via findhom. Per qseriesdoc, `findhom([theta3, theta4, theta3(q^2), theta4(q^2)], 2, 0)` should yield two relations. That validates kernel indirectly; Phase 7 tests should focus on kernel correctness in isolation.

---

## References

### Primary (HIGH confidence)

- **SPEC.md §Layer 7** — `kernel(M)` API, row-reduction over exact rationals
- **qseriesdoc.md §4.1** — "The set of relations is found by computing the kernel of the transpose of this matrix"
- **SPEC PITFALLS** — findhom matrix size C(n+k-1,k-1); kernel must handle large matrices

### Secondary (Standard algorithms)

- Gaussian elimination over fields: first nonzero pivot, row swaps, scale to 1, eliminate
- Kernel from RREF: one basis vector per free column via back-substitution
- Wikipedia / MIT Linear Algebra (Strang): RREF preserves null space; special solutions from free columns

---

## Common Pitfalls

### Pitfall 1: Modifying Caller's Matrix
**What goes wrong:** Caller passes M, expects it unchanged; kernel mutates it.
**How to avoid:** Copy M at entry; all reduction on the copy.

### Pitfall 2: Wrong Free-Variable Assignment
**What goes wrong:** For each free column j, set x[j]=1 and x[other free]=0. Getting the sign or index wrong yields wrong kernel vectors.
**How to avoid:** Use standard back-substitution from RREF; each pivot row expresses pivot variable in terms of free variables.

### Pitfall 3: Ragged Rows
**What goes wrong:** Rows of different lengths cause out-of-bounds access.
**How to avoid:** Either assume rectangular (document) or add a guard: if rows differ in length, return `{}` or handle gracefully.

### Pitfall 4: Frac Denom Growth
**What goes wrong:** Repeated scaling without reduce can grow denominators.
**How to avoid:** Frac::reduce() is called in Frac constructors/operations. No extra action needed if using Frac ops; avoid constructing Frac without going through reduce.

---

## Code Example (Pseudocode)

```cpp
// Kernel extraction (conceptual)
std::vector<std::vector<Frac>> kernel(std::vector<std::vector<Frac>>& M) {
    if (M.empty() || M[0].empty()) return {};
    auto A = M;  // copy
    std::vector<int> pivot_cols = gauss_to_rref(A);  // returns pivot column indices
    int n = (int)A[0].size();
    std::vector<int> free_cols;
    for (int j = 0; j < n; j++)
        if (find(pivot_cols.begin(), pivot_cols.end(), j) == pivot_cols.end())
            free_cols.push_back(j);

    std::vector<std::vector<Frac>> basis;
    for (int j : free_cols) {
        std::vector<Frac> v(n, Frac(0));
        v[j] = Frac(1);
        for (int pi = 0; pi < (int)pivot_cols.size(); pi++) {
            int pc = pivot_cols[pi];
            Frac sum(0);
            for (int fc : free_cols)
                if (fc <= j)  // only free cols up to j in our construction
                    sum = sum - A[pi][fc] * v[fc];  // back-sub: x[pc] = -sum
            v[pc] = sum;  // A[pi][pc]=1 at pivot
        }
        basis.push_back(std::move(v));
    }
    return basis;
}
```

(Actual implementation must correctly map pivot rows to pivot columns and perform back-substitution according to RREF structure.)

---

## RESEARCH COMPLETE
