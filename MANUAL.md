# Q-Series REPL — User Manual

A standalone, zero-dependency REPL for q-series computation. Reimplements the core functionality of Frank Garvan's `qseries` Maple package for researchers in number theory, partition theory, and modular forms. All arithmetic is exact rational — no floating point anywhere in the mathematical pipeline.

---

## Table of Contents

1. [Getting Started](#1-getting-started)
2. [REPL Features](#2-repl-features)
3. [Built-in Functions Reference](#3-built-in-functions-reference)
4. [Workflow Tutorials](#4-workflow-tutorials)
5. [Mathematical Background](#5-mathematical-background)

---

## 1. Getting Started

### Build

```bash
make
```

Or directly:

```bash
g++ -std=c++20 -O2 -o qseries src/main.cpp
```

### Launch

Interactive mode:

```bash
./qseries
```

Script mode (non-interactive):

```bash
./qseries < script.qs
```

### Basic Usage

Assign variables with `:=` and evaluate expressions:

```
qseries> f := etaq(1, 30)
1 - q - q² + q⁵ + q⁷ - q¹² - q¹⁵ + q²² + q²⁶ + O(q³⁰)
qseries> g := 1/f
1 + q + 2q² + 3q³ + 5q⁴ + 7q⁵ + 11q⁶ + 15q⁷ + ... + O(q³⁰)
```

Set the default truncation order:

```
qseries> set_trunc(100)
Truncation set to 100
```

Suppress output by ending a statement with `:` (like Maple's `;`):

```
qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8):
qseries>
```

Get help:

```
qseries> help()
q-series REPL. Commands: expr, var := expr, help, help(func).
Built-ins: T, add, aqprod, checkmult, checkprod, ...

qseries> help(prodmake)
prodmake(f,T) — Andrews' algorithm: series → infinite product
```

### Arithmetic

Standard arithmetic operators work on q-series: `+`, `-`, `*`, `/`, `^`.

```
qseries> etaq(1,20) * etaq(2,20)
1 - q - q² + q⁵ + q⁶ + q⁷ - q⁸ - 2q⁹ - q¹⁰ + q¹¹ + q¹² + 2q¹³ - q¹⁶ - q¹⁷ - q¹⁸ + q¹⁹ + O(q²⁰)
qseries> etaq(1,20)^2
1 - 2q + q² + 2q⁵ - 2q⁷ - 2q¹² + 2q¹⁴ + O(q²⁰)
```

---

## 2. REPL Features

### Tab Completion

Press **Tab** to autocomplete function names and variable names:

- Type `prod` then Tab → completes to `prodmake`
- Type `et` then Tab → shows `etamake etaq`
- Also completes user-defined variables

### Arrow Keys

- **Left/Right**: move cursor within the current input line
- **Up/Down**: navigate command history

### Backslash Continuation

Split long expressions across multiple lines with `\`:

```
qseries> rr := sum(q^(n^2) / \
  > aqprod(q,q,n,50), n, 0, 8)
```

### Timing

Elapsed time is shown after each command in interactive mode:

```
qseries> etaq(1, 500)
1 - q - q² + q⁵ + ...
[0.001s]
```

### Script Mode

Pipe commands from a file. The banner and prompts are suppressed:

```bash
echo 'set_trunc(50):
rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8):
prodmake(rr, 50)' | ./qseries
```

### Version

```
qseries> version()
qseries 2.0
```

---

## 3. Built-in Functions Reference

### Series Construction

#### `aqprod(a, q, n, T)` — Rising q-factorial

Computes `(a; q)_n = (1-a)(1-aq)(1-aq²)···(1-aq^{n-1})`.

```
qseries> aqprod(q, q, 5, 20)
1 - q - q² + q⁵ + q⁶ + q⁷ - q⁸ - q⁹ - q¹⁰ + q¹³ + q¹⁴ - q¹⁵ + O(q²⁰)
```

#### `etaq(k)`, `etaq(k, T)`, `etaq(q, k, T)` — Eta product

Computes `Π_{n≥1} (1 - q^{kn})`. If `T` is omitted, uses the default truncation. The `k=1` case is optimized via Euler's pentagonal theorem.

```
qseries> etaq(1, 15)
1 - q - q² + q⁵ + q⁷ - q¹² + O(q¹⁵)

qseries> etaq(2, 20)
1 - q² - q⁴ + q¹⁰ + q¹⁴ + O(q²⁰)
```

#### `qbin(m, n, T)` or `qbin(q, m, n, T)` — Gaussian polynomial

Computes the q-binomial coefficient `[n; m]_q`.

```
qseries> qbin(3, 5, 20)
1 + q + 2q² + 2q³ + 2q⁴ + q⁵ + q⁶ + O(q²⁰)
```

#### `theta2(T)` or `theta2(q, T)` — Theta function θ₂

```
qseries> theta2(15)
2 + 2q² + 2q⁶ + 2q¹² + O(q¹⁵)
```

#### `theta3(T)` or `theta3(q, T)` — Theta function θ₃

```
qseries> theta3(15)
1 + 2q + 2q⁴ + 2q⁹ + O(q¹⁵)
```

#### `theta4(T)` or `theta4(q, T)` — Theta function θ₄

```
qseries> theta4(15)
1 - 2q + 2q⁴ - 2q⁹ + O(q¹⁵)
```

#### `theta(z, T)` or `theta(z, q, T)` — Generalized theta function

```
qseries> theta(q^2, 20)
1 + 2q² + 2q⁸ + 2q¹⁸ + O(q²⁰)
```

#### `tripleprod(z, q, T)` — Jacobi triple product

Computes `Π_{n≥1} (1-q^n)(1+zq^{n-1})(1+z⁻¹q^n)`.

```
qseries> tripleprod(q^2, q^5, 50)
1 - q² - q³ + q⁹ + q¹¹ - q²¹ - q²⁴ + q³⁸ + q⁴² + q⁴⁸ + O(q⁵⁰)
```

#### `quinprod(z, q, T)` — Quintuple product

```
qseries> quinprod(q, q^5, 50)
1 + q - q³ - q⁷ - q⁸ - q¹⁴ + q²⁰ + q²⁹ + q³¹ + q⁴² - q⁴⁹ + O(q⁵⁰)
```

#### `winquist(a, b, q, T)` — Winquist's identity

```
qseries> winquist(q, q^2, q^5, 50)
```

#### `T(r, n)` or `T(r, n, T)` — Finite q-product T_{r,n}

```
qseries> T(8, 8, 50)
q⁶ + q⁷ + 2q⁸ + 3q⁹ + 5q¹⁰ + ... + O(q⁵⁰)
```

#### `sum(expr, var, lo, hi)` / `add(expr, var, lo, hi)` — Summation

Computes `Σ_{var=lo}^{hi} expr`. Both `sum` and `add` are equivalent.

```
qseries> sum(q^(n^2), n, 0, 5)
1 + q + q⁴ + q⁹ + q¹⁶ + q²⁵ + O(q³⁰)
```

#### `eisenstein(k, T)` — Normalized Eisenstein series

Computes the normalized Eisenstein series `E_{2k}(q) = 1 - (4k/B_{2k}) Σ σ_{2k-1}(n) q^n`. Supports `k = 1` through `10` (weights 2 through 20). Uses exact rational arithmetic via hardcoded Bernoulli numbers.

```
qseries> eisenstein(2, 10)
1 + 240q + 2160q² + 6720q³ + 17520q⁴ + 30240q⁵ + 60480q⁶ + 82560q⁷ + 140400q⁸ + 181680q⁹ + O(q¹⁰)

qseries> eisenstein(1, 10)
1 - 24q - 72q² - 96q³ - 168q⁴ - 144q⁵ - 288q⁶ - 192q⁷ - 360q⁸ - 312q⁹ + O(q¹⁰)
```

### Series Inspection

#### `series(f)` or `series(f, T)` — Display series

Display coefficients of `f` up to `O(q^T)`. If `T` is omitted, uses the series' truncation.

```
qseries> series(etaq(1, 50), 15)
1 - q - q² + q⁵ + q⁷ - q¹² + O(q¹⁵)
```

#### `coeffs(f, from, to)` — List coefficients

Returns the coefficients of `q^from` through `q^to` as a list.

```
qseries> coeffs(etaq(1, 30), 0, 10)
[1, -1, -1, 0, 0, 1, 0, 1, 0, 0, 0]
```

#### `qdegree(f)` — Highest exponent

Returns the highest exponent with a nonzero coefficient.

```
qseries> qdegree(etaq(1, 30))
26
```

#### `lqdegree(f)` — Lowest exponent

Returns the lowest exponent with a nonzero coefficient.

```
qseries> lqdegree(etaq(1, 30))
0
```

#### `subs_q(f, k)` — Substitute q^k for q

Replaces `q` with `q^k` in series `f`.

```
qseries> subs_q(etaq(1, 20), 2)
1 - q² - q⁴ + q¹⁰ + q¹⁴ - q²⁴ - q³⁰ + O(q⁴⁰)
```

#### `sift(f, n, k, T)` — Extract coefficients

Extracts coefficients `a_{ni+k}` from `f` to form a new series `Σ a_{ni+k} q^i`.

```
qseries> sift(etaq(1, 120), 4, 0, 30)
1 - q³ + O(q⁸)
```

#### `coeff(f, n)` — Single coefficient

Extracts the coefficient of `q^n` from series `f`, displayed as a bare rational.

```
qseries> coeff(etaq(1, 30), 5)
1
qseries> coeff(etaq(1, 30), 1)
-1
```

#### `dissect(f, m, T)` — m-dissection

Computes all `m` siftings of `f`: `sift(f, m, k, T)` for `k = 0, 1, ..., m-1`. Each component is labeled.

```
qseries> dissect(etaq(1, 30), 3, 10)
k=0: 1 - q + O(q³)
k=1: -q + O(q³)
k=2: -q + q² + O(q³)
```

#### `qdiff(f)` — q-derivative

Computes the formal q-derivative `θ_q f = q · d/dq f = Σ n·a_n·q^n`. Returns a composable series.

```
qseries> set_trunc(10):
qseries> f := 1 + q + q^2 + q^3
1 + q + q² + q³ + O(q¹⁰)
qseries> qdiff(f)
q + 2q² + 3q³ + O(q¹⁰)
```

### Product Conversion

#### `prodmake(f, T)` — Andrews' algorithm (series → product)

The central algorithm: converts a q-series into an infinite product `Π (1-q^n)^{a_n}`.

```
qseries> set_trunc(50):
qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8):
qseries> prodmake(rr, 50)
1 / (((1-q) (-q⁴+1) (-q⁶+1) (-q⁹+1) (-q¹¹+1) ...))
```

#### `mprodmake(f, T)` — Product of (1+q^n) factors

Converts series to a product of the form `(1+q^n₁)(1+q^n₂)...`

```
qseries> mprodmake(rr, 50)
(1+q)(1+q⁴)(1+q⁶)(1+q⁹)(1+q¹¹)...
```

#### `etamake(f, T)` — Identify as eta product

Identifies `f` as a product of Dedekind eta functions.

```
qseries> etamake(theta3(100), 100)
η(2τ)⁵ / (η(τ)⁻² η(4τ)⁻²)
```

#### `jacprodmake(f, T)` — Identify as Jacobi product

Identifies `f` as a Jacobi product using `(a, q^b)_∞` notation.

```
qseries> jacprodmake(rr, 50)
1 / (((q,q^5)_∞ (q^4,q^5)_∞ (q^5,q^5)_∞))
```

#### `jac2prod(var)` — Display Jacobi product

Displays the Jacobi product form stored in a variable (after `jacprodmake`).

#### `jac2series(var)` or `jac2series(var, T)` — Jacobi product → series

Converts a stored Jacobi product back to a series for further computation.

#### `qfactor(f)` or `qfactor(f, T)` — Factorize finite q-product

Factorizes a finite q-product (rational function of q).

```
qseries> qfactor(T(8, 8, 50), 50)
q⁶·(1-q⁹)(1-q¹⁰)(1-q¹¹)(1-q¹⁶) / (((1-q¹)(1-q²)(1-q³)(1-q⁴)))
```

#### `checkprod(f)`, `checkprod(f, T)`, `checkprod(f, M, T)` — Check product form

Checks whether `f` is a "nice" infinite product (all exponents `|a[n]| < M`).

```
qseries> checkprod(1/etaq(1, 50), 50)
nice product (minExp=0)
```

#### `checkmult(f)`, `checkmult(f, T)`, `checkmult(f, T, 1)` — Check multiplicativity

Checks whether the coefficients of `f` are multiplicative.

```
qseries> checkmult(1/etaq(1, 50), 50)
NOT MULTIPLICATIVE (m=2,n=3)
```

### Relation Finding

#### `findhom(L, n, topshift)` — Homogeneous polynomial relations

Finds degree-`n` homogeneous polynomial relations among the series in list `L`.

```
qseries> a := theta2(100)^2:
qseries> b := theta3(100)^2:
qseries> c := theta4(100)^2:
qseries> d := theta3(100)*theta4(100):
qseries> findhom([a, b, c, d], 2, 0)
-X₄²+X₂X₃
```

This recovers the Gauss AGM relation: `θ₃²θ₄² = (θ₃θ₄)²`, i.e., `X₂X₃ = X₄²`.

#### `findnonhom(L, n, topshift)` — Nonhomogeneous polynomial relations

Like `findhom` but allows nonhomogeneous polynomials (constant + degree 1 + ... + degree n).

#### `findhomcombo(f, L, n, topshift[, etaopt])` — Express f as polynomial in L

Finds a degree-`n` homogeneous polynomial `P(L₁,...,Lₖ)` such that `f = P(L₁,...,Lₖ)`.

#### `findnonhomcombo(f, L, n_list, topshift[, etaopt])` — Nonhomogeneous combo

Finds a nonhomogeneous polynomial expressing `f` in terms of the series in `L`. `n_list` is a list of degrees to try.

#### `findpoly(x, y, deg1, deg2[, check])` — Polynomial relation between two series

Finds a polynomial `P(x,y) = 0` of degree `deg1` in `x` and `deg2` in `y`.

#### `findlincombo(f, L, topshift)` — Linear combination

Expresses `f` as a linear combination of the series in `L`, if possible.

```
qseries> findlincombo(etaq(1, 50), [etaq(2, 50), etaq(3, 50)], 0)
(no solution)
```

#### `findmaxind(L)` or `findmaxind(L, topshift)` — Maximal independent subset

Finds a maximal linearly independent subset of the q-series in `L`. Returns 1-based indices.

```
qseries> findmaxind([etaq(1, 50), etaq(2, 50), etaq(1, 50)*etaq(2, 50)], 0)
indices: [1, 2, 3]
```

### Number Theory Helpers

#### `sigma(n)` or `sigma(n, k)` — Divisor sum

Returns `σ_k(n) = Σ_{d|n} d^k`. Default `k=1`.

```
qseries> sigma(12)
28
qseries> sigma(12, 2)
210
```

#### `divisors(n)` — Divisor list

Returns the sorted list of all positive divisors of `n`.

```
qseries> divisors(12)
[1, 2, 3, 4, 6, 12]
qseries> divisors(1)
[1]
```

#### `euler_phi(n)` — Euler's totient

Returns Euler's totient function `φ(n)` — the count of integers `1 ≤ k ≤ n` coprime to `n`.

```
qseries> euler_phi(12)
4
qseries> euler_phi(100)
40
```

#### `jacobi(a, n)` — Jacobi symbol

Returns the Jacobi symbol `(a/n)` for odd positive `n`, generalizing the Legendre symbol to composite moduli. Uses the quadratic reciprocity algorithm.

```
qseries> jacobi(2, 15)
1
qseries> jacobi(2, 3)
-1
```

#### `kronecker(a, n)` — Kronecker symbol

Returns the Kronecker symbol `(a/n)`, extending the Jacobi symbol to all integers `n` (including even, negative, and zero).

```
qseries> kronecker(2, 6)
0
qseries> kronecker(-1, -1)
-1
qseries> kronecker(1, 0)
1
```

#### `legendre(a, p)` — Legendre symbol

Returns the Legendre symbol `(a/p)` for prime `p`.

```
qseries> legendre(3, 7)
-1
```

#### `mobius(n)` — Möbius function

Returns `μ(n)`: `0` if `n` has a squared prime factor, `(-1)^k` if `n` is a product of `k` distinct primes.

```
qseries> mobius(6)
1
qseries> mobius(12)
0
qseries> mobius(30)
-1
```

#### `partition(n)` — Partition number

Computes the partition number `p(n)` — the number of ways to write `n` as a sum of positive integers. Uses the generating function `1/Π(1-q^n)` with the pentagonal-optimized `etaq`.

```
qseries> partition(5)
7
qseries> partition(100)
190569292
```

### Utilities

#### `set_trunc(N)` — Set default truncation

Sets the default truncation order for all subsequent computations. Also clears the `etaq` cache.

```
qseries> set_trunc(200)
Truncation set to 200
```

#### `clear_cache()` — Clear memoization caches

Clears the `etaq` memoization cache. Useful if memory usage is a concern.

#### `help()` — General help

Lists all available built-in functions.

#### `help(func)` — Function help

Shows the signature and description for a specific function.

```
qseries> help(etaq)
etaq(k) or etaq(k,T) or etaq(q,k,T) — eta product Π(1-q^{kn})
```

#### `version()` — Version info

```
qseries> version()
qseries 2.0
```

---

## 4. Workflow Tutorials

### Tutorial 1: Rogers-Ramanujan Identity

The Rogers-Ramanujan identity states that `Σ_{n≥0} q^{n²}/(q;q)_n` equals an infinite product with denominators only at exponents ≡ ±1 (mod 5).

```
qseries> set_trunc(50):
qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8):
qseries> series(rr, 30)
1 + q + q² + q³ + 2q⁴ + 2q⁵ + 3q⁶ + 3q⁷ + 4q⁸ + 5q⁹ + 6q¹⁰ + 7q¹¹
+ 9q¹² + 10q¹³ + 12q¹⁴ + 14q¹⁵ + 17q¹⁶ + 19q¹⁷ + 23q¹⁸ + 26q¹⁹
+ 31q²⁰ + 35q²¹ + 41q²² + 46q²³ + 54q²⁴ + 61q²⁵ + 70q²⁶ + 79q²⁷
+ 91q²⁸ + 102q²⁹ + O(q³⁰)
```

Convert to infinite product form (Andrews' algorithm):

```
qseries> prodmake(rr, 50)
1 / (((1-q) (-q⁴+1) (-q⁶+1) (-q⁹+1) (-q¹¹+1) ...))
```

The denominators appear at exponents 1, 4, 6, 9, 11, 14, 16, 19, 21, ... — all ≡ ±1 (mod 5).

Identify as a Jacobi product:

```
qseries> jacprodmake(rr, 50)
1 / (((q,q^5)_∞ (q^4,q^5)_∞ (q^5,q^5)_∞))
```

### Tutorial 2: Product Identification

Identify theta functions as eta products:

```
qseries> set_trunc(100):
qseries> etamake(theta3(100), 100)
η(2τ)⁵ / (η(τ)⁻² η(4τ)⁻²)

qseries> etamake(theta4(100), 100)
η(τ)² / η(2τ)
```

Factorize a finite q-product:

```
qseries> qfactor(T(8, 8, 50), 50)
q⁶·(1-q⁹)(1-q¹⁰)(1-q¹¹)(1-q¹⁶) / (((1-q¹)(1-q²)(1-q³)(1-q⁴)))
```

### Tutorial 3: Finding Relations (Gauss AGM)

Find algebraic relations among theta function squares:

```
qseries> set_trunc(100):
qseries> a := theta2(100)^2:
qseries> b := theta3(100)^2:
qseries> c := theta4(100)^2:
qseries> d := theta3(100)*theta4(100):
qseries> findhom([a, b, c, d], 2, 0)
-X₄²+X₂X₃
```

This recovers the classical identity: `θ₃(q)² · θ₄(q)² = (θ₃(q)θ₄(q))²`, or equivalently `X₂X₃ = X₄²` where `X₂ = θ₃²`, `X₃ = θ₄²`, `X₄ = θ₃θ₄`.

### Tutorial 4: Partition Function

The partition function `p(n)` counts the number of ways to write `n` as a sum of positive integers. Its generating function is `1/etaq(1,T)`:

```
qseries> set_trunc(50):
qseries> p := 1/etaq(1, 50):
qseries> coeffs(p, 0, 10)
[1, 1, 2, 3, 5, 7, 11, 15, 22, 30, 42]
```

The sequence 1, 1, 2, 3, 5, 7, 11, 15, 22, 30, 42 gives p(0) through p(10).

---

## 5. Mathematical Background

A **q-series** is a formal power series in a variable `q` whose coefficients carry combinatorial or number-theoretic meaning. The prototypical example is the **partition generating function** `Σ p(n) q^n = Π_{n≥1} 1/(1-q^n)`, where `p(n)` counts the integer partitions of `n`.

Key building blocks include the **Dedekind eta function** `η(τ) = q^{1/24} Π_{n≥1}(1-q^n)` (computed by `etaq`), **Jacobi theta functions** θ₂, θ₃, θ₄ (whose squares satisfy the AGM relation), and the **rising q-factorial** `(a;q)_n`. The central computational problem is converting a q-series back into a product form — solved by **Andrews' algorithm** (`prodmake`), which is the heart of this package.

For a comprehensive tutorial with worked examples, see `qseriesdoc.md` (Frank Garvan's "Updated q-Product Tutorial for a q-Series Maple Package").
