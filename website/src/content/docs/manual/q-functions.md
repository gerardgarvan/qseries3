---
title: q-Series Functions
description: "Building q-series: aqprod, etaq, theta functions, and product identities"
---

These functions construct the fundamental q-series objects — finite and infinite
q-products, theta functions, and product identities.

Many functions support an optional **q-injection** form: passing `q` as the
first argument to substitute a different base variable. When the `q` argument is
omitted, the default series variable $q$ is used.

---

## Finite q-Products

### aqprod

Rising q-factorial (the $q$-Pochhammer symbol):

$$(a;q)_n = \prod_{k=0}^{n-1}(1 - aq^k)$$

**Syntax**

```text
aqprod(a: Series, q: Series, n: int, T: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `a` | Series | Base parameter |
| `q` | Series | Nome parameter |
| `n` | int | Number of factors |
| `T` | int | Truncation order |

**Example**

```text
qseries> aqprod(q, q, 5, 20)
1 - q - q^2 + q^5 + q^7 - q^12 - q^15 + O(q^20)
```

This computes $(q;q)_5 = (1-q)(1-q^2)(1-q^3)(1-q^4)(1-q^5)$.

---

### qbin

Gaussian polynomial (q-binomial coefficient):

$$\binom{n}{m}_q = \frac{(q;q)_n}{(q;q)_m\,(q;q)_{n-m}}$$

**Syntax**

```text
qbin(m: int, n: int, T?: int) → Series
qbin(q: Series, m: int, n: int, T?: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `m` | int | Lower index |
| `n` | int | Upper index |
| `T` | int (optional) | Truncation order |
| `q` | Series (optional) | q-injection: use a different base variable |

**Example**

```text
qseries> qbin(3, 5, 20)
1 + q + 2*q^2 + 2*q^3 + 2*q^4 + q^5 + q^6 + O(q^20)
```

---

### T

Andrews' rational function $T_{r,n}$, used in Rogers' proof of the
Rogers-Ramanujan identities. Defined by a recurrence relation and memoized
internally for performance.

**Syntax**

```text
T(r: int, n: int, T?: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `r` | int | Row index |
| `n` | int | Column index |
| `T` | int (optional) | Truncation order |

**Example**

```text
qseries> T(8, 8)
1 + q + q^2 + q^3 + 2*q^4 + 2*q^5 + 3*q^6 + 3*q^7 + ... + O(q^50)
```

---

## Infinite Products (Eta and Theta)

### etaq

Eta product — the infinite $q$-Pochhammer symbol:

$$\text{etaq}(k) = \prod_{n=1}^{\infty}(1 - q^{kn})$$

When $k=1$, this is Euler's product, whose expansion is the pentagonal number
theorem:

$$\prod_{n=1}^{\infty}(1-q^n) = \sum_{n=-\infty}^{\infty} (-1)^n q^{n(3n-1)/2}$$

**Syntax**

```text
etaq(k: int, T?: int) → Series
etaq(q: Series, k: int, T?: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `k` | int | Step size (positive integer) |
| `T` | int (optional) | Truncation order |
| `q` | Series (optional) | q-injection |

**Example**

```text
qseries> etaq(1, 20)
1 - q - q^2 + q^5 + q^7 - q^12 - q^15 + O(q^20)
```

The nonzero terms appear at pentagonal numbers $n(3n-1)/2$.

---

### theta

Generalized theta function:

$$\theta(z,q) = \sum_{n=-\infty}^{\infty} z^n q^{n^2}$$

**Syntax**

```text
theta(z: Series, T?: int) → Series
theta(z: Series, q: Series, T?: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `z` | Series | The base variable |
| `T` | int (optional) | Truncation order |
| `q` | Series (optional) | q-injection |

**Example**

```text
qseries> theta(q, 20)
1 + 2*q^2 + 2*q^8 + 2*q^18 + O(q^20)
```

---

### theta2

Jacobi theta function $\theta_2$:

$$\theta_2(q) = \sum_{n=-\infty}^{\infty} q^{(n+1/2)^2} = 2q^{1/4}\sum_{n=0}^{\infty} q^{n(n+1)}$$

Since qseries works with integer exponents, `theta2` returns the series with
the factor $q^{1/4}$ absorbed into integer-exponent terms.

**Syntax**

```text
theta2(T?: int) → Series
theta2(q: Series, T?: int) → Series
```

**Example**

```text
qseries> theta2(20)
2 + 2*q^2 + 2*q^6 + 2*q^12 + O(q^20)
```

---

### theta3

Jacobi theta function $\theta_3$:

$$\theta_3(q) = \sum_{n=-\infty}^{\infty} q^{n^2} = 1 + 2\sum_{n=1}^{\infty} q^{n^2}$$

The number of representations of $n$ as a sum of two squares is related to the
coefficients of $\theta_3^2$.

**Syntax**

```text
theta3(T?: int) → Series
theta3(q: Series, T?: int) → Series
```

**Example**

```text
qseries> theta3(20)
1 + 2*q + 2*q^4 + 2*q^9 + 2*q^16 + O(q^20)
```

---

### theta4

Jacobi theta function $\theta_4$:

$$\theta_4(q) = \sum_{n=-\infty}^{\infty} (-1)^n q^{n^2} = 1 + 2\sum_{n=1}^{\infty} (-1)^n q^{n^2}$$

**Syntax**

```text
theta4(T?: int) → Series
theta4(q: Series, T?: int) → Series
```

**Example**

```text
qseries> theta4(20)
1 - 2*q + 2*q^4 - 2*q^9 + 2*q^16 + O(q^20)
```

---

## Product Identities

### tripleprod

Jacobi triple product identity:

$$\sum_{n=-\infty}^{\infty} (-1)^n z^n q^{n(n-1)/2} = \prod_{n=1}^{\infty}(1-zq^{n-1})(1-z^{-1}q^n)(1-q^n)$$

**Syntax**

```text
tripleprod(z: Series, q: Series, T: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `z` | Series | Base variable |
| `q` | Series | Nome variable |
| `T` | int | Truncation order |

**Example**

Setting $z = q$, $q \to q^3$ gives Euler's pentagonal theorem:

```text
qseries> tripleprod(q, q^3, 20)
1 - q - q^2 + q^5 + q^7 - q^12 - q^15 + O(q^20)
```

---

### quinprod

Quintuple product identity. Expands the quintuple product to $O(q^T)$.

**Syntax**

```text
quinprod(z: Series, q: Series, T: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `z` | Series | Base variable |
| `q` | Series | Nome variable |
| `T` | int | Truncation order |

**Example**

```text
qseries> quinprod(q, q^5, 20)
1 - q - q^3 + q^7 + q^8 - q^13 + O(q^20)
```

---

### winquist

Winquist's identity expansion. Used in proving Ramanujan's partition congruence
$p(11n+6) \equiv 0 \pmod{11}$.

**Syntax**

```text
winquist(a: Series, b: Series, q: Series, T: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `a` | Series | First parameter |
| `b` | Series | Second parameter |
| `q` | Series | Nome variable |
| `T` | int | Truncation order |

**Example**

```text
qseries> winquist(q^5, q^3, q^11, 20)
1 - q^3 - q^5 + q^6 - q^8 + q^14 - q^16 + q^17 + O(q^20)
```
