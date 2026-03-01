# Phase 25: checkprod and checkmult — Research

**Researched:** 2026-02-25  
**Domain:** Validation utilities for q-series  
**Confidence:** HIGH

## Summary

Phase 25 implements two Maple qseries validation functions: **checkprod** (check if f is a "nice" product) and **checkmult** (check if coefficients are multiplicative). Both call existing convert/relations infrastructure.

---

## Maple References

### checkprod

**Source:** [qseries.org/.../checkprod.html](https://qseries.org/fgarvan/qmaple/qseries/functions/checkprod.html)

| Item | Detail |
|------|--------|
| **Call** | checkprod(f, M, Q) |
| **Params** | f — q-series; M, Q — positive integers |
| **Semantics** | Computes first Q exponents from prodmake; returns [c, 1] if all \|a[n]\| < M ("nice"); else [c, list]. c = smallest power of q in f (minExp). |

**Example:** Rogers-Ramanujan series F; checkprod(F, 10, 20) → [0, 1] (nice); checkprod(F, 10, 50) → [0, 1150] (large exponent list = not nice within bound M=10).

**Algorithm:** Run prodmake(f, T) with T ≥ Q. If all \|a[n]\| < M for n=1..Q, return [minExp, 1]. Else return [minExp, full exponent list or list of violating n].

### checkmult

**Source:** [qseries.org/.../checkmult.html](https://qseries.org/fgarvan/qmaple/qseries/functions/checkmult.html)

| Item | Detail |
|------|--------|
| **Call** | checkmult(QS, T) or checkmult(QS, T, yes) |
| **Params** | QS — q-series; T — truncation; x — optional "yes" for verbose |
| **Semantics** | Check if coefficients a(n) are multiplicative up to q^T: a(mn) = a(m)*a(n) when gcd(m,n)=1. Returns 1 if multiplicative, 0 if not. If arg3=yes, print all (m,n) failures. |

** multiplicative:** a(mn) = a(m)·a(n) for coprime m, n.

**Example:** F1+F2 and F1-F2 (eta combos) → "MULTIPLICATIVE", 1. F1 alone → "NOT MULTIPLICATIVE", failure at m=3, n=2, returns 0.

**Algorithm:** For 2 ≤ m,n ≤ sqrt(T) or similar range with gcd(m,n)=1, check a[m*n] == a[m]*a[n]. First failure → return 0; else return 1. If verbose, collect all failures and print.

---

## API Design (for CONTEXT)

**checkprod:**  
- C++: `checkprod(f, M, Q)` or simplified `checkprod(f, T)` with M=2 (default "nice" = exponents in {-1,0,1}).  
- Return: struct or pair (int minExp, bool nice) or (int minExp, std::vector) per Maple.  
- REPL: checkprod(f, T) or checkprod(f, M, T) to match Maple.

**checkmult:**  
- C++: `checkmult(f, T, bool verbose = false)`.  
- Return: bool (true = multiplicative).  
- REPL: checkmult(f, T) or checkmult(f, T, yes).

---

## Dependencies

| Dependency | Location | Purpose |
|------------|----------|---------|
| prodmake | convert.h | checkprod needs exponent list |
| Series | series.h | coeff, minExp, truncTo |
| gcd | qfuncs.h or nthelpers | checkmult needs gcd(m,n)==1 |

---

## Implementation Location

- **checkprod, checkmult:** convert.h (validation of product form / coeffs) or a new validation.h. Prefer convert.h to keep with prodmake.
- **REPL:** repl.h dispatch, help table.
