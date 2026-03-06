# Phase 89: List Indexing — Context

**Phase:** 89  
**Goal:** User can index into list results from findhom/findnonhom  
**Depends on:** Phase 88

## Requirements

| ID | Description |
|----|-------------|
| GAP-IDX-01 | x[1] returns first element when x is findhom/findnonhom result |

## Success Criteria (ROADMAP)

1. x[1] returns first element when x is result of findhom or findnonhom
2. x[n] returns nth element (1-based); x[2], x[3] work
3. Out-of-range index throws clear error
4. Maple checklist Block 21 passes

## Current State

- findhom/findnonhom return RelationKernelResult { basis: vector<vector<Frac>>, monomialExponents }
- Assignment: EnvValue = Series | vector<JacFactor> | Partition | Phi1Result — RelationKernelResult NOT storable
- Parser: no subscript syntax x[1]; Expr has Var but no Subscript/Index
- Block 21: EQNS[1] — Maple indexes into findnonhom result

## Design

1. Add RelationKernelResult to EnvValue and assignment handling
2. Parser: add Subscript expression (base, index) — parse Var LBRACK expr RBRACK
3. Eval: subscript on RelationKernelResult → basis[i-1] (1-based), return RelationKernelResult with single row
4. Out-of-range: throw with clear message
