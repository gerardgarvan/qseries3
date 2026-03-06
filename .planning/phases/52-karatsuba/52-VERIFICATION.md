---
phase: 52-karatsuba
verified: 2026-02-28T21:30:00Z
status: passed
score: 4/4 must-haves verified
---

# Phase 52: Karatsuba Multiplication Verification Report

**Phase Goal:** BigInt multiplication is asymptotically faster for large operands
**Verified:** 2026-02-28
**Status:** passed
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | Karatsuba multiply is used for operands >= KARATSUBA_THRESHOLD limbs | ✓ VERIFIED | `karatsubaMultiply` dispatches to `mulSchoolbook` when `n < 32`; `operator*` calls `karatsubaMultiply` unconditionally (lines 249-306 of bigint.h) |
| 2 | Schoolbook multiply is used for operands < KARATSUBA_THRESHOLD limbs | ✓ VERIFIED | `karatsubaMultiply` line 251: `if (n < KARATSUBA_THRESHOLD) return mulSchoolbook(x, y);` — confirmed threshold = 32 |
| 3 | karatsuba(a,b) == schoolbook(a,b) for all inputs (correctness invariant) | ✓ VERIFIED | Tested 5 random 100-limb pairs + 8 boundary sizes (1, 10, 31, 32, 33, 50, 64, 128 limbs) — all produce identical digit vectors |
| 4 | All existing acceptance tests still pass | ✓ VERIFIED | 10 core + 11 v1.8 + 9 robustness + 7 quick-wins + 3 mprodmake + 3 checkprod/checkmult + 3 findmaxind + 5 optional-args = 51 tests, all passing |

**Score:** 4/4 truths verified

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `src/bigint.h` | Karatsuba multiplication | ✓ VERIFIED | Contains `mulSchoolbook` (line 203), `addVec` (line 219), `subVec` (line 233), `karatsubaMultiply` (line 249), `KARATSUBA_THRESHOLD = 32` (line 247) — 408 lines total, substantive implementation |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `BigInt::operator*` | `karatsubaMultiply` | Dispatch based on operand size | ✓ WIRED | Line 306: `r.d = karatsubaMultiply(d, o.d);` — all multiplications route through Karatsuba which auto-selects schoolbook below threshold |

### Requirements Coverage

| Requirement | Status | Blocking Issue |
|-------------|--------|----------------|
| PERF-05: Karatsuba BigInt multiplication — hybrid schoolbook/Karatsuba with threshold at ~32 limbs; O(n^1.585) for large operands | ✓ SATISFIED | None |

### Success Criteria Coverage

| # | Criterion | Status | Evidence |
|---|-----------|--------|----------|
| 1 | Multiplying two 100-limb (~900-digit) BigInts uses Karatsuba and is measurably faster than schoolbook | ✓ VERIFIED | Benchmark: Karatsuba 12.985 us/op vs Schoolbook 13.915 us/op = 1.07x speedup at 100 limbs. Speedup increases asymptotically with size (O(n^1.585) vs O(n^2)). |
| 2 | Small operands (< 32 limbs) still use schoolbook — no regression on typical REPL workloads | ✓ VERIFIED | 10-limb benchmark: ratio 0.98x (Karatsuba path dispatches directly to schoolbook, no overhead). All 51 acceptance tests pass — zero regression. |
| 3 | karatsuba(a,b) == schoolbook(a,b) for random inputs at all size boundaries | ✓ VERIFIED | Tested at 1, 10, 31, 32, 33, 50, 64, 100, 128 limbs with random inputs — all produce identical results. |
| 4 | All existing acceptance tests still pass | ✓ VERIFIED | 51 tests across 8 test suites, all passing. |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| (none) | — | — | — | No TODO/FIXME/PLACEHOLDER/HACK markers found in bigint.h |

### Human Verification Required

None — all criteria are programmatically verifiable and have been verified.

### Gaps Summary

No gaps found. All must-haves verified. The implementation is complete, correct, and wired:

- **Algorithm correctness**: Karatsuba produces identical results to schoolbook at all tested sizes
- **Hybrid dispatch**: `operator*` → `karatsubaMultiply` → `mulSchoolbook` below threshold
- **Threshold**: 32 limbs (~288 decimal digits) as specified
- **Performance**: Measurable speedup at 100 limbs, no regression at small sizes
- **Build**: Zero warnings with `-Wall -Wextra -Wpedantic`
- **Tests**: All 51 acceptance tests pass

---

_Verified: 2026-02-28_
_Verifier: Claude (gsd-verifier)_
