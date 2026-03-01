# Phase 4: qfuncs — Context

**Gathered:** 2025-02-24
**Status:** Ready for planning

**Guiding principle:** Match [Garvan's qseries conventions](https://qseries.org/fgarvan/qmaple/qseries/functions/); follow q-series theory; choose practical options for number-theory helpers.

<domain>
## Phase Boundary

q-series building blocks and number theory helpers. Per ROADMAP: aqprod, etaq, theta2/3/4, theta, qbin, tripleprod, quinprod, winquist; divisors, mobius, legendre, sigma, euler_phi. Success: etaq(1,T) matches Euler pentagonal; partition 1/etaq(1,50) matches SPEC; theta/eta/prod tests pass.
</domain>

<decisions>
## Implementation Decisions

### Function Signatures — Garvan Convention

Match [Garvan's calling sequences](https://qseries.org/fgarvan/qmaple/qseries/functions/):

| Function | Garvan | Our C++ (q = Series::q(T) or caller-provided) |
|----------|--------|----------------------------------------------|
| etaq | etaq(q,a,T) | etaq(q, k, T) — q first, k (eta parameter), T truncation |
| theta2 | theta2(q,T) | theta2(q, T) |
| theta3 | theta3(q,T) | theta3(q, T) |
| theta4 | theta4(q,T) | theta4(q, T) |
| theta | theta(z,q,T) | theta(z, q, T) — general theta Σ z^n q^{n²} |
| aqprod | aqprod(a,q,n) | aqprod(a, q, n, T) — add T for Series truncation |
| qbin | qbin(m,n) | qbin(m, n, T) — add T for truncation |
| tripleprod | tripleprod(z,q,T) | tripleprod(z, q, T) |
| quinprod | quinprod(a,b,q,T) or quinprod(z,q,T) | quinprod(z, q, T) per SPEC |
| winquist | winquist(a,b,q,T) | winquist(a, b, q, T) |

All `q`, `z`, `a`, `b` are Series. Caller passes `Series::q(T)` for standard q; `theta3(q.subs_q(2), T)` for theta3(q²).

### theta2 — Half-Integer Exponents

Garvan returns full θ₂(q) including q^{1/4} factor. Our Series use integer exponents only.

**Decision:** Output θ₂(q)/q^{1/4} with integer exponents — coefficients 2 at exponents n(n+1) for n≥0. Document as "theta2 returns θ₂/q^{1/4} (integer exponents only)". Matches SPEC and standard workaround for formal power series.

### General theta(z,q,T)

**In Phase 4.** Garvan has theta(z,q,T); include it. Used by tripleprod-related identities. Σ_{i=-T}^{T} z^i q^{i²}.

### Number Theory Helpers — Most Practical

- **divisors(n):** `std::vector<int>` — n typically < trunc
- **mobius(n):** `int` — returns -1, 0, 1
- **legendre(a, p):** `int` — returns -1, 0, 1; (a, p) as int or int64_t
- **sigma(n, k=1):** `int64_t` — overflow acceptable for typical research T
- **euler_phi(n):** `int64_t` — same

**Placement:** qfuncs.h or small nthelpers.h — implementation choice.

**prodmake** needs divisors; **findhomcombo** (Phase 8) needs legendre. Implement what Phase 5+ need.

### Edge Cases (Garvan / SPEC)

- aqprod(a,q,0) = 1; aqprod(a,q,-n) per Garvan (optional; implementation choice)
- qbin: return 0 if m<0 or m>n; return 1 if m==0 or m==n
- etaq, theta: T positive; invalid args — implementation choice

</decisions>

<specifics>
## Specific Ideas

- etaq: Π_{n≥1} (1 - q^{kn}), truncate to T after each factor
- aqprod: Π_{k=0}^{n-1} (1 - a·q^k); a and q are Series
- qbin: product formula [n;m]_q = Π_{i=1}^{m} (1-q^{n-m+i})/(1-q^i) to avoid series division
- theta3: coeff[0]=1, coeff[n²]+=2 for n≥1
- PITFALLS: propagate truncation; etaq(1,T) O(T·√T) acceptable per PROJECT
</specifics>

<deferred>
## Deferred Ideas

- Euler pentagonal optimization for etaq(1,T) — defer to post-v1
- etaq memoization — defer per PROJECT
- aqprod negative n — Garvan supports; implementation choice for Phase 4
</deferred>

---
*Phase: 04-qfuncs*
*Context gathered: 2025-02-24*
*Reference: https://qseries.org/fgarvan/qmaple/qseries/functions/*
