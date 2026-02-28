---
title: Relation Finding
description: "Discovering polynomial relations among q-series"
---

If q-series are modular forms of a certain weight, they satisfy polynomial
relations. These functions convert the problem to linear algebra: expand
monomials as q-series, build a coefficient matrix, and find the kernel via
Gaussian elimination over $\mathbb{Q}$.

The **topshift** parameter controls how many leading terms to skip when building
the matrix. Usually 0 is fine; increase it to eliminate spurious relations if
the input series have coincidental low-order agreement.

---

## findhom

Find **homogeneous** polynomial relations of degree $n$ among q-series in a list.
Generates all monomials of degree $n$, expands each as a q-series, and finds
the kernel of the resulting coefficient matrix.

**Syntax**

```text
findhom(L: List, n: int, topshift: int) → RelationSet
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `L` | List | List of q-series |
| `n` | int | Polynomial degree |
| `topshift` | int | Leading terms to skip |

**Example**

Find degree-2 relations among the four Jacobi theta functions (Gauss AGM):

```text
qseries> findhom([theta3(100), theta4(100), theta3(subs_q(q,2),100), theta4(subs_q(q,2),100)], 2, 0)
```

This discovers relations like $\theta_3^2 = \theta_3(q^2)^2 + \theta_4(q^2)^2$.

---

## findnonhom

Find **nonhomogeneous** polynomial relations of degree $\le n$ among q-series in
a list. Unlike `findhom`, this includes monomials of all degrees up to $n$, plus
a constant term. Used for modular functions (ratios of modular forms) where
relations need not be homogeneous.

**Syntax**

```text
findnonhom(L: List, n: int, topshift: int) → RelationSet
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `L` | List | List of q-series |
| `n` | int | Maximum polynomial degree |
| `topshift` | int | Leading terms to skip |

**Example**

```text
qseries> f := theta3(100)^2 / theta4(100)^2
qseries> findnonhom([f, subs_q(f, 2)], 3, 0)
```

---

## findhomcombo

Express a target series $f$ as a **homogeneous polynomial of degree $n$** in the
series from list $L$. If the optional `etaopt` parameter is `"yes"`, each
monomial in the result is also converted to an eta product via `etamake`.

**Syntax**

```text
findhomcombo(f: Series, L: List, n: int, topshift: int, etaopt?: string) → Expression
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | Target series to express |
| `L` | List | Basis series |
| `n` | int | Polynomial degree |
| `topshift` | int | Leading terms to skip |
| `etaopt` | string (optional) | Pass `"yes"` to show eta product forms |

**Example**

```text
qseries> a := etaq(1, 100)
qseries> b := etaq(2, 100)
qseries> findhomcombo(etaq(3, 100)^8, [a, b], 8, 0)
```

---

## findnonhomcombo

Express a target series $f$ as a **nonhomogeneous polynomial** in the series from
list $L$. The `n_list` parameter specifies the maximum degree for each variable
independently.

**Syntax**

```text
findnonhomcombo(f: Series, L: List, n_list: List, topshift: int, etaopt?: string) → Expression
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | Target series to express |
| `L` | List | Basis series |
| `n_list` | List | Maximum degree for each variable in $L$ |
| `topshift` | int | Leading terms to skip |
| `etaopt` | string (optional) | Pass `"yes"` to show eta product forms |

**Example**

```text
qseries> set_trunc(200)
qseries> E := etaq(1, 200)
qseries> F := etaq(7, 200)
qseries> x := E/subs_q(E, 7)
qseries> y := (F/E)^4
qseries> findnonhomcombo(y, [x], [8], 0)
```

---

## findlincombo

Express a target series $f$ as a **linear combination** of series in list $L$.
This is the degree-1 special case of `findhomcombo`.

**Syntax**

```text
findlincombo(f: Series, L: List, topshift: int) → Expression
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | Target series to express |
| `L` | List | Basis series |
| `topshift` | int | Leading terms to skip |

**Example**

```text
qseries> f := theta3(100)^4
qseries> g := theta4(100)^4
qseries> findlincombo(theta3(subs_q(q,2), 100)^4, [f, g], 0)
```

---

## findpoly

Find a polynomial $P(X,Y) = 0$ satisfied by two q-series $x$ and $y$, with
$X$-degree $\le$ `deg1` and $Y$-degree $\le$ `deg2`.

**Syntax**

```text
findpoly(x: Series, y: Series, deg1: int, deg2: int, check?: int) → Polynomial
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `x` | Series | First q-series |
| `y` | Series | Second q-series |
| `deg1` | int | Maximum degree in $X$ |
| `deg2` | int | Maximum degree in $Y$ |
| `check` | int (optional) | Additional verification terms |

**Example**

```text
qseries> set_trunc(200)
qseries> x := theta3(200)^4/theta4(200)^4
qseries> y := theta3(subs_q(q,2), 200)^4/theta4(subs_q(q,2), 200)^4
qseries> findpoly(x, y, 3, 1)
```

This finds a cubic modular equation relating $x$ and $y$.
