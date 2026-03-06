---
phase: 86-rr-identity-search
plan: 01
subsystem: q-series
tags: [Rogers-Ramanujan, Göllnitz-Gordon, q-series, aqprod, jac2series, QP2]

requires:
  - phase: theta_ids
    provides: QP2 for Geta exponent
  - phase: convert
    provides: jac2series, prodmake

provides:
  - RRG, RRH, RRGstar, RRHstar, geta
  - REPL commands for Rogers-Ramanujan and Göllnitz-Gordon functions

tech-stack:
  added: rr_ids.h
  patterns: sum form for n=1, JAC form via geta for n>1

key-files:
  created: src/rr_ids.h, tests/acceptance-rr-id.sh
  modified: src/repl.h

duration: ~15min
completed: 2026-03-03
---

# Phase 86 Plan 01: RR Functions Summary

**RRG, RRH, RRGstar, RRHstar, geta in rr_ids.h with REPL dispatch; Rogers-Ramanujan and Göllnitz-Gordon functions**

## Accomplishments

- RRG1/RRH1 sum forms: Σ q^(n²)/(q;q)_n and Σ q^(n²+n)/(q;q)_n
- RRG/RRH with geta(1,5,n) and geta(2,5,n) for n>1
- RRGstar1/RRHstar1 Göllnitz-Gordon sums
- geta(g,d,n,T) = q^(n·QP2(g/d)·d/2)·JAC(ng,nd,∞)/JAC(0,nd,∞)
- REPL: RRG(n), RRH(n), RRGstar(n), RRHstar(n), geta(g,d,n,T)
- prodmake(RRG(1), 50) yields denominators at ±1 mod 5

## Task Commits

1. **feat(86-01)** - `a926af6`

## Deviations

None - plan executed as written.
