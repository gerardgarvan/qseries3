---
title: Modular Equations
description: Discovering polynomial relations among theta functions and modular forms
---

## The idea

If q-series are modular forms of a particular weight, the theory guarantees they
satisfy polynomial relations. qseries converts this existence theorem into a
computational tool: expand all monomials of a given degree as q-series, form a
matrix of their coefficients, and compute the kernel. Nontrivial kernel vectors
correspond to relations.

The function `findhom` finds homogeneous relations, `findhomcombo` expresses a
given series as a homogeneous polynomial in a basis, `findnonhomcombo` handles
the nonhomogeneous case, and `findpoly` finds a polynomial relation between two
series.

## Gauss AGM and theta relations

Consider $\theta_3(q)$, $\theta_4(q)$, $\theta_3(q^2)$, and $\theta_4(q^2)$.
Since these are modular forms, we might expect polynomial relations among them.
Use `findhom` to search for quadratic relations:

```text
qseries> set_trunc(200)

qseries> t3 := theta3(200)
1 + 2q + 2q⁴ + 2q⁹ + 2q¹⁶ + 2q²⁵ + 2q³⁶ + ... + O(q²⁰⁰)

qseries> t4 := theta4(200)
1 - 2q + 2q⁴ - 2q⁹ + 2q¹⁶ - 2q²⁵ + 2q³⁶ - ... + O(q²⁰⁰)

qseries> t3q2 := subs_q(t3, 2)
1 + 2q² + 2q⁸ + 2q¹⁸ + 2q³² + ... + O(q⁴⁰⁰)

qseries> t4q2 := subs_q(t4, 2)
1 - 2q² + 2q⁸ - 2q¹⁸ + 2q³² - ... + O(q⁴⁰⁰)

qseries> findhom([t3, t4, t3q2, t4q2], 2, 0)
-X₄²+X₁X₂
-2X₃²+X₂²+X₁²
```

Two quadratic relations are found. Writing $X_1 = \theta_3(q)$,
$X_2 = \theta_4(q)$, $X_3 = \theta_3(q^2)$, $X_4 = \theta_4(q^2)$:

$$
\theta_4(q^2)^2 = \theta_3(q)\,\theta_4(q)
$$

$$
\theta_3(q^2)^2 = \frac{\theta_3(q)^2 + \theta_4(q)^2}{2}
$$

These are Gauss's parametrization of the arithmetic-geometric mean iteration.
They show that if we define the AGM sequences $a_{n+1} = (a_n + b_n)/2$ and
$b_{n+1} = \sqrt{a_n b_n}$ starting from $a_0 = \theta_3^2$, $b_0 = \theta_4^2$,
then the iterates are $a_n = \theta_3(q^{2^n})^2$ and $b_n = \theta_4(q^{2^n})^2$.

## Eisenstein series and eta products

For an odd prime $p$, the Eisenstein series with Legendre character is:

$$
U_{p,k}(q) = \sum_{m=1}^{\infty} \sum_{n=1}^{\infty} \left(\frac{m}{p}\right) n^{k-1} q^{mn}
$$

where $\left(\frac{m}{p}\right)$ is the Legendre symbol. Kolberg found many
relations between these Eisenstein series and eta products.

Define the weight-2 eta product basis:

$$
B_1 = \frac{\eta(\tau)^5}{\eta(5\tau)}, \qquad B_2 = q\,\frac{\eta(5\tau)^5}{\eta(\tau)}
$$

We expect $U_{5,6}$ to be a homogeneous cubic in $B_1$ and $B_2$ (since
$U_{5,6}$ has weight 6 and each $B_i$ has weight 2):

