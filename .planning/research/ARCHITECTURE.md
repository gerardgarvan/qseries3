# Architecture Patterns: qseries3 Gap Closure

**Project:** qseries3
**Domain:** q-series REPL, modular forms, eta/theta identity proving
**Researched:** 2025-03-06
**Milestone Context:** v11.1 — gaps and problems to resolve. Existing: modforms.h, eta_cusp.h, theta_ids.h, convert.h, linalg.h. Integration test passes.

---

## Executive Summary

Missing features (provemodfuncid enhancements, RR identity search, Block 25 fix, etc.) integrate into a well-defined data-flow pipeline. **provemodfuncid** and **RR identity search** are already implemented; the main gaps are extensions (provemodfuncidBATCH, findids types 3–10), robustness (Block 25 q-shift normalization), and deferred stubs (U_p operator). No new top-level components are required—only modifications and extensions to existing headers.

---

## Current Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  REPL + Parser (repl.h, parser.h)                                            │
├─────────────────────────────────────────────────────────────────────────────┤
│  Built-ins dispatch: provemodfuncGAMMA0id, provemodfuncid, RRG, RRH,        │
│  checkid, findids, jac2eprod, jac2series, prodmake, etamake, jacprodmake     │
├─────────────────────────────────────────────────────────────────────────────┤
│  modforms.h  │ eta_cusp.h      │ theta_ids.h  │ convert.h  │ rr_ids.h       │
│  DELTA12     │ provemodfunc    │ provemodfunc │ prodmake   │ RRG, RRH       │
│  makebasisM  │ GAMMA0id        │ id           │ etamake    │ checkid        │
│  makeALTbasisM│ etaprodtoqseries│ jac2eprod   │ jacprodmake│ findids        │
│              │ cuspmake        │ jac2series   │ jac2series │ geta           │
├─────────────────────────────────────────────────────────────────────────────┤
│  Series (std::map<int,Frac>)  │  Frac  │  BigInt                            │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Data Flow: Identity Proving Pipeline

### ETA identity (Gamma_0(N))

```
etaid: [(coeff, GP), ...]  →  etaprodtoqseries(GP)  →  Series
                                                         ↓
                                              Sturm bound from mintotGAMMA0ORDS
                                                         ↓
                                              Verify sum = 0 to O(q^sturm)
                                                         ↓
                                              provemodfuncGAMMA0id → proven=0/1
```

- **Component:** eta_cusp.h
- **Input:** `[[c, GP], ...]` where GP = `[t1,r1,t2,r2,...]` (eta-quotient)
- **Output:** `ProveModfuncResult { proven, sturm_bound, message }`

### Theta/Jacobi identity (Gamma_1(N))

```
jacid: [(coeff, JacFactor[]), ...]  →  jac2eprod(jac)  →  GETA [(n,a,c),...]
       |                                                    ↓
       └────────────────→ jac2series(jac) ─────────→ Series (q-expansion)
                                                         ↓
                                              Gamma1ModFunc(gl,N), getaprodcuspORDS
                                                         ↓
                                              Sturm bound from mintotORDS
                                                         ↓
                                              provemodfuncid → proven=0/1
```

- **Component:** theta_ids.h (uses convert.h jac2series)
- **Input:** `[[coeff, [[a,b,c],...]], ...]` (Jacobi product list)
- **Output:** `ProveModfuncIdResult { proven, sturm_bound, message }`

### Rogers-Ramanujan → Proof chain

```
Series (RR sum)  →  jacprodmake  →  JacFactor  →  jac2eprod  →  GETA
                                                         ↓
                                              provemodfuncid(jacid, N)
```

- **Integration test flow:** `rr := sum(...) → j := jacprodmake(rr,50) → jac2eprod(j) → provemodfuncid([...], 25)`

---

## Integration Points: Missing Features vs Existing

| Missing / Gap Feature            | Integrates With              | Data Flow Change | New vs Modified    |
|----------------------------------|------------------------------|------------------|--------------------|
| provemodfuncid enhancements      | theta_ids.h                  | None             | **Modified**       |
| provemodfuncidBATCH              | theta_ids.h                  | Batch jacid list | **New function**   |
| RR identity search (findids)     | rr_ids.h                     | Uses RRG/RRH, checkid | **Implemented** |
| checkid                          | convert.h (prodmake, etamake)| Series → CheckidResult | **Implemented** |
| Block 25 fix (q-shift)           | Series::addAligned, findpoly | Normalize q_shift in + | **Modified** |
| U_p operator (provemodfuncGAMMA0UpETAid) | eta_cusp.h          | Stub only        | **New impl**       |
| findlincombomodp                 | linalg.h, relations.h        | modp + solve     | **New function**   |

---

## Component Dependencies (Build Order)

```
BigInt → Frac → Series
         ↓
    qfuncs (aqprod, etaq, theta, EISENq, Phiq)
         ↓
    convert (prodmake, etamake, jacprodmake, jac2series, sift)
         ↓
    linalg (kernel, solve)
         ↓
    relations (findhom, findnonhom, findpoly)
         ↓
┌───────┴───────┬───────────────┬──────────────┐
│               │               │              │
eta_cusp    theta_ids      modforms       rr_ids
(Gamma_0)   (Gamma_1)      (M_k bases)    (RR, checkid, findids)
    │               │               │              │
    └───────────────┴───────────────┴──────────────┘
                           ↓
                      repl.h (dispatch)
```

