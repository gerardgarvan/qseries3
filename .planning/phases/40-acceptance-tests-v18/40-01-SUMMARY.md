---
phase: 40-acceptance-tests-v18
plan: 01
status: COMPLETED
duration: ~8min
---

## What Was Done

Created acceptance test script covering all 10 new v1.8 built-in functions.

### Test script: tests/acceptance-v18.sh

11 test cases covering all 10 functions:
1. divisors(12) - checks for [1, 2, 3, 4, 6, 12]
2. mobius(6) - checks for 1 (squarefree)
3. mobius(12) - checks for 0 (squared factor)
4. euler_phi(12) - checks for 4
5. jacobi(2, 15) - checks for 1
6. kronecker(-1, -1) - checks for -1
7. coeff(etaq(1,30), 5) - checks for 1
8. dissect(etaq(1,30), 3, 10) - checks for k=0 label
9. eisenstein(2, 10) - checks for 240q (E_4 coefficient)
10. partition(100) - checks for 190569292
11. qdiff(1+q+q^2+q^3) - checks for 2q in output

## Files Created

- tests/acceptance-v18.sh - 11 test cases, executable

## Verification

- v1.8 tests: 11/11 passed
- Main acceptance: 10/10 passed
- Quick-wins: 7/7 passed
- Total: 28/28 passed, 0 failed
