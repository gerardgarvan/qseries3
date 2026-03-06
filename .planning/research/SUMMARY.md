# Project Research Summary — v11.1 Gap Closure & Improvements

**Project:** qseries3 (q-series REPL)  
**Domain:** Maple qseries parity, eta/theta identity proving, modular forms  
**Milestone:** v11.1 — Gap Closure & Improvements  
**Researched:** 2026-03-06  
**Confidence:** HIGH (mature codebase, zero-deps constraint, clear gaps)

---

## Executive Summary

qseries3 is a zero-dependency C++20 REPL for exact q-series arithmetic and Andrews-style series-to-product conversion. The codebase achieves 40/41 maple-checklist blocks. Remaining gaps are **algorithmic and integrative**—no stack additions required. For v11.1, close maple-checklist failures (Blocks 25, 10, 13–14), RR identity search (RRID-01..03), THETA-06 provemodfuncid extensions, Block 24 rationale, and findlincombomodp.

**Recommendation:** Keep C++20, single-file Makefile, in-house BigInt/Frac/Series. Address gaps by extending existing headers (theta_ids.h, rr_ids.h, linalg.h, relations.h). Phases 97–99 (Block 25, Block 24, findlincombomodp) form Tier 1; Theta IDs (Phase 85) and RR identity (Phase 86) form Tiers 2–3.

**Key risks:** (1) q-shift/truncation misalignment in series arithmetic—Block 25 and findpoly; (2) Sturm-bound precision and mintot→int overflow in provers; (3) Maple semantic drift (optional args, output format). Mitigation: align q_shifts before findpoly, document Sturm cap, audit optional args per new built-in.

---

## Key Findings

### Stack Additions: None

| Verdict | Rationale |
|---------|-----------|
| **No stack changes** | Gaps (provemodfuncid, findids, findlincombomodp, Block 25/24) reduce to existing Series, Frac, BigInt, and headers |
| Stay C++20 | Portability; C++23 not required |
| Keep Makefile, single TU | Zero deps; no CMake, Conan, vcpkg |
| No external libs | GMP, Boost, fmt, spdlog explicitly excluded |

**Optional low-priority tweaks:** `[[nodiscard]]` on critical returns; cppcheck in CI if zero install cost.

---

### Feature Table Stakes vs Differentiators

#### Table Stakes (users expect for Maple/Garvan parity)

| Feature | Source | Complexity |
|---------|--------|------------|
| Block 25 fix (findpoly q-shift) | maple-checklist, EX-10 | Medium |
| Block 10 / RootOf(ω) for b(q) | maple-checklist, EX-04c | High |
| Block 13–14 (Jacobi half-integer) | maple-checklist, Garvan §3.4 | Medium |
| THETA-06 provemodfuncid | REQUIREMENTS | High |
| RRID-01..03 (RRG, RRH, checkid, findids) | REQUIREMENTS | Medium–High |
| jac2series(f,T) user API | FEATURE-GAPS | Trivial |

#### Differentiators (advanced users)

| Feature | Value | Complexity |
|---------|-------|------------|
| factor(t8) cyclotomic (Block 4) | Polynomial factor into Φ_n | High |
| EISENqmake / Eisenstein memoization | Speed for repeated modform calls | Low |
| BAILEY-01..03 | Bailey pair/chain manipulation | Medium |
| ETA-01..08 full cusp prover | Gamma_0 eta-quotient proofs | High |
| findlincombomodp, findhommodp | Mod-p relation finding | Medium |

#### Defer to v2+

- Block 4 (factor cyclotomic); qfactor sufficient
- ETA-01..08 full cusp prover
- CRANK-01..05
- makeEISENbasisPX, symbolic bases

---

### Architecture: Integration Points and Build Order

#### Data Flow Pipeline

```
REPL/Parser → Built-ins (provemodfuncid, checkid, findids, findpoly, …)
                    ↓
eta_cusp | theta_ids | rr_ids | modforms | convert | relations | linalg
                    ↓
Series (map<int,Frac>) → Frac → BigInt
```

#### Integration Points for Gap Closure

| Gap | Integrates With | Type | Action |
|-----|-----------------|------|--------|
| Block 25 (q-shift) | Series::addAligned, findpoly | **Modified** | Use addAligned before findpoly |
| provemodfuncidBATCH | theta_ids.h | **New function** | Batch jacid list |
| findids types 3–10 | rr_ids.h | **Extension** | Implement remaining types |
| findlincombomodp | linalg.h, relations.h | **New function** | modp + F_p solve |
| Block 24 | repl.h | **Docs** | N/A rationale; no collect impl |

#### Recommended Build Order (Phase Tiers)

| Tier | Phases | Rationale |
|------|--------|-----------|
| **Tier 1** | 97, 98, 99 | Block 25 fix unblocks findpoly; Block 24 N/A rationale; findlincombomodp completes mod-p story. No new headers. |
| **Tier 2** | 85 | provemodfuncidBATCH, theta_aids regression. Depends on theta_ids (already integrated). |
| **Tier 3** | 86 | findids types 3–10, acceptance-rr-id. RRG/RRH/checkid exist. |
| **Deferred** | provemodfuncGAMMA0UpETAid | U_p operator; stub only; complex. |

