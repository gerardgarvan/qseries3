# Phase 86: RR Identity Search — Context

**Phase:** 86  
**Goal:** Rogers-Ramanujan/Göllnitz-Gordon functions and systematic identity search  
**Depends on:** Phases 78 (newprodmake), 81 (ETA)

## Requirements

| ID | Description |
|----|-------------|
| RRID-01 | RRG(n), RRH(n), RRGstar(n), RRHstar(n) |
| RRID-02 | checkid(expr, T) — check if expression is eta/theta product |
| RRID-03 | findids(type, T) — systematic identity search (types 1–10) |

## Success Criteria (ROADMAP)

1. RRG(1) and RRH(1) match standard Rogers-Ramanujan products  
2. checkid(RRG(1)*RRH(1), 50) identifies as eta-quotient  
3. findids(1, 20) discovers ≥3 type-1 identities  
4. findids(2, 20) discovers ≥2 type-2 identities  

## Wave Structure

| Wave | Plan | Objective |
|------|------|-----------|
| 1 | 86-01 | RRG, RRH, RRGstar, RRHstar, geta, REPL |
| 2 | 86-02 | checkid, REPL |
| 3 | 86-03 | findids(1,2), GE/HE, ABCOND, REPL |

## Key Files

- **86-RESEARCH.md** — Maple→C++ mapping, Geta, CHECKRAMIDF, findtype1/2
- **86-01-PLAN.md** — RR functions and geta
- **86-02-PLAN.md** — checkid
- **86-03-PLAN.md** — findids types 1 and 2
