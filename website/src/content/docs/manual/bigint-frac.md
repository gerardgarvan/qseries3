---
title: Exact Arithmetic
description: "BigInt and Frac: the foundation of exact computation"
---

Every computation in qseries is **exact**. There is no floating point anywhere
in the math pipeline — coefficients are arbitrary-precision rationals, and
results are suitable for *proving* identities, not just checking them
numerically.

## Architecture

Three layers build on each other:

| Layer | Type | Role |
|-------|------|------|
| **BigInt** | Arbitrary-precision integer | Raw integer storage |
| **Frac** | Exact rational number | Coefficient type |
| **Series** | Truncated formal power series | The main object |

## BigInt

Arbitrary-precision integers stored internally in base $10^9$. Supports all
standard arithmetic operations: addition, subtraction, multiplication, division,
modular arithmetic, and comparison. Users rarely interact with BigInt directly —
it is the foundation beneath `Frac`.

## Frac

Exact rational numbers represented as a numerator/denominator pair of BigInts.
Every arithmetic operation **auto-reduces** via GCD, so fractions never
accumulate unnecessary factors:

- $6/4$ is always stored as $3/2$
- $0/5$ is always stored as $0/1$
- The denominator is always positive

This auto-reduction is critical for performance: without it, numerators and
denominators would grow exponentially during series arithmetic.

## Series

Truncated formal power series with `Frac` coefficients. Internally, each series
is a sparse map from exponent to coefficient, plus a truncation order $T$:

$$f(q) = \sum_{n} a_n q^n + O(q^T)$$

Only nonzero coefficients are stored, so sparse series (common in q-series
theory) use minimal memory. All binary operations propagate truncation:

$$\text{trunc}(f \cdot g) = \min(\text{trunc}(f),\, \text{trunc}(g))$$

## Why exact?

Floating-point arithmetic introduces rounding errors that compound across
operations. In q-series computation, this causes two problems:

1. **False positives** — two series that differ at high-order terms may appear
   equal when rounded
2. **False negatives** — genuine identities may appear to fail due to
   accumulated error

Exact rational arithmetic eliminates both risks entirely. When qseries reports
that two expressions are equal to $O(q^T)$, they are *provably* equal to that
order.

## Example

The partition function $p(n)$ counts the number of partitions of $n$. Its
generating function is the reciprocal of Euler's product. Every coefficient
is computed exactly:

```text
qseries> coeffs(1/etaq(1, 20), 0, 10)
[1, 1, 2, 3, 5, 7, 11, 15, 22, 30, 42]
```

These are the exact partition numbers $p(0)$ through $p(10)$ — no rounding, no
approximation.
