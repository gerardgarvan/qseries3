# Phase 102: Cross-Package Integration — Research

**Researched:** 2026-03-05
**Domain:** Integration testing of ETA identity prover, theta IDs, and modular forms
**Confidence:** HIGH

## Summary

The integration script `tests/integration-eta-theta-modforms.sh` exists and **passes** (5/5) when run in an environment with `grep` in PATH. It exercises three **separate** end-to-end flows—ETA+modforms, theta IDs, and etamake—but does **not** satisfy the strict reading of success criterion 2: "eta identity + theta ID + modular form basis used in a **single proof chain**." The script does not invoke `makebasisM` or `makeALTbasisM`; it uses `DELTA12` (a modular form) but not a modular form **basis**.

**Primary recommendation:** Decide whether success criterion 2 requires (A) three flows covering all three subsystems (current script satisfies) or (B) one flow that uses eta identity, theta ID, and modular form basis together. If (B), add a new integration test that chains all three in a single proof; Phase 102 scope would include both ensuring the script passes robustly and adding that test.

## User Constraints

*No CONTEXT.md exists for Phase 102. Research is unconstrained.*

## Research Questions — Answers

### 1. Does the script already satisfy "single proof chain"? Or three separate flows?

**Answer:** Three **separate** flows. There is no single proof chain.

| Test | Flow | Subsystems Used |
|------|------|-----------------|
| 1 | DELTA12(50) - q*etaq^24 = 0; provemodfuncGAMMA0id | ETA identity prover + modforms (DELTA12) |
| 2 | rr → jacprodmake → jac2eprod (GETA/EETA); provemodfuncid | Theta IDs + eta (jac2eprod → generalized eta) |
| 3 | etamake(theta3(100), 100) | Theta → eta product |

**"Single proof chain" (strict):** One computation where a theta expression is converted to eta, an eta identity is proved, and a modular form basis (e.g. `makebasisM` / `makeALTbasisM`) is used—all in one logical sequence. Example: theta → jacprodmake → jac2eprod → GETA → provemodfuncid, while the left-hand side is expressed in terms of `makebasisM(12,T)` and `DELTA12`. The current script does **not** do this.

### 2. Does the script use makeALTbasisM or makebasisM?

**Answer:** No. The script uses:
- `DELTA12(T)` — single modular form (discriminant cusp form)
- `provemodfuncGAMMA0id` — eta identity prover
- `provemodfuncid` — theta/Jacobi identity prover
- `jacprodmake`, `jac2eprod`, `etamake` — theta/eta conversions

**Recommendation:** If "modular form basis" means `makebasisM`/`makeALTbasisM`, Phase 102 should add a test that uses one of them in the chain (e.g. express a cusp form in a basis and prove an identity).

### 3. What does "passes" mean — does the script currently pass or fail?

**Answer:** The script **passes** when run with `bash` and standard Unix tools in PATH.

- **Environment:** Under Cygwin with `export PATH=/usr/bin:$PATH`, all 5 tests pass.
- **Failure mode:** If `grep` is not in PATH (e.g. minimal Cygwin), the script fails with "grep: command not found" and reports 0 passed, 5 failed.
- **Fix:** Ensure test runners (Makefile, CI) invoke bash with a PATH that includes `/usr/bin` (or use `env`/`PATH` in the script).

### 4. What acceptance tests and maple-checklist exist? What "no regressions" entails?

**Answer:**

| Test Suite | Location | What It Covers |
|------------|----------|----------------|
| maple-checklist | `tests/maple-checklist.sh` | 41 blocks from maple_checklist.md; qseriesdoc parity |
| run-all | `tests/run-all.sh` | 14 acceptance scripts (acceptance.sh, acceptance-eta-cusp.sh, etc.); does **not** include integration-eta-theta-modforms.sh |
| acceptance-all (Makefile) | `make acceptance-all` | maple-checklist + run-all + acceptance-exercises + acceptance-factor + acceptance-modforms + acceptance-theta-ids + **integration-eta-theta-modforms** |

**No regressions:** All of the above must pass. Integration test is part of `acceptance-all`, not `run-all`.

## Standard Stack

### Existing Components (no new libraries)

| Component | Location | Purpose |
|-----------|----------|---------|
| provemodfuncGAMMA0id | eta_cusp.h | Prove eta-quotient identity on Gamma_0(N) via Sturm |
| provemodfuncid | theta_ids.h | Prove Jacobi theta identity on Gamma_1(N) |
| DELTA12, makebasisM, makeALTbasisM | modforms.h | Modular forms: discriminant, E4/E6 basis, Delta basis |
| jacprodmake, jac2eprod, etamake | convert.h, theta_ids.h | Theta ↔ eta conversions |

### Test Infrastructure

| Script | Pattern |
|--------|---------|
| integration-eta-theta-modforms.sh | `check(desc, input, grep_pattern)`; `run(cmd1, cmd2, ...)` pipes to BIN |

