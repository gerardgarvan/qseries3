# Phase 87: Integration Testing — Research

**Researched:** 2025-03-02  
**Domain:** End-to-end test orchestration, cross-package verification, regression testing  
**Confidence:** HIGH

## Summary

Phase 87 is about ensuring all v6.0 packages work together. The codebase already has a substantial test suite: `maple-checklist.sh` (41 blocks from qseriesdoc), `acceptance.sh` (9 SPEC tests), 25+ `acceptance-*.sh` scripts (per-phase), and `integration-tcore.sh`. The main gaps are: (1) **run-all.sh** does not include `maple-checklist`, `acceptance-modforms`, or `acceptance-theta-ids`; (2) no dedicated cross-package integration tests for ETA + theta IDs + modforms; (3) partition-statistics ↔ rank/crank GF consistency is only partially covered.

**Primary recommendation:** Add a single `make acceptance-all` target that runs maple-checklist, run-all, acceptance-modforms, and acceptance-theta-ids; add one new cross-package integration script (`tests/integration-eta-theta-modforms.sh`) that chains etamake → provemodfuncGAMMA0id and jacprodmake → jac2eprod → provemodfuncid; extend acceptance-ptnstats or add a consistency check that sum_m N(m,n)=p(n) and sum_m M(m,n)=p(n) are validated (already in acceptance-rank-crank) and that PDP/PRR counts match enumeration where feasible.

---

<user_constraints>

## User Constraints (from CONTEXT.md)

### Locked Decisions

- Phase goal: End-to-end verification of all v6.0 packages, full regression  
- Depends on: All v6.0 phases (76–86)

### Claude's Discretion

- How to implement full regression (single target vs multiple targets)
- How to structure cross-package tests (new script vs extending existing)
- Whether to add partition-statistics ↔ rank/crank consistency checks beyond existing acceptance-rank-crank

### Deferred Ideas (OUT OF SCOPE)

- None specified

</user_constraints>

---

## Test Inventory

### Scripts

| Script | Purpose | Phase | In run-all.sh? |
|--------|---------|-------|----------------|
| `maple-checklist.sh` | qseriesdoc.md validation (blocks 1–41) | Core | **No** |
| `acceptance.sh` | 9 SPEC acceptance tests | Core | Yes |
| `acceptance-rank-crank.sh` | rankgf, crankgf, N(m,n), M(m,n), Σ=p(n) | 69 | Yes |
| `acceptance-crank.sh` | Crank/Rank tables (NS, MBAR, M2N, pbar, GFDM2N) | 79 | Yes |
| `acceptance-bailey.sh` | betafind, alphaup/alphadown, bailey_sum | 80 | Yes |
| `acceptance-eta-cusp.sh` | cuspmake, gammacheck, provemodfuncGAMMA0id | 81/82 | Yes |
| `acceptance-modforms.sh` | DELTA12, makebasisM, makebasisPX | 83 | **No** |
| `acceptance-theta-ids.sh` | QP2, cuspmake1, jac2eprod, provemodfuncid | 84 | **No** |
| `acceptance-partition-type.sh` | Partition-type predicates | — | Yes |
| `acceptance-tcore.sh` | t-core functions | 75 | Yes |
| `acceptance-gsk.sh` | GSK-related | — | Yes |
| `acceptance-vectors.sh` | Vector ops | — | Yes |
| `acceptance-tcrank-display.sh` | t-crank display | — | Yes |
| `acceptance-ptnstats.sh` | drank, agcrank, PDP, PRR, ptnDP, ptnRR, etc. | 76 | Yes |
| `acceptance-mock.sh` | mockdesorder, mockqs (f3, phi3, f05, …) | 77 | Yes |
| `acceptance-misc.sh` | newprodmake, EISENq, dilly, sieveqcheck, polyfind | 78 | Yes |
| `integration-tcore.sh` | makebiw, tcoreofptn, phi1/invphi1, nvec roundtrip | 75 | Yes |

### Other acceptance scripts (not in run-all.sh)

