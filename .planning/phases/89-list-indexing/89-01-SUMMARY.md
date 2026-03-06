---
phase: 89-list-indexing
plan: 01
subsystem: repl
tags: [findhom, findnonhom, RelationKernelResult, subscript, indexing, maple-checklist]

# Dependency graph
requires:
  - phase: 88
    provides: prodid/seriesid (pre-existing REPL infra)
provides:
  - 1-based list indexing for findhom/findnonhom results via x[1], x[n]
  - RelationKernelResult in EnvValue and assignment handling
  - Subscript expression parsing and evaluation
affects: maple-checklist Block 21

# Tech tracking
tech-stack:
  added: []
  patterns: [subscript-on-variant, 1-based-indexing]

key-files:
  created: []
  modified: [src/repl.h, src/parser.h, tests/maple-checklist.sh]

key-decisions:
  - "1-based indexing for RelationKernelResult (Maple parity)"
  - "RelationKernelResult not serialized in saveSession (relations typically recomputed)"

patterns-established:
  - "Subscript uses left=base, right=index (same as BinOp)"

# Metrics
duration: ~15min
completed: 2026-03-03
---

# Phase 89 Plan 01: List Indexing Summary

**1-based list indexing for findhom/findnonhom results via EQNS[1], EQNS[n]; RelationKernelResult stored in variables and subscript returns single relation via formatRelation.**

## Performance

- **Duration:** ~15 min
- **Tasks:** 3
- **Files modified:** 3 (src/repl.h, src/parser.h, tests/maple-checklist.sh)

## Accomplishments

- RelationKernelResult can be assigned to variables (EQNS := findnonhom(...))
- Subscript syntax x[1], expr[n] parses and evaluates on RelationKernelResult
- Indexed result displays as single relation via formatRelation
- Out-of-range index throws "index out of range"
- Maple checklist Block 21 passes (real test instead of skip)

## Task Commits

Each task was committed atomically:

1. **Task 1: EnvValue and assignment** - `b56082c` (feat)
2. **Task 2: Parser subscript syntax** - `f9559e7` (feat)
3. **Task 3: Eval Subscript + Block 21** - `ac6a5be` (feat)

## Files Created/Modified

- `src/repl.h` - EnvValue variant, assignment, Var case, saveSession skip, Subscript eval
- `src/parser.h` - Expr::Tag::Subscript, makeSubscript, parseExpr LBRACK loop
- `tests/maple-checklist.sh` - Block 21 real test (EQNS[1], grep X)

## Decisions Made

- RelationKernelResult moved before EnvValue (struct definition order for variant)
- saveSession skips RelationKernelResult (no serialization; relations recomputed)
- 1-based indexing: EQNS[1] = first relation (Maple parity)

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

None. Build and tests pass.

## Next Phase Readiness

- GAP-IDX-01 satisfied: x[1] returns first element when x is findhom/findnonhom result
- Maple checklist Block 21 passes
- Ready for further list-indexing or Maple-parity features

## Self-Check: PASSED

---
*Phase: 89-list-indexing*
*Completed: 2026-03-03*
