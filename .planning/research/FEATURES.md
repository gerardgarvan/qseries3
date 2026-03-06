# Feature Landscape: qseries3 Gap Closure

**Domain:** q-series REPL — Maple qseries parity, Garvan tutorial coverage, gap closure  
**Researched:** 2026-03-06  
**Sources:** REQUIREMENTS.md, maple_checklist.md, maple-checklist.sh, acceptance-exercises.sh, gaps/wprogmodforms.txt, gaps/BAILEY.txt, FEATURE-GAPS.md

---

## Executive Summary

qseries3 implements most core Maple qseries functionality. Remaining gaps fall into: (1) **maple-checklist failures** — Block 4 (factor), Block 10 (RootOf), Blocks 13–14 (Jacobi half-integer exponents), Block 25 (findpoly q-shift), plus symbolic triple/quinprod; (2) **REQUIREMENTS.md pending** — THETA-06, RRID-01..03, GAP-01..04, ETA-01..08, CRANK-01..05, BAILEY-01..03; (3) **acceptance-exercises failures** — EX-04c (b(q) double-sum with RootOf); (4) **modforms extensions** — EISENqmake, makeEISENbasisPX, makeSYMbasisM, makeALTSYMbasisM, etc. from gaps/wprogmodforms.txt. Table stakes: fix Block failures, pass acceptance-exercises, close THETA-06 and RRID. Differentiators: Bailey chains, full ETA cusp prover, mod-p identity search.

---

## Maple Checklist: Block Status

### Blocks 1–41 Summary

| Status | Blocks | Notes |
|--------|--------|-------|
| Verified | 1–3, 5–12, 15–23, 26–27, 29, 33–41 | Core prodmake, etamake, jacprodmake, findhom/nonhom, sift, Winquist |
| FAIL | 4, 10, 13, 14, 25 | factor, RootOf, Jacobi half-integer, findpoly q-shift |
| SKIP | 24, 28, 30–32 | collect (N/A), triple/quinprod symbolic z, quinprod prodid/seriesid |
| N/A | 4, 10, 21, 24 | factor→qfactor workaround; RootOf; EQNS[1]; collect |

**Note:** Phases 88–90 added quinprod prodid/seriesid and symbolic z for tripleprod/quinprod. maple-checklist.sh expects Blocks 28–32 to pass; verify with current script.

### Critical Block Failures (Must Fix)

| Block | Maple Feature | qseries3 Gap | Table Stakes? |
|-------|---------------|--------------|---------------|
| 4 | factor(t8) cyclotomic | No factor builtin; qfactor handles q-product only | Differentiator (polynomial factor) |
| 10 | omega := RootOf(z²+z+1=0); b(q) | No Q(ω); b(q) via eta identity works | Table stakes for Exercise 4 |
| 13–14 | jacprodmake Slater; jac2series | Half-integer JAC exponents; jac2series on frac exponents | Table stakes for Garvan §3.4 |
| 25 | findpoly on theta2/theta3 quotients | q-shift mismatch in series addition | Table stakes for Exercise 10 |

---

## REQUIREMENTS.md Pending Items

### Theta IDs / Ramanujan-Robins

| Req | Function | Purpose | Complexity |
|-----|----------|---------|------------|
| THETA-06 | provemodfuncid(jacid, N) | Prove identity on Gamma_1(N) via Sturm bound | High |
| RRID-01 | RRG(n), RRH(n), RRGstar(n), RRHstar(n) | Rogers-Ramanujan / Göllnitz-Gordon | Medium |
| RRID-02 | checkid(expr, T) | Check if expression is eta/theta product | Medium |
| RRID-03 | findids(type, T) | Systematic identity search (types 1–10) | High |

### GAP Functions (v1.4)

| Req | Function | Purpose | Status |
|-----|----------|---------|--------|
| GAP-01 | mprodmake(f,T) | (1+q^n1)*(1+q^n2)*... product form | Phase 24 (Pending) |
| GAP-02 | checkprod(f,T) | Check "nice" product | Phase 25 (Pending) |
| GAP-03 | checkmult(f,T) | Check multiplicative coefficients | Phase 25 (Pending) |
| GAP-04 | findmaxind(L,n,topshift) | Maximal independent subset | Phase 26 (Done per traceability) |

### ETA Cusp / Identity Prover

| Req | Function | Purpose | Complexity |
|-----|----------|---------|------------|
| ETA-01..06 | cuspmake, cuspord, gammacheck, cuspORDS, etaprodtoqseries, vp | Gamma_0 cusp theory | High |
| ETA-07..08 | provemodfuncGAMMA0id, provemodfuncGAMMA0UpETAid | Eta-quotient identity proof via Sturm | High |

