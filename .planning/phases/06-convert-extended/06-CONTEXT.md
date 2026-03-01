# Phase 6: convert-extended — Context

**Gathered:** 2025-02-24
**Status:** Ready for planning

**Guiding principle:** Match [Garvan's q-series package](https://qseries.org/fgarvan/qmaple/qseries/) where specified; follow SPEC and PITFALLS for etamake/jacprodmake safeguards.

<domain>
## Phase Boundary

Product identification and coefficient extraction. Per ROADMAP: etamake, jacprodmake, jac2prod, jac2series, qfactor, sift. Success: TEST-03 (etamake on theta3/4), TEST-04 (jacprodmake on Rogers-Ramanujan → JAC(0,5,∞)/JAC(1,5,∞)), TEST-06 (sift + Rødseth), TEST-07 (qfactor T(8,8)), TEST-09 (jacprodmake on sifted etaq(1)).
</domain>

<decisions>
## Implementation Decisions

### etamake behavior and failure
- **Stopping:** Iterate until g=1 (normalized) or **max iterations** reached. Cap at e.g. 2*T or 3*T (research to fix). PITFALLS: must not loop forever on non-eta input.
- **Failure:** On max iterations without g=1, report failure (e.g. return empty or optional type, and/or warn "not an eta product"); do not hang.
- **Output:** Match Garvan: eta-product display (e.g. η(2τ)⁵/(η(τ)²·η(4τ)²)). Internal representation: list of (k, e_k) for η(kτ)^{e_k} so downstream can use it; plus human-readable string for REPL/display.

### jacprodmake periodicity and output
- **Periodicity:** Use **~80% match** over a suitable range (e.g. e[n+b] == e[n] for ≥80% of n in range), then **verify by reconstruction** (build series from JAC factors, compare to original). PITFALLS and RESEARCH: 100% match causes false negatives.
- **No period found:** Return empty or failure indicator; do not guess.
- **Output:** jacprodmake(f, T) → structured list of JAC factors (e.g. (a, b, exponent) or equivalent) so jac2prod and jac2series can consume it. Display format: Garvan style `JAC(0,5,∞)/JAC(1,5,∞)`.
- **jac2prod(jac_expr):** Expand to (q^a;q^b)_∞ notation (string or structured form).
- **jac2series(jac_expr, T):** Return Series — product of theta-type series to O(q^T).

### sift API and semantics
- **Signature:** sift(f, n, k, T) per SPEC and [Garvan sift](https://qseries.org/fgarvan/qmaple/qseries/functions/sift.html): n = modulus, k = residue (0 ≤ k < n).
- **Semantics:** Extract a_{n·i+k} from f = Σ a_j q^j; return Series Σ_i a_{n·i+k} q^i + O(q^{ceil((T-1-k)/n)}). So output Series has exponent i and coeff a_{ni+k}; trunc ≈ (T - 1 - k) / n + 1.
- **Garvan alignment:** sift(s, q, n, k, T) in Maple; we have no q in signature (f is the series), so sift(f, n, k, T).

### qfactor scope and T(8,8)
- **Input:** Rational function in q (numerator and denominator as polynomials, or Series for polynomial case). SPEC: "Like prodmake but for polynomials."
- **Output:** q^e · Π (1-q^i)^{b_i} form; display as in Garvan/qseriesdoc (numerator/denominator products).
- **T(8,8) for TEST-07:** T(r,n) is defined by recurrence in qseriesdoc §3.2. Provide a **built-in helper** T(r, n) returning the polynomial as a Series (or equivalent) so that qfactor(T(8,8), 20) can be tested. Default T for qfactor when omitted: 4d+3 per doc (d = max degree of num/den).

### Claude's Discretion
- Exact max-iteration value for etamake; etamake return type (optional vs empty + flag).
- JAC factor representation (struct vs tuple vs map).
- qfactor internal steps (factor then prodmake per doc; C++ has no polynomial factorisation — may need to use prodmake on rational or simplified approach).
</decisions>

<specifics>
## Specific Ideas

- Garvan etamake: converts to eta-product; uses globals _etalisttop, _etalistbot; display η(τ), η(2τ), etc.
- Garvan sift(s,q,n,k,T): sum a_{n*i+k} q^i; we use sift(f, n, k, T).
- Rogers-Ramanujan: jacprodmake → JAC(0,5,∞)/JAC(1,5,∞); jac2prod → 1/((q,q⁵)_∞ (q⁴,q⁵)_∞).
- Euler pentagonal (TEST-09): EULER = etaq(q,1,500); E0 = sift(EULER, 5, 0, 499); jacprodmake(E0, 50) → JAC(2,5,∞)·JAC(0,5,∞)/JAC(1,5,∞).
- Rødseth (TEST-06): sift(PD, 5, 1, 199) then etamake → η(5τ)³·η(2τ)²/(η(10τ)·η(τ)⁴).
- qfactor(t8, 20): t8 = T(8,8); expected q⁶·(1-q⁹)(1-q¹⁰)(1-q¹¹)(1-q¹⁶)/((1-q)(1-q²)(1-q³)(1-q⁴)).
</specifics>

<deferred>
## Deferred Ideas

- REPL display formatting for eta/JAC — Phase 10
- LaTeX output for eta (Garvan _etalist for ramarobinsids) — out of scope
</deferred>

---
*Phase: 06-convert-extended*
*Context gathered: 2025-02-24*
*References: SPEC §5.2–5.4, §6; qseriesdoc §3.2, §3.4, §6.2; PITFALLS §7–8; Garvan etamake, sift, jacprodmake*
