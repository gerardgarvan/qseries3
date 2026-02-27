# Phase 8: relations — Research

**Researched:** 2025-02-25  
**Domain:** Polynomial relation discovery between q-series (findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly)  
**Confidence:** HIGH

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions (SPEC)
- **findhom(L, n, topshift):** Homogeneous relations of degree n among L; matrix = monomial q-expansions; kernel of transpose.
- **findnonhom(L, n, topshift):** Nonhomogeneous relations degree ≤ n; include degree-0 monomial (constant 1).
- **findhomcombo(f, L, n, topshift, etaoption):** Express f as homogeneous polynomial in L; solve f = Σ c_i·monomial_i. etaoption → etamake for display.
- **findnonhomcombo(f, L, n_list, topshift, etaoption):** Express f as poly in L; n_list = max degree per variable.
- **findpoly(x, y, deg1, deg2, check):** Polynomial relation P(X,Y)=0 between two series.

### Claude's Discretion (Research to Decide)
- **Output format:** Representation (kernel vectors, structured polynomials, strings); display convention (X₁, X₂, etc.); variable mapping.
- **Truncation and topshift:** How T and topshift affect matrix size; defaults; relation to series truncation.
- **findhomcombo / findnonhomcombo:** Solve vs kernel; augment matrix approach; handling unique vs many solutions.
- **etaoption, n_list, edge cases:** etamake-on-fail behavior; n_list semantics; empty-result return value.

### Deferred Ideas (OUT OF SCOPE)
- REPL display formatting for relations — Phase 10
</user_constraints>

---

## Summary

Phase 8 implements five relation-finding functions that reduce to linear algebra over Q: build a matrix of q-expansion coefficients, then either compute the kernel (findhom, findnonhom, findpoly) or solve a linear system (findhomcombo, findnonhomcombo). Per qseriesdoc §4.1, the relation matrix has rows = monomial q-expansions; the **kernel of the transpose** yields homogeneous relations. For combo functions, we must **solve** M^T·c = f to express f as a polynomial in the monomials. Phase 7 (linalg) provides `kernel(M)` only; Phase 8 must add a solve capability (either in linalg or via augmented RREF in relations.h). Matrix size scales as C(n+k-1,k-1) for homogeneous degree n with k series — PITFALLS warn of blow-up (e.g. 126 monomials for k=6, n=4). Success criteria: TEST-05 (Gauss AGM), TEST-08 (Watson findnonhomcombo).

**Primary recommendation:** Build matrix rows = monomials, cols = q-powers; transpose before kernel for findhom/findnonhom/findpoly; add `solve(M, b)` (or equivalent) for findhomcombo/findnonhomcombo. Use T = min(series truncations) and add topshift to column count.

---

## Dependencies

| Dependency | Layer | Purpose |
|------------|-------|---------|
| linalg.h | Phase 7 | `kernel(M)` for relation discovery |
| series.h | Layer 3 | coeff, coeffList, truncTo, *, pow |
| frac.h | Layer 2 | Exact rationals |
| convert.h | Phase 6 | etamake (for etaoption display) |
| qfuncs.h | Phase 4 | etaq, theta3, theta4, subs_q, etc. |

**Critical gap:** linalg exposes only `kernel(M)`. findhomcombo/findnonhomcombo need to solve M^T·c = f. Either add `solve(M, b)` to linalg or implement solve locally via augmented matrix [M | b] → RREF → back-substitute.

---

## findhom

### Algorithm (qseriesdoc §4.1, SPEC §7.1)

1. **Input:** List L = [f₁,…,fₖ], degree n, topshift.
2. **Monomials:** Generate all monomials of total degree exactly n: f₁^{a₁}·…·fₖ^{aₖ} where a₁+…+aₖ = n. Count: C(n+k-1, k-1) (stars-and-bars).
3. **Matrix M:** Each row = one monomial’s q-expansion [coeff q⁰, coeff q¹, …, coeff q^{T+topshift-1}]. Rows = monomials, cols = q-powers.
4. **Relation extraction:** Compute kernel of M^T. Each kernel vector v (length = num_monomials) satisfies Σ v_i · monomial_i = 0.
5. **Output:** One relation per kernel basis vector.

### Matrix orientation

qseriesdoc: "kernel of the transpose of this matrix." With M = rows monomials, cols q-powers:
- Relation: Σ c_i · monomial_i = 0.
- For each q-power j: Σ c_i · M[i][j] = 0 ⇒ (M^T · c)[j] = 0.
- So kernel(M^T) gives coefficient vectors c. **Pass transpose of M to kernel().**