```text
qseries> set_trunc(100)

qseries> B1 := etaq(1,50)^5/etaq(5,50)
1 - 5q + 5q² + 10q³ - 15q⁴ - 5q⁵ - 10q⁶ + 30q⁷ + ... + O(q⁵⁰)

qseries> B2 := q*etaq(5,50)^5/etaq(1,50)
q + q² + 2q³ + 3q⁴ + 5q⁵ + 2q⁶ + 6q⁷ + ... + O(q⁵⁰)

qseries> f := sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)
q + 31q² + 242q³ + 993q⁴ + 3125q⁵ + 7502q⁶ + ... + O(q¹⁰⁰)

qseries> findhomcombo(f, [B1, B2], 3, 0)
335X₂³+40X₁X₂²+X₁²X₂
```

The result is:

$$
U_{5,6} = B_1^2 B_2 + 40\, B_1 B_2^2 + 335\, B_2^3
$$

In terms of eta functions:

$$
U_{5,6} = \eta(\tau)^9 \eta(5\tau)^3 + 40\,\eta(\tau)^3 \eta(5\tau)^9 + 335\,\frac{\eta(5\tau)^{15}}{\eta(\tau)^3}
$$

The proof is a straightforward exercise using the theory of modular forms.

## Watson's modular equation

Watson used a modular equation involving the Dedekind eta function to prove
Ramanujan's partition congruences for powers of 7. Define:

$$
\xi = \frac{\eta(49\tau)}{\eta(\tau)}, \qquad T = \left(\frac{\eta(7\tau)}{\eta(\tau)}\right)^4
$$

Use `findnonhomcombo` to express $T^2$ as a polynomial in $T$ and $\xi$:

```text
qseries> set_trunc(100)

qseries> xi := q^2*etaq(49,100)/etaq(1,100)
q² + q³ + 2q⁴ + 3q⁵ + 5q⁶ + 7q⁷ + 11q⁸ + ... + O(q¹⁰⁰)

qseries> TT := q*(etaq(7,100)/etaq(1,100))^4
q + 4q² + 14q³ + 40q⁴ + 105q⁵ + 252q⁶ + 574q⁷ + ... + O(q¹⁰⁰)

qseries> findnonhomcombo(TT^2, [TT, xi], [1, 7], 0)
X₂+7X₂²+21X₂³+49X₂⁴+147X₂⁵+343X₂⁶+343X₂⁷+7X₁X₂+35X₁X₂²+49X₁X₂³
```

Collecting by powers of $T$:

$$
T^2 = (49\xi^3 + 35\xi^2 + 7\xi)\, T + 343\xi^7 + 343\xi^6 + 147\xi^5 + 49\xi^4 + 21\xi^3 + 7\xi^2 + \xi
$$

This is Watson's modular equation. The right side factors: the coefficient of
$T$ is $7\xi(7\xi + 1)(7\xi + 5)$ and the remaining terms form
$\xi(7\xi+1)^2(7\xi^2+1)^2$ (after appropriate factoring). Watson used this
equation to prove that $p(7^n \cdot m + \delta_n) \equiv 0 \pmod{7^n}$ for
suitable residues $\delta_n$.

## Finding polynomial relations with findpoly

The `findpoly` function finds a polynomial relation $P(X, Y) = 0$ between two
q-series, given bounds on the degrees in $X$ and $Y$.

The syntax is `findpoly(x, y, deg1, deg2)` where `deg1` is the degree bound in
$X$ and `deg2` in $Y$. An optional fifth argument specifies an order to which
the relation is verified.

For example, given modular functions $x$ and $y$ derived from theta quotients,
`findpoly` can discover relations like:

$$
(X + 6)^3\, Y - 27(X + 2)^2 = 0
$$

This is a relation between cubic modular functions studied by Borwein, Borwein,
and Garvan. The power of `findpoly` is that it discovers such identities
automatically — the user only needs to provide the q-series and degree bounds.

The relation-finding functions (`findhom`, `findnonhom`, `findhomcombo`,
`findnonhomcombo`, `findpoly`) all work by the same principle: expand products
of the input series as q-series, form a coefficient matrix, and compute its
kernel over $\mathbb{Q}$. When the kernel is nontrivial, each basis vector
corresponds to a polynomial identity satisfied (at least to the computed
truncation order) by the input series. Proving that the identity holds exactly
requires modular form theory.
