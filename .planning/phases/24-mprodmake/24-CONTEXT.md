# Phase 24: mprodmake — Context

**Gathered:** 2026-02-25  
**Status:** Ready for planning

**Guiding principle:** Match [Maple mprodmake](https://qseries.org/fgarvan/qmaple/qseries/functions/mprodmake.html) — convert q-series to product (1+q^n1)(1+q^n2)....

<domain>
## Phase Boundary

Convert q-series f to m-product form ∏(1+q^n) for some set S of exponents. Reuse prodmake; extract S from a[] and verify consistency.
</domain>

<decisions>
## Implementation Decisions

### API and return type
- **Signature:** mprodmake(f, T) — f = Series, T = truncation. No q parameter (matches prodmake).
- **Return:** std::vector<int> — sorted exponents n in S. Empty vector = failure or constant 1.

### Failure behavior
- **Not an m-product:** Any a[n] ∉ {-1,0,1} or non-integer → stderr message, return {}.
- **Constant 1:** S empty → return {}; display shows "1".
- **No constant term / b[0]=0:** Same as prodmake — fail, return {}.

### Algorithm (locked)
1. Normalize f (leading coeff 1); fail if minExp>0.
2. prodmake(f, T) → a[n].
3. Check all a[n] ∈ {-1,0,1}, integer.
4. S = {n : a[n]=1} ∪ {n : a[2n]=-1}.
5. Sort S, return.

### Display format
- Match Maple: (1+q)(1+q³)(1+q⁵)... — use Series::expToUnicode for exponents.
- formatMprodmake(std::vector<int> S) → string; empty S → "1".

### Acceptance test
- ep := etaq(2,T)^2 / etaq(4,T) / etaq(1,T); mprodmake(ep, 17) → (1+q)(1+q³)(1+q⁵)...(1+q¹⁷)
</decisions>

<specifics>
## Specific Ideas

- Maple example: etaq(2)^2/etaq(4)/etaq(1) → odd exponents 1,3,5,...,17.
- prodmake returns a[n] with f = Π(1-q^n)^{-a[n]}; for (1+q^n) we have a[n]=1, a[2n]=-1.
- Overlapping factors: (1+q)(1+q²) gives a[1]=1, a[2]=0, a[4]=-1; S={1,2}.
</specifics>

<deferred>
## Deferred

- Truncation "..." for very large S — show all for typical T.
- mprodmake with q-power (minExp>0) — fail for now; could extend later.
</deferred>
