# Phase 113: Build Hygiene — Research

**Researched:** 2026-03-06
**Domain:** CXXFLAGS alignment across build entry points
**Confidence:** HIGH

## Summary

Phase 113 aligns CXXFLAGS across Makefile, build.sh, and .github/workflows/release.yml. Phase 112 established the canonical flags: `-std=c++20 -O2 -Wall -Wextra -Wpedantic -Wshadow`.

## Current State

| File | CXXFLAGS | Missing |
|------|----------|---------|
| Makefile | -std=c++20 -O2 -Wall -Wextra -Wpedantic -Wshadow | none |
| build.sh | -std=c++20 -O2 -static -Wall -Wextra | -Wpedantic, -Wshadow |
| release.yml | -std=c++20 -O2 -Wall -Wextra ${{ matrix.static }} | -Wpedantic, -Wshadow |

## Locations

- **build.sh** lines 6, 8, 9: three g++ invocations (mingw, g++ with -static, g++ without -static)
- **release.yml** line 29: single g++ invocation in Build step

## Action

Add `-Wpedantic -Wshadow` to each g++ command in build.sh and release.yml. Order: `-Wall -Wextra -Wpedantic -Wshadow` to match Makefile.

## Pitfalls

- build.sh fallback (line 9) drops -static; ensure -Wpedantic -Wshadow added to both branches
- release.yml: add flags before ${{ matrix.static }} so they apply on both linux and macos
