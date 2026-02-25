# Plan 03-02: main.cpp Series test suite — Summary

**Completed:** 2025-02-24  
**Status:** Done

## Deliverables

- **src/main.cpp** — Extended with full Series test suite
  - BigInt + Frac + Series test driver
  - All tests pass; exits 0

## Test Coverage

### Task 1: SPEC inverse, minExp>0, invalid inverse
- (1-q)*(1/(1-q))=1 to truncation (20 terms)
- Inverse of q*(1-q): g[-1]=1, g[0]=1, g[1]=1
- Series::zero(10).inverse() throws std::invalid_argument
- Note: Series::q(10).inverse() succeeds (minExp>0 handled via shift)

### Task 2: add/sub/mul/div, truncation
- (1+q)+(1-q)=2; (1+q+q²)-(1)=q+q²
- Truncation: (1+q)_T5 + (1+q)_T3 → result.trunc==3
- (1-q)*(1+q)=1-q²; (1+q)²=1+2q+q²
- (1-q²)/(1-q)=1+q
- Division by zero series throws

### Task 3: subs_q, pow, str
- (1+q+q²).subs_q(2)=1+q²+q⁴; trunc=20
- subs_q(0): (1+q+q²).subs_q(0) → constant 3
- (1+q).pow(2)=1+2q+q²; pow(0)=1
- (1/(1-q)).pow(-1)==(1-q)
- str() contains "1", "q", "O(q^"

## Verification

- g++ -std=c++20 -O2 -o qseries src/main.cpp succeeds
- ./qseries exits 0
- All BigInt and Frac tests still pass
- All Series tests pass

## Next

Phase 3 complete. Ready for Phase 4 (qfuncs).
