# Research Summary: factor(t8) + Block 4

**Project:** q-series REPL — v9.0 factor(t8) + Close Block 4
**Researched:** 2026-03-03
**Confidence:** HIGH

## Executive Summary

Add `factor(expr)` that factors univariate polynomials in q into cyclotomic form for maple-checklist Block 4. Two viable approaches:

**Approach A (recommended):** Reuse qfactor → cyclotomic expansion. qfactor(t8) already yields q-product form. Expand each (1-q^n) = ∏_{d|n} Φ_d(q) to get cyclotomic exponents. Simpler, reuses existing code.

**Approach B:** Direct cyclotomic extraction. Compute Φ_n via Möbius product, divide polynomial by Φ_n repeatedly, record multiplicities. More general; works when qfactor doesn't apply.

**For Block 4:** T(8,8) is factored by qfactor; Approach A suffices. Use Series as polynomial (map<int,Frac>); output FactorResult with cyclotomic exponents and formatted string (Φ_n notation).

## Key Findings

- **Representation:** Series = polynomial when minExp≥0, q_shift=0. No new Poly type.
- **Output:** Struct `FactorResult { map<int,int> cyclotomic; Frac content; }` plus display string.
- **Scope:** Cyclotomic factorization for Block 4. General irreducibles (Berlekamp–Zassenhaus) deferred.
- **Integration:** New polyfactor.h or extend convert.h; dispatch factor(expr) in REPL.

## Watch Out For

- Coefficient growth in polynomial division (use Frac reduction)
- qfactor vs factor: different output forms (q-product vs cyclotomic)
- Content and primitive part before expansion
