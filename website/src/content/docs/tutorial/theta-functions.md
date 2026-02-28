---
title: Theta & Eta Functions
description: Theta functions as eta products and the etamake algorithm
---

## Jacobi theta functions

The Jacobi theta functions $\theta_2$, $\theta_3$, $\theta_4$ (in the $z=1$
specialization) are:

$$
\theta_2(q) = \sum_{n=-\infty}^{\infty} q^{(n+1/2)^2}, \quad
\theta_3(q) = \sum_{n=-\infty}^{\infty} q^{n^2}, \quad
\theta_4(q) = \sum_{n=-\infty}^{\infty} (-1)^n q^{n^2}
$$

These functions are fundamental in the theory of modular forms and elliptic
functions. Every modular form on a congruence subgroup can be expressed in terms
of theta functions and the Dedekind eta function.

## The Dedekind eta function

The Dedekind eta function is defined as:

$$
\eta(\tau) = q^{1/24} \prod_{n=1}^{\infty}(1-q^n), \quad q = e^{2\pi i \tau}
$$

In qseries, `etaq(k, T)` computes $\prod_{n=1}^{\infty}(1-q^{kn})$ — the eta
product without the $q^{1/24}$ factor. With $k=1$ this gives Euler's product,
whose expansion is the pentagonal number theorem:

```text
qseries> etaq(1, 20)
1 - q - q² + q⁵ + q⁷ - q¹² - q¹⁵ + O(q²⁰)
```

The nonzero terms appear at the pentagonal numbers $n(3n-1)/2$ for
$n = 0, \pm 1, \pm 2, \ldots$, with alternating signs.

## Theta functions as eta products

A key result in the theory of modular forms is that the theta functions can be
expressed as products of eta functions. The `etamake` algorithm identifies these
representations automatically.

For $\theta_3$:

```text
qseries> etamake(theta3(100), 100)
η(2τ)⁵ / (η(τ)⁻² η(4τ)⁻²)
```

This tells us $\theta_3(q) = \eta(2\tau)^5 / (\eta(\tau)^2 \eta(4\tau)^2)$, or
equivalently in q-product notation:

$$
\theta_3(q) = \frac{(q^2;q^2)_\infty^5}{(q;q)_\infty^2\, (q^4;q^4)_\infty^2}
$$

For $\theta_4$:

```text
qseries> etamake(theta4(100), 100)
η(τ)² / (η(2τ)⁻¹)
```

So $\theta_4(q) = \eta(\tau)^2 / \eta(2\tau)$:

$$
\theta_4(q) = \frac{(q;q)_\infty^2}{(q^2;q^2)_\infty}
$$

These are well-known identities. The power of `etamake` is that it discovers
them automatically from the q-series coefficients.

## The etamake algorithm

The algorithm is greedy and direct. Given a q-series $f$ with leading term
$1 + c\,q^k + \cdots$, multiply by $\text{etaq}(k)^{-c}$ to cancel the $q^k$
term. Repeat on the resulting series. If $f$ is genuinely an eta product, this
process terminates (up to the truncation order). The accumulated eta factors
give the answer.

## Partition generating functions

The partition function $p(n)$ counts the number of partitions of $n$. Its
generating function is the reciprocal of Euler's product:

$$
\sum_{n=0}^{\infty} p(n)\, q^n = \prod_{n=1}^{\infty} \frac{1}{1-q^n} = \frac{1}{\eta(\tau)} \cdot q^{-1/24}
$$

In qseries:

```text
qseries> p := 1/etaq(1, 50)
1 + q + 2q² + 3q³ + 5q⁴ + 7q⁵ + 11q⁶ + 15q⁷ + 22q⁸ + 30q⁹ + 42q¹⁰
  + 56q¹¹ + 77q¹² + 101q¹³ + 135q¹⁴ + ... + O(q⁵⁰)
```

Each coefficient is $p(n)$: there are 5 partitions of 4, 11 of 6, 42 of 10,
and so on.

To see a specific number of terms, use `series`:

```text
qseries> series(p, 15)
1 + q + 2q² + 3q³ + 5q⁴ + 7q⁵ + 11q⁶ + 15q⁷ + 22q⁸ + 30q⁹ + 42q¹⁰
  + 56q¹¹ + 77q¹² + 101q¹³ + 135q¹⁴ + O(q¹⁵)
```

## p-core partition generating functions

For a prime $p$, the $p$-core partition function $a_p(n)$ counts partitions of
$n$ with no hook of length divisible by $p$. Its generating function is an eta
quotient:

$$
\sum_{n=0}^{\infty} a_p(n)\, q^n = \prod_{n=1}^{\infty} \frac{(1-q^{pn})^p}{1-q^n}
$$

For instance, the 5-core generating function is `etaq(5, 100)^5 / etaq(1, 100)`.
Granville and Ono used properties of these eta products to prove a long-standing
conjecture in group representation theory.
