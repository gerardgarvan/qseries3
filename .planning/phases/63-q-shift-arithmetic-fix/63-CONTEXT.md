# Phase 63: Q-Shift Arithmetic Fix - Context

**Gathered:** 2026-03-01
**Status:** Ready for planning

<domain>
## Phase Boundary

Series addition works for operands whose q_shifts differ by an integer. Normalizing integer q_shift parts into coefficient indices so that compatible series can be combined without errors.

</domain>

<decisions>
## Implementation Decisions

### Normalization Strategy
- **Eager normalization** — After every operation that changes q_shift (multiply, divide, pow, inverse), immediately absorb integer part into coefficient map
- q_shift always kept in **[0, 1)** range — matches Maple convention
- With eager normalization, both operands of operator+ will already have matching q_shifts if their fractional parts are the same — no special alignment logic needed in operator+

### Error Handling for Non-Integer Differences
- **Throw for non-integer differences** — e.g., q_shift 1/4 vs 1/3 is a genuine error
- Common denominator approach rejected (would require fundamental Series type redesign to fractional indices)
- All known Maple checklist failures involve integer differences only

### Display Format
- **Expanded individual terms** — `2q^(1/4) + 2q^(5/4) + 2q^(9/4) + ...` instead of `q^(1/4) * (2 + 2q + 2q³ + ...)`
- **Parenthesized notation** for fractional exponents — `q^(1/4)` not Unicode fractions
- **Integer-exponent series unchanged** — When q_shift is 0, display stays as `1 + 2q + 3q² + ...` (no q^(0) prefix)
- **O() term uses integer truncation** — `O(q^50)` not `O(q^(50+1/4))`

</decisions>

<specifics>
## Specific Ideas

- Model on Maple UX throughout — user should never think about q_shift mechanics
- Eager normalization means the operator+ path barely changes — just verify q_shifts match (they will after normalization)

</specifics>

<deferred>
## Deferred Ideas

None — discussion stayed within phase scope

</deferred>

---

*Phase: 63-q-shift-arithmetic-fix*
*Context gathered: 2026-03-01*
