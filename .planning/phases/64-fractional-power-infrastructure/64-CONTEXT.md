# Phase 64 Context: Fractional Power Infrastructure

## Decisions

### Algorithm
- **Coefficient recurrence via logarithmic differentiation** (O(T²))
- If `h = (1+g)^α`, then `(1+g)·h' = α·g'·h` gives a coefficient recurrence
- Computes h[n] from h[0..n-1] and g[0..n] without computing powers of g
- Avoids the O(T³) cost of direct binomial summation

### REPL Exposure
- **Overload existing `pow`** — `pow(f, 1/2)` dispatches to `powFrac` automatically
- `pow(f, 3)` stays integer path, `pow(f, 1/2)` dispatches to fractional path
- Parser must handle fractional second argument (currently expects integer)

### Constant Term ≠ 1
- **Auto-normalize**: factor out c₀, compute c₀^(p/q) as exact rational, apply binomial to remainder
- Throw error if c₀^(p/q) is irrational (e.g., `3^(1/2)`)
- Requires `Frac::rational_pow(p, q)` or equivalent

### Zero Constant Term
- **Factor out leading q^k**: if f = q^k · g where g has nonzero constant, then f^(p/q) = q^{k·p/q} · g^(p/q)
- The fractional q-power `k·p/q` adjusts the result's q_shift
- Uses Phase 63's q_shift + normalize_q_shift() infrastructure

### Negative Exponents
- **Supported directly** via binomial recurrence — α can be any rational including negative
- No need to compose with inverse()

### Validation
- Throw if constant term is zero AND no leading q^k can be factored (empty series)
- Throw if c₀^(p/q) is not rational
