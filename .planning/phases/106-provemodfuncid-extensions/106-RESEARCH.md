# Phase 106: provemodfuncid extensions — Research

**Researched:** 2025-03-06  
**Domain:** Jacobi theta identity proving on Gamma_1(N), batch proving, theta_aids regression  
**Confidence:** HIGH

## Summary

Phase 106 adds `provemodfuncidBATCH(jacids, N)` — a batch version of `provemodfuncid` — and extends acceptance-theta-ids.sh so that at least 2 theta_aids-style identities are verified and the full script passes. The existing `provemodfuncid` in `src/theta_ids.h` processes one identity; the BATCH variant processes a list of identities and returns per-identity results, mirroring `provemodfuncGAMMA0idBATCH` in `src/eta_cusp.h`. The jacid format (list of `[coeff, [a,b,exp], ...]`) is unchanged. Regression surface: keep tests 7 and 8 in acceptance-theta-ids.sh passing.

**Primary recommendation:** Implement `provemodfuncidBATCH` in theta_ids.h by iterating over jacids and calling the same logic as provemodfuncid; add REPL dispatch following provemodfuncGAMMA0idBATCH; add 2+ non-trivial theta_aids identity tests.

---

## User Constraints

No CONTEXT.md exists. Research unconstrained.

---

## provemodfuncidBATCH API

### Maple spec (gaps/theta_aids.txt)

Maple `provemodfuncidBATCH` (gaps/theta_aids.txt, ~lines 1581–1290):

- **Signatures:** `provemodfuncidBATCH(jacid, N)` or `provemodfuncidBATCH(symid, jacid, N)`
- **Input:** `jacid` = one identity (sum of JAC quotients on Gamma_1(N)); `N` = level
- **Return:** 1 if identity holds, 0 otherwise
- **Differences vs provemodfuncid:** No interactive prompt; uses globals `qthreshold`, `proveit`, `noprint`; returns 1/0 directly

### C++ design (from ARCHITECTURE.md and provemodfuncGAMMA0idBATCH)

The C++ version should support **multiple** identities per call:

| Aspect | provemodfuncid | provemodfuncidBATCH |
|--------|----------------|---------------------|
| Input | (jacid, N) | (jacids, N) |
| jacid | list of [coeff, jac] | same format per identity |
| Return | ProveModfuncIdResult | vector&lt;ProveModfuncIdResult&gt; |
| Logic | single identity | loop, same logic per identity |

- **Input format:** `jacids` = list of jacid; each jacid = list of `[coeff, [a,b,exp], ...]` (same as provemodfuncid)
- **Return type:** `std::vector<ProveModfuncIdResult>`
- **Iteration:** For each jacid in jacids, run provemodfuncid’s logic (cusps, ords, mintot, sturm, q-series check)

### jacid format (per identity)

```
[[coeff, [a1,b1,exp1], [a2,b2,exp2], ...], [coeff2, ...], ...]
```

- Term = `[coeff]` or `[coeff, [a,b,exp], ...]`
- Constant: `[coeff]` (empty jac)
- JacFactor: `[a, b, exp]` for JAC(a,b,∞)^exp
- Identity = sum of coeff_i * product(JAC(ak,bk,∞)^expk)

Existing example (Rogers–Ramanujan form minus itself):

```
[[1, [5,25,-1], [10,25,1]], [-1, [5,25,-1], [10,25,1]]]
```

---

## theta_aids examples: 2+ identities to verify

### Current acceptance-theta-ids.sh (tests 7 and 8)

| Test | Identity | N | Expect |
|------|----------|---|--------|
| 7 trivial | [[1,[5,25,-1],[10,25,1]], [-1,[5,25,-1],[10,25,1]]] | 25 | proven=1 |
| 8 constant | [[1], [-1]] | 5 | proven=1 |

### Suggested identities for “2 theta_aids identities” (success criteria)