### Monomial generation (stars-and-bars)

Enumerate (a₁,…,aₖ) with a_i ≥ 0 and Σ a_i = n. Iterate over combinations with replacement. C++: recursion or iterative enumeration; standard combinatorics.

---

## findnonhom

### Algorithm (qseriesdoc §4.3, SPEC §7.3)

Same as findhom except:
- Include **all** monomials of degree 0 through n (not just n).
- Degree 0 = constant 1.
- Monomial count: Σ_{d=0}^{n} C(d+k-1, k-1).

Build matrix M with these monomials as rows; kernel(M^T) gives relations.

---

## findhomcombo

### Algorithm (qseriesdoc §4.2, SPEC §7.2)

Find c such that f = Σ c_i · monomial_i (homogeneous degree n). This is solving M^T · c = f, where:
- M: rows = monomials, cols = q-powers.
- f: column vector of f’s q-coefficients (same length as num cols).

### Solve vs kernel

- findhom: kernel → many relations.
- findhomcombo: **solve** → one (or a particular) solution. Different operation.

### Implementation options

1. **Add solve(M, b) to linalg:** Solves M·x = b; returns optional vector x if consistent. Implement via [M | b] RREF, check consistency (no pivot in last column), back-substitute.
2. **Implement in relations.h:** Build augmented [M^T | f], call internal RREF (linalg would need to expose gauss_to_rref or similar), extract solution. Alternatively, add solve as a small extension in linalg.

**Recommendation:** Add `solve(M, b)` (or `std::optional<std::vector<Frac>> solve(...)`) to linalg. Reuse RREF logic; plan a small Phase 7 addendum or include solve in Phase 8 with minimal linalg changes. If solve is not added, relations must implement augmented RREF locally — possible but duplicated logic.

### Multiple solutions

If the system is underdetermined, many solutions exist. Return one particular solution (e.g. from RREF with free vars = 0). Garvan findhomcombo returns one representation.

---

## findnonhomcombo

### Algorithm (qseriesdoc §4.4, SPEC §7.4)

Express f as polynomial in L with **per-variable degree bounds** n_list = [n₁,…,nₖ]. Monomials: f₁^{d₁}·…·fₖ^{dₖ} where 0 ≤ d_i ≤ n_list[i]. Total count: (n_list[0]+1)·…·(n_list[k-1]+1).

### n_list semantics

- findhomcombo: single n → all monomials of total degree n.
- findnonhomcombo: n_list[i] = max exponent of variable i. So monomials span a rectangle, not a simplex.

Example (TEST-08): n_list = [1, 7] → X₁^0..1, X₂^0..7 → 2·8 = 16 monomials. Order: e.g. (0,0), (1,0), (0,1), (1,1), …, (1,7).

### Same solve step

Build M with rows = these monomials, solve M^T · c = f.

---

## findpoly

### Algorithm (qseriesdoc §4.5, SPEC §7.5)

Two series x, y. Monomials: X^i · Y^j for i ≤ deg1, j ≤ deg2. Count: (deg1+1)·(deg2+1). Build matrix, kernel(M^T) gives polynomial relations P(X,Y) = 0. The `check` parameter: if provided, verify the relation to O(q^check).

---

## Truncation and topshift

### Truncation T

- Use min of all input series truncations: T = min{f.trunc : f in L} (and f for combo).
- Safety: optionally subtract a small margin (e.g. T-2) to avoid boundary effects; SPEC does not mandate this.
- **Recommendation:** T = min(series truncations) without margin for Phase 8; document that user should use sufficient truncation.

### topshift

qseriesdoc: "If spurious relations appear then a higher value of topshift should be taken." Interpretation:
- Matrix columns = q-coefficients for q^0, q^1, …, q^{T-1+topshift} (or T+topshift).
- More columns → more constraints → fewer spurious kernel vectors.
- **Recommendation:** num_cols = T + topshift. Default topshift = 0. So columns 0..T-1 with topshift=0; 0..T+topshift-1 with topshift>0.

---

## Output format

### Internal representation

- **findhom / findnonhom / findpoly:** Return `std::vector<std::vector<Frac>>` — each vector is a relation (coefficients for monomials).
- **findhomcombo / findnonhomcombo:** Return `std::optional<std::vector<Frac>>` — one solution vector, or empty if no solution.

### Display convention (qseriesdoc)

- Use X₁, X₂, … for L[0], L[1], ….
- Format as "X₁² + X₂² - 2X₃²" (integers when coeff is integer).
- Helper: `formatRelation(coeffs, monomials, varNames) -> string`.

