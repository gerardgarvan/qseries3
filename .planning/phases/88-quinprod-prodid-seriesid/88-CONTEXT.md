# Phase 88: quinprod prodid/seriesid — Context

**Phase:** 88  
**Goal:** quinprod returns identity formulas when called with prodid or seriesid mode  
**Depends on:** Phase 87

## Requirements

| ID | Description |
|----|-------------|
| GAP-QP-01 | quinprod(z,q,prodid) — returns product form (string) |
| GAP-QP-02 | quinprod(z,q,seriesid) — returns series form (string) |

## Success Criteria (ROADMAP)

1. quinprod(z,q,prodid) returns the quintuple product identity in product form
2. quinprod(z,q,seriesid) returns the quintuple product identity in series form
3. Maple checklist Blocks 30 and 31 pass
4. help(quinprod) documents prodid and seriesid modes

## Identity Formulas (qseriesdoc 6.3)

**Product form:**
(-z;q)_∞ (-q/z;q)_∞ (z²q;q²)_∞ (q/z²;q²)_∞ (q;q)_∞

**Series form:**
Σ_{n=-∞}^{∞} ((-z)^(-3n) - (-z)^(3n+1)) q^(n(3n+1)/2)

## Current Implementation

- quinprod(z, q, T) in qfuncs.h — T is int, returns Series
- REPL: evi(2) for 3rd arg — expects integer
