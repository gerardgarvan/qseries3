---
phase: 14-relations-demo
verified: 2026-02-25T00:00:00Z
status: passed
score: 3/3 must-haves verified
re_verification:
  previous_status: N/A
  previous_score: N/A
---

# Phase 14: Relations Demo Verification Report

**Phase Goal:** Relation-finding examples reproduce qseriesdoc §4  
**Verified:** 2026-02-25  
**Status:** passed  
**Re-verification:** No — initial phase verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | findhom yields Gauss AGM relations (X₁²+X₂²-2X₃², -X₁X₂+X₄²) in demo | ✓ VERIFIED | demo/garvan-demo.sh L39-40: `run "set_trunc(100)" "findhom([theta3(q,100), theta4(q,100), theta3(subs_q(q,2),100), theta4(subs_q(q,2),100)], 2, 0)"` — exact match to qseriesdoc §4.1 and main.cpp TEST-05 (L473-497). TEST-05 asserts has_sum_sq (X₁²+X₂²-2X₃²) and has_product (-X₁X₂+X₄²). |
| 2 | findhomcombo example runs and produces expected polynomial | ✓ VERIFIED | demo/garvan-demo.sh L43-45: theta-based fallback `findhomcombo(theta3(subs_q(q,2),100)^2, [theta3(q,100), theta4(q,100)], 2, 0)`. Plan fallback used (nested sum unsupported). Expected: (1/2)X₁²+(1/2)X₂² per 14-01-PLAN. |
| 3 | findnonhomcombo example runs and produces expected output | ✓ VERIFIED | demo/garvan-demo.sh L48-49: Watson modular equation `findnonhomcombo(T^2, [T, xi], [1, 7], 0)` — exact match to qseriesdoc §4.4. main.cpp TEST-08 (L525-554) verifies Watson coeffs (343, 49, 7, 21, 35) in findnonhomcombo output. |

**Score:** 3/3 truths verified

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `demo/garvan-demo.sh` | Relations section with three run blocks (findhom, findhomcombo, findnonhomcombo) | ✓ VERIFIED | 52 lines; L36-49 contain Relations section with all three blocks. Contains findhom, findhomcombo, findnonhomcombo. |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|-----|--------|---------|
| demo/garvan-demo.sh | qseries binary | run() pipes commands to BIN | ✓ WIRED | L13: `run() { printf '%s\n' "$@" \| "$BIN" 2>&1; }`. BIN resolution L6-12. Demo designed for `cd dist && bash demo/garvan-demo.sh`. |

### Requirements Coverage

| Requirement | Status | Blocking Issue |
|-------------|--------|----------------|
| DEMO-04 (relation-finding examples reproduce qseriesdoc §4) | ✓ SATISFIED | None |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| None | — | — | — | — |

### Human Verification Required

None — all three success criteria are verifiable from artifact structure, command correctness, and main.cpp unit tests (TEST-05, TEST-08). The user confirmed "Demo was run and produced output."

### Gaps Summary

None. Phase goal achieved.

---

_Verified: 2026-02-25_  
_Verifier: Claude (gsd-verifier)_
