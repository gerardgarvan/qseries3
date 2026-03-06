# Phase 103: Block 25 fix — Context

**Gathered:** 2026-03-06
**Status:** Ready for planning

---

## Phase Boundary

Make findpoly work on theta2/theta3 quotients via q-shift alignment. Phase 97 (v10.0) already implemented `Series::addAligned` and `operator+` uses it when q_shifts differ. Phase 103 verifies that Block 25 passes and, if not, fixes any regression or gap. Scope: verification-first; implementation only if Block 25 fails.

---

## Implementation Decisions

### 1. Overlap with Phase 97

- **Phase 97** already implemented addAligned and operator+ alignment. Block 25 test (`x1+x2` then `findpoly(x,x,3,3)`) should pass.
- **Phase 103** = verification-first: run maple-checklist Block 25 and acceptance-exercises EX-10. If both pass, phase is verification-only (no code changes). If Block 25 or EX-10 fails, fix the regression.
- Do NOT re-implement addAligned or operator+ logic. Reuse Phase 97 work.

### 2. findpoly Internal Alignment

- **Rely on user pre-alignment.** Block 25 test does `x := x1 + x2` then `findpoly(x, x, 3, 3)`. The sum uses operator+ which calls addAligned. findpoly receives one aligned series.
- **findpoly(x1, x2, ...)** with different q_shifts: out of scope for this phase. Current design assumes user pre-aligns (e.g. pass x1+x2). No change to findpoly internals unless Block 25/EX-10 fail due to this case.

### 3. Verification Scope

- **maple-checklist Block 25** — Must pass (findpoly on theta2/theta3 quotient sum).
- **acceptance-exercises EX-10** — EX-10a (findpoly contains coefficient 27), EX-10b (findpoly uses X variables) must pass.
- Both scopes are in phase. If either fails, fix.

### 4. Edge Cases

- Empty series, single-term series: addAligned and operator+ already handle (Phase 97). No new handling needed.
- If verification reveals edge-case failures, address them. Otherwise defer.

---

## Claude's Discretion

- Exact fix approach if Block 25/EX-10 fail (e.g. whether to touch relations.h, series.h, or test setup).
- How to structure verification tasks (single verification task vs separate Block 25 and EX-10 tasks).

---

## Specific Ideas

- Block 25 test commands: `x1 := theta2(q,100)^2/theta2(q^3,40)^2`, `x2 := theta3(q,100)^2/theta3(q^3,40)^2`, `x := x1 + x2`, `findpoly(x, x, 3, 3)` — output must contain "X".

---

## Deferred Ideas

- findpoly(x, y, ...) internal alignment when x and y have different q_shifts — future phase if needed.
- Extending findpoly to accept more than two series — out of scope.

---

*Phase: 103-block-25-fix*
*Context gathered: 2026-03-06*
