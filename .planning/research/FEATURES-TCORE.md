# Feature Research: t-Core Partition Package

**Domain:** q-series REPL — t-core partition functions (Garvan's Maple `tcore` package v0.2)
**Researched:** 2026-03-02
**Sources:** Maple `tcore` package source (t-core.txt), Garvan-Kim-Stanton "Cranks and t-cores" (Invent. Math. 1990), qseriesdoc.md, partition theory literature

---

## 1. Mathematical Background

### What is a t-core?

A partition λ is a **t-core** if its Young diagram contains no hook of length t (equivalently: no hook length divisible by t). The t-core of an arbitrary partition λ is obtained by repeatedly removing rim t-hooks until none remain — the result is independent of removal order.

### The Littlewood Decomposition

Every partition λ has a unique decomposition into:
- **t-core:** core_t(λ) — a t-core partition
- **t-quotient:** quot_t(λ) = (λ⁰, λ¹, ..., λᵗ⁻¹) — a tuple of t partitions

**Fundamental identity:**

    |λ| = |core_t(λ)| + t × Σᵢ |λⁱ|

This identity MUST hold for every partition. It is the primary correctness check.

### Generating Function

The generating function for the number of t-core partitions of n is:

    Σ c_t(n) qⁿ = (qᵗ; qᵗ)_∞ᵗ / (q; q)_∞ = etaq(t)^t / etaq(1)

This connects t-cores directly to existing REPL infrastructure (eta products).

---

## 2. Function Classification: Core vs Internal

### User-Facing Functions (must expose in REPL)

| Function | Signature | Purpose | Priority |
|----------|-----------|---------|----------|
| `tcoreofptn` | `tcoreofptn(ptn, t)` | Compute t-core of partition | **Critical** |
| `istcore` | `istcore(ptn, t)` | Test if partition is a t-core | **Critical** |
| `tcores` | `tcores(t, n)` | List all t-cores of n | **High** |
| `tquot` | `tquot(ptn, t)` | Compute t-quotient | **Critical** |
| `PHI1` | `phi1(ptn, t)` | [t-core, t-quotient] pair | **High** |
| `invphi1` | `invphi1([core, quotient], t)` | Reconstruct partition from core+quotient | **High** |
| `ptn2nvec` | `ptn2nvec(ptn, t)` | Compute n-vector | **Medium** |
| `nvec2ptn` | `nvec2ptn(nvec)` | Reconstruct t-core from n-vector | **Medium** |
| `ptn2rvec` | `ptn2rvec(ptn, t)` | Compute r-vector | **Medium** |
| `tcrank` | `tcrank(ptn, t)` | t-core crank statistic | **High** |
| `tresdiag` | `tresdiag(ptn, t)` | Print t-residue diagram | **Medium** |

### Internal Helper Functions (not user-facing)

| Function | Used by | Purpose |
|----------|---------|---------|
| `rvec(ptn, t, k)` | `ptn2rvec`, `ptn2nvec`, `istcore`, `tcoreofptn` | Count residue-k nodes in t-residue diagram |
| `addrimthook(ptn, j, L, t)` | `invphi1` | Add rim t-hook starting at part j |
| `findhookinpos(ptn, t, w, p)` | `invphi1` | Find position for rim hook insertion |
| `nvec2alphavec(nvec)` | Specialized (t=5, t=7 only) | Alpha-vector from n-vector |
| `makebiw(ptn, t, m)` | Debugging/visualization | Print bi-infinite words |
| `np(ptn)` | Everywhere | Number of parts (trivial) |

### Omit Entirely

| Function | Reason |
|----------|--------|
| `avec2nvec` | Only t=5 implemented; very specialized |
| `aveccyc` | Cyclic permutation helper for alpha-vectors |
| `randpcore` | Random generation; testing utility only |
| `addrimcell` | Young diagram cell-level manipulation |
| `markrimhookV2` | Diagram marking; visualization only |
| `removerimhook` | Diagram manipulation; not needed for core computation |
| `darray2ptn` | Diagram array conversion; internal to Maple |
| `printdarray` | Maple-specific display |
| `findcell` | Maple-specific diagram search |
| `freqtab` | Generic frequency table; not partition-specific |
| `nep`, `nepo`, `numnepo` | Even-part counting; tangential utilities |

---

## 3. Algorithm Details

### 3.1 rvec(ptn, t, k) — The Foundation

Everything builds on this. Counts nodes colored k in the t-residue diagram.

**Algorithm:** For partition λ = [λ₁, λ₂, ..., λₘ] (parts in non-decreasing order, Maple convention):

    rvec(λ, t, k) = Σⱼ₌₁ᵐ floor((λⱼ + ((t - (m-j+1) - k) mod t)) / t)

The Maple source iterates j from 1 to m (number of parts), with index reversal `j = nn - m + 1` to convert between row-from-top and row-from-bottom.

**Key detail from Maple source:**
```
for m from 1 to nn do
    j := nn - m + 1
    x := x + trunc((ptn[j] + modp(p - m - k, p)) / p)
```

Here `m` counts from the bottom row (largest part) upward. `ptn[j]` accesses parts in the original (non-decreasing) order. The residue offset `(t - m - k) mod t` accounts for the row position in the t-residue diagram.

**Edge cases:**
- Empty partition `[]`: rvec = 0 for all k
- Single part `[n]`: rvec(k) = floor((n + (t-1-k) mod t) / t)

### 3.2 istcore(ptn, t) — t-Core Test

**Algorithm:** Compute all r-values R[0], R[1], ..., R[t-1] using rvec, then check:

    Σᵢ₌₀ᵗ⁻¹ (R[i]² - R[i]·R[i+1 mod t]) = R[0]

If this equality holds, the partition is a t-core.

**Mathematical basis:** For a t-core, the beta-set decomposes into t arithmetic progressions. The r-vector encodes their lengths. The quadratic form above equals the size contribution from the residue-class structure, which equals R[0] precisely when no rim t-hooks exist.

**Edge cases:**
- Every partition is a 1-core (t=1)
- The empty partition `[]` is a t-core for all t
- `[1]` is a t-core for all t ≥ 2

### 3.3 tcoreofptn(ptn, t) — Compute t-Core

**Algorithm:**
1. Compute the n-vector: `nvec[k] = rvec(ptn, t, k) - rvec(ptn, t, k+1)` for k = 0, ..., t-1 (indices mod t)
2. Reconstruct the t-core via `nvec2ptn(nvec)`

This bypasses iterative rim-hook removal entirely, making it O(m·t) where m = number of parts.

**Why this works:** The n-vector uniquely determines a t-core partition. When you compute the n-vector of any partition, the rim-hook content doesn't affect it — only the core structure contributes. This is because adding a rim t-hook changes all r-values by the same amount, so their differences (the n-vector) remain invariant.

### 3.4 nvec2ptn(nvec) — Reconstruct t-Core from n-Vector

**Algorithm (Frobenius coordinates):**

Let t = len(nvec). Compute X = Σ max(nvec[j], 0) (the Durfee square size).

Positive parts of Frobenius representation:
```
ppartsA = sorted list of {t*(k-1) + i : i where nvec[i+1] > 0, k = 1..nvec[i+1]}
```

For conjugate parts, negate and reverse the n-vector:
```
cnvec[j] = -nvec[t-j+1]  for j = 1..t
ppartsB = sorted list of {t*(k-1) + i : i where cnvec[i+1] > 0, k = 1..cnvec[i+1]}
```

Convert Frobenius coordinates to partition:
```
partsA = [ppartsA[j] + X - j + 1 for j = 1..X]
pp2a = [ppartsB[j] - j + 1 for j = 1..X], remove zeros
partsB = conjugate(pp2a)
result = [partsB..., partsA...]
```

**Edge case:** nvec = [0, 0, ..., 0] → empty partition

**Dependencies:** Requires a conjugate partition function. This is straightforward: for partition λ, conjugate λ' has λ'ⱼ = |{i : λᵢ ≥ j}|.

### 3.5 tquot(ptn, t) — t-Quotient via Beta Numbers

**Algorithm:**
1. Let λ = [λ₁, ..., λₘ] in non-decreasing order. Reverse to get descending order ptnz, then append t zeros.
2. Compute beta-set: `β[j] = ptnz[j] - j` for j = 1, ..., m+t
3. Partition beta-set by residue class: for each i = 0, ..., t-1, collect `μᵢ_bar = {β[j] : β[j] ≡ i (mod t)}`
4. For each residue class, compute the quotient partition:
   ```
   For k = 1, ..., len(μᵢ_bar)-1:
       nᵢₖ = floor(μᵢ_bar[k] / t) - floor(μᵢ_bar[k+1] / t) - 1
       if nᵢₖ > 0: add k repeated nᵢₖ times to λⁱ
   ```
5. Return [λ⁰, λ¹, ..., λᵗ⁻¹]

**Key insight:** The beta-set of a partition encodes it via the first-column hook lengths. Splitting by residue mod t gives t independent subsequences, each encoding one quotient partition.

**Edge cases:**
- t-core partition → all quotients are empty `[[], [], ..., []]`
- t=1 → single quotient equals the partition itself
- Empty partition → all quotients empty

### 3.6 PHI1 / invphi1 — The Bijection and Its Inverse

**PHI1(ptn, t):**
Simply returns `[tcoreofptn(ptn, t), tquot(ptn, t)]`.

**invphi1([core, quotient], t):**
Reconstructs the partition by inserting rim t-hooks into the core according to the quotient data.

**Algorithm for invphi1:**
1. Start with ptn = reverse(core) (convert to descending order)
2. For each residue class j from t down to 1:
   - For each part p of quotient[j] from last to first:
     - Find position: `pos = findhookinpos(ptn, t, j-1, p)`
     - Insert rim hook: `ptn = addrimthook(ptn, pos, p, t)`
3. Reverse back and remove trailing zeros

**findhookinpos(ptn, t, w, p):** Scans parts to find the p-th position where `(ptn[j] - j) mod t = w`, using the bi-infinite word W_w.

**addrimthook(ptn, j, L, t):** Adds a rim hook of length L·t starting at part j. Incrementally builds the hook by adding cells, choosing between extending the current row or moving to the row above (whichever maintains a valid partition).

**Critical correctness check:** `invphi1(PHI1(λ, t), t) = λ` for all partitions λ.

### 3.7 tcrank(ptn, t) — t-Core Crank

**Algorithm:**
```
h(x) = (x - (t-1)/2)^(t-3)
tcrank(λ, t) = Σᵢ₌₁ᵐ (h(λᵢ - i) - h(i - 1))  mod t
```

where m is the number of parts.

**Key properties:**
- Takes values in {0, 1, ..., t-1}
- For t prime and |λ| ≡ 0 (mod t), the crank equitably distributes partitions into t classes (this is the combinatorial proof of Ramanujan's congruences)
- The polynomial h(x) is specifically chosen to make this statistic work modulo t

**Note from Maple source:** The formula uses `h(t) = (t - (p-1)/2)^(p-3)` where p is the prime. For t=5: h(x) = (x-2)² (quadratic). For t=7: h(x) = (x-3)⁴ (quartic).

**Edge case:** t must be an odd prime for the standard GKS crank to work correctly.

### 3.8 tresdiag(ptn, t) — t-Residue Diagram

**Algorithm:** For each cell (i,j) of the Young diagram, the residue is `(j - i) mod t`. Print the diagram row by row.

From the Maple source:
```
for j from n by -1 to 1 do
    row = seq(modp(j - i, t), j=1..ptn[j])
    print row
    i := i + 1
```

This is purely a display function — outputs text, no return value.

---

## 4. Data Representation Decisions

### Partition Representation

In the Maple source, partitions are stored in **non-decreasing order** (smallest part first): `[1, 1, 2, 4, 4, 5]` represents 5+4+4+2+1+1 = 17.

The REPL must decide on a convention. Options:
- **Non-decreasing (Maple style):** Matches the source code directly. Most formulas in t-core.txt use this.
- **Non-increasing (standard math):** More common in textbooks. `[5, 4, 4, 2, 1, 1]`.

**Recommendation:** Use non-decreasing order to match Garvan's Maple source. This avoids translating all index math. The `tquot` function reverses internally when it needs descending order.

### Return Types

New types needed in the REPL's `EvalResult` variant:
- **Partition:** `std::vector<int>` — a list of non-negative integers
- **List of partitions:** `std::vector<std::vector<int>>` — for t-quotient, tcores()
- **Pair [core, quotient]:** Nested structure for PHI1 result

The parser already supports list literals `[a, b, c]`. The key question is whether to add a proper partition/list value type to the environment, or use the existing Series type with conventions.

**Recommendation:** Add `std::vector<int64_t>` as a value type. Many tcore functions input and output integer lists. Without this, users can't store partitions in variables or pass them between functions.

---

## 5. Correctness Criteria — Must-Pass Identities

### Identity 1: Size Preservation
```
For all λ, t:  |λ| = |tcoreofptn(λ,t)| + t * Σ|tquot(λ,t)[i]|
```

### Identity 2: Bijection Roundtrip
```
For all λ, t:  invphi1(phi1(λ, t), t) = λ
For all (core, quot), t:  phi1(invphi1([core, quot], t), t) = [core, quot]
```

### Identity 3: Core Idempotence
```
For all λ, t:  tcoreofptn(tcoreofptn(λ, t), t) = tcoreofptn(λ, t)
For all λ, t:  istcore(tcoreofptn(λ, t), t) = true
```

### Identity 4: Core of a Core
```
If istcore(λ, t) then:  tquot(λ, t) = [[], [], ..., []]  (t empty partitions)
If istcore(λ, t) then:  tcoreofptn(λ, t) = λ
```

### Identity 5: n-vector Roundtrip
```
For t-cores:  nvec2ptn(ptn2nvec(λ, t)) = λ
For all nvec with Σnvec[i]=0:  ptn2nvec(nvec2ptn(nvec), t) = nvec
```

### Identity 6: r-vector Sum
```
Σₖ rvec(λ, t, k) = |λ|  (for non-decreasing convention, this counts total nodes)
Wait — more precisely, Σₖ R[k] should relate to the partition size.
Actually: |λ| = t × (Σ R[k]²/2) - something more complex.
The simpler check: rvec values are non-negative integers.
```

### Identity 7: Generating Function
```
Σ_{λ is t-core of n} 1 = [q^n] etaq(t)^t / etaq(1)
```
i.e., `nops(tcores(t, n))` must match the coefficient of q^n in `etaq(t,T)^t * etaq(1,T).inverse()`.

### Identity 8: Crank Equidistribution
```
For t prime, for all n ≡ 0 (mod t):
  |{λ ⊢ n : tcrank(λ,t) = j}| = p(n)/t   for each j = 0, 1, ..., t-1
```

---

## 6. Edge Cases

### Empty Partition `[]`
- `tcoreofptn([], t)` = `[]`
- `istcore([], t)` = `true`
- `tquot([], t)` = `[[], [], ..., []]`
- `ptn2rvec([], t)` = `[0, 0, ..., 0]`
- `ptn2nvec([], t)` = `[0, 0, ..., 0]`
- `tcrank([], t)` = `0`
- `tcores(t, 0)` = `[[]]`

### Single Part `[n]`
- `istcore([n], t)` = true iff n < t (single part has hook lengths 1, 2, ..., n; it's a t-core iff n < t)
- `tcoreofptn([n], t)` = `[n mod t]` if n mod t > 0, else `[]`
  Wait — this is wrong. Single-part cores are more nuanced. A partition [n] is a t-core iff no hook equals t, and the hook lengths are 1, 2, ..., n. So [n] is a t-core iff n < t.
  For n ≥ t, the t-core of [n] is [n mod t] (if nonzero) or [] (if n divisible by t).
  Actually this isn't exactly right either — for [5] with t=3, hooks are 1,2,3,4,5; has hook 3, so not a 3-core. The 3-core would be found via the n-vector algorithm.

### t=1
- Every partition is a 1-core already (no hooks of length 1... wait, every cell has hook ≥ 1, so actually [n] for n ≥ 1 has hook 1 in the rightmost cell... )
  Actually: hook lengths in a partition are always ≥ 1. A 1-core would be a partition with no hook of length 1. But every non-empty partition has hook length 1 (the corner cells). So the only 1-core is the empty partition `[]`.
  For t=1: `tcoreofptn(λ, 1) = []` for all λ, and `tquot(λ, 1) = [λ]`.
  **Verify:** |λ| = |[]| + 1 × |λ| = |λ| ✓

### t=2
- 2-cores are staircase partitions: [], [1], [2,1], [3,2,1], [4,3,2,1], ...
- These are partitions with distinct parts forming a triangle
- `tcores(2, n)` = at most 1 element (2-core of n exists iff n is a triangular number)

### Already a t-Core
- `tcoreofptn(λ, t) = λ` when λ is already a t-core
- `tquot(λ, t) = [[], ..., []]`

### Large t (t > |λ|)
- Every partition of n is a t-core when t > n (no hook can be ≥ t+1... actually hooks can be up to λ₁ + λ'₁ - 1 which is bounded by n, so if t > n, it's a t-core)
- `tcoreofptn(λ, t) = λ` for t > |λ|

---

## 7. Implementation Dependencies on Existing Infrastructure

### What We Already Have
- `etaq(t, T)` — for generating function verification
- `partition_number(n)` — for equidistribution checks
- Parser with list support `[a, b, c]`
- Series arithmetic for generating function comparisons

### What We Need to Add
1. **Partition value type** in the REPL environment — `std::vector<int64_t>` stored as `EnvValue`
2. **Conjugate partition function** — needed by `nvec2ptn`
3. **List-of-partitions display** — for `tquot`, `tcores`, `phi1` output
4. **Integer list enumeration** — `tcores(t, n)` needs to enumerate all partitions of n, which is already available as generating function coefficients but not as explicit lists

### Partition Enumeration

`tcores(t, n)` calls `partition(n)` to get all partitions, then filters by `istcore`. This requires an explicit partition enumeration algorithm (not just counting). Standard approach: recursive descent or iterative algorithm that generates partitions in lexicographic or reverse-lexicographic order.

This is a new capability — the current `partition_number(n)` only counts partitions; it doesn't enumerate them.

**Complexity concern:** p(n) grows exponentially. For n=50, p(50)=204,226. Enumeration is feasible for n ≤ ~100, but gets slow for large n. This is acceptable — Maple has the same limitation.

---

## 8. What Makes This a Differentiator

### Table Stakes (Any t-core Package Must Have)
- `tcoreofptn` — the core operation
- `istcore` — basic predicate
- `tquot` — the other half of the Littlewood decomposition
- `phi1` / `invphi1` — the bijection (the whole point of GKS theory)
- `tcrank` — the key statistic from the GKS paper

### Differentiators (Value-Added Features)
- **Integration with series arithmetic:** Users can compute `Σ c_t(n) q^n` via `etaq(t)^t / etaq(1)` and verify against `tcores(t, n)` counts. No other standalone tool offers this.
- **Modular arithmetic integration:** `modp` and `findhommodp` can be used alongside t-core computations to study congruences.
- **Rank/crank generating functions:** `rankgf(m, T)` and `crankgf(m, T)` are already implemented. The t-core crank extends this to t-core congruences.

### What Maple Has That We Should Skip
- `makebiw` (bi-infinite words) — visualization-heavy, low research value
- `nvec2alphavec` — hardcoded for t=5 and t=7 only; too specialized
- `avec2nvec` — only for t=5; inverse of above
- Diagram array manipulation (`tresdiag2array`, `markrimhookV2`, etc.) — Maple-specific data structures

---

## 9. Recommended Implementation Priority

### Phase 1: Core Algorithms (must have)
1. `rvec(ptn, t, k)` — foundation for everything
2. `ptn2rvec(ptn, t)` — convenience wrapper
3. `ptn2nvec(ptn, t)` — uses rvec differences
4. `istcore(ptn, t)` — uses rvec, quadratic form check
5. `nvec2ptn(nvec)` — Frobenius coordinate reconstruction + conjugate
6. `tcoreofptn(ptn, t)` — ptn2nvec then nvec2ptn
7. `tquot(ptn, t)` — beta-number decomposition
8. `phi1(ptn, t)` — combines tcoreofptn + tquot

### Phase 2: Bijection Inverse + Crank
9. `findhookinpos(ptn, t, w, p)` — helper for invphi1
10. `addrimthook(ptn, j, L, t)` — helper for invphi1
11. `invphi1([core, quot], t)` — rim hook insertion
12. `tcrank(ptn, t)` — GKS crank statistic

### Phase 3: Enumeration + Display
13. Partition enumeration algorithm (generate all partitions of n)
14. `tcores(t, n)` — filter partitions by istcore
15. `tresdiag(ptn, t)` — display function

### Infrastructure (needed before Phase 1)
- Add partition (integer list) as a value type in `EvalResult` / `EnvValue`
- Add conjugate partition utility function
- Wire list literals to function arguments

---

## 10. Risks and Mitigations

| Risk | Impact | Mitigation |
|------|--------|------------|
| Partition representation mismatch (ascending vs descending) | Off-by-one errors everywhere | Pick one convention (ascending = Maple), document it, enforce in all functions |
| `nvec2ptn` conjugate partition dependency | Blocks Phase 1 | Implement conjugate first; it's 10 lines |
| `invphi1` is algorithmically complex | Hard to debug | Verify roundtrip identity exhaustively for small n (n ≤ 15) |
| Partition enumeration for large n | Performance cliff | Cap at n ≤ 200, warn user; this matches Maple behavior |
| `tcrank` h(x) polynomial is irrational for even t | Incorrect results | Only support odd prime t (the GKS theory requires this) |
| Adding list type to variant breaks existing code | Compilation errors | Use a distinct tagged type, not raw `vector<int64_t>` |

---

## RESEARCH COMPLETE