### Crank / Bailey / Mod-p

| Req | Functions | Purpose | Complexity |
|-----|-----------|---------|------------|
| CRANK-01..05 | NS, sptcrankresnum, MBAR, ocrankresnum, GFDM2N, etc. | SPT-crank, overpartition crank, M2 tables | High |
| BAILEY-01..03 | betafind, alphafind, alphaup/down, betaup/down, catalog | Bailey pair computation and chain ops | Medium |
| MODP-01..02 | findlincombomodp, findhommodp | Mod-p linear / homogeneous relations | Medium |

---

## Acceptance-Exercises Failures

From `tests/acceptance-exercises.sh`:

| Test | Requirement | Current Status | Root Cause |
|------|-------------|----------------|------------|
| EX-04c | b(q) = Σ Σ ω^(n-m) q^(n²+nm+m²) starts 1 - 3q | FAIL | RootOf(3) / omega not implemented; sum with ω in exponent fails |
| EX-04a, 04b, 04d, 04e | b(q) eta, a(q), c(q) | PASS | eta identity works |
| EX-09a/b/c | findnonhomcombo N(q) in a, x | PASS | Relation finding works |
| EX-10a/b | findpoly cubic m–y relation | PASS (after Block 25 fix) | findpoly works if q-shift fixed |

**Table stakes:** EX-04c requires RootOf / omega for b(q) double-sum; Block 10 and Block 25 fixes are prerequisites.

---

## gaps/wprogmodforms.txt: Maple modforms Package

### Implemented in qseries3

| Maple modforms | qseries3 | Notes |
|----------------|----------|-------|
| makebasisM(k,T) | makebasisM(k,T) | E4, E6 basis ✓ |
| makeALTbasisM(k,T) | makeALTbasisM(k,T) | DELTA12, E6 basis ✓ |
| makebasisPX(k,T) | makebasisPX(k,T) | P, Phiq(1,3,5) basis ✓ |
| misc[EISENq] | EISENq / eisenstein | Eisenstein series ✓ |
| DELTA12, P | DELTA12(T), 1/etaq(1,T) | Cusp form, partition GF ✓ |

### Missing from qseries3

| Maple modforms | Purpose | Complexity |
|----------------|---------|------------|
| **EISENqmake(LT)** | Compute E2,E4,E6,E8,E10,E14,DELTA12,P once; save to global; reuse | Low — memoization / precompute |
| **makeEISENbasisPX(k,T)** | Basis P*X[k] using E2,E4,E6 instead of P1,P3,P5 | Medium |
| **makeSYMbasisM(k)** | Symbolic basis _E4^a*_E6^b (no T) | Low — display / teaching |
| **makeALTSYMbasisM(k)** | Symbolic basis with DELTA12 | Low |
| **makeSYMbasisPX(k)** | Symbolic P*P1^a*P3^b*P5^c | Low |
| **makeSYMEISENbasisPX(k)** | Symbolic P*E2^a*E4^b*E6^c | Low |
| **briefmfhelp, mfhelp, mffunctions, modformschanges, modformspversion** | Help / meta | Trivial |

**Recommendation:** EISENqmake is the most valuable missing piece for performance when repeatedly calling makebasisM/makeALTbasisM at different T. Symbolic bases are useful for teaching, not for computation.

---

## Table Stakes

Features users expect for Maple/Garvan parity. Missing = product feels incomplete.

| Feature | Why Expected | Complexity | Source |
|---------|--------------|------------|--------|
| Block 25 fix (findpoly q-shift) | Exercise 10, findpoly on theta quotients | Medium | maple-checklist, acceptance-exercises |
| Block 10 / RootOf(ω) | Exercise 4, b(q) double-sum, EX-04c | High | maple-checklist, Exercise 4 |
| Block 13–14 (Jacobi half-integer) | Slater identity, jacprodmake/jac2series | Medium | maple-checklist |
| THETA-06 provemodfuncid | Gamma_1 identity proofs | High | REQUIREMENTS v6.0 |
| RRID-01..03 | Rogers-Ramanujan identity search | Medium–High | REQUIREMENTS v6.0 |
| jac2series(f,T) user API | Verification of Jacobi products | Trivial | FEATURE-GAPS |

---

## Differentiators

Valuable for advanced users and research workflows.

