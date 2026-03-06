# Requirements: qseries3 — v11.3 Code Health

**Defined:** 2026-03-06
**Core Value:** Rogers-Ramanujan must work; prodmake recovers product form with denominators at exponents ≡ ±1 (mod 5)
**Milestone:** v11.3 — Code health audit and plan

## v11.3 Requirements

Requirements for milestone v11.3. Each maps to roadmap phases.

### Warning Audit

- [ ] **HEALTH-01**: Zero compiler warnings — fix or annotate unused params in tcore.h, eta_cusp.h
- [ ] **HEALTH-02**: Add -Wshadow to build; fix any new shadow warnings (no global -Wno-\*)

### Build Hygiene

- [ ] **HEALTH-03**: CXXFLAGS aligned across Makefile, build.sh, .github/workflows/release.yml (same -Wall -Wextra -Wpedantic)
- [ ] **HEALTH-04**: make lint target runs cppcheck (--enable=warning,style,performance); no compile DB required

### Tech Debt Inventory

- [ ] **HEALTH-05**: Lightweight tech debt inventory — one-pass document: complexity hotspots, duplication, known brittle areas

## v11.3 Deferred (Future Milestone)

| Feature | Reason |
|---------|--------|
| make coverage (gcov/lcov) | Low ROI for single-TU; defer unless requested |
| make tidy (clang-tidy) | Bear/compile_commands on Cygwin needs validation |
| -Werror in CI | Escape hatch for compiler upgrades; optional |
| -Wconversion | Noisy in math code; revisit after HEALTH-01/02 |

## Out of Scope

| Feature | Reason |
|---------|--------|
| CMake / compile_commands.json | Keep Makefile; Bear optional for clang-tidy only |
| SonarQube / SaaS | Zero-dependency, local tooling only |
| Coverage gate 80%+ | Arbitrary for REPL; acceptance tests matter more |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| HEALTH-01 | Phase 112 | Pending |
| HEALTH-02 | Phase 112 | Pending |
| HEALTH-03 | Phase 113 | Pending |
| HEALTH-04 | Phase 114 | Pending |
| HEALTH-05 | Phase 115 | Pending |

**Coverage:**
- v11.3 requirements: 5 total
- Mapped to phases: 5/5
- Unmapped: 0

---
*Requirements defined: 2026-03-06*
*Last updated: 2026-03-06 after v11.3 research synthesis*
