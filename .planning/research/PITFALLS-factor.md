# Domain Pitfalls: Polynomial Factorization Over Q

**Domain:** Polynomial factorization over Q (rationals), factor(poly) for q-series context
**Context:** Adding factor(poly) over Q; zero dependencies; T(8,8) degree 42; integration with Series (q as variable)
**Researched:** 2025-03-03

---

## Critical Pitfalls

### Pitfall 1: Confusing qfactor with factor()

**What goes wrong:** The project already has `qfactor`, which uses Andrews' prodmake algorithm. That algorithm assumes the input is a *q-product*: `q^e · Π (1-q^n)^{b_n}`. It is **not** general polynomial factorization. Maple's `factor(t8)` factors into **cyclotomic polynomials**; `qfactor(t8,20)` returns the q-product form. These are different outputs for the same input.

**Why it happens:** qfactor = Andrews' algorithm (logarithmic derivative, divisor extraction). factor = Zassenhaus/Hensel + recombination, or cyclotomic-specific methods.

**Consequences:** If you try to extend qfactor to handle arbitrary polynomials, it will fail on anything that is not a q-product. Non-integer `a[n]` from prodmake signals "not a q-product," not "irreducible."

**Prevention:** Implement `factor()` as a distinct code path. Use qfactor when you know the polynomial has q-product form (T(r,n), dixson, etc.); use factor() for general irreducibles/cyclotomics.

**Detection:** prodmake returns non-integer `a[n]` → polynomial is not a q-product; need general factor().

---

### Pitfall 2: Coefficient Growth in Hensel Lifting

**What goes wrong:** Hensel lifting mod p^a to recover integer factors causes intermediate coefficients to grow. The lifting bound p^a must be large enough to recover factors, but over-lifting makes coefficients explode. Without GMP, BigInt (base 10^9, schoolbook multiplication) will be slow; coefficient growth directly hits runtime and memory.

**Why it happens:** Zassenhaus-style algorithms lift p-adic factors; coefficient size of recovered factors grows with the lifting exponent. Van Hoeij notes that LLL-style algorithms historically worked with vectors of O(d) entries and O(1000) digits for degree 1000 polynomials.

**Consequences:** Degree 42 with moderate coefficient size can still produce 100+ digit intermediates. Schoolbook BigInt multiplication is O(n·m); repeated GCD/reduce on large Fracs dominates cost.

**Prevention:**
1. **Early termination** (van Hoeij, ISSAC 2011): Lift only as high as needed. For polynomials with one large factor and several small factors (common for q-series), p^a need not reach the Zassenhaus default.
2. Reduce fractions aggressively after each step.
3. Consider lazy evaluation or delayed normalization where safe.
4. For degree 42, target T ≈ 4d+3 (per qseriesdoc) or similar; avoid over-truncation that hides factors, but also avoid excessive T.