| Script | Purpose |
|--------|---------|
| `acceptance-modp.sh` | Modular series arithmetic (Phase 67) |
| `acceptance-worksheet-mod7.sh` | Garvan mod-7 eta dissection (Phase 68) |
| `acceptance-powfrac.sh` | Fractional power infrastructure |
| `acceptance-exercises.sh` | Exercise solutions regression |
| `acceptance-rr-id.sh` | Rogers-Ramanujan ID |
| `acceptance-checkprod-checkmult.sh` | checkprod, checkmult |
| `acceptance-mprodmake.sh` | mprodmake |
| `acceptance-wins.sh` | Phase 23 quick wins |
| `acceptance-v18.sh` | v1.8 function tests |
| `acceptance-suppress-output.sh` | Semicolon suppress |
| `acceptance-arrow-keys.sh` | TTY arrow keys |
| `acceptance-history.sh` | Up/down history |
| `acceptance-optional-args.sh` | Optional-arg variants |
| `acceptance-findmaxind.sh` | findmaxind |
| `acceptance-robustness.sh` | Robustness/edge cases |

### Makefile targets

| Target | Script |
|--------|--------|
| `acceptance` | acceptance.sh |
| `acceptance-maple` | maple-checklist.sh |
| `acceptance-eta-cusp` | acceptance-eta-cusp.sh |
| `acceptance-modforms` | acceptance-modforms.sh |
| `acceptance-theta-ids` | acceptance-theta-ids.sh |
| `acceptance-mprodmake` | acceptance-mprodmake.sh |
| `acceptance-checkprod-checkmult` | acceptance-checkprod-checkmult.sh |
| Others | acceptance-qol, acceptance-wins, acceptance-suppress-output, etc. |

**Gap:** There is no single Makefile target that runs the full regression set (maple-checklist + run-all + acceptance-modforms + acceptance-theta-ids).

---

## maple-checklist.sh — Blocks 1–41

| Block range | Content |
|-------------|---------|
| 1–2 | Rogers-Ramanujan series, prodmake |
| 3–6 | T(8,8), qfactor, dixson |
| 7–9 | etamake(theta2/3/4) |
| 10 | RootOf (skip) |
| 11–14 | jacprodmake, jac2prod, jac2series on RR and Slater |
| 15–16 | findhom degree 1/2 (AGM) |
| 17–18 | UE Eisenstein, findhomcombo |
| 19–23 | findnonhom, Watson T²=f(T,ξ) |
| 24 | collect (skip) |
| 25 | findpoly theta quotients |
| 26–27 | sift, etamake sifted PD |
| 28–32 | tripleprod, quinprod (some skip for symbolic) |
| 33–35 | sift Euler, jacprodmake, jac2prod E0 |
| 36–41 | quinprod, Winquist IDG, jacprodmake IDG, winquist, IDG−winquist=0 |

**Note:** maple-checklist allows up to 5 failures (`[ "$FAIL" -le 5 ]`) — some blocks may be known to skip or fail.

---

## Cross-Package Verification Scenarios

### Scenario 1: ETA identity prover + modular forms basis

**Packages:** `eta_cusp.h` (provemodfuncGAMMA0id), `modforms.h` (makebasisM, DELTA12)

**Flow:**

1. `DELTA12(50) - q*etaq(1,50)^24` — modforms and qfuncs produce identical Delta (already in acceptance-modforms).
2. `provemodfuncGAMMA0id([[1, 2,24, 1,-24], [-1, 2,24, 1,-24]], 2)` — proves η(2τ)²⁴/η(τ)²⁴ − same = 0 on Γ₀(2) (already in acceptance-eta-cusp).
3. **End-to-end:** `etamake(theta3, 100)` → eta form; verify with `provemodfuncGAMMA0id` that the identified form equals itself. Or: build a form from `makebasisM(12, 50)` and show it matches `q*etaq(1,50)^24` as eta product via etamake.

**Current coverage:** acceptance-modforms and acceptance-eta-cusp each test their package; no single test chains both.

### Scenario 2: Theta IDs (jac2eprod) + provemodfuncid

**Packages:** `convert.h` (jacprodmake), `theta_ids.h` (jac2eprod, provemodfuncid)

**Flow:**

