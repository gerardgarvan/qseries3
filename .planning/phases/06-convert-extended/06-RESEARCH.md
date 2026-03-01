# Phase 6: convert-extended — Research

**Researched:** 2025-02-25  
**Domain:** Product identification and coefficient extraction (etamake, jacprodmake, jac2prod, jac2series, qfactor, sift, T(r,n))  
**Confidence:** HIGH

## Summary

Phase 6 extends `convert.h` with product identification and coefficient extraction: **etamake** (eta-product identification), **jacprodmake** (Jacobi-product identification plus jac2prod/jac2series), **sift** (extract coefficients by residue class), **qfactor** (rational function as q-product), and a **T(r,n)** helper for TEST-07. All algorithms are specified in SPEC §5.2–5.4, §6 and in qseriesdoc §3.2–3.4, §5, §6.2. The codebase already has `prodmake`, `etaq`, `aqprod`, `qbin`, and full Series arithmetic; no new external dependencies. The main design choices are: cap etamake iterations at 2*T or 3*T to avoid infinite loops; use ~80% periodicity match then verify by reconstruction for jacprodmake; implement qfactor by normalizing the series and running prodmake (no polynomial factorization in C++); and implement T(r,n) via the recurrence from qseriesdoc using qbin.

**Primary recommendation:** Implement etamake, sift, and T(r,n) first (no prodmake variant); then jacprodmake (calling prodmake, then periodicity + JAC decomposition); then jac2prod/jac2series; then qfactor as prodmake on normalized series with numerator/denominator interpretation.

---

<user_constraints>

## User Constraints (from CONTEXT.md)

### Locked Decisions

- **etamake:** Max iterations 2*T or 3*T; on failure report "not an eta product"; output list (k, e_k) + display string.
- **jacprodmake:** ~80% periodicity match then verify by reconstruction; structured JAC list; jac2prod, jac2series.
- **sift(f, n, k, T):** n = modulus, k = residue; return Series Σ a_{ni+k} q^i.
- **qfactor:** Rational in q; built-in T(r,n) helper for T(8,8); default T = 4d+3.

### Claude's Discretion

- Exact max-iteration value for etamake (2*T vs 3*T); etamake return type (optional vs empty + flag).
- JAC factor representation (struct vs tuple vs map).
- qfactor internal steps (factor then prodmake per doc; C++ has no polynomial factorisation — use prodmake on normalized series and interpret result as rational q-product).

### Deferred Ideas (OUT OF SCOPE)

- REPL display formatting for eta/JAC — Phase 10.
- LaTeX output for eta (Garvan _etalist for ramarobinsids) — out of scope.

</user_constraints>

---

## Dependencies

| Dependency | Location | Purpose |
|------------|----------|---------|
| `prodmake(f, T)` | convert.h | Returns `a[n]` for f = leading · Π (1-q^n)^{-a[n]}; required by jacprodmake and qfactor. |
| `etaq(q, k, T)` | qfuncs.h | Π_{n≥1} (1 - q^{kn}), used by etamake to cancel terms. |
| `aqprod`, `qbin` | qfuncs.h | qbin used by T(r,n); aqprod used by jac2series for (q^a;q^b)_∞. |
| Series | series.h | coeff, setCoeff, minExp, *, /, inverse, truncTo, pow. |
| `divisors(n)` | qfuncs.h | Already used by prodmake; useful if any divisor iteration needed in JAC. |

No new headers or libraries. All of Phase 6 lives in `convert.h` except T(r,n), which may live in `qfuncs.h` or `convert.h` (discretion: recommend `convert.h` to keep qfactor and T together).

---

## etamake

**SPEC §5.2 algorithm:**

1. Start with g = f normalized (leading coefficient 1; if f has minExp > 0, work with g = f / q^minExp and account for q-power in output).
2. Find smallest exponent k > 0 such that g has nonzero coefficient c at q^k.
3. Multiply g by `etaq(q, k, T)^c` (c is the coefficient, so we cancel the q^k term). Record η(kτ)^{-c} — i.e. exponent e_k = -c for factor k.
4. Repeat until g = 1 (constant 1) or **max_iter** reached.

**Stopping / failure (locked):** Cap iterations at **max_iter = 2*T or 3*T**. On exit without g=1, return empty list and report "not an eta product".

**Return type:** e.g. `std::vector<std::pair<int,Frac>>` for (k, exponent e_k); empty on failure. Optionally a separate display string: e.g. `η(2τ)^5 / (η(τ)^2 η(4τ)^2)`.

**Implementation notes:**

- Normalize: g = f / f.coeff(minExp) and track leading q-power if minExp > 0; final display can show q^e · eta-product.
- Coefficient c at q^k is a Frac; multiply g by `etaq(q,k,T).pow(c)` is not directly available (pow is int). So use: if c is integer, use pow; else we only support integer exponents for eta. Per Garvan, etamake produces integer exponents; so round/truncate or require integer and fail otherwise. **Recommendation:** assume integer exponents (as in Garvan); if coeff is non-integer, treat as failure.
- So: get integer exponent e = -c (c from g at k); then g = g * etaq(q,k,T).pow(e) would *add* the term; we want to cancel, so multiply by etaq(q,k,T).pow(-e) = etaq(q,k,T).pow(c) when c is integer. So **multiply by etaq(q,k,T)^(c)** with c integer; record (k, -c) = (k, e_k).
- **Dependencies:** etaq, Series multiplication, inverse (if negative exponent), normalization. prodmake is **not** used by etamake.

