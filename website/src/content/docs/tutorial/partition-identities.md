---
title: Partition Identities
description: Sifting coefficients and discovering partition congruences
---

## The sift operation

Given a q-series $A(q) = \sum a_n q^n$, the `sift` function extracts
coefficients along an arithmetic progression:

$$
\texttt{sift}(A, m, r) = \sum_{n=0}^{\infty} a_{mn+r}\, q^n
$$

This is the key tool for studying partition congruences. Ramanujan discovered
that $p(5n+4) \equiv 0 \pmod{5}$, $p(7n+5) \equiv 0 \pmod{7}$, and
$p(11n+6) \equiv 0 \pmod{11}$. Sifting lets us extract the generating
functions for these subsequences and identify their product forms.

## Distinct partitions mod 5

Let $pd(n)$ denote the number of partitions of $n$ into distinct parts. Its
generating function is:

$$
\sum_{n=0}^{\infty} pd(n)\, q^n = \prod_{n=1}^{\infty}(1+q^n) = \frac{(q^2;q^2)_\infty}{(q;q)_\infty}
$$

What happens when we extract $pd(5n+1)$?

```text
qseries> set_trunc(200)

qseries> PD := etaq(2, 200) / etaq(1, 200)
1 + q + q² + 2q³ + 2q⁴ + 3q⁵ + 4q⁶ + 5q⁷ + 6q⁸ + 8q⁹ + 10q¹⁰
  + 12q¹¹ + 15q¹² + 18q¹³ + 22q¹⁴ + ... + O(q²⁰⁰)
```

Sift out the $pd(5n+1)$ subsequence and identify its eta product form:

```text
qseries> PD1 := sift(PD, 5, 1, 199)
1 + 4q + 12q² + 32q³ + 76q⁴ + 165q⁵ + 340q⁶ + 668q⁷ + 1260q⁸
  + 2304q⁹ + ... + O(q⁴⁰)

qseries> etamake(PD1, 38)
η(2τ)² η(5τ)³ / (η(τ)⁻⁴ η(10τ)⁻¹)
```

The `etamake` result tells us:

$$
\sum_{n=0}^{\infty} pd(5n+1)\, q^n = \frac{(q^2;q^2)_\infty^2\, (q^5;q^5)_\infty^3}{(q;q)_\infty^4\, (q^{10};q^{10})_\infty}
$$

This identity was found originally by Rødseth.

## Ramanujan's most beautiful identity

Hardy and MacMahon both agreed that Ramanujan's most beautiful identity is:

$$
\sum_{n=0}^{\infty} p(5n+4)\, q^n = 5 \prod_{n=1}^{\infty} \frac{(1-q^{5n})^5}{(1-q^n)^6}
$$

We verify this by sifting the partition function:

```text
qseries> p := 1/etaq(1, 200)
1 + q + 2q² + 3q³ + 5q⁴ + 7q⁵ + 11q⁶ + 15q⁷ + 22q⁸ + 30q⁹ + ... + O(q²⁰⁰)

qseries> p5 := sift(p, 5, 4, 199)
5 + 30q + 135q² + 490q³ + 1575q⁴ + 4565q⁵ + 12310q⁶ + 31185q⁷
  + 75175q⁸ + ... + O(q³⁹)

qseries> etamake(p5, 38)
η(5τ)⁵ / (η(τ)⁻⁶)
```

The leading coefficient is 5 (since $p(4) = 5$), and the eta product structure
is $\eta(5\tau)^5 / \eta(\tau)^6$. Together these confirm Ramanujan's identity:
the generating function for $p(5n+4)$ is $5 \cdot (q^5;q^5)_\infty^5 / (q;q)_\infty^6$.

Every coefficient of `p5` is divisible by 5, which is the congruence
$p(5n+4) \equiv 0 \pmod{5}$.

## The T(r,n) functions and qfactor

Andrews defined rational functions $T(r, n)$ used in Rogers' original proof of
the Rogers-Ramanujan identities. These are defined recursively using Gaussian
binomial coefficients. The `qfactor` function factors them into q-products:

```text
qseries> set_trunc(64)

qseries> t8 := T(8, 8)
q⁶ + q⁷ + 2q⁸ + 3q⁹ + 5q¹⁰ + 6q¹¹ + 9q¹² + 11q¹³ + 15q¹⁴ + 17q¹⁵
  + 21q¹⁶ + 23q¹⁷ + 28q¹⁸ + 29q¹⁹ + 33q²⁰ + 34q²¹ + 37q²² + 36q²³
  + 38q²⁴ + 36q²⁵ + 37q²⁶ + 34q²⁷ + ... + O(q⁶⁴)

qseries> qfactor(t8, 20)
q⁶·(1-q⁹)(1-q¹⁰)(1-q¹¹) / (((1-q¹)(1-q²)(1-q³)(1-q⁴)))
```

The result is a q-product:

$$
T(8, 8) = \frac{(q^9;q)_3\, q^6}{(q;q)_4}
$$

where $(q^9;q)_3 = (1-q^9)(1-q^{10})(1-q^{11})$. The `qfactor` function is a
variant of `prodmake` that also handles finite rational functions in $q$.

## Euler's product and the quintuple product

Euler's product $\prod_{n=1}^{\infty}(1-q^n)$ can be dissected by residue
mod 5:

$$
\prod_{n=1}^{\infty}(1-q^n) = E_0(q^5) + q\, E_1(q^5) + q^2\, E_2(q^5) + q^3\, E_3(q^5) + q^4\, E_4(q^5)
$$

By Euler's pentagonal number theorem, the exponents $n(3n-1)/2$ satisfy
$n(3n-1)/2 \not\equiv 3$ or $4 \pmod{5}$, so $E_3 = E_4 = 0$:

```text
qseries> set_trunc(500)

qseries> EULER := etaq(1, 500)
1 - q - q² + q⁵ + q⁷ - q¹² - q¹⁵ + q²² + q²⁶ - q³⁵ - q⁴⁰ + ... + O(q⁵⁰⁰)

qseries> E3 := sift(EULER, 5, 3, 499)
0 + O(q¹⁰⁰)
```

Now identify $E_0$ using `prodmake`:

```text
qseries> E0 := sift(EULER, 5, 0, 499)
1 + q - q³ - q⁷ - q⁸ - q¹⁴ + q²⁰ + q²⁹ + q³¹ + q⁴² - q⁵² - ... + O(q¹⁰⁰)

qseries> prodmake(E0, 50)
((-q²+1) (-q³+1) (-q⁵+1) (-q⁷+1) (-q⁸+1) ...) /
  (((1-q) (-q⁴+1) (-q⁶+1) (-q⁹+1) (-q¹¹+1) ...))
```

The numerator factors appear at exponents $\equiv 0, \pm 2 \pmod{5}$ and the
denominator at exponents $\equiv \pm 1 \pmod{5}$. This gives:

$$
E_0 = \frac{(q^2;q^5)_\infty\, (q^3;q^5)_\infty\, (q^5;q^5)_\infty}{(q;q^5)_\infty\, (q^4;q^5)_\infty}
$$

This product is an instance of the quintuple product identity — specifically,
the case obtained by substituting $q \to q^5$ and $z \to q$ in Watson's
quintuple product formula.