1. `rr := sum(q^(n²)/aqprod(q,q,n,50), n, 0, 8)`
2. `j := jacprodmake(rr, 50)`
3. `jac2eprod(j)` → GETA/EETA form
4. `provemodfuncid([[1, [5,25,-1], [10,25,1]], [-1, [5,25,-1], [10,25,1]]], 25)` — proves trivial identity (already in acceptance-theta-ids).

**Current coverage:** acceptance-theta-ids Test 6 runs rr → jacprodmake → jac2eprod; Test 7/8 run provemodfuncid on predefined jacids. No test runs jacprodmake → jac2eprod → provemodfuncid on the same identity.

### Scenario 3: Full ETA + theta + modforms chain

**Proposed integration test:**

```
# Build RR series, convert to Jacobi, then to GETA, prove trivial identity
rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)
j := jacprodmake(rr, 50)
e := jac2eprod(j)
# Verify e is GETA/EETA
# provemodfuncid on LHS-RHS=0 for a known theta identity
```

And:

```
# DELTA12 is in M_12; makebasisM(12,50) spans it
# etamake on DELTA12 yields [[1,24]]
# provemodfuncGAMMA0id proves Delta - Delta = 0
```

---

## Mock Theta + Bailey Chain Test Locations

### Mock theta (`acceptance-mock.sh`)

- `mockdesorder(3)`, `mockdesorder(5)`, `mockdesorder(2)`, `mockdesorder(10)` — lists and counts
- `mockqs(f3, 3, 20)`, `mockqs(phi3, 3, 15)`, `mockqs(f05, 5, 15)`, `mockqs(phi10, 10, 15)`, `mockqs(A2, 2, 15)`, `mockqs(S08, 8, 15)` — series coefficients
- help(mockqs), help(mockdesorder)

### Bailey chain (`acceptance-bailey.sh`)

- betafind(0,20) unit beta
- betafind(1,20) coeff 5 = 6
- alphadown(alphaup(1,20),20) = 1
- bailey_sum(0/1/2, 5, 25) valid
- bailey_sum(0,8,30) coeff 3

**Verification:** Both are in run-all.sh; no additional mock/Bailey integration tests exist.

---

## Partition Statistics Validation (rank/crank GFs)

### Rank/crank GFs (Phase 69)

- **rankgf(m, T):** Σ_n N(m,n) q^n
- **crankgf(m, T):** Σ_n M(m,n) q^n
- **Identities:** Σ_m N(m,n) = p(n), Σ_m M(m,n) = p(n)

### acceptance-rank-crank.sh coverage

- Symmetry: rankgf(2)=rankgf(-2), crankgf(3)=crankgf(-3)
- N(0,5)=1, N(0,7)=3, N(1,10)=5
- M(0,0)=1, M(0,1)=-1, M(1,7)=2, M(2,10)=4
- Σ_m N(m,10) = p(10) = 42 (Test 10)
- Σ_m M(m,10) = p(10) = 42 (Test 11)

### acceptance-ptnstats.sh coverage

- drank, agcrank on specific partitions
- ptnDP, ptnOP, ptnRR, ptnCC, ptnSCHUR, ptnOE
- PDP(n), POE(n), PRR(n), PSCHUR(n)
- overptns, overptnrank, overptncrank

### Consistency check

**Success criterion 4:** Partition statistics consistent with rank/crank GFs.

- **Already verified:** Σ_m N(m,n)=p(n) and Σ_m M(m,n)=p(n) (acceptance-rank-crank).
- **Possible extension:** For small n, enumerate partitions with ptnRR, count PDP(n), PRR(n), etc., and compare with coefficients of rank/crank GFs or known identities. This would require iterating `partitions(n)` and applying predicates — more costly and not currently implemented.

**Recommendation:** Success criterion 4 is satisfied by acceptance-rank-crank (Tests 10–11). Optional enhancement: add a small-n check that PDP(5)=3, PRR(10)=6 etc. match the counting functions (already in acceptance-ptnstats).

---

## Binary Path Inconsistency

- **run-all.sh** runs scripts from repo root; scripts use `./dist/qseries.exe` or `./qseries.exe` fallbacks.
- **integration-tcore.sh** uses `QSERIES="./qseries --no-banner"` with no dist/ fallback.
- **acceptance-bailey.sh** uses `./qseries.exe` directly.

