---
title: Series Operations
description: Creating and inspecting power series
---

These functions create, inspect, and transform truncated formal power series.
Every series in qseries carries a truncation order $T$, meaning it represents

$$f(q) = \sum_{n} a_n q^n + O(q^T)$$

The default truncation is 50; change it with [`set_trunc`](/manual/repl-commands/#set_trunc).

---

## series

Display a series, optionally re-truncating it to a lower order.

**Syntax**

```text
series(f: Series, T?: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | The series to display |
| `T` | int (optional) | New truncation order; if given, the output is truncated to $O(q^T)$ |

**Example**

```text
qseries> series(1/etaq(1, 50), 10)
1 + q + 2*q^2 + 3*q^3 + 5*q^4 + 7*q^5 + 11*q^6 + 15*q^7 + 22*q^8 + 30*q^9 + O(q^10)
```

---

## coeffs

Extract a range of coefficients as a list.

**Syntax**

```text
coeffs(f: Series, from: int, to: int) → List
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | The series to extract from |
| `from` | int | Starting exponent (inclusive) |
| `to` | int | Ending exponent (inclusive) |

Returns the list $[a_{\text{from}}, a_{\text{from}+1}, \ldots, a_{\text{to}}]$.

**Example**

```text
qseries> coeffs(1/etaq(1, 20), 0, 5)
[1, 1, 2, 3, 5, 7]
```

---

## qdegree

Returns the highest exponent with a nonzero coefficient.

**Syntax**

```text
qdegree(f: Series) → int
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | The series to inspect |

**Example**

```text
qseries> qdegree(1 + q + q^5)
5
```

---

## lqdegree

Returns the lowest exponent with a nonzero coefficient (the *leading q-degree*).

**Syntax**

```text
lqdegree(f: Series) → int
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | The series to inspect |

**Example**

```text
qseries> lqdegree(q^3 + q^5)
3
```

---

## subs_q

Substitute $q^k$ for $q$. Every term $a_n q^n$ in the input becomes $a_n q^{kn}$
in the output.

**Syntax**

```text
subs_q(f: Series, k: int) → Series
```

**Parameters**

| Parameter | Type | Description |
|-----------|------|-------------|
| `f` | Series | The series to transform |
| `k` | int | The substitution exponent |

This is useful for constructing modular forms at different levels, and for
building the arguments to relation-finding functions.

**Example**

```text
qseries> subs_q(1 + q + q^2, 2)
1 + q^2 + q^4 + O(q^50)
```
