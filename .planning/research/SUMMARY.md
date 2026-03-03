# Project Research Summary

**Project:** q-series REPL — RootOf / Q(ω) (Milestone v8.0)
**Domain:** Exact q-series arithmetic with algebraic number support
**Researched:** 2026-03-03
**Confidence:** HIGH

## Executive Summary

RootOf and Q(ω) support are needed for Maple Block 10 parity and Exercise 4’s b(q) = Σ ω^(n-m) q^(n²+nm+m²). Experts build this by using a cyclotomic-specific representation (a,b) for a+bω with ω² = -ω-1 — no polynomial ring, no external libraries. BigInt and Frac are reused; Omega3 is added as a pair of Frac with custom arithmetic; SeriesOmega parallels Series for coefficients in Q(ω).

**Recommendation:** Use Omega3 (a+bω) with ω² = -ω-1 reduction. No GMP, no Boost, no general RootOf. Parser/REPL add `omega` and RootOf(3); sum(omega^expr) evaluates ω^k → Omega3. prodmake/etamake stay Q-only; for b(q) product form, use the eta identity b(q) = η(τ)³/η(3τ).

**Risks:** Coefficient growth in Q(ω) without normalization; normal-form ambiguity (ω vs ω²); findhom assumes Q. Mitigations: normalize after every Q(ω) op; document canonical root choice; keep findhom Q-only initially; use eta identity for b(q) in etamake.

---

## Key Findings

### Recommended Stack

Cyclotomic-specific representation: `struct Omega3 { Frac a, b; }` for a + b·ω. Multiplication uses (a+bω)(c+dω) = (ac−bd) + (ad+bc−bd)ω; division via conjugate: (a+bω)⁻¹ = (a+bω²)/N where N = a²−ab+b². No polynomial machinery, no external deps.

**Core technologies:**
- **Omega3** — scalar type for Q(ω) — ω²+ω+1=0, reduces via ω² = -ω-1
- **SeriesOmega** — `std::map<int, Omega3>` — parallel to Series; no templating
- **Parser** — `omega` literal or `RootOf(z^2+z+1=0)` / `RootOf(3)` — Maple parity
- **No GMP, Boost, NTL** — BigInt + Frac + Omega3 only

### Expected Features

**Must have (table stakes):**
- `omega` / RootOf(z²+z+1=0) — symbol for ζ₃
- Algnum/Omega3 — Q(ω) arithmetic (add, mul, pow with ω^k → ω^(k mod 3))
- Series with Q(ω) coefficients — SeriesOmega
- `sum(omega^expr, ...)` — evaluate ω^exponent → Omega3
- Display `a + b*omega` — user visibility

**Should have (differentiators):**
- convert(omega, radical) — optional display
- Indexed RootOf — ω vs ω² selection

**Defer (v2+):**
- General RootOf(poly)
- Cyclotomic ζₙ for n > 3
- findhom over Q(ω); prodmake/etamake on Q(ω) series

### Architecture Approach

Add Omega3 (or algebraic.h) for cyclotomic Q(ω); add SeriesOmega with same layout as Series; treat RootOf as builtin call; extend EnvValue and EvalResult. Build order: **algebraic type** → **SeriesOmega** → **parser/REPL wiring**.

**Major components:**
1. **omega3.h** (or algebraic.h) — Omega3 struct, arithmetic, str(), fromRational(Frac), conjugate
2. **series_omega.h** — SeriesOmega (map<int, Omega3>), mirror Series arithmetic
3. **repl.h** — RootOf dispatch, EnvValue+Algebraic, evalStmt/Var, BinOp (Algebraic * Series → SeriesOmega), display

### Critical Pitfalls

1. **Coefficient growth** — Q(ω) has no built-in reduction; normalize after every op; gcd-reduce numerators
2. **Normal form (ω vs ω²)** — Document canonical choice; never equate ω and ω²
3. **Q(ω) division** — Implement (a+bω)⁻¹ = conjugate / norm; ensure nonzero check
4. **findhom stays Q-only** — No kernel over Q(ω) in v1; restrict to rational series
5. **b(q) product** — Use eta identity b(q)=η³/η(3τ); do not pass algebraic series to etamake

