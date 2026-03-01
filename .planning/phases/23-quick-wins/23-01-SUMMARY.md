# Plan 23-01: Quick wins — Summary

**Executed:** 2026-02-25  
**Status:** Complete

## What was built

1. **tests/acceptance-wins.sh** — Verification script for Phase 23 quick wins:
   - `--version` flag
   - `version` built-in
   - `qdegree(f)` and `lqdegree(f)`
   - `jac2series(var)` / `jac2series(var,T)` via Rogers-Ramanujan → jacprodmake → jac2series
   - `findlincombo(f,L,topshift)`
   - `make` with default CXX

2. **Makefile** — Added `acceptance-wins` target and `.PHONY` entry.

3. **MANUAL.md** — Documented:
   - `--version` in section 1 (Running)
   - `version` built-in
   - `qdegree`, `lqdegree` in section 3.3 (Coefficient Extraction)
   - `findlincombo` in section 3.4 (Relations)
   - `jac2series` in section 3.6 (Other)

## Pre-existing implementation (verified)

- `src/repl.h`: version, qdegree, lqdegree, jac2series, findlincombo built-ins; help table
- `src/main.cpp`: --version flag
- `Makefile`: CXX ?= g++ default

## Verification

Run from project root in Cygwin/Linux/macOS:

```bash
make acceptance-wins
```

Or directly:

```bash
./tests/acceptance-wins.sh
```
