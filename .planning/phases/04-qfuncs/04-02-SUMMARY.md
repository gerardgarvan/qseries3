# Plan 04-02: theta2, theta3, theta4, theta — Summary

**Completed:** 2025-02-24  
**Status:** Done

## Deliverables

- **theta2(q, T)** — θ₂/q^{1/4} with integer exponents; coeff 2 at n(n+1)
- **theta3(q, T)** — 1 + 2Σ q^{n²}
- **theta4(q, T)** — 1 + 2Σ (-1)^n q^{n²}
- **theta(z, q, T)** — Σ_{i=-T}^{T} z^i q^{i²}

## Verification

- theta2: coeff[0]=2, coeff[2]=2
- theta3: coeff[0]=1, coeff[1]=2, coeff[4]=2, coeff[9]=2
- theta4: coeff[0]=1, coeff[1]=-2, coeff[4]=2
- theta(-1, q, T) matches theta4 structure

## Notes

- theta2 builds series with integer exponents only (Garvan divides out q^{1/4})
- theta(z,q,T) handles negative i via z.inverse().pow(-i)
- Ready for 04-03 (tripleprod, quinprod, winquist)