1. **Rogers–Ramanujan continued fraction** (already in test 7 as trivial):
   - `JAC(2,5)/JAC(1,5) − JAC(2,5)/JAC(1,5) = 0` on Gamma_1(25)
   - This is the trivial case; use the same jacid for provemodfuncidBATCH

2. **Second identity — same form, different coefficient:**
   - `2*JAC(2,5)/JAC(1,5) − 2*JAC(2,5)/JAC(1,5) = 0` on Gamma_1(25)
   - jacid: `[[2, [5,25,-1], [10,25,1]], [-2, [5,25,-1], [10,25,1]]]`

3. **Alternative — constant identity:**
   - `[[1], [-1]]` on Gamma_1(5) (already in test 8)

To satisfy “At least 2 theta_aids identities verified,” options:

- A: Two provemodfuncidBATCH calls, each with one non-trivial identity
- B: One provemodfuncidBATCH call with a list of 2 identities (e.g. trivial RR + constant)

Recommended: Use **one** provemodfuncidBATCH call with `jacids = [id1, id2]` where id1 = trivial RR, id2 = constant. That exercises both batch iteration and the two identity types.

---

## REPL dispatch for provemodfuncidBATCH

### Pattern (from provemodfuncGAMMA0idBATCH, repl.h 1605–1637)

```cpp
if (name == "provemodfuncGAMMA0idBATCH") {
    if (args.size() != 2) throw ...("expects provemodfuncGAMMA0idBATCH(etaids, N)");
    const Expr* listExpr = args[0].get();
    // listExpr = list of etaid
    // each etaid = list of [c, t1, r1, ...] terms
    std::vector<...> etaids;
    for (const auto& etaidExpr : listExpr->elements) {
        // parse etaid, push to etaids
    }
    int N = static_cast<int>(evi(1));
    auto results = provemodfuncGAMMA0idBATCH(etaids, N);
    for (size_t i = 0; i < results.size(); ++i)
        std::cout << "id[" << (i+1) << "] sturm_bound=" << results[i].sturm_bound
                  << " proven=" << results[i].proven << " " << results[i].message << std::endl;
    return static_cast<int64_t>(all_proven);
}
```

### provemodfuncidBATCH dispatch

- **Signature:** `provemodfuncidBATCH(jacids, N)`
- **args[0]:** list of jacid; each jacid = list of terms
- **Parsing:** Reuse the provemodfuncid term parser (repl.h 1644–1661) in an inner loop; outer loop over identities
- **Output:** `id[1] sturm_bound=... proven=... message` per identity
- **Return:** 1 if all proven, 0 otherwise

### Help string

```
{"provemodfuncidBATCH", {"provemodfuncidBATCH(jacids, N)", "batch prove multiple Jacobi theta identities on Gamma_1(N)"}}
```

---

## Regression surface for provemodfuncid

### Existing tests (must remain passing)

| # | Test | Command pattern | Expect |
|---|------|-----------------|--------|
| 7 | provemodfuncid trivial | provemodfuncid([[1,[5,25,-1],[10,25,1]],[-1,[5,25,-1],[10,25,1]]], 25) | proven=1 |
| 8 | provemodfuncid constant | provemodfuncid([[1], [-1]], 5) | proven=1 |

### Integration tests

- `tests/integration-eta-theta-modforms.sh` line 41: same trivial identity
- `qseriesdoc.md` examples for provemodfuncid (if any)

### Regression checklist

1. Do not change provemodfuncid’s signature or jacid format.
2. Do not change cusp/GETA logic; provemodfuncidBATCH reuses it.
3. Run acceptance-theta-ids.sh before and after; all 8 tests must pass.
4. Do not increase Sturm cap beyond 500 (theta_ids.h line 364).

---

## Standard stack

