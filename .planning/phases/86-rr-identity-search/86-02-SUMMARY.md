---
phase: 86-rr-identity-search
plan: 02
subsystem: q-series
tags: [checkid, CHECKRAMIDF, etamake, prodmake]

requires:
  - phase: 86-01
    provides: RRG, RRH

provides:
  - checkid(expr,T) — CHECKRAMIDF logic
  - CheckidResult struct
  - REPL checkid dispatch

tech-stack:
  added: CheckidResult, checkid
  patterns: normalize, prodmake, max|a_n|<acc, etamake

key-files:
  modified: src/rr_ids.h, src/repl.h, tests/acceptance-rr-id.sh

duration: ~5min
completed: 2026-03-03
---

# Phase 86 Plan 02: checkid Summary

**checkid(expr,T) identifies eta/theta products via CHECKRAMIDF**

## Accomplishments

- checkid(f,T,acc): normalize, prodmake, plx<acc, etamake
- checkid(RRG(1)*RRH(1), 50) → η(5τ)/(q^(1/6) η(τ))
- checkid(1+2*q, 50) → "not an eta product"
- REPL dispatch with formatEtamake display

## Task Commits

1. **feat(86-02)** - `6614304`

## Deviations

**1. [Rule 1]** formatEtamake used before definition
- **Fix:** Forward declaration in repl.h
- **Committed in:** 6614304
