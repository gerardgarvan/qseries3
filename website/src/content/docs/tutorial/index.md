---
title: Tutorial
description: Worked examples in q-series computation
---

These tutorials demonstrate qseries through worked examples drawn from Frank
Garvan's q-series tutorial. They cover the core computational problems in
q-series research: series-to-product conversion, eta and theta product
identification, coefficient sifting, and polynomial relation discovery.

Each tutorial states a mathematical identity, then verifies it computationally
in the REPL. The emphasis is on what qseries can do — not on proving the
identities (for proofs, see the references cited in each section).

## Prerequisites

These tutorials assume familiarity with:

- **Formal power series** and their manipulation (addition, multiplication,
  inversion)
- **Partitions of integers** — for instance, $p(5) = 7$ because 5 can be
  written as a sum of positive integers in 7 ways
- **q-series notation** — the rising q-factorial (or q-Pochhammer symbol):

$$
(a;q)_n = \prod_{k=0}^{n-1}(1 - aq^k)
$$

  and its infinite version $(a;q)_\infty = \prod_{k=0}^{\infty}(1-aq^k)$

- **Infinite product representations** of generating functions — the idea that
  a power series $\sum a_n q^n$ might equal a product $\prod (1-q^n)^{b_n}$

No prior experience with Maple or any other computer algebra system is needed.
All examples use the qseries REPL.

## Tutorial pages

- [Rogers-Ramanujan Identities](/tutorial/rogers-ramanujan/) — the central
  example: `prodmake` and `jacprodmake` identify infinite product forms
- [Theta & Eta Functions](/tutorial/theta-functions/) — `etamake` identifies
  theta functions as eta products; partition generating functions
- [Partition Identities](/tutorial/partition-identities/) — sifting
  coefficients with `sift`, Ramanujan's congruences, the `T(r,n)` functions
- [Modular Equations](/tutorial/modular-equations/) — `findhom`, `findhomcombo`,
  `findnonhomcombo`, and `findpoly` discover polynomial relations

## Reference

These tutorials follow the structure of Frank Garvan's
[Updated q-Product Tutorial for a q-Series Maple Package](https://qseries.org/fgarvan/qmaple/qseries/index.html).
For the full mathematical background, see the references therein and George
Andrews' *q-Series: Their Development and Application in Analysis, Number
Theory, Combinatorics, Physics, and Computer Algebra* (CBMS Regional
Conference Series, 1986).