**Garvan alignment (TEST-03):** theta3 → η(2τ)^5 / (η(4τ)^2 η(τ)^2); theta4 → η(τ)^2 / η(2τ). Our theta3/theta4 are integer series; etamake should match these.

---

## jacprodmake

**Algorithm (SPEC §5.3, qseriesdoc §3.4):**

1. Run **prodmake(f, T)** to get a[n]. Define **e[n] = -a[n]** (power of (1-q^n) in the product).
2. **Search for period b:** try b = 2, 3, … up to about T/2. For each b, check periodicity: e[n+b] == e[n] for n in a suitable range. Use **~80% match**: count n in [1, T-1-b] for which e[n+b]==e[n], require count >= 80% of checked indices. If no b qualifies, return failure.
3. **Verify:** Build the product series from the one-period pattern (e[1..b]) interpreted as JAC factors; compare to f (e.g. to O(q^T)). If mismatch, reject or try next b.
4. **Decompose one-period into JAC(a,b,∞):** For 0 ≤ a < b, JAC(a,b,∞) contributes to (1-q^n) only for n ≡ 0,a,b-a (mod b) (and for a=0 only n ≡ 0 (mod b)). So from e[1..b] compute exponents for each JAC(a,b,∞). Return list of (a, b, exponent).

**JAC definitions (locked):**

- **JAC(0, b, ∞)** = (q^b; q^b)_∞.
- **JAC(a, b, ∞)** for 0 < a < b = (q^a; q^b)_∞ (q^{b-a}; q^b)_∞ (q^b; q^b)_∞.

So (1-q^n) appears in: JAC(0,b) only for n = b, 2b, …; JAC(a,b) for n = a, b-a, b, 2b±a, etc. Decompose e[n] for n=1..b into a linear combination of these contributions (one period of each JAC factor).

**Return type:** Structured list of (a, b, exponent), e.g. `std::vector<std::tuple<int,int,Frac>>` or a small struct, so that jac2prod and jac2series can consume it.

**jac2prod(jac_expr):** Expand to (q^a;q^b)_∞ notation — string or structured product/quotient. Per Garvan Output (14): `1 / ((q,q^5)_∞ (q^4,q^5)_∞)`.

**jac2series(jac_expr, T):** For each JAC(a,b,∞)^e, build the series: (q^a;q^b)_∞ · (q^{b-a};q^b)_∞ · (q^b;q^b)_∞ (if a>0) or (q^b;q^b)_∞ (if a=0), raise to exponent e, then multiply all. Use aqprod-style products truncated to T: (q^a;q^b)_∞ = Π_{j≥0} (1 - q^{a+jb}) to O(q^T).

**Garvan alignment (TEST-04, TEST-09):** Rogers–Ramanujan → JAC(0,5,∞)/JAC(1,5,∞). Euler sifted E0 → JAC(2,5,∞)·JAC(0,5,∞)/JAC(1,5,∞).

---

## sift

