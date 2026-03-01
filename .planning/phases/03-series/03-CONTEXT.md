# Phase 3: Series - Context

**Gathered:** 2025-02-24
**Status:** Ready for planning

<domain>
## Phase Boundary

Truncated power series with correct arithmetic and inversion. Per SPEC: std::map<int,Frac> c (sparse), int trunc; +, -, *, /, pow, inverse; subs_q(k); truncation propagation. Inverse recurrence uses j=1..n (not j=0). Multiplication skips pairs where e1+e2 >= trunc. Result trunc = min(a.trunc, b.trunc) for binary ops.

</domain>

<decisions>
## Implementation Decisions

### Inverse Failure Handling
- Throw on invalid inverse() (c₀=0, zero series, identically zero)
- Same rule for series with minExp>0 when shift/invert fails
- Division by non-invertible series: Claude's discretion (match inverse or define separately)

### Display Format
- str() format: "1 + q + 2q² + 3q³ + O(q^T)" — compact with Unicode exponents
- Constant term shown as "1" (omit *q^0)
- When maxTerms reached: " + O(q^T)"
- Negative coefficients: Claude's discretion

### subs_q Edge Cases
- subs_q(0): Allow — return constant series (collapse all terms)
- subs_q(k) for k<0: Claude's discretion
- subs_q(1) no-op vs full copy: Claude's discretion
- k limit: No limit; consider warning for very large k

### Validation Checkpoint
- Both standalone Series tests and integration tests in Phase 4
- Test location: Claude's discretion
- Broader suite: SPEC cases ((1-q)*(1/(1-q))=1, inverse j=1..n, truncation, mult skip) plus add/sub/mul/div, subs_q, pow, inverse edge cases
- Critical: must pass before Phase 4 (qfuncs)

### Claude's Discretion
- Division by non-invertible (match inverse or not)
- Negative coefficient display format
- subs_q(k) for k<0 and subs_q(1) behavior
- subs_q warning for very large k (whether to add)
- Test file location

</decisions>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches. PITFALLS: Inverse recurrence j from 1 to n (not 0); truncation propagation in all binary ops; multiplication skip e1+e2 >= trunc.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---
*Phase: 03-series*
*Context gathered: 2025-02-24*
