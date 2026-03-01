# Plan 04-03: tripleprod, quinprod, winquist — Summary

**Completed:** 2025-02-24  
**Status:** Done

## Deliverables

- **tripleprod(z, q, T)** — Π_{n≥1} (1 - z·q^{n-1})(1 - z⁻¹·q^n)(1 - q^n)
- **quinprod(z, q, T)** — (-z;q)_∞ · (-q/z)_∞ · (z²q;q²)_∞ · (q/z²;q²)_∞ · (q;q)_∞
- **winquist(a, b, q, T)** — Double sum per SPEC formula with a^{±...}, b^{±...} terms

## Verification

- tripleprod(z, q³, 15): coeff(0) == 1 (z = q)
- quinprod(q, q⁵, 15): coeff(0) == 1 (Garvan example quinprod(q,q^5,20) = 1 + q - q³ - ...)
- winquist(1, 1, q, 30): trunc == 30

## Notes

- z⁻¹ = z.inverse(); requires invertible z (nonzero constant term)
- quinprod(z=1, q, T) has constant term 2 from (-1;q)_∞; use z=q for coeff(0)=1 verification
- winquist uses a^{-3n} = ainv.pow(3*n), b^{-3n-1} = binv.pow(3*n+1), etc.
- Ready for 04-04 (main.cpp qfuncs test suite)
