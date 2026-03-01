---
phase: 10-repl
plan: 02
subsystem: repl
tags: [repl, qfuncs, convert, q-series, cpp, dispatch]

# Dependency graph
requires:
  - phase: 10-01
    provides: Env, EvalResult, evalToInt, eval, Sum with sumIndices, display helpers
  - phase: 06-convert-extended
    provides: prodmake, etamake, jacprodmake, jac2prod, qfactor, T_rn
provides:
  - dispatchBuiltin for all qfuncs and convert built-ins
  - q-injection (etaq(1,50), theta3(100) etc. get q from env)
  - Rogers-Ramanujan, partition, theta/eta, Jacobi RR, qfactor, sift via REPL
affects: [10-03 relations]

# Tech tracking
tech-stack:
  added: []
  patterns: [q-injection for 1-arg-less calls, evalAsSeries/evalToInt arg dispatch]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "q-injection: when C++ takes (q,...) and user provides 1 fewer arg, prepend env['q']"
  - "jac2prod(var) looks up env by variable name; throws if not vector<JacFactor>"

patterns-established:
  - "dispatchBuiltin(callName, args, env, sumIndices) returns EvalResult; throw unknown built-in"

# Metrics
duration: ~15min
completed: 2026-02-25
---

# Phase 10 Plan 02: Built-in Dispatch Summary

**Complete built-in dispatch for qfuncs (aqprod, etaq, theta2/3/4, qbin, sift, T) and convert (prodmake, etamake, jacprodmake, jac2prod, qfactor), with q-injection and sum/add Rogers-Ramanujan verification**

## Performance

- **Duration:** ~15 min
- **Tasks:** 3
- **Files modified:** 1 (src/repl.h)

## Accomplishments

- dispatchBuiltin with q-injection: etaq(1,50)→etaq(q,1,50), theta3(100)→theta3(q,100), qbin(m,n,T)→qbin(q,m,n,T)
- qfuncs dispatch: aqprod, etaq, theta2/3/4, theta, qbin, tripleprod, quinprod, winquist, sift, T(r,n)
- convert dispatch: prodmake, etamake, jacprodmake, jac2prod(var), qfactor, series, coeffs, set_trunc
- Rogers-Ramanujan verified: sum(q^(n^2)/aqprod(q,q,n,50),n,0,8); prodmake yields denominators only at n≡±1 (mod 5)

## Task Commits

1. **Task 1+2: q-injection and qfuncs+convert built-in dispatch** - `878b71e` (feat)

Task 3 (sum/add with sumIndices) required no code change — the Sum implementation from 10-01 was already correct. Verification confirmed Rogers-Ramanujan and jacprodmake/jac2prod workflow.

## Files Created/Modified

- `src/repl.h` - Added dispatchBuiltin, evalAsSeries; Call now delegates to dispatchBuiltin; all qfuncs and convert built-ins implemented

## Decisions Made

- q-injection applied when user arg count is 1 less than C++ API (q as first param)
- jac2prod takes variable name; looks up env; throws if not jacprodmake result
- series/coeffs print internally and return DisplayOnly; set_trunc returns monostate

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- Git global config adds --trailer; used HOME=/tmp/gittest for commit
- Build/run via Cygwin bash (g++ in PATH)

## Next Phase Readiness

- Rogers-Ramanujan (Test 1), Partition (Test 2), Theta/eta (Test 3), Jacobi RR (Test 4), Qfactor (Test 7), Sifting (Test 6), Euler (Test 9) runnable via REPL
- Ready for 10-03: findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly (relations)

## Self-Check

- [x] etaq(1,50), theta3(100), aqprod(q,q,5,50) return correct Series
- [x] rr:=sum(q^(n^2)/aqprod(q,q,n,50),n,0,8); prodmake(rr,40) shows product with denominators at ±1 mod 5
- [x] jacprodmake(rr,40), jp:=jacprodmake(rr,40); jac2prod(jp) work
- [x] p:=1/etaq(1,50); series(p,20) shows partition coeffs 1,1,2,3,5,7,11,15,22,30,42
- [x] Build succeeds; --test passes

---
*Phase: 10-repl*
*Completed: 2026-02-25*