**Pitfall:** If binary is only at `dist/qseries.exe`, integration-tcore and acceptance-bailey may fail when run via run-all.sh from repo root. Verify each script's BIN/QSERIES resolution or standardize to `./dist/qseries.exe` (or `$BIN` from a shared setup).

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Test runner | Custom harness | bash scripts + Makefile | Existing pattern, simple, works |
| Output parsing | Complex parsers | grep -qE, tail -1 | Matches current scripts |
| Binary path | Hardcoded paths | Fallback chain (dist/qseries.exe, qseries.exe, qseries) | Cross-platform, local builds |

---

## Common Pitfalls

### Pitfall 1: run-all.sh omits critical tests

**What goes wrong:** Full regression believed passing while maple-checklist, acceptance-modforms, acceptance-theta-ids are never run.

**How to avoid:** Add `acceptance-all` target that runs maple-checklist, run-all, acceptance-modforms, acceptance-theta-ids (or fold the missing scripts into run-all.sh).

### Pitfall 2: Binary path mismatch

**What goes wrong:** integration-tcore or acceptance-bailey fails when run from repo root because they expect `./qseries` but binary is at `./dist/qseries.exe`.

**How to avoid:** Standardize BIN resolution: try `./dist/qseries.exe`, `./dist/qseries`, `./qseries.exe`, `./qseries` in that order (as acceptance-eta-cusp does).

### Pitfall 3: maple-checklist allows 5 failures

**What goes wrong:** Regression could creep in while checklist still "passes" (exit 0).

**How to avoid:** Document which blocks are expected to fail/skip; consider tightening to FAIL=0 for blocks that should always pass.

---

## Architecture Patterns

### Existing test script pattern

```bash
PASS=0; FAIL=0
check() {
    local desc="$1" input="$2" pattern="$3"
    out=$(echo "$input" | "$BIN" 2>&1)
    if echo "$out" | grep -qE "$pattern"; then
        echo "PASS: $desc"; ((PASS++))
    else
        echo "FAIL: $desc"; ((FAIL++))
    fi
}
check "label" "command" "pattern"
echo "Total: $PASS passed, $FAIL failed"
exit $FAIL
```

### run-all.sh aggregation pattern

```bash
for t in tests/script1.sh tests/script2.sh ...; do
    result=$(bash "$t" 2>&1)
    p=$(echo "$result" | tail -1 | grep -oP '[0-9]+ passed' | grep -oP '[0-9]+')
    f=$(echo "$result" | tail -1 | grep -oP '[0-9]+ failed' | grep -oP '[0-9]+')
    total_pass=$((total_pass + ${p:-0}))
    total_fail=$((total_fail + ${f:-0}))
done
```

**Note:** grep -oP (Perl regex) may not be available on all systems (e.g., macOS grep). Consider `grep -oE` or a portable fallback.

---

## Recommended Implementation

1. **Full regression target:** Add `acceptance-all` to Makefile:
   - `acceptance-maple`
   - `run-all.sh` (or expand run-all to include maple-checklist, acceptance-modforms, acceptance-theta-ids)
   - `acceptance-modforms`
   - `acceptance-theta-ids`

2. **Cross-package script:** Add `tests/integration-eta-theta-modforms.sh` with 2–3 tests:
   - DELTA12 = q*eta^24 and provemodfuncGAMMA0id trivial eta identity
   - rr → jacprodmake → jac2eprod → GETA; provemodfuncid trivial

3. **Binary path:** Add dist/ fallback to integration-tcore.sh and acceptance-bailey.sh (or source a shared `tests/common.sh` with BIN resolution).

---

## Sources

### Primary (HIGH confidence)
- Codebase: tests/*.sh, Makefile, src/eta_cusp.h, src/theta_ids.h, src/modforms.h, src/repl.h

### Metadata

**Confidence breakdown:**
- Test inventory: HIGH — direct file read
- Cross-package scenarios: HIGH — traced from repl.h and acceptance scripts
- Pitfalls: MEDIUM — inferred from script patterns

**Research date:** 2025-03-02  
**Valid until:** 30 days
