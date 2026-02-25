# Phase 2: Frac - Context

**Gathered:** 2025-02-24
**Status:** Ready for planning

<domain>
## Phase Boundary

Exact rational arithmetic with no coefficient growth. Foundation for Series. Per SPEC: BigInt num, den; den > 0 after reduce(); reduce() in every constructor and after every operation. 6/4 → 3/2, 0/5 → 0/1. Standard arithmetic: a/b + c/d = (ad+bc)/(bd) then reduce. Comparison: a/b < c/d iff a*d < c*b when b,d > 0.

</domain>

<decisions>
## Implementation Decisions

### Error Handling
- Throw on Frac(n,0) and on operator/(a,b) when b is zero
- Same rule for both: both throw
- Overflow handling: Claude's discretion
- Exception type: Claude's discretion

### String Output Format
- str() format (integers, zero, negatives, large fractions, round-trip, reduced form, whitespace): Claude's discretion

### Validation Checkpoint
- Both standalone Frac tests and integration tests in Phase 3
- Test location (extend main.cpp vs separate file vs mode flag): Claude's discretion
- Broader test suite: SPEC cases (6/4→3/2, 0/5→0/1, long-chain growth) plus add/sub/mul/div and sign cases
- Critical: must pass before Phase 3

### Reduce Policy
- When reduce() is called (strict per-op vs batched), compound expressions, intermediate values, deferral: Claude's discretion

### Claude's Discretion
- Overflow handling and exception type
- String output format (all details)
- Test file location
- Reduce policy (timing, batching)

</decisions>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches consistent with SPEC and .cursorrules. PITFALLS: Frac reduction omitted causes exponential BigInt growth; reduce after every construction and operation.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---
*Phase: 02-frac*
*Context gathered: 2025-02-24*
