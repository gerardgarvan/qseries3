---
title: Quick Start
description: Your first qseries session
---

## Starting the REPL

Launch qseries to enter the interactive prompt:

```bash
./qseries
```

You will see a banner and the `qseries>` prompt:

```text
=== qseries v2.0 ===
Type 'help' for available commands.

qseries>
```

## Basic arithmetic

The variable `q` is the fundamental power series variable. It represents the
formal series $q + O(q^{50})$, truncated at the default precision:

```text
qseries> q
q + O(q^50)

qseries> 1 + q + q^2
1 + q + q^2 + O(q^50)
```

All standard arithmetic operations work on power series — addition, subtraction,
multiplication, division, and exponentiation:

```text
qseries> (1 - q) * (1 + q + q^2 + q^3)
1 - q^4 + O(q^50)
```

## The partition function

The partition function $p(n)$ counts the number of ways to write $n$ as a sum
of positive integers. Its generating function is Euler's product:

$$\sum_{n=0}^{\infty} p(n) q^n = \prod_{n=1}^{\infty} \frac{1}{1 - q^n}$$

In qseries, `etaq(1)` computes $\prod_{n=1}^{\infty}(1-q^n)$, so the partition
generating function is its reciprocal:

```text
qseries> p := 1/etaq(1)
1 + q + 2*q^2 + 3*q^3 + 5*q^4 + 7*q^5 + 11*q^6 + 15*q^7 + 22*q^8 + ... + O(q^50)
```

Each coefficient is $p(n)$: there are 5 partitions of 4, 11 partitions of 6, and so on.

## Rogers-Ramanujan

Compute the first Rogers-Ramanujan function and identify its product form:

```text
qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)
1 + q + q^2 + q^3 + 2*q^4 + 2*q^5 + 3*q^6 + ... + O(q^50)

qseries> prodmake(rr, 40)
(1-q)^(-1) * (1-q^4)^(-1) * (1-q^6)^(-1) * (1-q^9)^(-1) * ...
```

The denominators appear only at exponents $\equiv \pm 1 \pmod{5}$, confirming the
Rogers-Ramanujan identity.

## Getting help

List all available functions:

```text
qseries> help
Available commands: aqprod, coeffs, etaq, etamake, ...
```

Get help for a specific function:

```text
qseries> help(prodmake)
prodmake(f, T?) - Convert series to infinite product form (Andrews' algorithm)
  f: Series to convert
  T: Number of product terms (optional, default from truncation)
```

## Default truncation

By default, all series are truncated at $O(q^{50})$. Change the default with
`set_trunc`:

```text
qseries> set_trunc(100)
Truncation set to 100

qseries> q
q + O(q^100)
```

Higher truncation gives more coefficients but takes longer to compute. A value of
50 is sufficient for most explorations; increase to 100 or 200 for identity
verification or relation finding.

## Next steps

- [Reference Manual](/manual/q-functions/) — full documentation for all 50+ built-in functions
- [Tutorials](/tutorial/) — worked examples from partition theory, theta functions, and modular equations