#### Component Dependency Order

```
BigInt → Frac → Series → qfuncs → convert → linalg → relations
                    ↓
    eta_cusp | theta_ids | modforms | rr_ids
                    ↓
                  repl.h (dispatch)
```

---

### Pitfalls to Avoid

#### Critical (must address)

1. **Series inverse and truncation** — Wrong recurrence or q-shift causes prodmake/etamake to fail. Detection: Rogers-Ramanujan yields denominators at exponents ≠ ±1 (mod 5).

2. **q-shift alignment** — findpoly on theta quotients fails without addAligned. Prevention: align q_shift before addition; propagate min truncation. Phase: 63, 97.

3. **Maple semantic drift** — Optional-arg order, 1-based vs 0-based, output format. Prevention: run exact Maple command, document output, add real test in maple-checklist.sh.

4. **Double-sum truncation (EX-04c)** — b(q) double-sum needs sufficient T and range. Prevention: increase T (80–100), range (±15); verify SeriesOmega semantics.

5. **Sturm-bound precision** — mintot→int overflow or fractional mintot. Prevention: BigInt-based Sturm when needed; document 500 cap; explicit depth in output.

#### Phase-specific warnings

| Phase | Pitfall | Mitigation |
|-------|---------|------------|
| Block 25 | q-shift misalignment | addAligned before findpoly |
| Block 24 | Implementing collect | Document N/A; do not implement |
| provemodfuncid | mintot overflow | BigInt Sturm; document cap |
| findlincombomodp | F_p solve | Use existing modp, linalg kernel |
| New built-ins | Optional-arg drift | Audit 1/2/3-arg; add optional-arg test |

---

## Implications for Roadmap

### Phase 1: Block 25 Fix (Phase 97)
**Rationale:** Unblocks findpoly on theta quotients; EX-10; no new components.  
**Delivers:** Block 25 passes; findpoly works on theta2/theta3 quotients.  
**Addresses:** maple-checklist, EX-10.  
**Avoids:** Pitfall 2 (q-shift alignment).

### Phase 2: Block 24 Rationale (Phase 98)
**Rationale:** Clarify scope; avoid collect implementation.  
**Delivers:** N/A rationale documented; no implementation.  
**Avoids:** Pitfall 6 (Block 24 scope creep).

### Phase 3: findlincombomodp (Phase 99)
**Rationale:** Completes mod-p linear algebra story.  
**Delivers:** `findlincombomodp(f, L, p, T)` built-in.  
**Uses:** linalg.h F_p kernel, modp, relations.h.  
**Avoids:** Optional-arg drift.

### Phase 4: provemodfuncid Extensions (Phase 85)
**Rationale:** provemodfuncid exists; add BATCH and theta_aids regression.  
**Delivers:** provemodfuncidBATCH; 2+ theta_aids verified.  
**Avoids:** Pitfall 5 (Sturm-bound precision).

### Phase 5: RR Identity Search (Phase 86)
**Rationale:** RRG, RRH, checkid, findids types 1–2 exist.  
**Delivers:** findids types 3–10; acceptance-rr-id passes.  
**Avoids:** Maple semantic drift; output format mismatch.

### Research Flags

| Phase | Flag | Reason |
|-------|------|--------|
| 85 | Needs research if theta_aids grow | cusp formulas differ Gamma_0 vs Gamma_1 |
| 94 (Block 10 / RootOf) | Needs research | omega/SeriesOmega, b(q) double-sum semantics |
| 97, 98, 99 | Standard patterns | addAligned exists; linalg F_p exists; Block 24 is docs-only |

---

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | SPEC, .cursorrules, zero-deps explicit; no stack change needed |
| Features | HIGH | maple-checklist, REQUIREMENTS, FEATURE-GAPS well documented |
| Architecture | HIGH | Integration test passes; data flow documented in headers |
| Pitfalls | HIGH | SPEC §576+, known failure modes (Block 25, EX-04c, inverse) |

**Overall confidence:** HIGH

### Gaps to Address

- **EX-04c / Block 10:** RootOf(ω) and b(q) double-sum—Phases 91–94; may need phase research.
- **Sturm cap 500:** Document in output; consider BigInt path for large mintot.
- **maple-checklist Blocks 28–32:** Quinprod symbolic z; verify against current script.

---

## Sources

### Primary (HIGH confidence)
- `SPEC.md` — Zero-deps, single static binary
- `src/eta_cusp.h`, `src/theta_ids.h`, `src/rr_ids.h`, `src/convert.h`
- `tests/integration-eta-theta-modforms.sh`
- `maple_checklist.md`, `REQUIREMENTS.md`, `FEATURE-GAPS.md`

### Secondary
- `.planning/ROADMAP.md`, `.planning/PROJECT.md`
- `gaps/wprogmodforms.txt`, `gaps/BAILEY.txt`

---
*Research completed: 2026-03-06*  
*Ready for roadmap: yes*