---

## Implications for Roadmap

Based on research, suggested phase structure:

### Phase 1: Omega3 Type (omega3.h)
**Rationale:** Foundation; no Series/REPL deps; arithmetic must be correct before Series.
**Delivers:** Omega3 struct, +/−/*, inverse, str(), fromRational, conjugate; normalization after ops.
**Addresses:** omega representation, Q(ω) arithmetic.
**Avoids:** Coefficient growth; wrong root choice.

### Phase 2: SeriesOmega
**Rationale:** Depends on Omega3; needed before any omega-involving series.
**Delivers:** SeriesOmega (map<int, Omega3>), add, mul, truncTo; Algebraic * Series → SeriesOmega.
**Uses:** Omega3 from Phase 1.
**Avoids:** Series inverse with wrong Q(ω) division; mixed Q/Q(ω) confusion.

### Phase 3: Parser/REPL Integration
**Rationale:** Depends on Omega3 and SeriesOmega; wires omega into user-facing flow.
**Delivers:** RootOf(3) dispatch; omega in env; Var lookup; BinOp handling; sum(omega^expr); display.
**Implements:** parser/REPL wiring, omega binding.
**Avoids:** Type inconsistency; findhom on algebraic (keep Q-only).

### Phase 4: b(q) and Block 10
**Rationale:** End-to-end validation; uses eta identity for prodmake path.
**Delivers:** b(q) via sum or eta identity; a(q), c(q); Block 10 parity.
**Avoids:** etamake on algebraic series — use b(q)=η³/η(3τ) workaround.

### Phase Ordering Rationale

- Omega3 first: all other phases depend on it; normalization prevents growth.
- SeriesOmega before REPL: REPL needs SeriesOmega for omega * q and sum(omega^expr).
- findhom stays Q-only: defer Q(ω) kernel; Block 10 does not require findhom on a,b,c.

### Research Flags

Phases likely needing deeper research during planning:
- **Phase 4:** b(q) double-sum performance vs eta identity; acceptance tests.

Phases with standard patterns (skip research-phase):
- **Phase 1:** Well-documented cyclotomic arithmetic; Regina/SymPy references.
- **Phase 2:** Mirrors existing Series; straightforward.
- **Phase 3:** Builtin dispatch and EnvValue extension; established patterns.

---

## Confidence Assessment

| Area      | Confidence | Notes                                            |
|-----------|------------|--------------------------------------------------|
| Stack     | HIGH       | Maple, PARI, GAP, Regina sources; formula verified |
| Features  | HIGH       | Maple Block 10, qseriesdoc, PROJECT.md alignment |
| Architecture | HIGH    | Matches existing Series/Frac patterns; clear deps |
| Pitfalls  | HIGH       | Verified against relations.h, convert.h, linalg  |

**Overall confidence:** HIGH

### Gaps to Address

- Maple index convention for z²+z+1 roots — verify during implementation.
- Performance threshold for coefficient growth (T≈100+) — validate in Phase 1 tests.

---

## Sources

### Primary (HIGH confidence)
- Maple RootOf — maplesoft.com/support/help/Maple/view.aspx?path=RootOf
- Regina Cyclotomic — regina-normal.github.io/engine-docs/cyclotomic_8h.html
- GAP3 Cyclotomics — Zumbroich basis, conductor
- qseriesdoc.md, maple_checklist Block 10, .planning/STATE.md

### Secondary (MEDIUM confidence)
- SymPy numberfields — AlgebraicNumber representation
- PARI/GP t_POLMOD — mod T representation
- src/relations.h, src/linalg.h, src/convert.h — codebase verification

### Tertiary (LOW confidence)
- Coefficient growth thresholds — infer from tests; document if observed

---
*Research completed: 2026-03-03*
*Ready for roadmap: yes*
