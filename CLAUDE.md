# CLAUDE.md — Project Instructions for AI Assistants

Read `SPEC.md` for the full specification. Read `.cursorrules` for behavioral constraints.

## One-Sentence Summary
Build a zero-dependency C++20 REPL that does exact q-series arithmetic and implements Andrews' algorithm for series-to-product conversion, plus eta/theta/Jacobi product identification and polynomial relation finding over Q.

## Build Command
```bash
g++ -std=c++20 -O2 -static -o qseries main.cpp
```

## Quick Architecture Reference
```
BigInt (base 10^9 arbitrary precision) 
  → Frac (exact rationals, auto-GCD) 
    → Series (truncated formal power series, std::map<int,Frac>)
      → qfuncs (aqprod, etaq, theta, products)
        → convert (prodmake, etamake, jacprodmake)
          → relations (findhom, findnonhom — needs linalg kernel over Q)
            → parser + REPL
```

## The One Test That Matters Most
Rogers-Ramanujan: compute `Σ_{n=0}^{8} q^{n²} / (q;q)_n` then run `prodmake` on it. The result must show denominators only at exponents ≡ ±1 (mod 5). If this works, the core math is correct.

## Key File: qseriesdoc.md
This is the reference document with all worked examples. Every output shown there is an acceptance test.
