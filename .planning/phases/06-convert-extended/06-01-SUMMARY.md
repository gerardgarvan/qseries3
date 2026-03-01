# Plan 06-01 Summary

**Completed:** 2025-02-24
**Phase:** 06-convert-extended

## Deliverables

- **sift(f, n, k, T)** in convert.h: returns Series Σ_i a_{n·i+k} q^i with trunc = floor((T-1-k)/n)+1
- **etamake(f, T)** in convert.h: identifies eta-product; returns vector of (k, e_k); max iter 2*T; failure report
- **main.cpp:** sift test, TEST-03 (theta3/theta4), TEST-06 (Rødseth), etamake failure test

## Verification

- sift(etaq(q,1,50), 5, 0, 49) → trunc 10, coeff[0]=1
- etamake(theta3,100) → η(2τ)^5/(η(4τ)^2 η(τ)^2)
- etamake(theta4,100) → η(τ)^2/η(2τ)
- TEST-06: PD = etaq(2)/etaq(1); sift(PD,5,1,199); etamake → Rødseth η(5τ)³·η(2τ)²/(η(10τ)·η(τ)⁴)
- Non-eta input (1+q/2) returns empty
