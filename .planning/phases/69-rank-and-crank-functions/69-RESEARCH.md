# Phase 69 Research: Rank and Crank Functions

## Formula Verification

### Rank

The **rank** of a partition λ is (largest part) − (number of parts).
N(m,n) = number of partitions of n with rank m.

**Single-variable generating function** (for fixed m):
```
Σ_{n≥0} N(m,n) q^n = δ_{m,0} + (1/(q;q)_∞) × Σ_{k≥1} (-1)^{k-1} q^{k(3k-1)/2 + |m|k} (1 - q^k)
```

The δ_{m,0} correction is needed because for m=0 the sum has minimum exponent 1 (from k=1: exponent = 1), so the q^0 coefficient is 0, but N(0,0)=1 (empty partition has rank 0). For m≠0 the correction is zero since N(m,0)=0.

**Equivalent direct formula** (from Garvan's Maple `RANK.txt`):
```
N(m,n) = Σ_{j=1}^{J} (-1)^{j-1} [ p(n - j(3j-1)/2 - |m|j) - p(n - j(3j-1)/2 - |m|j - j) ]
```
where p(k) = 0 for k < 0, and J is determined by j(3j-1)/2 + |m|j ≤ n.

#### Verification of specific values

**N(0,0) = 1**: Empty partition has 0 parts and largest part 0, rank = 0−0 = 0. The formula sum is empty (smallest exponent is 1 > 0), so the δ_{m,0} correction provides this. ✓

**N(0,1) = 1**: Partition (1) has rank 1−1 = 0.
Formula: j=1: kk=1≤1, +p(0)=1; kk2=2>1. N(0,1)=1. ✓

**N(1,2) = 1**: Partition (2) has rank 2−1 = 1.
Formula: j=1: kk=2≤2, +p(0)=1; kk2=3>2. N(1,2)=1. ✓

**N(1,4) = 1**: Partition (3,1) has rank 3−2 = 1.
Formula: j=1: kk=2≤4, +p(2)=2; kk2=3≤4, −p(1)=1. j=2: kk=7>4. N(1,4)=2−1=1. ✓

**N(0,5) = 1**: Enumerating all partitions of 5 with their ranks:

| Partition | Largest part | # Parts | Rank |
|-----------|-------------|---------|------|
| (5)       | 5           | 1       | 4    |
| (4,1)     | 4           | 2       | 2    |
| (3,2)     | 3           | 2       | 1    |
| (3,1,1)   | 3           | 3       | 0    |
| (2,2,1)   | 2           | 3       | −1   |
| (2,1,1,1) | 2           | 4       | −2   |
| (1,1,1,1,1)| 1          | 5       | −4   |

Only (3,1,1) has rank 0, so N(0,5) = 1. ✓
Formula: j=1: +p(4)−p(3)=5−3=2; j=2: −p(0)=−1. Total = 2−1=1. ✓

### Crank

The **crank** of a partition λ:
- If λ has no 1s: crank = largest part
- If λ has ω(λ) ones: crank = μ(λ) − ω(λ), where μ(λ) = number of parts strictly larger than ω(λ)

M(m,n) = number of partitions of n with crank m.

**Important convention** (Andrews-Garvan 1988): The generating function
```
C(z,q) = (q;q)_∞ / ((zq;q)_∞ (q/z;q)_∞)
```
gives coefficient of z^0 q^1 = −1. This means M(0,1) = −1 and M(1,1) = M(−1,1) = 1 by the generating function convention. The physical partition (1) has crank −1, but the generating function assigns "virtual" values at n=1 to make all identities work. This convention is standard in Garvan's qseries package.

**Single-variable generating function** (for fixed m):
```
Σ_{n≥0} M(m,n) q^n = (1/(q;q)_∞) × Σ_{k≥1} (-1)^{k-1} q^{k(k-1)/2 + |m|k} (1 - q^k)
```

No δ correction needed: for m=0 the k=1 term has exponent k(k-1)/2 = 0, contributing +1 at q^0, so M(0,0) = 1 comes naturally. For m≠0 the minimum exponent is |m| ≥ 1, giving M(m,0) = 0 correctly.

**Equivalent direct formula** (from Garvan's Maple `CRANK.txt`):
```
M(m,n) = Σ_{j=1}^{J} (-1)^{j-1} [ p(n - j(j-1)/2 - |m|j) - p(n - j(j-1)/2 - |m|j - j) ]
```

#### Verification of specific values

**M(−1,1) = 1**: Partition (1) has ω=1, μ=0 (no parts > 1), crank = 0−1 = −1. ✓
Formula (m=−1, |m|=1): j=1: kk=0+1=1≤1, +p(0)=1; kk2=2>1. M(−1,1)=1. ✓

**M(0,1) = −1** (Andrews-Garvan convention):
Formula: j=1: kk=0≤1, +p(1)=1; kk2=1≤1, −p(0)=−1. val=0.
j=2: kk=1≤1, −p(0)=−1. val=−1. ✓

**M(m,2)**: (2) has no 1s, crank=2. (1,1) has ω=2, μ=0, crank=−2.
So M(2,2) = M(−2,2) = 1, all others 0. ✓

**M(m,3)**: (3) crank=3. (2,1): ω=1, parts>1={2}, μ=1, crank=0. (1,1,1): ω=3, μ=0, crank=−3.
So M(3,3) = M(0,3) = M(−3,3) = 1. ✓

**M(0,5) = 1**: Enumerating partitions of 5 with their cranks:

| Partition | ω | μ (parts > ω) | Crank |
|-----------|---|---------------|-------|
| (5)       | 0 | —             | 5     |
| (4,1)     | 1 | 1 ({4})       | 0     |
| (3,2)     | 0 | —             | 3     |
| (3,1,1)   | 2 | 1 ({3})       | −1    |
| (2,2,1)   | 1 | 2 ({2,2})     | 1     |
| (2,1,1,1) | 3 | 0             | −3    |
| (1,1,1,1,1)| 5| 0             | −5    |

Only (4,1) has crank 0, so M(0,5) = 1. ✓

### Key difference between rank and crank formulas

The two formulas are structurally identical except for the exponent of k in the triangular-number-like term:
- **Rank**: exponent = k(3k−1)/2 + |m|k  (pentagonal-type)
- **Crank**: exponent = k(k−1)/2 + |m|k  (triangular-type)

Both yield: `(-1)^{k-1} × q^{exponent} × (1 − q^k)`, summed over k ≥ 1, then divided by (q;q)_∞.

## Implementation Approach

### Recommended: Direct partition-number convolution (O(T√T))

This matches Garvan's Maple `nrmake`/`ncrmake` procedures. It avoids the O(T²) series inverse.

**Algorithm:**

1. Compute p(0), p(1), ..., p(T−1) using Euler's pentagonal recurrence:
   ```
   p(n) = Σ_{k≥1} (-1)^{k+1} [p(n − k(3k−1)/2) + p(n − k(3k+1)/2)]
   ```
   This is O(T√T).

2. For given m, compute N(m,n) [or M(m,n)] for each n using:
   ```
   N(m,n) = Σ_j (-1)^{j-1} [p(n − kk) − p(n − kk − j)]
   where kk = j(3j−1)/2 + |m|j   (rank)
      or kk = j(j−1)/2 + |m|j     (crank)
   ```
   Each n requires O(√n) work, so total is O(T√T).

3. Build Series from the computed coefficients.

**Special cases:**
- Rank m=0: set coefficient of q^0 to 1 (empty partition)
- Crank m=0: the formula naturally gives M(0,0)=1, no correction needed
- Crank m=0, n=1: the formula naturally gives M(0,1)=−1 (Andrews-Garvan convention)

### Code placement
- Functions `rankgf(int m, int T)` and `crankgf(int m, int T)` in `qfuncs.h`
- REPL dispatch in `repl.h`: `rankgf(m, T)` and `crankgf(m, T)` taking 2 integer arguments
- Help entries: describe what rank and crank are

### Implementation sketch

```cpp
inline Series rankgf(int m, int T) {
    int am = (m < 0) ? -m : m;
    std::vector<Frac> p(T, Frac(0));
    p[0] = Frac(1);
    for (int n = 1; n < T; ++n) {
        for (int k = 1; ; ++k) {
            int e1 = k * (3*k - 1) / 2;
            int e2 = k * (3*k + 1) / 2;
            if (e1 > n && e2 > n) break;
            Frac sign(k % 2 == 1 ? 1 : -1);
            if (e1 <= n) p[n] = p[n] + sign * p[n - e1];
            if (e2 <= n) p[n] = p[n] + sign * p[n - e2];
        }
    }
    Series result;
    result.trunc = T;
    if (m == 0) result.setCoeff(0, Frac(1));
    for (int n = 1; n < T; ++n) {
        Frac val(0);
        for (int j = 1; ; ++j) {
            int kk = j * (3*j - 1) / 2 + am * j;
            if (kk > n) break;
            Frac sign = (j % 2 == 1) ? Frac(1) : Frac(-1);
            val = val + sign * p[n - kk];
            int kk2 = kk + j;
            if (kk2 <= n) val = val - sign * p[n - kk2];
        }
        if (!val.isZero()) result.setCoeff(n, val);
    }
    return result;
}

inline Series crankgf(int m, int T) {
    int am = (m < 0) ? -m : m;
    std::vector<Frac> p(T, Frac(0));
    p[0] = Frac(1);
    for (int n = 1; n < T; ++n) {
        for (int k = 1; ; ++k) {
            int e1 = k * (3*k - 1) / 2;
            int e2 = k * (3*k + 1) / 2;
            if (e1 > n && e2 > n) break;
            Frac sign(k % 2 == 1 ? 1 : -1);
            if (e1 <= n) p[n] = p[n] + sign * p[n - e1];
            if (e2 <= n) p[n] = p[n] + sign * p[n - e2];
        }
    }
    Series result;
    result.trunc = T;
    for (int n = 0; n < T; ++n) {
        Frac val(0);
        for (int j = 1; ; ++j) {
            int kk = j * (j - 1) / 2 + am * j;
            if (kk > n) break;
            Frac sign = (j % 2 == 1) ? Frac(1) : Frac(-1);
            val = val + sign * p[n - kk];
            int kk2 = kk + j;
            if (kk2 <= n) val = val - sign * p[n - kk2];
        }
        if (!val.isZero()) result.setCoeff(n, val);
    }
    return result;
}
```

### REPL dispatch (in repl.h)

```cpp
if (name == "rankgf") {
    if (args.size() != 2)
        throw std::runtime_error(runtimeErr(name, "expects 2 arguments: rankgf(m, T)"));
    return rankgf(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
}
if (name == "crankgf") {
    if (args.size() != 2)
        throw std::runtime_error(runtimeErr(name, "expects 2 arguments: crankgf(m, T)"));
    return crankgf(static_cast<int>(evi(0)), static_cast<int>(evi(1)));
}
```

### Help entries

```cpp
{"rankgf", {"rankgf(m,T)", "GF for partitions with rank m: Σ N(m,n) q^n"}},
{"crankgf", {"crankgf(m,T)", "GF for partitions with crank m: Σ M(m,n) q^n"}},
```

## Test Data

### Rank N(m,n) — verified against Garvan's Maple `nrmake` algorithm

Symmetry: N(m,n) = N(−m,n) for all m, n. Only m ≥ 0 shown.

| n\m |  0 |  1 |  2 |  3 |  4 |  5 |
|-----|----|----|----|----|----|----|
|  0  |  1 |  0 |  0 |  0 |  0 |  0 |
|  1  |  1 |  0 |  0 |  0 |  0 |  0 |
|  2  |  0 |  1 |  0 |  0 |  0 |  0 |
|  3  |  1 |  0 |  1 |  0 |  0 |  0 |
|  4  |  1 |  1 |  0 |  1 |  0 |  0 |
|  5  |  1 |  1 |  1 |  0 |  1 |  0 |
|  6  |  1 |  2 |  1 |  1 |  0 |  1 |
|  7  |  3 |  1 |  2 |  1 |  1 |  0 |
|  8  |  2 |  3 |  2 |  2 |  1 |  1 |
|  9  |  4 |  3 |  3 |  2 |  2 |  1 |
| 10  |  4 |  5 |  3 |  4 |  2 |  2 |

Row sums (accounting for N(−m,n) = N(m,n)):
- n=5: 1 + 2(1+1+0+1+0) = 7 = p(5) ✓
- n=7: 3 + 2(1+2+1+1+0+1) = 15 = p(7) ✓
- n=10: 4 + 2(5+3+4+2+2+1+1+0+1+0) = 42 = p(10) ✓

### Crank M(m,n) — verified against Garvan's Maple `ncrmake` algorithm

Symmetry: M(m,n) = M(−m,n) for all m, n ≥ 2. For n=1: M(−1,1)=M(1,1)=1, M(0,1)=−1.

| n\m |  0 |  1 |  2 |  3 |  4 |  5 |
|-----|----|----|----|----|----|----|
|  0  |  1 |  0 |  0 |  0 |  0 |  0 |
|  1  | −1 |  1 |  0 |  0 |  0 |  0 |
|  2  |  0 |  0 |  1 |  0 |  0 |  0 |
|  3  |  1 |  0 |  0 |  1 |  0 |  0 |
|  4  |  1 |  0 |  1 |  0 |  1 |  0 |
|  5  |  1 |  1 |  0 |  1 |  0 |  1 |
|  6  |  1 |  1 |  1 |  1 |  1 |  0 |
|  7  |  1 |  2 |  1 |  1 |  1 |  1 |
|  8  |  2 |  2 |  2 |  1 |  2 |  1 |
|  9  |  2 |  3 |  2 |  3 |  1 |  2 |
| 10  |  4 |  3 |  4 |  2 |  3 |  2 |

Row sums (accounting for symmetry):
- n=5: 1 + 2(1+0+1+0+1) = 7 = p(5) ✓
- n=7: 1 + 2(2+1+1+1+1+0+1) = 15 = p(7) ✓
- n=9: 2 + 2(3+2+3+1+2+1+1+0+1) = 30 = p(9) ✓
- n=10: 4 + 2(3+4+2+3+2+2+1+1+0+1) = 42 = p(10) ✓

### Cross-verification with partition enumeration

**Partitions of 7 — rank and crank for each:**

| Partition     | Rank | Crank |
|---------------|------|-------|
| (7)           |   6  |   7   |
| (6,1)         |   4  |   0   |
| (5,2)         |   3  |   5   |
| (5,1,1)       |   2  |  −1   |
| (4,3)         |   2  |   4   |
| (4,2,1)       |   1  |   1   |
| (4,1,1,1)     |   0  |  −2   |
| (3,3,1)       |   0  |   1   |
| (3,2,2)       |   0  |   3   |
| (3,2,1,1)     |  −1  |  −1   |
| (3,1,1,1,1)   |  −2  |  −4   |
| (2,2,2,1)     |  −2  |   2   |
| (2,2,1,1,1)   |  −3  |  −3   |
| (2,1,1,1,1,1) |  −4  |  −5   |
| (1^7)         |  −6  |  −7   |

Rank counts: N(0,7)=3 ✓, N(1,7)=1 ✓, N(2,7)=2 ✓
Crank counts: M(0,7)=1 ✓, M(1,7)=2 ✓, M(2,7)=1 ✓, M(3,7)=1 ✓

## Acceptance Tests

### Test 1: Symmetry
```
rankgf(2, 20) - rankgf(-2, 20)
```
Expected: 0 (zero series)

```
crankgf(3, 20) - crankgf(-3, 20)
```
Expected: 0

### Test 2: Specific coefficients
```
coeff(rankgf(0, 20), 5)
```
Expected: 1

```
coeff(rankgf(1, 20), 4)
```
Expected: 1

```
coeff(crankgf(0, 20), 1)
```
Expected: −1 (Andrews-Garvan convention)

```
coeff(crankgf(0, 20), 3)
```
Expected: 1

### Test 3: Partition count identity
For n < T, the sum over all ranks should equal p(n):
```
coeff(rankgf(0,15) + 2*add(rankgf(m,15), m, 1, 14), 10)
```
Expected: 42 = p(10)

```
coeff(crankgf(0,15) + 2*add(crankgf(m,15), m, 1, 14), 10)
```
Expected: 42 = p(10)

### Test 4: Ramanujan congruence for rank mod 5
The rank provides a combinatorial explanation of Ramanujan's congruence p(5n+4) ≡ 0 (mod 5).
For n=4 (≡ 4 mod 5), p(4)=5, each residue class mod 5 has exactly 1 partition:
- N(m,4) for m = 0,1,−1,3,−3 are 1,1,1,1,1
- Residues mod 5: 0→1, 1→1, 4→1, 3→1, 2→1

Test with sift:
```
sift(rankgf(0,30) + rankgf(5,30) + rankgf(-5,30) + rankgf(10,30) + rankgf(-10,30) + rankgf(15,30) + rankgf(-15,30) + rankgf(20,30) + rankgf(-20,30) + rankgf(25,30) + rankgf(-25,30), 5, 4, 30)
```
This gives the GF for N(0,5,5n+4) = #{partitions of 5n+4 with rank ≡ 0 mod 5}.

The coefficient of q^{5n+4} should equal p(5n+4)/5 for each residue class. In practice, verify:
```
coeff(rankgf(0,30) + rankgf(5,30) + rankgf(-5,30) + rankgf(10,30) + rankgf(-10,30) + rankgf(15,30) + rankgf(-15,30) + rankgf(20,30) + rankgf(-20,30) + rankgf(25,30) + rankgf(-25,30), 4)
```
Expected: 1 = p(4)/5 = 5/5

### Test 5: Table verification (comprehensive)
```bash
# Rank values
coeff(rankgf(0,12), 0)  # expect 1
coeff(rankgf(0,12), 7)  # expect 3
coeff(rankgf(1,12), 10) # expect 5
coeff(rankgf(2,12), 9)  # expect 3
coeff(rankgf(3,12), 10) # expect 4

# Crank values
coeff(crankgf(0,12), 0)  # expect 1
coeff(crankgf(0,12), 1)  # expect -1
coeff(crankgf(1,12), 7)  # expect 2
coeff(crankgf(2,12), 10) # expect 4
coeff(crankgf(3,12), 9)  # expect 3
coeff(crankgf(4,12), 8)  # expect 2
```

### Test 6: Crank resolves all three Ramanujan congruences
The crank was introduced by Andrews-Garvan specifically because it provides a combinatorial proof of all three Ramanujan congruences:
- p(5n+4) ≡ 0 (mod 5)
- p(7n+5) ≡ 0 (mod 7)
- p(11n+6) ≡ 0 (mod 11)

For mod 7: verify M(0,7,7n+5) = M(1,7,7n+5) = ... = M(6,7,7n+5) = p(7n+5)/7.
At n=5 (≡ 5 mod 7): p(5)=7, each residue class mod 7 should have 1 partition.

## Source References

- Garvan's Maple `RANK.txt`: contains `nrmake` which computes N(m,n) table using the direct convolution formula. The Maple code uses the same pentagonal-number-based formula verified above.
- Garvan's Maple `CRANK.txt`: contains `ncrmake` which computes M(m,n) table using the triangular-number-based formula. Also contains `crankgen` which uses the two-variable GF `(q;q)_∞² × (1−z) / tripleprod(z,q,T)`.
- Both Maple files use `_ranktable[m,n]` / `_cranktable[m,n]` arrays with `N(m,n) = N(−m,n)` symmetry (only m ≥ 0 stored).

## RESEARCH COMPLETE
