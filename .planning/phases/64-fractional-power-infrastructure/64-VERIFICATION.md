---
phase: 64-fractional-power-infrastructure
verified: 2026-03-02T01:15:00Z
status: passed
score: 6/6 must-haves verified
---

# Phase 64: Fractional Power Infrastructure Verification Report

**Phase Goal:** Series can be raised to any rational exponent via generalized binomial coefficients
**Verified:** 2026-03-02T01:15:00Z
**Status:** passed
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | `(1-q)^(1/2)` produces coefficients 1, -1/2, -1/8, -1/16, -5/128, -7/256, ... | ✓ VERIFIED | REPL output matches exactly. Independently verified via generalized binomial formula C(1/2,n)(-1)^n |
| 2 | `f.powFrac(1/2).powFrac(2)` roundtrip equals `f` to truncation | ✓ VERIFIED | `f := (1-q)^(1/2); f^2` → `1 - q + O(q^50)`. Also verified at T=8 |
| 3 | `BigInt::iroot(n,k)` and `Frac::rational_pow(alpha)` work as validation guards | ✓ VERIFIED | `(4-4*q)^(1/2)` → `2 - q - 1/4q² - ...` (c0=4, iroot(4,2)=2, scale=2). `bigpow` and `iroot` are substantive implementations (binary exponentiation, binary search with bit-length bounds). `rational_pow` calls both and handles sign, negative exponents, even-root-of-negative guard |
| 4 | `powFrac` handles arbitrary rational exponents (1/2, 3/2, 13/2) with correct truncation | ✓ VERIFIED | `(1-q)^(3/2)` → `1 - 3/2q + 3/8q² + 1/16q³ + ...` (correct, = (1-q)·(1-q)^(1/2)). `(1-q)^(13/2)` → `1 - 13/2q + 143/8q² - 429/16q³ + ...` (correct). Truncation: `set_trunc(10); (1-q)^(3/2)` → `... + O(q¹⁰)`. Negative exponent: `(1-q)^(-1/2)` → `1 + 1/2q + 3/8q² + 5/16q³ + 35/128q⁴ + ...` (correct) |
| 5 | `(1+q+q^2)^(1/2)` produces coefficient 3/8 at q² | ✓ VERIFIED | Output: `1 + 1/2q + 3/8q² - 3/16q³ + ...`. Coefficient at q² is 3/8 as required |
| 6 | Chained fractional powers: `((q-q^2)^(1/2))^(1/3)` produces q_shift = 1/6 | ✓ VERIFIED | Output: `q^(1/6) - 1/6q^(7/6) - 5/72q^(13/6) - ...`. q_shift = 1·(1/2)·(1/3) = 1/6, exponents spaced by 1 in integer part. Correct |

**Score:** 6/6 truths verified

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `src/bigint.h` — `bigpow()` | Binary exponentiation for BigInt | ✓ VERIFIED | Lines 408-419: 12 lines, O(log n) binary exponentiation. Handles exp=0 and negative guard |
| `src/bigint.h` — `iroot()` | Integer k-th root via binary search | ✓ VERIFIED | Lines 421-459: 39 lines, binary search with bit-length upper bound estimation, throws on non-perfect-power |
| `src/frac.h` — `rational_pow()` | Exact rational exponentiation | ✓ VERIFIED | Lines 110-154: 44 lines, integer shortcut + general p/q case with bigpow+iroot. Guards: even root of negative, zero to non-positive |
| `src/series.h` — `powFrac()` | Fractional power via log-differentiation recurrence | ✓ VERIFIED | Lines 282-345: 63 lines, extracts leading q-power, factors out c0, O(T²) recurrence, applies scale and q_shift |
| `src/repl.h` — `^` dispatch | Falls back to powFrac for non-integer exponents | ✓ VERIFIED | Lines 1025-1039: try integer pow, catch → evaluate as series, check for rational constant, dispatch to powFrac |
| `tests/acceptance-powfrac.sh` | 6 acceptance tests | ✓ VERIFIED | All 6 tests pass: (1-q)^(1/2) coeffs, roundtrip, (1-q)^(-1/2), (4-4*q)^(1/2) scaling, q_shift, multi-term 3/8 |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| `series.h::powFrac` | `frac.h::rational_pow` | `c0.rational_pow(alpha)` at line 314 | ✓ WIRED | Scale factor computed from leading coefficient |
| `frac.h::rational_pow` | `bigint.h::bigpow` | `bigpow(num.abs(), pabs)` at lines 123,145 | ✓ WIRED | Used in both integer-exponent and general cases |
| `frac.h::rational_pow` | `bigint.h::iroot` | `iroot(num_p, qint)` at lines 147-148 | ✓ WIRED | Used for q-th root extraction of numerator and denominator |
| `repl.h::Pow dispatch` | `series.h::powFrac` | `l.powFrac(rs.c.at(0))` at line 1035 | ✓ WIRED | Fallback from integer pow catch block |
| `series.h::powFrac` | `series.h::normalize_q_shift` | `result.normalize_q_shift()` at line 343 | ✓ WIRED | Fractional q_shift absorbed into integer coefficients when possible |

### Requirements Coverage

No REQUIREMENTS.md entries mapped to phase 64 — coverage determined by success criteria above (all 6/6 verified).

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| — | — | None found | — | No TODOs, FIXMEs, placeholders, stubs, or empty implementations in any modified file |

### Regression Check

Maple-checklist: 31 passed, 2 failed, 8 skipped (41 total) — consistent with pre-phase-64 baseline. The 2 failures (blocks 13, 14) are fractional Jacobi exponent tests, which are phase 65 targets. No regressions introduced by phase 64.

### Human Verification Required

None — all success criteria verified programmatically via REPL output and coefficient comparison against known binomial series formulas.

### Gaps Summary

No gaps found. All 6 success criteria verified with correct mathematical output. The full pipeline — `BigInt::bigpow` → `BigInt::iroot` → `Frac::rational_pow` → `Series::powFrac` → REPL `^` dispatch — is implemented, substantive, and correctly wired.

---

_Verified: 2026-03-02T01:15:00Z_
_Verifier: Claude (gsd-verifier)_
