# Phase 90: Symbolic z tripleprod/quinprod — Context

**Phase:** 90  
**Goal:** tripleprod and quinprod accept symbolic z and return bivariate series in z and q  
**Depends on:** Phase 89

## Requirements

| ID | Description |
|----|-------------|
| GAP-SYM-01 | tripleprod(z,q,T) with symbolic z returns bivariate series |
| GAP-SYM-02 | quinprod(z,q,T) with symbolic z returns bivariate series |

## Success Criteria (ROADMAP)

1. tripleprod(z,q,T) with symbolic z returns bivariate series (Laurent in z, power series in q)
2. quinprod(z,q,T) with symbolic z returns bivariate series
3. Display supports both z and q exponents
4. Maple checklist Blocks 28 and 32 pass

## Identity Formulas

**Triple product series form:** Σ (-1)^n z^n q^(n(n-1)/2)  
**Quintuple product series form:** Σ ((-z)^(-3n) - (-z)^(3n+1)) q^(n(3n+1)/2)

## Current State

- tripleprod(z,q,T), quinprod(z,q,T): require z and q as Series; ev(0) fails when z undefined
- Block 28: tripleprod(z,q,10) — skip (symbolic z)
- Block 32: quinprod(z,q,3) — skip (symbolic z)

## Design Options

1. **Symbolic z detection:** When args[0] is Var and not in env (or name "z"), use symbolic path
2. **Bivariate type:** map<pair<int,int>, Frac> for (z_exp, q_exp) or similar
3. **Direct series expansion:** Use identity formulas; iterate n, emit z^n q^m terms; no new Series type if we use a display-only representation
