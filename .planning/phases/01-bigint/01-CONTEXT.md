# Phase 1: BigInt - Context

**Gathered:** 2025-02-24
**Status:** Ready for planning

<domain>
## Phase Boundary

Arbitrary-precision signed integers (base 10⁹) for exact arithmetic. Foundation layer for Frac and Series. Per SPEC: vector<uint32_t>, least-significant first; +, -, *, divmod, gcd; long division with binary-search quotient. Zero is always non-negative.

</domain>

<decisions>
## Implementation Decisions

### Error Handling
- Throw on invalid situations (division by zero, invalid string input)
- Validate divisor before division; throw if zero
- Exception type: Claude's discretion (std::invalid_argument, std::runtime_error, or custom)

### String I/O Contract
- str() format (zero, negatives, leading zeros): Claude's discretion
- Constructor: whitespace handling, leading zeros: Claude's discretion

### Edge Case Scope
- Spec-driven — test only what SPEC and .cursorrules explicitly list
- SPEC mentions: 0*anything=0, (-a)*(-b)=a*b, division (1000000000/1, 999999999/1000000000, 123456789012345/123, negatives)

### Validation Checkpoint
- Standalone test binary vs integration-only: Claude's discretion
- Test coverage and invocation: Claude's discretion

### Claude's Discretion
- Exception type for thrown errors
- String I/O format and constructor behavior
- Validation approach (standalone test, integration, or both)

</decisions>

<specifics>
## Specific Ideas

No specific requirements — open to standard approaches consistent with SPEC and .cursorrules.

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---
*Phase: 01-bigint*
*Context gathered: 2025-02-24*