**Sources:** [van Hoeij, math.fsu.edu](https://www.math.fsu.edu/~hoeij/algorithms/Qx.html); "Factoring polynomials and the knapsack problem" (J. Number Theory 2002).

---

### Pitfall 3: Cyclotomic vs Non-Cyclotomic Assumptions

**What goes wrong:** T(8,8) factors into cyclotomic polynomials (Maple `factor(t8)`). Cyclotomics Φ_n(q) have known structure: degree φ(n), integer coefficients, roots = primitive n-th roots of unity. Many q-series polynomials from partition theory, theta functions, and T(r,n)-style recurrences share this structure. Assuming "all q-series polys are cyclotomic" is wrong; assuming "none are" wastes optimization opportunities.

**Why it happens:** Cyclotomics are a special case. General factorization (Zassenhaus, Cantor–Zassenhaus) handles arbitrary polynomials but is heavier. Cyclotomic factorization can use x^n - 1 = Π_{d|n} Φ_d(x) and structure of the polynomial.

**Consequences:** 
- Cyclotomic-only path: Fails on irreducible non-cyclotomic factors (e.g., some resultants, generic polynomials).
- General-only path: Correct but slower for the common q-series case.
- Wrong assumption: Debugging "why doesn't factor() match Maple?" when Maple returns cyclotomic form and your impl returns a different (but correct) irreducible factorization.

**Prevention:**
1. Document that T(8,8) and similar q-series polynomials are expected to factor into cyclotomics.
2. Consider a fast path: if the polynomial is a product of (1 - q^n) powers (q-product), use qfactor.
3. For general factor(), follow standard pipeline: content/primitive part → square-free → factor each square-free factor.
4. Do **not** assume cyclotomic structure to shortcut the algorithm; use it only for output formatting or validation.

**Detection:** If the polynomial divides some x^k - 1 (or q^k - 1), it may be cyclotomic; otherwise treat as general.

---

### Pitfall 4: Degree 42 and Worst-Case Explosion

**What goes wrong:** Zassenhaus has exponential worst-case complexity in the number of p-adic factors. For degree 42, worst-case inputs (e.g., Swinnerton–Dyer type) can have r ≈ degree/2 p-adic factors, making recombination O(2^{21}). In practice, q-series polynomials like T(8,8) tend to have fewer factors, so performance is usually acceptable.

**Why it happens:** Recombination tests subsets of p-adic factors; 2^r subsets when r factors. Van Hoeij's knapsack/LLL approach reduces this by working with r-dimensional binary vectors instead of degree-sized vectors.

**Consequences:** Degree 42 is at the boundary where naive Zassenhaus can be slow on bad inputs. With early termination and sensible prime choice, typical T(8,8)-like inputs should factor in sub-second to few seconds on modern hardware (with schoolbook BigInt).

**Prevention:**
1. Use a prime p such that f has no repeated factors mod p (so factorization mod p is square-free).
2. Prefer van Hoeij–style knapsack/LLL recombination if implementing from scratch; otherwise use Zassenhaus with early termination.
3. Set a timeout or iteration limit for recombination; report "factorization incomplete" rather than hanging.
4. Degree 42 is feasible; avoid extending to degree 100+ without profiling.

**Sources:** van Hoeij algorithms page; "gradual sub-lattice reduction" (HN 2009, ISSAC 2011).

---

### Pitfall 5: Series ↔ Polynomial Conversion (q as Variable)

**What goes wrong:** Series is `map<int, Frac>` (sparse) with `trunc` and optional `q_shift`. General factorization expects a dense coefficient vector `[c_0, c_1, …, c_d]` for polynomial c_0 + c_1·q + … + c_d·q^d. Mistakes: (1) treating sparse map as dense and reading zeros incorrectly; (2) ignoring q_shift (series represents q^{q_shift} · Σ c[n] q^n); (3) truncation semantics (terms with exponent ≥ trunc are discarded); (4) missing exponents filled as 0.

**Why it happens:** Series is optimized for sparse q-series; factor() expects a univariate polynomial in q. The variable is q (exponent = power of q).

**Consequences:** 
- Wrong polynomial: Off-by-one in exponents, or q_shift absorbed incorrectly.
- Garbage factors: If trunc is too low, high-degree terms are dropped and the polynomial is wrong.
- For T(8,8), degree is 42; trunc must be ≥ 43.

**Prevention:**
1. **Before factor():** Normalize q_shift (e.g., `normalize_q_shift()`), then build dense `vector<Frac>` for exponents 0..maxExp(). Use `f.coeff(n)` for 0 ≤ n ≤ maxExp(); default to Frac(0) for gaps.
2. **Truncation:** Ensure `trunc > maxExp()` so no nonzero terms are lost. For T(8,8), `set_trunc(64)` or higher.
3. **Leading term:** factor() typically expects primitive polynomial (content = 1). Extract content (GCD of coefficients), factor primitive part, then multiply content back.
4. **Output:** factor() returns factors as polynomials. Represent each factor as Series (map<int,Frac>) for consistency with the rest of the REPL.

**Detection:** Run factor(T(8,8)) and verify product of factors equals T(8,8). Check that maxExp() of the product matches 42.

---

### Pitfall 6: Content and Primitive Part

**What goes wrong:** Skipping the content/primitive-part step. Polynomials with rational coefficients must be converted to Z[x] (or scaled) before modular methods apply. Content = GCD of coefficients; primitive part = polynomial / content. For rational coefficients, clear denominators first (multiply by LCM of denominators), then content is over Z.

**Why it happens:** Hensel lifting and factorization mod p operate on integer polynomials. Frac coefficients need normalization.

**Consequences:** Mod-p factorization on non-integer coefficients is undefined. Wrong factors or crashes.

**Prevention:** 
1. Given Series with Frac coefficients, compute LCM of denominators and multiply through → integer polynomial.
2. Compute content (GCD of integer coefficients).
3. Factor primitive part (content = 1).
4. Final result = content × product of factor factors. Adjust for the scaling factor (LCM) in the output.

---

## Moderate Pitfalls

### Pitfall 7: Square-Free Factorization First

**What goes wrong:** Factoring a polynomial that has repeated roots (e.g., f = g^2 · h) without square-free decomposition. Standard approach: gcd(f, f') gives product of repeated factors; square-free decomp is f = a_1 · a_2^2 · a_3^3 · … where a_i are square-free and pairwise coprime. Skipping this complicates Hensel lifting.

**Prevention:** Always do square-free decomposition before factoring each square-free factor. Over Q, gcd(f, f') is cheap (Euclidean algorithm on polynomials with Frac/BigInt).

---

### Pitfall 8: Choice of Prime p

**What goes wrong:** Choosing p such that f has a repeated factor mod p. Then factorization mod p is not square-free, and Hensel lifting gets more complex.

**Prevention:** Choose p that does not divide the discriminant (or resultant of f and f'). In practice, try small primes; if f mod p has repeated factors, try another p. For integer polynomials, only finitely many bad primes.

---

### Pitfall 9: Zero Constant Term (q^e Factor)

**What goes wrong:** T(8,8) has minExp 6 (leading term q^6). factor() must handle the q^e factor separately. Extract q^e first (as in qfactor), then factor the remaining polynomial (which has nonzero constant term).

**Prevention:** Match qfactor's lead extraction: shift by minExp, factor the remaining polynomial, prepend q^minExp to the factor list. Ensures compatibility with qfactor's q_power output.

---

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation |
|-------------|----------------|------------|
| factor() API design | Confusing with qfactor | Use distinct name and help text; document when to use each |
| Hensel lifting | Coefficient growth | Early termination; aggressive Frac reduction |
| Series → poly | Sparse/dense, q_shift, trunc | Normalize, build dense vector, validate trunc |
| Degree 42 | Worst-case recombination | Timeout; early termination; test on T(8,8) first |
| Cyclotomic output | Format vs algorithm | Factor into irreducibles; optionally recognize cyclotomics for display |
| Zero deps | BigInt performance | Profile; consider caching, lazy reduce; accept slower than GMP |

---

## Performance Expectations for Degree 42

- **T(8,8):** Polynomial of degree 42, integer coefficients, factors into cyclotomics. With schoolbook BigInt and zero deps, expect roughly 0.5–5 seconds depending on implementation. Maple/Magma with GMP are much faster.
- **Coefficient size:** T(8,8) coefficients are moderate. Explosive growth is more likely with higher degree or larger coefficients.
- **Bottlenecks:** BigInt multiplication/division, Frac::reduce (GCD), and polynomial multiplication during recombination.

---

## Sources

- van Hoeij, "The complexity of factoring univariate polynomials over the rationals": https://www.math.fsu.edu/~hoeij/algorithms/Qx.html
- van Hoeij, "Factoring polynomials and the knapsack problem" (J. Number Theory 2002)
- Zassenhaus algorithm; Cantor–Zassenhaus; Hensel lifting (Wikipedia, standard refs)
- qseriesdoc.md §3.2 (T(8,8), factor vs qfactor)
- SPEC.md, convert.h (qfactor, prodmake, Series structure)