| Feature | Value Proposition | Complexity | Source |
|---------|-------------------|------------|--------|
| factor(t8) cyclotomic | Block 4; polynomial factor into Φ_n | High | maple-checklist, v9.0 |
| EISENqmake / Eisenstein memoization | Speed for repeated modform basis calls | Low | gaps/wprogmodforms.txt |
| makeEISENbasisPX | Alternative P*X basis using Eisenstein | Medium | gaps/wprogmodforms.txt |
| BAILEY-01..03 | Bailey pair / chain manipulation | Medium | gaps/BAILEY.txt, REQUIREMENTS |
| ETA-01..08 | Full Gamma_0 cusp prover | High | REQUIREMENTS v6.0 |
| CRANK-01..05 | SPT-crank, overpartition crank tables | High | REQUIREMENTS v6.0 |
| findlincombomodp, findhommodp | Mod-p relation finding | Medium | v4.3, v10.0 |

---

## Anti-Features

| Anti-Feature | Why Avoid | Alternative |
|--------------|-----------|-------------|
| General RootOf(poly) | Arbitrary algebraic extensions; scope creep | Restrict to ω = RootOf(z²+z+1=0) |
| Full polynomial factorization | Berlekamp–Zassenhaus; Block 4 needs cyclotomics only | Cyclotomic-only factor |
| collect() builtin | Maple formatting-only; no computational need | Keep relation output as-is |
| qetamake variant | etamake covers main use | Use etamake |

---

## Feature Dependencies

```
RootOf(ω) / Omega3
    └── EX-04c (b(q) double-sum)
    └── Block 10

q-shift alignment in series addition
    └── Block 25 (findpoly theta quotients)
    └── EX-10

Jacobi half-integer exponents (JAC^(n/2))
    └── Block 13 (jacprodmake Slater)
    └── Block 14 (jac2series)

factor(t8) cyclotomic
    └── Block 4
    └── qfactor → cyclotomic expansion or Φ_n extraction

THETA-06 provemodfuncid
    └── cuspmake1, getacuspord, Gamma1ModFunc (THETA-01..05 done)
    └── Sturm bound computation

RRID-01..03
    └── RRG, RRH, RRGstar, RRHstar
    └── checkid, findids
```

---

## MVP Recommendation for v11.1

**Prioritize:**

1. **Block 25 fix** — q-shift alignment in series add (Phase 97); unblocks EX-10, findpoly parity  
2. **Block 10 / RootOf(ω)** — Omega3 type, b(q) double-sum (Phases 91–94); unblocks EX-04c  
3. **Block 13–14** — Jacobi half-integer exponents (Phases 64–65); completes Slater workflow  
4. **THETA-06** — provemodfuncid for Gamma_1 identity proofs  
5. **RRID-01** — RRG, RRH, RRGstar, RRHstar as built-ins  

**Defer:**

- Block 4 (factor cyclotomic) — v9.0; qfactor sufficient for most use  
- ETA-01..08 full cusp prover — high effort  
- CRANK-01..05 — niche; Bailey chains first  
- makeEISENbasisPX, symbolic bases — nice-to-have  

---

## Maple modforms Functions: Coverage Matrix

| Maple modforms | qseries3 | Status |
|----------------|----------|--------|
| makebasisM | makebasisM | ✓ |
| makeALTbasisM | makeALTbasisM | ✓ |
| makebasisPX | makebasisPX | ✓ |
| EISENq (misc) | EISENq, eisenstein | ✓ |
| DELTA12, P | DELTA12, 1/etaq(1) | ✓ |
| EISENqmake | — | Missing |
| makeEISENbasisPX | — | Missing |
| makeSYMbasisM | — | Missing |
| makeALTSYMbasisM | — | Missing |
| makeSYMbasisPX | — | Missing |
| makeSYMEISENbasisPX | — | Missing |
| briefmfhelp, mfhelp, mffunctions, modformschanges, modformspversion | help system | Partial (help exists, not modforms-specific) |

---

## Sources

- `.planning/REQUIREMENTS.md` — THETA-06, RRID, GAP-*, ETA-*, CRANK-*, BAILEY-*, MODP-*, BLOCK25, BLOCK4, BLOCK10
- `maple_checklist.md` — Block 1–41 status (FAIL, SKIP, N/A, Verified)
- `tests/maple-checklist.sh` — 41 blocks, exit allows ≤5 fail
- `tests/acceptance-exercises.sh` — EX-04a–e, EX-09a–c, EX-10a–b
- `gaps/wprogmodforms.txt` — Maple modforms package function list and help
- `gaps/BAILEY.txt` — Bailey package (alphafind, betafind, alphaup/down, betaup/down)
- `FEATURE-GAPS.md` — mprodmake, checkprod, checkmult, findmaxind, etc.
