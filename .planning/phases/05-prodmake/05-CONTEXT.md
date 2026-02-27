# Phase 5: prodmake — Context

**Gathered:** 2025-02-24
**Status:** Ready for planning

**Guiding principle:** Match [Garvan's q-series package](https://qseries.org/fgarvan/qmaple/qseries/) output and behavior; optimize for identity discovery workflow.

<domain>
## Phase Boundary

Andrews' algorithm recovers infinite product from q-series; Rogers-Ramanujan is the canary. Per ROADMAP: prodmake(f,q,T) converts series to product form; divisor sum excludes d=n; recurrence uses j=1..n-1. Success: prodmake on Σ q^(n²)/(q;q)_n yields product with denominators only at exponents ≡ ±1 (mod 5).
</domain>

<decisions>
## Implementation Decisions

### Output representation
- Match [Garvan's prodmake](https://qseries.org/fgarvan/qmaple/qseries/functions/prodmake.html): product form display like `1 / ((1-q)(1-q⁴)(1-q⁶)...)`
- Internal: return `a[n]` exponent array for downstream (jacprodmake, etamake) — identity discovery workflow
- Leading coefficient: optimize for identity discovery; normalization per Garvan

### Input handling
- Match [Garvan's q-series package](https://qseries.org/fgarvan/qmaple/qseries/) behavior for: b[0]=0, b[0]<0, non-integer coefficients, truncation T vs input truncation
- Research during planning to verify Garvan's edge-case behavior; if unclear, use sensible defaults

### Non-product and failure
- **Warn** (don't throw) when: non-integer a[n], leading coeff zero (minExp>0), empty/trivial input, other non-product conditions
- Still return best-effort result (raw a[n]) so callers can decide
- Research Garvan's behavior where possible

### Rogers-Ramanujan test specifics
- **Truncation T:** default 50, allow caller to specify range (e.g. 40–60)
- **Sum terms:** default Garvan (n=0..8, 9 terms), allow higher n for accuracy
- **Verification scope:** research during planning (which n to check, how far)
- **Acceptance threshold:** research during planning (exact zero vs Frac rounding tolerance)

### Claude's Discretion
- Exact a[n] return type (std::map<int,Frac>, std::map<int,int>, struct)
- Display string format details
- Warning delivery (stderr, callback, etc.)
</decisions>

<specifics>
## Specific Ideas

- Garvan prodmake on Rogers-Ramanujan: `1 / ((1-q)(1-q⁴)(1-q⁶)(1-q⁹)...)` — denominators at 1,4,6,9,11,14,... ≡ ±1 (mod 5)
- jacprodmake uses prodmake exponents for periodicity search — structured a[n] essential
- Divisor sum excludes d=n (locked per PITFALLS); c[n] recurrence j=1..n-1 (locked per Andrews)
</specifics>

<deferred>
## Deferred Ideas

- etamake, jacprodmake, qfactor — Phase 6 (convert-extended)
- REPL display formatting — Phase 10
</deferred>

---
*Phase: 05-prodmake*
*Context gathered: 2025-02-24*
*Reference: https://qseries.org/fgarvan/qmaple/qseries/functions/prodmake.html*
