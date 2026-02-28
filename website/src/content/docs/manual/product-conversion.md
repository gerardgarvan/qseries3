---
title: Product Conversion
description: "Converting series to infinite products: prodmake, etamake, jacprodmake"
---

These functions convert q-series into structured product representations —
infinite products, eta products, and Jacobi products. **prodmake** is the
central algorithm: it is what makes computational q-series identification
possible.

---

## prodmake

Andrews' algorithm: converts a q-series $f(q)$ with $f(0) = 1$ into an
infinite product

$$f(q) = \prod_{n=1}^{T} (1-q^n)^{-a_n}$$

that agrees with $f$ to $O(q^T)$.

**Algorithm.** Given $f(q) = 1 + \sum_{n=1} b_n q^n$:

1. Compute $c_n = n\,b_n - \sum_{j=1}^{n-1} b_{n-j}\,c_j$
2. Extract exponents: $a_n = \left(c_n - \sum_{\substack{d \mid n \\ d < n}} d\,a_d\right) / n$
3. Result: $f = \prod_{n=1}^{T} (1-q^n)^{-a_n}$

**Syntax**

```text
prodmake(f: Series, T: int) → ProductForm
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | Series to convert (must have $f(0)=1$) |
| `T` | int | Number of product terms to compute |

**Example**

The Rogers-Ramanujan identity — the product form has denominators only at
exponents $\equiv \pm 1 \pmod{5}$:

```text
qseries> rr := sum(q^(n^2)/aqprod(q, q, n, 50), n, 0, 8)
qseries> prodmake(rr, 40)
(1-q)^(-1) * (1-q^4)^(-1) * (1-q^6)^(-1) * (1-q^9)^(-1) * ...
```

---

## etamake

Identify a series as a product of Dedekind eta functions. Given a series $f$,
finds integers $e_k$ such that

$$f(q) = \prod_k \eta(k\tau)^{e_k}$$

where $\eta(k\tau) = q^{k/24}\prod_{n=1}^{\infty}(1-q^{kn})$.

**Syntax**

```text
etamake(f: Series, T: int) → EtaProduct
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | Series to identify |
| `T` | int | Truncation order for matching |

**Example**

Express $\theta_3$ as an eta product:

```text
qseries> etamake(theta3(100), 100)
eta(2*tau)^5 / (eta(tau)^2 * eta(4*tau)^2)
```

This confirms $\theta_3(q) = \frac{\eta(2\tau)^5}{\eta(\tau)^2\,\eta(4\tau)^2}$.

---

## jacprodmake

Identify a series as a product of Jacobi-type theta products. The result is
expressed in JAC notation:

$$\text{JAC}(a, b, \infty) = (q^a;q^b)_\infty\,(q^{b-a};q^b)_\infty\,(q^b;q^b)_\infty$$

**Syntax**

```text
jacprodmake(f: Series, T: int) → JacProduct
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | Series to identify |
| `T` | int | Truncation order for matching |

**Example**

The Rogers-Ramanujan function expressed as a Jacobi product:

```text
qseries> rr := sum(q^(n^2)/aqprod(q, q, n, 50), n, 0, 8)
qseries> jacprodmake(rr, 40)
JAC(0,5,inf) / JAC(1,5,inf)
```

This says $G(q) = \frac{(q^5;q^5)_\infty}{(q;q^5)_\infty\,(q^4;q^5)_\infty}$, confirming the Rogers-Ramanujan identity.

---

## qfactor

Factor a finite rational function in $q$ as a $q$-product. Combines standard
polynomial factoring with prodmake.

**Syntax**

```text
qfactor(f: Series, T?: int) → ProductForm
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | Series to factor |
| `T` | int (optional) | Truncation order |

**Example**

Factor Andrews' $T_{8,8}$:

```text
qseries> set_trunc(64)
qseries> t8 := T(8, 8)
qseries> qfactor(t8, 20)
```

---

## jac2prod

Display a stored Jacobi product in $(q^a;q^b)_\infty$ notation. Takes a
variable that holds a `JacProduct` value from a previous `jacprodmake` call.

**Syntax**

```text
jac2prod(var: JacProduct) → string
```

**Example**

```text
qseries> rr := jacprodmake(sum(q^(n^2)/aqprod(q, q, n, 50), n, 0, 8), 40)
qseries> jac2prod(rr)
(q^5;q^5)_inf / ((q;q^5)_inf * (q^4;q^5)_inf)
```

---

## jac2series

Convert a stored Jacobi product back to a q-series for further computation.

**Syntax**

```text
jac2series(var: JacProduct, T?: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `var` | JacProduct | A variable holding a Jacobi product |
| `T` | int (optional) | Truncation order for the output series |

**Example**

```text
qseries> rr := jacprodmake(sum(q^(n^2)/aqprod(q, q, n, 50), n, 0, 8), 40)
qseries> series(jac2series(rr), 10)
1 + q + q^2 + q^3 + 2*q^4 + 2*q^5 + 3*q^6 + 3*q^7 + 4*q^8 + 5*q^9 + O(q^10)
```
