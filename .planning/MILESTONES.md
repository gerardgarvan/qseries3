# Milestones

## v11.4 Tech Debt (Shipped: 2026-03-06)

**Phases completed:** 4 phases (116–119), 4 plans

**Key accomplishments:**
- Built-in registration — getBuiltinRegistry, dispatchBuiltin lookup; getHelpTable derives from registry
- Duplication reduction — expectArgCount, readLineFn TTY abstraction; etaq/prodmake/etamake migrated
- Brittle guards — q-guard in dispatchBuiltin; named constants (kPowExponentLimit, kMaxHistory, etc.)
- Fix Cygwin build — H macro SEE2/SEE3/SEE4/EX2; forward decls; tripleprod/quinprod/add/sum; L.push_back

**TD-01–TD-06:** All 6 requirements satisfied. Cygwin g++ 13 build succeeds; acceptance tests pass.

---

## v11.3 Code Health (Shipped: 2026-03-06)

**Phases completed:** 4 phases (112–115), 4 plans

**Key accomplishments:**
- Warning audit — zero warnings; -Wshadow; fix tcore.h, eta_cusp.h
- Build hygiene — CXXFLAGS aligned across Makefile, build.sh, release.yml
- make lint — cppcheck target; --enable=warning,style,performance
- TECH_DEBT.md — hotspots, duplication, brittle areas (HEALTH-05)

---

## v10.0 Close Remaining Gaps (Shipped: 2026-03-05)

**Phases completed:** 4 phases (97–100), 4 plans

**Key accomplishments:**
- Block 25 fix — Series::addAligned, operator+ q-shift alignment; findpoly on theta2/theta3 quotients passes
- Block 24 — Documented as N/A (collect is Maple formatting-only); explicit skip rationale
- findlincombomodp — solve_modp, findlincombomodp; REPL dispatch; acceptance-modp.sh
- Phase 100 — Added VERIFICATION.md to phases 97, 98, 99; audit blocker resolved

** maple-checklist:** 40 pass, 0 fail, 1 skip (Block 24)

---


## v11.0 Modular Forms (Shipped: 2026-03-06)

**Phases completed:** 2 phases (101–102), 2 plans

**Key accomplishments:**
- makeALTbasisM(k,T) — alternative Delta-based basis of M_k(SL_2(Z)) using Es·E6^(2r−2i)·DELTA12^i; REPL dispatch and help
- Cross-package integration — single proof chain (makeALTbasisM + provemodfuncGAMMA0id + etamake); PATH hardening; integration script in run-all.sh

---


## v11.1 Gap Closure & Improvements (Shipped: 2026-03-06)

**Phases completed:** 10 phases (60, 61, 67, 68, 100, 103–107), 10 plans

**Key accomplishments:**
- Block 25 fix (103) — verification-only; findpoly q-shift via addAligned; Block 25 + EX-10 pass
- Block 24 N/A rationale (104) — documented; GAP11-02 satisfied
- findlincombomodp (105) — verified; acceptance-modp.sh BIN resolution fixed
- provemodfuncid extensions (106) — BATCH + theta_aids; acceptance-theta-ids Test 9
- etamake scalar/format (60) — scalar factor, formatEtamake abs(ex); acceptance-etamake-format.sh
- q_shift / integer exponents (61) — theta2 q^(1/4); qdiff preserves q_shift
- Modular series arithmetic (67) — modp, nterms, findhommodp (verified pre-existing)
- Modular worksheet verification (68) — acceptance-worksheet-mod7.sh; 8 tests
- RR Identity Search (107) — findids types 4 and 5; type 4 discovers 16 identities

---