## Architecture Patterns

- **Integration test:** Pipe REPL commands to `dist/qseries.exe`, capture output, grep for expected strings.
- **Binary discovery:** `./dist/qseries.exe` or `./qseries.exe` or `./qseries`.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Modular form basis | Custom basis | makebasisM / makeALTbasisM | Already implemented |
| Eta identity proof | Custom Sturm | provemodfuncGAMMA0id | Phase 82 delivered |
| Theta identity proof | Custom Sturm | provemodfuncid | theta_ids.h |

## Common Pitfalls

### Pitfall 1: Minimal PATH in CI/Cygwin
**What goes wrong:** `grep: command not found` → all checks fail.
**How to avoid:** Set `PATH=/usr/bin:$PATH` before running integration script, or ensure Makefile/CI uses a shell with full PATH.

### Pitfall 2: Confusing "modular form" with "modular form basis"
**What goes wrong:** Using only DELTA12 and assuming it satisfies "modular form basis."
**Clarification:** DELTA12 is a single form. `makebasisM(k,T)` and `makeALTbasisM(k,T)` return a **basis** of M_k(SL_2(Z)).

### Pitfall 3: Interpreting "single proof chain" loosely
**What goes wrong:** Treating three separate flows as equivalent to one chain.
**Clarification:** Success criterion 2 explicitly says "in a **single** proof chain." Three flows = three chains unless the planner/user explicitly relaxes the criterion.

## Code Examples

### Current integration test pattern

```bash
# From tests/integration-eta-theta-modforms.sh
check "DELTA12(50) - q*etaq^24 = 0" "DELTA12(50) - q*etaq(1,50)^24" "^0"
check "provemodfuncGAMMA0id eta identity" "provemodfuncGAMMA0id([[1, 2,24, 1,-24], [-1, 2,24, 1,-24]], 2)" "proven=1"
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "j := jacprodmake(rr, 50)" "jac2eprod(j)" | grep -qE "GETA|EETA"
```

### Possible single proof chain (illustrative)

One way to satisfy "single proof chain" would be a test that:
1. Uses `makebasisM(12, 100)` or `makeALTbasisM(12, 100)` to get a basis (modular form basis)
2. Expresses `DELTA12` in that basis (modforms)
3. Relates to an eta product via `etaq` (eta)
4. Converts a theta sum (e.g. RR) to GETA via `jacprodmake` + `jac2eprod` (theta IDs)
5. Proves the identity with `provemodfuncid` or `provemodfuncGAMMA0id`

Or a simpler chain: `theta3 → etamake → eta product` then `provemodfuncGAMMA0id` on an eta-quotient derived from that product, while checking the product lies in the span of `makebasisM(4,T)`.

## Plan Scope Recommendations

### Option A: Minimal (script fix only)
- Ensure `integration-eta-theta-modforms.sh` passes robustly (PATH handling).
- Interpret success criterion 2 as "three flows cover eta + theta + modforms" → **already satisfied**.
- **Scope:** Fix test runner environment if needed; no new tests.

### Option B: Add single proof chain (recommended if criterion 2 is strict)
- Add a new integration test that chains eta identity + theta ID + modular form basis in one flow.
- Use `makebasisM` or `makeALTbasisM` in that chain.
- **Scope:** 1–2 new tests, PATH fix if needed.

### Option C: Clarify then decide
- Document the ambiguity in success criterion 2 in CONTEXT.md.
- Planner implements Option A; if user wants Option B, add a follow-up task.

## Open Questions

1. **Criterion 2 interpretation:** Is "single proof chain" strict (one flow using all three) or permissive (three flows covering all three)?
2. **Modular form basis:** Does INT-02 require `makebasisM`/`makeALTbasisM`, or is DELTA12 sufficient?
3. **Environment:** Should Phase 102 harden the integration script for minimal PATH (e.g. `#!/usr/bin/env bash` + explicit `PATH` export)?

## Sources

### Primary (HIGH confidence)
- `tests/integration-eta-theta-modforms.sh` — read in full
- `tests/maple-checklist.sh` — structure, run pattern
- `Makefile` — acceptance-all, integration target
- `tests/run-all.sh` — scripts included
- `.planning/ROADMAP.md` — Phase 102 success criteria
- `.planning/REQUIREMENTS.md` — INT-02

### Secondary (verified)
- `src/modforms.h`, `src/eta_cusp.h`, `src/theta_ids.h` — provemodfunc*, DELTA12, makebasisM, makeALTbasisM
- Phase 101 RESEARCH — makeALTbasisM implementation

## Metadata

**Confidence breakdown:**
- Script behavior: HIGH — ran successfully with proper PATH
- Single vs three flows: HIGH — script structure is clear
- makebasisM usage: HIGH — grep confirms not used
- Criterion 2 intent: LOW — requires user/planner decision

**Research date:** 2026-03-05
**Valid until:** ~30 days (requirements stable)