**SPEC §6:** sift(f, n, k, T) = Σ_i a_{n·i+k} q^i + O(q^{T'}) where T' = effective length.

**Semantics (locked):** n = modulus, k = residue (0 ≤ k < n). Output series: exponent i, coefficient f.coeff(n*i+k). Truncation: while n*i+k < T we have valid coeffs; so last i is floor((T-1-k)/n), effective trunc = floor((T-1-k)/n)+1.

**Implementation:** New Series; for i = 0,1,… while n*i+k < T: set coeff i to f.coeff(n*i+k); set result.trunc = floor((T-1-k)/n)+1 (or i_max+1). Simple loop, no prodmake or eta.

**Garvan:** sift(s,q,n,k,T); we have sift(f, n, k, T) with f the series.

---

## qfactor

**Doc (qseriesdoc §3.2):** "First the function is factored as usual, and then it uses prodmake to do further factorisation into q-products." In C++ we have no polynomial factorization. **Approach (locked in CONTEXT):** Treat input as Series (polynomial = finite support). Normalize so constant term 1 (or leading term 1): if f is polynomial, f/coeff(minExp) and track q^minExp. Run **prodmake** on this normalized series to get a[n] with f = leading · Π (1-q^n)^{-a[n]}.

**Output:** f = q^e · (Π (1-q^n)^{e_n} for e_n>0) / (Π (1-q^n)^{e_n} for e_n<0). So: from prodmake, exponent of (1-q^n) in the product is -a[n]. So numerator factors (1-q^n)^{positive} when -a[n] > 0 ⇒ a[n] < 0; denominator when a[n] > 0. Leading power: e = minExp of original f (or from normalization). **qfactor(f, T)** = run prodmake on normalized f, then output q^e · num/den with num = Π (1-q^n)^{-a[n]} for a[n]<0, den = Π (1-q^n)^{a[n]} for a[n]>0.

**Default T:** When T omitted, use 4d+3 where d = max degree (maxExp) of numerator/denominator. For a single polynomial f, d = f.maxExp().

**TEST-07:** qfactor(T(8,8), 20) must give q^6 · (1-q^9)(1-q^10)(1-q^11)(1-q^16) / ((1-q)(1-q^2)(1-q^3)(1-q^4)).

---

## T(r,n) helper

**Source:** qseriesdoc §3.2, Eqs (3.3)–(3.5) and Maple code.

- T(r, 0) = 1, T(r, 1) = 0.
- T(r, N) = - Σ_{1≤2j≤N} [r+2j choose j]_q · T(r+2j, N-2j).

**Maple:** `[r+2k choose k]_q` = qbin(q, k, r+2*k). So in C++: **T(r, N)** = - Σ_{k=1}^{floor(N/2)} qbin(q, k, r+2*k, T) * T(r+2*k, N-2*k), with T(r,0)=1, T(r,1)=0. Returns a **Series** (polynomial). Truncation T for the returned series: choose T large enough (e.g. N*(r+N) or (r+2*ceil(N/2))*(N) order; or pass T as parameter). For TEST-07, T(8,8) is a polynomial of degree 42 (doc Output 3).

**Placement:** qfuncs.h (with qbin) or convert.h. Recommend **convert.h** next to qfactor so that T(8,8) and qfactor(t8, 20) are in one place.

**Implementation:** Recursive or iterative; build Series by summing terms. Each term is qbin(q, k, r+2*k, T) * T(r+2*k, N-2*k) (Series * Series). Need a truncation bound for the whole polynomial; use e.g. T = (r + 2*N)^2 or user-provided T.

---

## Garvan Alignment

| Test | Requirement |
|------|-------------|
| TEST-03 | etamake(theta3(q,100), 100) → η(2τ)^5/(η(4τ)^2 η(τ)^2); etamake(theta4(q,100), 100) → η(τ)^2/η(2τ). |
| TEST-04 | jacprodmake(RR, 40) → JAC(0,5,∞)/JAC(1,5,∞); jac2prod → 1/((q,q^5)_∞ (q^4,q^5)_∞). |
| TEST-06 | PD = etaq(2,200)/etaq(1,200); sift(PD, 5, 1, 199); etamake(PD1, 38) → Rødseth η(5τ)^3 η(2τ)^2/(η(10τ) η(τ)^4). |
| TEST-07 | t8 = T(8,8); qfactor(t8, 20) → q^6·(1-q^9)(1-q^10)(1-q^11)(1-q^16)/((1-q)(1-q^2)(1-q^3)(1-q^4)). |
| TEST-09 | EULER = etaq(1,500); E0 = sift(EULER, 5, 0, 499); jacprodmake(E0, 50) → JAC(2,5,∞)·JAC(0,5,∞)/JAC(1,5,∞). |

Garvan display uses η(τ), η(2τ), etc.; we output (k, e_k) and a string of the form η(kτ)^{e_k}.

---

## Pitfalls

1. **etamake infinite loop:** Must cap iterations at 2*T or 3*T; on failure return empty and "not an eta product". (SPEC PITFALLS §5.)
2. **jacprodmake 100% periodicity:** Do not require e[n+b]==e[n] for all n; use ~80% match then **verify by reconstruction** to avoid false negatives. (SPEC PITFALLS §6.)
3. **prodmake divisor sum:** Already correct in convert.h: exclude d=n in Σ_{d|n, d<n} d·a[d]. jacprodmake uses prodmake output as-is.
4. **qfactor input:** For polynomials, normalize (constant term 1) before prodmake; handle leading q-power in output. No need for general rational numerator/denominator factorization in C++; single series (or numerator/denominator as two series) and prodmake on the combined form is one approach; for strict "rational in q", if we have num/den we can do prodmake(num, T) and prodmake(den, T) and combine; doc says "factor then prodmake" — we only do prodmake on the series we have.
5. **T(r,n) truncation:** Ensure qbin and recursive T(..., j) use a sufficiently large T so that the product T(8,8) has all terms up to degree 42.
6. **Integer exponents:** etamake and JAC exponents in Garvan are integers; if prodmake yields non-integer a[n], jacprodmake may need to reject or round (per existing prodmake behavior we already warn on non-integer).

---

## References

- **SPEC.md:** §5.2 etamake, §5.3 jacprodmake, §5.4 qfactor, §6 sift; Acceptance Tests TEST-03, TEST-04, TEST-06, TEST-07, TEST-09; Common Pitfalls §5, §6.
- **qseriesdoc.md:** §3.2 qfactor and T(r,n) (Eqs 3.3–3.5, Maple T proc, Outputs 3–5); §3.3 etamake (Outputs 8–12); §3.4 jacprodmake, jac2prod, jac2series (Outputs 13–16); §5 sift and Rødseth (Outputs 27–28); §6.2 Euler pentagonal (Outputs 34–36).
- **06-CONTEXT.md:** Locked decisions and discretion for Phase 6.

---

## RESEARCH COMPLETE
