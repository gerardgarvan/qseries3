# Plan 03-01: series.h — Summary

**Completed:** 2025-02-24  
**Status:** Done

## Deliverables

- **src/series.h** — Full Series API per SPEC
  - struct Series with `std::map<int,Frac> c`, `int trunc`
  - Factories: zero, one, q, qpow, constant
  - coeff, setCoeff, minExp, maxExp, clean, truncTo
  - operator-(), +, -, *, scalar *, inverse, /, pow
  - subs_q(k), str(maxTerms), display, coeffList

## Verification

- g++ -std=c++20 -O2 -o qseries src/main.cpp **compiles**
- **(1-q)*(1/(1-q))=1** to truncation — PASS (20 terms verified)
- Inverse recurrence uses j=1..n (not j=0)
- Truncation propagation: result.trunc = min(a.trunc, b.trunc) in binary ops
- Multiplication skips pairs where e1+e2 >= trunc
- Invalid inverse (c₀=0) throws std::invalid_argument

## Implementation Notes

- Inverse: j MUST start at 1 (PITFALLS #1) — comment in code
- minExp>0: shift f/q^m, invert h, shift result by -m
- subs_q(0): return constant (sum of coefficients)
- subs_q(k<0): allowed; trunc = trunc * |k|
- str(): "1 + q + 2q² + O(q^T)" with Unicode exponents

## Next

Plan 03-02: main.cpp Series test suite (full SPEC + arithmetic + subs_q + pow)