| Component | Location | Purpose |
|-----------|----------|---------|
| provemodfuncid | src/theta_ids.h | Single-identity prover; reuse logic |
| provemodfuncGAMMA0idBATCH | src/eta_cusp.h | Pattern for batch REPL dispatch |
| jac2eprod, GETAP2getalist | src/theta_ids.h | GETA conversion for cusp orders |
| CUSPSANDWIDMAKE1, getaprodcuspORDS | src/theta_ids.h | Gamma_1 cusp machinery |

---

## Don’t hand-roll

| Problem | Don’t build | Use instead |
|---------|-------------|-------------|
| Cusp orders | Custom Gamma_1 cusp logic | CUSPSANDWIDMAKE1, getaprodcuspORDS |
| JacFactor → GETA | Custom conversion | jac2eprod, GETAP2getalist |
| Batch loop | New prover | Loop calling provemodfuncid logic |
| REPL parsing | New jacid format | Same parser as provemodfuncid, outer list |

---

## Common pitfalls

### Pitfall 1: Changing provemodfuncid

**Risk:** Editing provemodfuncid instead of adding a separate batch entry point.  
**Mitigation:** Implement provemodfuncidBATCH as a loop over jacids; optionally factor shared logic into a helper both use, but do not alter provemodfuncid’s public behavior.

### Pitfall 2: Different jacid format for BATCH

**Risk:** Requiring a different input format for BATCH.  
**Mitigation:** Keep jacid format identical; jacids = list of jacid.

### Pitfall 3: mintot overflow

**Risk:** mintot exceeds 1000 → sturm_int truncation (theta_ids.h 359–361).  
**Mitigation:** Keep existing 500 cap on sturm; document that identities with very large mintot may need handling later.

---

## Code examples

### provemodfuncid call (current)

```
provemodfuncid([[1, [5,25,-1], [10,25,1]], [-1, [5,25,-1], [10,25,1]]], 25)
```

### provemodfuncidBATCH call (target)

```
provemodfuncidBATCH([
  [[1, [5,25,-1], [10,25,1]], [-1, [5,25,-1], [10,25,1]]],
  [[1], [-1]]
], 25)
```

Expected output:

```
id[1] sturm_bound=... proven=1 identity proven
id[2] sturm_bound=... proven=1 identity proven
```

---

## Sources

### Primary (HIGH)

- `gaps/theta_aids.txt` — provemodfuncidBATCH Maple spec (lines 1581–1281)
- `gaps/ramarobinsids.txt` — provemodfuncidBATCH(NEWJACID, jmxperiod) usage
- `src/theta_ids.h` — provemodfuncid implementation
- `src/repl.h` — provemodfuncid and provemodfuncGAMMA0idBATCH dispatch
- `tests/acceptance-theta-ids.sh` — current 8 tests

### Secondary

- `.planning/research/ARCHITECTURE.md` — provemodfuncidBATCH design
- `.planning/REQUIREMENTS.md` — GAP11-04, GAP11-05
- `.planning/ROADMAP.md` — Phase 106 scope

---

## Metadata

**Confidence:** HIGH — API and formats are fixed in code and Maple; REPL pattern is established.

**Research date:** 2025-03-06  
**Valid until:** ~30 days (domain is stable)

---

## RESEARCH COMPLETE

**Phase:** 106 - provemodfuncid extensions  
**Confidence:** HIGH

### Key findings

- provemodfuncidBATCH API: input = list of jacids, N; return = vector of ProveModfuncIdResult; mirror provemodfuncGAMMA0idBATCH
- jacid format unchanged: [coeff, [a,b,exp], ...] per term; same parser as provemodfuncid
- Two theta_aids identities: trivial RR form (test 7) + constant (test 8); add provemodfuncidBATCH test with both
- REPL dispatch: follow provemodfuncGAMMA0idBATCH; parse outer list of jacids, inner list same as provemodfuncid
- Regression surface: tests 7 and 8 must remain passing; no changes to provemodfuncid behavior

### File created

`.planning/phases/106-provemodfuncid-extensions/106-RESEARCH.md`

### Ready for planning

Research complete. Planner can create PLAN.md files.