---

## Recommended Build Order for Phases

### Tier 1: Foundation (no new components)

| Phase | What | Depends On | Output |
|-------|------|------------|--------|
| 97 Block 25 fix | q-shift alignment in Series::+ before findpoly | Series | `addAligned` used; Block 25 passes |
| 98 Block 24 | collect-style formatter or N/A docs | — | Test rationale documented |
| 99 findlincombomodp | f ≡ Σ c_i L_i (mod p) | modp, linalg F_p | New built-in |

**Rationale:** Block 25 fix unblocks findpoly on theta quotients; findlincombomodp completes modular linear-algebra story. No new headers.

### Tier 2: Theta IDs Extensions

| Phase | What | Depends On | Output |
|-------|------|------------|--------|
| 85 provemodfuncid | provemodfuncidBATCH; verify theta_aids | theta_ids, jac2eprod | Batch proof; 2+ theta_aids verified |

**Rationale:** provemodfuncid is implemented; Phase 85 adds BATCH and theta_aids regression coverage. Depends only on theta_ids (already integrated).

### Tier 3: RR Identity Search (Implemented; Verification)

| Phase | What | Depends On | Output |
|-------|------|------------|--------|
| 86 RR identity | findids types 3–10; acceptance tweaks | rr_ids, checkid, RRG/RRH | Extended findids; acceptance-rr-id passes |

**Rationale:** RRG, RRH, checkid, findids (types 1–2) exist. Gaps: findids types 3–10, discovery count tuning, q-shift handling in addSeriesAligned.

### Tier 4: Deferred / Lower Priority

| Phase | What | Notes |
|-------|------|-------|
| provemodfuncGAMMA0UpETAid | U_p sifting proof | Stub in eta_cusp.h; complex, defer |
| findids types 3–10 | Extended RR search | Types 1–2 suffice for v11.1 |

---

## Data Flow Changes for Gap Closure

### Block 25 (q-shift alignment)

**Current:** `findpoly(x, y, ...)` fails when x and y have different `q_shift` (e.g. theta2/theta3 quotients).

**Change:** Ensure `Series::operator+` (or a normalized path into findpoly) aligns q_shifts via `addAligned` or equivalent. `addAligned` exists in rr_ids.h and is used by findids; findpoly's input path must use similar normalization.

**Modified files:** `series.h` (add path), `relations.h` or findpoly caller.

### provemodfuncidBATCH

**Current:** `provemodfuncid(jacid, N)` processes one identity.

**Change:** Add `provemodfuncidBATCH(jacids, N)` that returns `vector<ProveModfuncIdResult>`. Same logic per identity.

**Modified files:** `theta_ids.h`, `repl.h`.

### findlincombomodp

**Current:** `findhommodp` exists; no degree-1 linear combo mod p.

**Change:** Add `findlincombomodp(f, L, p, T)` → coefficients in F_p. Uses `modp`, F_p Gaussian elimination (existing), solve step.

**Modified files:** `relations.h` or new `modp_linalg` section, `repl.h`.

---

## Anti-Patterns to Avoid

1. **Don't duplicate Sturm logic** — provemodfuncGAMMA0id and provemodfuncid each have Sturm-bound computation; keep them separate (Gamma_0 vs Gamma_1 cusp formulas differ).
2. **Don't bypass jac2eprod for provemodfuncid** — provemodfuncid requires GETA (generalized eta) for Gamma_1 cusp orders; JacFactor → jac2eprod → getalist is mandatory.
3. **Don't add new REPL types for identity results** — `ProveModfuncResult` and `ProveModfuncIdResult` are sufficient; display as strings.
4. **Don't change provemodfuncid's jacid format** — It expects `[(Frac, vector<JacFactor>), ...]`; ramarobinsids.txt uses provemodfuncidBATCH with same per-identity format.

---

## Scalability / Performance Notes

| Concern | At T=50 | At T=200 | At T=500 |
|---------|---------|----------|----------|
| provemodfuncid Sturm bound | ~100 | ~400 (capped 500) | 500 cap |
| findids search space | Small | Moderate | O(n²) pairs |
| checkid (prodmake + etamake) | Fast | Slower | May need T limit |
| jac2series with JacFactor | Fast | Depends on product size | — |

**Recommendation:** Cap Sturm at 500 (already done). For findids, consider T limit or early termination for large T.

---

## Sources

### Primary (HIGH confidence)

- `src/eta_cusp.h` — provemodfuncGAMMA0id, cuspmake, etaprodtoqseries
- `src/theta_ids.h` — provemodfuncid, jac2eprod, Gamma1ModFunc, getaprodcuspORDS
- `src/convert.h` — prodmake, etamake, jacprodmake, jac2series
- `src/rr_ids.h` — RRG, RRH, checkid, findids
- `tests/integration-eta-theta-modforms.sh` — integration flow

### Secondary

- `.planning/ROADMAP.md` — Phases 82, 84, 85, 86
- `.planning/phases/102-cross-package-integration/102-RESEARCH.md` — single proof chain
- `gaps/etapackage.txt` — Maple provemodfuncGAMMA0id / provemodfuncidBATCH usage
