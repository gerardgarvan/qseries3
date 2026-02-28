---
title: Rogers-Ramanujan Identities
description: Computing and verifying the Rogers-Ramanujan continued fraction identities
---

## The first Rogers-Ramanujan identity

The Rogers-Ramanujan identity is one of the most celebrated results in partition
theory:

$$
\sum_{n=0}^{\infty} \frac{q^{n^2}}{(q;q)_n} = \prod_{n=1}^{\infty} \frac{1}{(1-q^{5n-1})(1-q^{5n-4})}
$$

The left side generates partitions whose parts differ by at least 2. The right
side generates partitions into parts $\equiv \pm 1 \pmod{5}$. The identity says
these two classes of partitions are equinumerous for every $n$.

## Computing the series

Compute the left side by summing to $n = 8$ — higher terms contribute only
beyond $q^{64}$ due to the $q^{n^2}$ factor:

```text
qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)
1 + q + q² + q³ + 2q⁴ + 2q⁵ + 3q⁶ + 3q⁷ + 4q⁸ + 5q⁹ + 6q¹⁰ + 7q¹¹
  + 9q¹² + 10q¹³ + 12q¹⁴ + 14q¹⁵ + 17q¹⁶ + 19q¹⁷ + 23q¹⁸ + 26q¹⁹
  + 31q²⁰ + 35q²¹ + 41q²² + 46q²³ + 54q²⁴ + ... + O(q⁵⁰)
```

## Identifying the product with prodmake

Andrews' `prodmake` algorithm converts a q-series into its infinite product
representation. Apply it to our Rogers-Ramanujan series:

```text
qseries> prodmake(rr, 40)
1 / (((1-q) (-q⁴+1) (-q⁶+1) (-q⁹+1) (-q¹¹+1) (-q¹⁴+1) (-q¹⁶+1)
  (-q¹⁹+1) (-q²¹+1) (-q²⁴+1) (-q²⁶+1) (-q²⁹+1) (-q³¹+1) (-q³⁴+1)
  (-q³⁶+1) (-q³⁹+1)))
```

The denominator factors appear at exponents 1, 4, 6, 9, 11, 14, 16, 19, 21,
24, 26, 29, 31, 34, 36, 39 — precisely the integers $\equiv \pm 1 \pmod{5}$.
We have computationally rediscovered the right side of the Rogers-Ramanujan
identity.

## Jacobi product form with jacprodmake

The `jacprodmake` function identifies the periodicity in the exponents and
writes the product in compact Jacobi notation:

```text
qseries> jp := jacprodmake(rr, 40)
1 / (((q,q^5)_∞ (q^4,q^5)_∞ (q^5,q^5)_∞))
```

Here $(q^a, q^b)_\infty = \prod_{n=0}^{\infty}(1 - q^{a+bn})$. The output tells
us the Rogers-Ramanujan series equals
$1 / ((q;q^5)_\infty (q^4;q^5)_\infty (q^5;q^5)_\infty)$, which is the
standard form of the right side.

The `jac2prod` function converts back to explicit product notation:

```text
qseries> jac2prod(jp)
1 / (((q,q^5)_∞ (q^4,q^5)_∞ (q^5,q^5)_∞))
```

## The second Rogers-Ramanujan identity

The companion identity replaces $q^{n^2}$ with $q^{n(n+1)}$:

$$
\sum_{n=0}^{\infty} \frac{q^{n(n+1)}}{(q;q)_n} = \prod_{n=1}^{\infty} \frac{1}{(1-q^{5n-2})(1-q^{5n-3})}
$$

Now the right side generates partitions into parts $\equiv \pm 2 \pmod{5}$:

```text
qseries> rr2 := sum(q^(n*(n+1))/aqprod(q,q,n,50), n, 0, 8)
1 + q² + q³ + q⁴ + q⁵ + 2q⁶ + 2q⁷ + 3q⁸ + 3q⁹ + 4q¹⁰ + 4q¹¹
  + 6q¹² + 6q¹³ + 8q¹⁴ + 9q¹⁵ + 11q¹⁶ + 12q¹⁷ + 15q¹⁸ + 16q¹⁹
  + 20q²⁰ + ... + O(q⁵⁰)
```

```text
qseries> prodmake(rr2, 40)
1 / (((-q²+1) (-q³+1) (-q⁷+1) (-q⁸+1) (-q¹²+1) (-q¹³+1) (-q¹⁷+1)
  (-q¹⁸+1) (-q²²+1) (-q²³+1) (-q²⁷+1) (-q²⁸+1) (-q³²+1) (-q³³+1)
  (-q³⁷+1) (-q³⁸+1)))
```

The denominator now appears at exponents 2, 3, 7, 8, 12, 13, 17, 18, ... —
precisely $\equiv \pm 2 \pmod{5}$.

And in Jacobi product form:

```text
qseries> jp2 := jacprodmake(rr2, 40)
1 / (((q^2,q^5)_∞ (q^3,q^5)_∞ (q^5,q^5)_∞))
```

## Andrews' prodmake algorithm

The `prodmake` algorithm is due to George Andrews. Given a q-series with
leading coefficient 1, write it formally as an infinite product:

$$
1 + \sum_{n=1}^{\infty} b_n q^n = \prod_{n=1}^{\infty} (1 - q^n)^{-a_n}
$$

Taking logarithmic derivatives of both sides yields a recurrence. Define
$c_n = \sum_{d \mid n} d \cdot a_d$, then:

$$
c_n = n \cdot b_n - \sum_{j=1}^{n-1} b_{n-j} \cdot c_j
$$

The exponents $a_n$ are recovered from $c_n$ by Möbius inversion:

$$
a_n = \frac{1}{n}\left(c_n - \sum_{\substack{d \mid n \\ d < n}} d \cdot a_d\right)
$$

When all $a_n$ are integers, the series has a genuine infinite product form.
The `prodmake` function computes this recurrence and displays the result. For a
complete treatment, see Andrews' *q-Series* (§10.7).
