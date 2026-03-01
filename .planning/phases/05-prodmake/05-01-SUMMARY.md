# Plan 05-01: prodmake + Rogers-Ramanujan (TEST-01) — Summary

**Completed:** 2025-02-24  
**Status:** Done

## Deliverables

- **convert.h** — prodmake(f, T) implementing Andrews' algorithm
- **main.cpp** — Phase 5 Rogers-Ramanujan acceptance test (TEST-01)

## Algorithm (locked)

1. Extract b[0..T-1] from f; effective T = min(T, f.trunc)
2. Normalize: if b[0]=0 or minExp>0, warn and return empty; else divide by b[0]
3. c[n] = n·b[n] - Σ_{j=1}^{n-1} b[n-j]·c[j] (j excludes n)
4. a[n] = (c[n] - Σ_{d|n, d<n} d·a[d]) / n (divisor sum excludes d=n)
5. Warn on non-integer a[n]; return best-effort anyway

## Verification

- g++ compiles; ./qseries exits 0
- Rogers-Ramanujan: Σ q^(n²)/(q;q)_n (n=0..8, T=50) → prodmake yields a[n]==1 for n≡±1 (mod 5), a[n]==0 otherwise (n=1..49)
- Phase 5 canary test passes — core math validated
