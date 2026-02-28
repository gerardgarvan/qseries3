---
title: Sifting & Summation
description: Extracting subsequences of coefficients and computing finite sums
---

## sift

Extract every $m$-th coefficient starting at residue $r$. Given a series

$$A(q) = \sum_{n=0}^{\infty} a_n q^n$$

`sift(A, m, r)` returns the subseries

$$\sum_{n=0}^{\infty} a_{mn+r}\, q^n$$

This is the key operation for studying partition congruences. For example,
Ramanujan's congruence $p(5n+4) \equiv 0 \pmod{5}$ can be verified by sifting
the partition generating function.

**Syntax**

```text
sift(f: Series, m: int, r: int, T?: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | The series to sift |
| `m` | int | Sifting modulus |
| `r` | int | Residue class ($0 \le r < m$) |
| `T` | int (optional) | Truncation order for the result |

**Example**

Verify that $p(5n+4)$ is divisible by 5 — every coefficient in the sifted
series should be a multiple of 5:

```text
qseries> p := 1/etaq(1, 200)
qseries> sift(p, 5, 4, 38)
5 + 30*q + 135*q^2 + 490*q^3 + 1575*q^4 + 4620*q^5 + 12635*q^6 + ... + O(q^38)
```

Every coefficient is divisible by 5, confirming Ramanujan's congruence.

---

## sum

Compute a finite summation. The summation variable ranges over integers from
`lo` to `hi`, and the expression is evaluated and accumulated at each step:

$$\sum_{\text{var}=\text{lo}}^{\text{hi}} \text{expr}$$

This is the standard way to build q-hypergeometric series.

**Syntax**

```text
sum(expr, var: symbol, lo: int, hi: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `expr` | expression | The summand (may contain `var` and `q`) |
| `var` | symbol | Summation variable name |
| `lo` | int | Lower bound (inclusive) |
| `hi` | int | Upper bound (inclusive) |

**Example**

Compute the first Rogers-Ramanujan series $G(q) = \sum_{n=0}^{\infty} \frac{q^{n^2}}{(q;q)_n}$:

```text
qseries> sum(q^(n^2)/aqprod(q, q, n, 50), n, 0, 8)
1 + q + q^2 + q^3 + 2*q^4 + 2*q^5 + 3*q^6 + ... + O(q^50)
```

---

## add

Alias for `sum`. Identical behavior.

**Syntax**

```text
add(expr, var: symbol, lo: int, hi: int) → Series
```

**Example**

```text
qseries> add(q^(n*(n+1))/aqprod(q, q, n, 50), n, 0, 8)
1 + q^2 + q^3 + q^4 + q^5 + 2*q^6 + ... + O(q^50)
```
