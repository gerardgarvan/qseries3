# Phase 23: Quick wins — Context

## Summary

Implement feature gaps from FEATURE-GAPS.md (quick wins): version, qdegree, lqdegree, jac2series, findlincombo, and Makefile CXX default.

## Decisions

- **Implementation already done** (2026-02-25): All five quick wins implemented in src/repl.h, src/main.cpp, Makefile. Plan 23-01 focuses on verification and acceptance tests.
- **version format**: "qseries 1.3"
- **qdegree/lqdegree**: Use Series::maxExp() and Series::minExp(); empty series → lqdegree returns 0
- **jac2series(var[,T])**: Takes variable holding jacprodmake result; optional T defaults to env.T
- **findlincombo(f,L,topshift)**: Wrapper over findhomcombo with n=1
- **Makefile**: CXX ?= g++; user can override with make CXX=x86_64-w64-mingw32-g++

## Deferred

- checkprod, checkmult, mprodmake, findmaxind — future phase
- Mod-p variants (findhommodp, etc.) — out of scope for v1.3
