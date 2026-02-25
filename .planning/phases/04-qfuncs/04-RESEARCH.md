# Phase 4: qfuncs — Research

**Researched:** 2025-02-24  
**Domain:** q-series building blocks (eta, theta, products), number theory helpers  
**Confidence:** HIGH  
**Reference:** [Garvan qseries functions](https://qseries.org/fgarvan/qmaple/qseries/functions/), SPEC.md, qseriesdoc.md

## Summary

Phase 4 delivers q-series building blocks and number theory helpers. Per CONTEXT: match Garvan's conventions; all functions take Series for q/z/a/b and int for truncation. **etaq** is the workhorse — Π(1-q^{kn}); **aqprod** builds rising q-factorials; **theta2/3/4** and **theta** provide theta series; **tripleprod, quinprod, winquist** implement standard product identities; **divisors, mobius, legendre, sigma, euler_phi** support prodmake and relations. Success: etaq(1,T) matches Euler pentagonal; partition 1/etaq(1,50) yields 1,1,2,3,5,7,11,15,22,30,42,... (TEST-02).

---

<user_constraints>

## User Constraints (from CONTEXT.md)

### Locked Decisions
- **Signatures:** Garvan convention — etaq(q,k,T), theta2/3/4(q,T), theta(z,q,T), aqprod(a,q,n,T), qbin(m,n,T), tripleprod(z,q,T), quinprod(z,q,T), winquist(a,b,q,T)
- **theta2:** Output θ₂/q^{1/4} with integer exponents — coeff 2 at n(n+1)
- **General theta:** In Phase 4 — theta(z,q,T)
- **NT helpers:** divisors→vector<int>, mobius/legendre→int, sigma/euler_phi→int64_t; placement at implementer discretion

### Claude's Discretion
- aqprod negative n; etaq/theta invalid args; nthelpers placement (qfuncs.h vs nthelpers.h)

</user_constraints>

---

## Standard Stack

### Dependencies
| Component | Source | Purpose |
|-----------|--------|---------|
| Series | Phase 3 | All qfuncs return Series |
| Frac, BigInt | Phase 1–2 | Coefficient arithmetic |
| Series::q(T), qpow, one, zero | series.h | Build factors |

### Build
Same as Phase 3: `g++ -std=c++20 -O2 -static -o qseries src/main.cpp`

---

## Algorithms

### etaq(q, k, T)
```
η_k(q) = Π_{n≥1} (1 - q^{kn})
```
- Start result = 1
- For n = 1, 2, ... while k*n < T: result *= (1 - q^{kn}), truncate to T after each multiply
- Uses Series::one(T), Series::qpow(k*n, T), subtraction, multiplication
- **Pentagonal check:** etaq(q,1,T) at T=20 should have coeff ±1 at 0,1,2,5,7,12,15 (pentagonal numbers n(3n-1)/2); coeff 0 elsewhere in range

### aqprod(a, q, n, T)
```
(a;q)_n = Π_{k=0}^{n-1} (1 - a·q^k)
```
- Start result = 1
- For k = 0..n-1: factor = 1 - a*q^k (a and q are Series; use a * q.pow(k) or build q^k via subs_q), result *= factor, truncate to T
- aqprod(a,q,0,T) = 1

### qbin(m, n, T)
```
[n;m]_q = (q;q)_n / ((q;q)_m · (q;q)_{n-m})
```
- Product formula: `Π_{i=1}^{m} (1 - q^{n-m+i}) / (1 - q^i)` — avoid series division
- Edge cases: return Series::zero(T) if m<0 or m>n; return Series::one(T) if m==0 or m==n
- Each step: multiply by (1 - q^{n-m+i}), divide by (1 - q^i) — use Series inverse for denominator

### theta2(q, T)
- Output θ₂(q)/q^{1/4} with integer exponents
- coeff[n(n+1)] = 2 for n ≥ 0 while n(n+1) < T
- coeff[0] = 2 (from n=0: n(n+1)=0)
- Garvan returns full θ₂ including q^{1/4}; we divide out for integer exponents

### theta3(q, T)
- coeff[0] = 1; coeff[n²] += 2 for n = 1,2,... while n² < T

### theta4(q, T)
- Same as theta3 but coeff[n²] += 2*(-1)^n

### theta(z, q, T)
- Σ_{i=-T}^{T} z^i · q^{i²}
- Iterate i, add z.pow(i) * q.pow(i²) scaled by coefficient 1; combine into result

### tripleprod(z, q, T)
```
Π_{n≥1} (1 - z·q^{n-1})(1 - z⁻¹·q^n)(1 - q^n)
```
- For n = 1..T-1: multiply by (1 - z·q^{n-1}), (1 - z⁻¹·q^n), (1 - q^n); truncate each step
- z⁻¹ = z.inverse() — requires z to have nonzero constant term (or minExp handling)

### quinprod(z, q, T)
```
(-z;q)_∞ · (-q/z;q)_∞ · (z²q;q²)_∞ · (q/z²;q²)_∞ · (q;q)_∞
```
- Interleave factors per SPEC: for n, multiply (1+z·q^{n-1}), (1+q^n/z), (1-q^n); for m with 2m-1<T, multiply (1-z²·q^{2m-1}), (1-q^{2m-1}/z²)

### winquist(a, b, q, T)
- Double sum over n≥0 and m (all integers) such that exponent 3n(n+1)/2 + m(3m+1)/2 < T
- Terms: (-1)^{n+m} · [(a^{-3n}-a^{3n+3})(b^{-3m}-b^{3m+1}) + (a^{1-3m}-a^{3m+2})(b^{3n+2}-b^{-3n-1})] · q^{...}
- a^{-k} = a.pow(-k) = a.inverse().pow(k); requires a,b invertible

### divisors(n)
- Return sorted positive divisors of n
- Trial division: for d=1..√n, if n%d==0 add d and n/d; sort

### mobius(n)
- μ(n) = 0 if n has squared factor; (-1)^k if n product of k distinct primes
- Factor n, check for squares; count primes

### legendre(a, p)
- (a/p) = a^{(p-1)/2} mod p for odd prime p; 0 if p|a
- Use repeated squaring mod p

### sigma(n, k=1)
- σ_k(n) = Σ_{d|n} d^k
- Use divisors(n), sum d^k

### euler_phi(n)
- φ(n) = count of 1..n coprime to n
- φ(n) = n · Π (1 - 1/p) over primes p|n

---

## Garvan Alignment

| Function | Garvan | Our | Notes |
|----------|--------|-----|-------|
| etaq | etaq(q,a,T) | etaq(q,k,T) | a→k (eta parameter) |
| theta2 | theta2(q,T) | theta2(q,T) | We output θ₂/q^{1/4} for integer exponents |
| theta3, theta4 | theta3(q,T), theta4(q,T) | same | |
| theta | theta(z,q,T) | theta(z,q,T) | Σ z^i q^{i²}, i=-T..T |
| aqprod | aqprod(a,q,n) | aqprod(a,q,n,T) | Add T for truncation |
| qbin | qbin(q,m,n) | qbin(m,n,T) | Garvan param order differs; we use (m,n,T) per SPEC |
| tripleprod | tripleprod(z,q,T) | tripleprod(z,q,T) | |
| quinprod | quinprod(a,b,q,T) | quinprod(z,q,T) | SPEC uses single z |
| winquist | winquist(a,b,q,T) | winquist(a,b,q,T) | |

---

## Common Pitfalls

### Pitfall 1: Truncation Not Propagated (PITFALLS #4)
- Every qfunc returning Series must set result.trunc = T
- Binary ops in factors: use min(trunc_a, trunc_b)
- **Check:** etaq * etaq, aqprod chains

### Pitfall 2: etaq / aqprod — Missing truncate Step
- After each multiply, truncate to T. Otherwise terms beyond T accumulate and downstream prodmake sees garbage.
- **Check:** 1/etaq(1,50) partition coeffs must match SPEC exactly

### Pitfall 3: theta2 Coefficient at 0
- θ₂/q^{1/4} = 2·Σ_{n≥0} q^{n(n+1)}. For n=0: exponent 0, coeff 2. So coeff[0]=2 (not 1).

### Pitfall 4: qbin Division by Zero
- (1-q^i) for i≥1 is (1-q) for i=1 — invertible. But (1-q^0)=0. Product formula uses i=1..m; denominator (1-q^i) never 1-q^0. Safe.

### Pitfall 5: tripleprod / quinprod — z⁻¹ When z Has No Constant Term
- If z = q (Series), z has minExp 1. z.inverse() exists (returns q^{-1} with negative exponents). Our Series supports negative exponents for Laurent series. **Verify** tripleprod(q, q, T) works — Garvan example tripleprod(q, q^3, 10).

### Pitfall 6: winquist — Negative Powers of a, b
- a^{-3n} = inverse(a).pow(3n). a must be invertible (nonzero constant term). Garvan uses a,b as variables (typically 1 for specialization). For generic a,b we need minExp≤0 with c₀≠0.

---

## Acceptance Tests (Phase 4 Relevant)

- **TEST-02 Partition:** p = 1/etaq(q,1,50); series(p,20) → 1,1,2,3,5,7,11,15,22,30,42,...
- **Euler pentagonal:** etaq(q,1,T) coeffs at 0,1,2,5,7,12,15 are ±1; zero elsewhere in pentagonal range
- **theta3/theta4:** Sparse series; theta3 coeff 1 at 0, 2 at 1,4,9,16,...; theta4 alternating signs

---

## References

- SPEC.md §§4.1–4.7, 8
- qseriesdoc.md §§2.1–2.2
- PITFALLS.md #4 (truncation), #3 (Frac growth in products)
- Garvan: https://qseries.org/fgarvan/qmaple/qseries/functions/
