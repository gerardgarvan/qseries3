# Integration Check: Milestone v10.0 (Close Remaining Gaps)

**Scope:** Phases 97, 98, 99  
**Date:** 2026-03-04

---

## Wiring Summary

**Connected:** 5 exports properly used  
**Orphaned:** 0 exports created but unused  
**Missing:** 0 expected connections not found

---

## Cross-Phase Wiring

### Phase 97 (Block 25 fix)

| Export | From | Used By | Status |
|--------|------|---------|--------|
| `Series::addAligned` | series.h | operator+, rr_ids addSeriesAligned | CONNECTED |
| `Series::operator+` (q-shift aware) | series.h | repl.h eval (BinOp::Add) | CONNECTED |
| `addSeriesAligned` | rr_ids.h | findids_type1, findids_type2 | CONNECTED |

**Trace:** Block 25 runs `x1 := theta2(q,100)^2/theta2(q^3,40)^2`, `x2 := theta3(q,100)^2/theta3(q^3,40)^2`, `x := x1 + x2`, `findpoly(x, x, 3, 3)`.

- theta2 has `q_shift = Frac(1,4)`, theta3 has `q_shift = 0`; their quotients have different q_shifts
- REPL `eval` for BinOp::Add (repl.h:2195) does `return l + r` → Series::operator+
- Series::operator+ (series.h:198–200) detects differing q_shifts and calls `addAligned`
- findpoly receives the aligned series via `ev(0)`, `ev(1)` (both `x`)

**Result:** findpoly correctly receives aligned series; Block 25 flow is wired.

### Phase 98 (Block 24)

Documentation only. No code wiring.

- maple-checklist.sh:237–239 — skip with rationale: "Block 24: collect — N/A (Maple formatting-only; output equivalent)"
- maple_checklist.md:248 — Block 24 note references Phase 98 rationale

### Phase 99 (findlincombomodp)

| Export | From | Used By | Status |
|--------|------|---------|--------|
| `solve_modp` | linalg.h | relations.h findlincombomodp | CONNECTED |
| `findlincombomodp` | relations.h | repl.h dispatch | CONNECTED |
| REPL dispatch | repl.h | User input | CONNECTED |

**Trace:** relations.h includes linalg.h (line 5). findlincombomodp builds mod-p matrix from L, transposes to MT, constructs RHS b from f, and calls `solve_modp(MT, b, p)` (relations.h:182).

**findhommodp vs findlincombomodp:** No conflict. findhommodp uses kernel_modp (null space); findlincombomodp uses solve_modp (Mx=b). Both share gauss_to_rref_modp; solve_modp uses maxCols so augmented [M|b] pivots only on M columns.

---

## API Coverage

**Consumed:** All relevant APIs have callers.

- Series::addAligned: operator+, addSeriesAligned
- solve_modp: findlincombomodp only
- kernel_modp: findhommodp only
- findlincombomodp: REPL dispatch only

**Orphaned:** None

---

## E2E Flows

### Flow 1: Block 25 (x1+x2, findpoly)

**Steps:**
1. Parse/assign x1, x2 (theta2²/theta2(q³)², theta3²/theta3(q³)²)
2. Parse `x := x1 + x2` → eval BinOp Add → Series::operator+ → addAligned
3. Parse `findpoly(x, x, 3, 3)` → ev(0), ev(1) get x → findpoly receives aligned series
4. findpoly returns basis; maple-checklist greps for "X"

**Status:** COMPLETE (code path verified; Phase 97 summary reports Block 25 pass)

### Flow 2: Block 24 skip

**Steps:**
1. maple-checklist hits Block 24
2. skip() with rationale
3. Block 25 runs next

**Status:** COMPLETE (rationale present in both maple-checklist.sh and maple_checklist.md)

### Flow 3: findlincombomodp (REPL + acceptance-modp.sh)

**Steps:**
1. User/script inputs `findlincombomodp(etaq(1,30)+etaq(2,30), [etaq(1,30), etaq(2,30)], 7)`
2. REPL dispatch (repl.h:1170–1178) parses, ev(0) → f, evalListToSeries → L, evi(2) → p
3. findlincombomodp(f, L, p, 0) → solve_modp → coefficients
4. formatFindlincombomodp displays "1 L1 + 1 L2 (mod 7)" or "no linear combination mod p"

**Status:** COMPLETE (Phase 99 summary reports acceptance-modp.sh pass)

---

## Integration Gaps

**None.** All exports are used; all expected connections exist.

---

## Broken Flows

**None.** Block 25, Block 24 skip, and findlincombomodp flows are complete.

---

## Test Environment Note

Execution in this environment (PowerShell, no bash/grep in PATH) prevented running maple-checklist.sh and acceptance-modp.sh directly. Code inspection and phase summaries confirm wiring. Phase 97 and 99 summaries report tests passing under Cygwin.

---

## Summary

| Category | Status |
|----------|--------|
| Phase 97 → findpoly | CONNECTED (addAligned via operator+ when q_shifts differ) |
| Phase 98 | DOC-ONLY (Block 24 N/A rationale) |
| Phase 99 → modp/linalg | CONNECTED (solve_modp, no conflict with findhommodp) |
| Block 25 E2E | COMPLETE |
| Block 24 skip | COMPLETE |
| findlincombomodp E2E | COMPLETE |
| Orphaned exports | 0 |
| Missing connections | 0 |
| Broken flows | 0 |
