# Phase 25: checkprod and checkmult — Context

**Gathered:** 2026-02-25  
**Status:** Ready for planning

**Guiding principle:** Match Maple qseries checkprod/checkmult semantics; simplify API to 2 args where reasonable.

<domain>
## Phase Boundary

Validation utilities: checkprod (is f a "nice" product?) and checkmult (are coefficients multiplicative?). Both in convert.h; REPL dispatch and help.
</domain>

<decisions>
## Implementation Decisions

### checkprod
- **API:** checkprod(f, M, T) — M = max allowed |a[n]| for "nice", T = truncation. If 2 args: checkprod(f, T) with M=2 (exponents in {-1,0,1} = nice).
- **Return:** Pair (int minExp, bool nice) or (int minExp, optional exponent list). Simple: (minExp, nice) with nice = all |a[n]| < M for n=1..T-1.
- **Display:** "nice product" or "not a nice product (bound M)", plus minExp.

### checkmult
- **API:** checkmult(f, T) or checkmult(f, T, yes) for verbose (print all failures).
- **Return:** bool — true if multiplicative, false otherwise.
- **Algorithm:** For coprime m,n with m*n < T, check a[mn] == a[m]*a[n]. Use gcd from qfuncs/nthelpers.
- **Display:** "MULTIPLICATIVE" / "NOT MULTIPLICATIVE" plus optional failure pairs.

### Placement
- Both in convert.h (next to prodmake).
- REPL: dispatch checkprod, checkmult; help table entries.
</decisions>

<deferred>
## Deferred

- Full Maple [c, list] return for checkprod when not nice — return simple bool for v1.4; extend later if needed.
</deferred>