### Variable order

- findhom/findnonhom: X_i = L[i-1].
- findnonhomcombo: same.
- findpoly: X = first series, Y = second.

---

## etaoption

qseriesdoc §4.2: "If etaoption=yes then each monomial in the combination is converted into an eta-product using etamake."

- When true: for each monomial in the displayed relation, call etamake(monomial_series, T). If successful, show eta form; if not, show raw or X_i form.
- **Recommendation:** Best-effort: try etamake; on failure (empty result) show monomial as "X₁^a X₂^b" etc. Not critical for correctness.

---

## n_list

For findnonhomcombo: n_list = [n₁,…,nₖ] with n_i = max degree of variable i. Generate all (d₁,…,dₖ) with 0 ≤ d_i ≤ n_list[i]. Total monomials = Π (n_list[i] + 1).

---

## TEST-05: Gauss AGM (qseriesdoc Output 18)

```
findhom([theta3(q,100), theta4(q,100), theta3(q^2,100), theta4(q^2,100)], 2, 0)
```

**Expected:** Two relations:
1. X₁² + X₂² - 2X₃²  
2. -X₁X₂ + X₄²  

Variable mapping: X1=θ₃(q), X2=θ₄(q), X3=θ₃(q²), X4=θ₄(q²). Implement theta3(q^2) as theta3(q,T).subs_q(2) (or theta3 with q² as base). Truncation 100 for all.

---

## TEST-08: Watson findnonhomcombo (qseriesdoc Output 24)

```
xi := q^2 * etaq(49)/etaq(1)   -- series
T  := q * (etaq(7)/etaq(1))^4  -- series
findnonhomcombo(T^2, [T, xi], [1, 7], 0)
```

**Expected:** T² = (49ξ³ + 35ξ² + 7ξ)T + 343ξ⁷ + 343ξ⁶ + 147ξ⁵ + 49ξ⁴ + 21ξ³ + 7ξ² + ξ

Variable order: X1 = T, X2 = ξ. n_list = [1,7] → monomials T^0..1, ξ^0..7.

---

## Pitfalls

### 1. Matrix size C(n+k-1, k-1)

**What goes wrong:** k=6, n=4 → 126 monomials; matrix 126 × (T+topshift). Kernel and solve can be slow and memory-heavy.

**Prevention:** Document scaling; consider degree/list caps for interactive use. No hard cap required for SPEC tests.

### 2. Kernel of transpose

**What goes wrong:** Passing M (rows=monomials) directly to kernel gives wrong interpretation. Must pass M^T so that kernel vectors index monomials.

**Prevention:** Build M with rows=monomials; transpose (swap rows/cols) before calling kernel, or build directly as rows=q-powers, cols=monomials and call kernel on that.

### 3. findhomcombo needs solve, not kernel

**What goes wrong:** Using kernel of [M | f] gives relations that include f; the desired output is one solution to M^T·c = f.

**Prevention:** Implement explicit solve for M^T·c = f (augment [M^T | f], RREF, back-substitute).

### 4. n_list vs single n

**What goes wrong:** Confusing findnonhomcombo’s n_list (per-variable max) with findhom’s n (total degree).

**Prevention:** n_list = [n1,…,nk] → monomials with exponent of var i ≤ n_list[i]. Different enumeration than stars-and-bars.

### 5. Truncation consistency

**What goes wrong:** Using different T for different series or for f vs monomials can cause inconsistent matrix dimensions.

**Prevention:** Use single T = min(all truncations); same column range for all rows.

---

## Code structure sketch

```cpp
// relations.h
std::vector<std::vector<Frac>> findhom(const std::vector<Series>& L, int n, int topshift);
std::vector<std::vector<Frac>> findnonhom(const std::vector<Series>& L, int n, int topshift);
std::optional<std::vector<Frac>> findhomcombo(const Series& f, const std::vector<Series>& L, int n, int topshift, bool etaoption);
std::optional<std::vector<Frac>> findnonhomcombo(const Series& f, const std::vector<Series>& L, const std::vector<int>& n_list, int topshift, bool etaoption);
std::vector<std::vector<Frac>> findpoly(const Series& x, const Series& y, int deg1, int deg2, std::optional<int> check);

// Helper: generate monomials (for findhom: total deg n; findnonhom: deg 0..n; findnonhomcombo: n_list bounds; findpoly: deg1×deg2)
// Helper: build matrix rows from monomial q-expansions
// Helper: format relation vector as "X1^2+X2^2-2*X3^2"
```

---

## RESEARCH COMPLETE
