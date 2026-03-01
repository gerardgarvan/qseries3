# Plan 04-01: qfuncs.h (nthelpers, etaq, aqprod, qbin) — Summary

**Completed:** 2025-02-24  
**Status:** Done

## Deliverables

- **src/qfuncs.h** — nthelpers + etaq, aqprod, qbin
  - divisors(n), mobius(n), legendre(a,p), sigma(n,k), euler_phi(n)
  - etaq(q, k, T) = Π(1 - q^{kn})
  - aqprod(a, q, n, T) = (a;q)_n
  - qbin(q, m, n, T) = [n;m]_q via product formula

## Verification

- divisors(12) = {1,2,3,4,6,12}; mobius(6)=1; sigma(6)=12; euler_phi(6)=2
- etaq(q,1,25) Euler pentagonal: coeff ±1 at 0,1,2,5,7,12
- aqprod(q,q,5,25): (1-q)(1-q²)...(1-q⁵); aqprod(q,q,0,T)=1
- qbin edge cases: m<0, m>n → 0; m==0, m==n → 1; qbin(q,1,3,T)=1+q+q²

## Notes

- Truncation propagated; truncate after each multiply in etaq, aqprod, qbin
- qbin uses product formula to avoid series division
- Ready for 04-02 (theta)
