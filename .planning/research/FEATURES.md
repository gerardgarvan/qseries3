# Feature Landscape: Code Health Audit & Remediation

**Domain:** Code health assessment and remediation for existing C++20 REPL  
**Researched:** 2026-03-06  
**Project context:** qseries3 — zero-dependency C++20 q-series REPL; acceptance tests (acceptance-*.sh, maple-checklist), Makefile, g++ -Wall -Wextra -Wpedantic. Known warnings in tcore.h, eta_cusp.h (unused params).

---

## How Code Health Assessments Typically Work

Code health assessment follows a four-pillar model:

1. **Warning audit** — Compiler flags, zero-warnings policy, selective promotion to errors  
2. **Test coverage** — Line/function/branch coverage, automation, thresholds  
3. **Tech debt identification** — Complexity metrics, duplication, hotspots, prioritization  
4. **Build hygiene** — Reproducibility, incremental correctness, dependency tracking  

Assessments run at three stages: IDE during development, PR before merge, CI/CD during builds. Focus on *new code* first (clean-as-you-code) to avoid overwhelming legacy remediation.

---

## Table Stakes

Features users and maintainers expect. Missing = project feels unmaintainable.

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| Zero compiler warnings | Professional C++ projects use -Wall -Wextra; warnings hide real issues | Low | Project already uses flags; need to fix tcore.h, eta_cusp.h |
| Acceptance tests pass | Regression baseline; SPEC/ROADMAP acceptance gates | Low | acceptance.sh, maple-checklist, run-all.sh already exist |
| Single-command build | `make` produces working binary; no manual steps | Low | Makefile present; already table-stakes met |
| Basic test harness | Tests run via Make targets; deterministic pass/fail | Low | `make acceptance` etc. present |
| No silent regressions | Changes don't break existing behavior without being caught | Low | Depends on acceptance tests; table stakes if tests cover critical paths |

---

## Differentiators

Features that set a well-maintained codebase apart. Not required, but valued.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| Warning audit report | Systematic inventory of all warnings; per-file remediation plan | Low | Document current warnings; fix or justify each |
| -Werror in CI | New warnings block merge; prevents regression | Low | Add -Werror to release.yml / build.sh for CI only |
| Test coverage metrics | Quantifies gaps; informs where to add tests | Medium | Needs gcov/lcov; single-file main.cpp complicates granularity |
| Tech debt inventory | Prioritized list: complexity, duplication, hotspots | Medium | Manual or tool-assisted (SonarQube, clang-tidy); must map to phase work |
| Build hygiene audit | Reproducible builds; correct incremental deps | Low | Single main.cpp → few deps; still valuable for wasm, bench targets |
| Sanitizers in debug build | Catches UB, leaks early | Low | Makefile already has `debug` target with -fsanitize=address,undefined |
| Static analysis (clang-tidy) | Finds bugs and style issues beyond compiler | Medium | No integration yet; would need config and CI |

---

## Anti-Features

Features to explicitly NOT build.

| Anti-Feature | Why Avoid | What to Do Instead |
|--------------|-----------|--------------------|
| SonarQube or heavy SaaS | Overkill for single-developer, zero-dependency REPL; adds infra | Use GCC warnings + clang-tidy locally if needed |
| Coverage gate at 80%+ | Arbitrary; project is REPL with acceptance tests, not unit-tested library | Track coverage trends; fix critical gaps only |
| Widespread -Wno-* suppressions | Hides real issues; creates maintenance debt | Fix or annotate (e.g. `[[maybe_unused]]`) explicitly |
| New build system (CMake/Bazel) | Project is small; Makefile works; migration is tech debt | Improve existing Makefile hygiene only |
| Duplicate test frameworks | Project uses bash acceptance tests; adding GTest changes paradigm | Extend acceptance tests; optional bench_main.cpp stays |

---

## Expected Behaviors by Pillar

### 1. Warning Audit

| Behavior | Expected | qseries3 Status |
|----------|----------|-----------------|
| Use -Wall -Wextra | Baseline for any C++ project | ✅ Already in CXXFLAGS |
| Use -Wpedantic | Standards compliance | ✅ Already in CXXFLAGS |
| Zero warnings on clean build | Professional baseline | ❌ tcore.h, eta_cusp.h have unused-parameter |
| Document or fix each warning | No silent suppressions | ❌ Not done |
| Optional: -Werror in CI | Block new warnings | ❌ release.yml uses -Wall -Wextra only |
| Unused-parameter handling | Fix, use `[[maybe_unused]]`, or (void)param | Partial (repl.h uses [[maybe_unused]]) |

**Remediation actions:** Fix or annotate unused params in tcore.h, eta_cusp.h; optionally add -Werror for CI.

### 2. Test Coverage

| Behavior | Expected | qseries3 Status |
|----------|----------|-----------------|
| Acceptance tests cover critical paths | Rogers-Ramanujan, SPEC tests | ✅ acceptance.sh, maple-checklist |
| Run tests in CI | Automated regression | ✅ run-all.sh, GitHub Actions |
| Coverage measurement | Optional; gcov + lcov | ❌ Not set up |
| Coverage target | 80%+ for production libs; not strict for REPL | N/A — REPL; acceptance coverage matters more |
| Branch/edge coverage | Tests both success and failure paths | Partial — acceptance tests focus on success |

**Remediation actions:** Ensure acceptance-all / run-all exercise all critical commands; add coverage target only if ROI justified (single main.cpp complicates line attribution).

### 3. Tech Debt Identification

| Behavior | Expected | qseries3 Status |
|----------|----------|-----------------|
| Identify high-complexity files | repl.h, parser.h likely | Manual; no metrics |
| Duplication scan | Copy-paste across headers | Manual |
| Hotspot analysis | Most-changed, most-tested | Can derive from git + test mapping |
| Prioritized backlog | Link debt to phases; fix in context | Phase plans exist; no explicit debt backlog |
| Focus on new-code quality | Don't boil the ocean | Align with clean-as-you-code |

**Remediation actions:** Create lightweight debt inventory (complexity, duplication, known hotspots); attach to phase work rather than separate “debt sprint.”

### 4. Build Hygiene

| Behavior | Expected | qseries3 Status |
|----------|----------|-----------------|
| Single-command build | `make` | ✅ |
| Clean rebuild works | `make clean && make` | ✅ |
| Incremental build correctness | Rebuild when headers change | ⚠️ main.cpp includes all; single TU → full rebuild on any change |
| Reproducible build | Same source → same binary (modulo timestamp) | Not verified |
| Multiple targets (debug, wasm, bench) | Separate builds for different purposes | ✅ debug, wasm, bench targets exist |
| Consistent CXXFLAGS | Same flags for main, wasm, CI | ⚠️ release.yml omits -Wpedantic; build.sh omits -Wpedantic |

**Remediation actions:** Align CXXFLAGS across Makefile, build.sh, release.yml; document reproducibility expectations (timestamp stripping if needed).

---

## Feature Dependencies

```
Zero compiler warnings → Warning audit report (warnings must be enumerated first)
Acceptance tests pass → Test coverage metrics (coverage measures what tests hit)
Makefile / build scripts → Build hygiene audit (audit existing build)
Tech debt inventory → Phase planning (debt items feed phase tasks)
```

---

## Dependencies on Existing qseries3

| Feature | Depends On | Status |
|---------|------------|--------|
| Warning audit | Makefile CXXFLAGS, g++ | CXXFLAGS already strict; fix remaining warnings |
| Test coverage | acceptance-*.sh, run-all.sh, maple-checklist | Tests exist; coverage tooling optional |
| Tech debt ID | src/*.h, repl.h, parser.h | Codebase present; needs systematic scan |
| Build hygiene | Makefile, build.sh, .github/workflows/release.yml | Build system present; align flags |

---

## MVP Recommendation for Code Health Phase

Prioritize:

1. **Warning audit + remediation** — Fix tcore.h, eta_cusp.h; document any remaining; optionally -Werror in CI  
2. **CXXFLAGS alignment** — Makefile, build.sh, release.yml use same -Wall -Wextra -Wpedantic  
3. **Lightweight tech debt inventory** — One-pass: complexity hotspots, duplication, known brittle areas  

Defer:

- **Coverage tooling** — ROI low for single-TU REPL; acceptance tests are primary gate  
- **clang-tidy / SonarQube** — Add only if warning cleanup doesn’t suffice  

---

## Complexity Summary

| Feature | Complexity | Reason |
|---------|------------|--------|
| Warning audit + fix | Low | Small set of known warnings; [[maybe_unused]] or removal |
| CXXFLAGS alignment | Low | Edit 3 files |
| Tech debt inventory | Medium | Requires manual or tool-assisted scan; interpretation |
| Coverage setup | Medium | gcov/lcov + CI; single TU dilutes value |
| Static analysis integration | Medium | clang-tidy config, CI integration |

---

## Sources

- GCC Warning Options: https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html (official)
- SonarQube Clean-as-you-code: https://docs.sonarsource.com/sonarqube-server/2025.1/user-guide/clean-as-you-code/introduction/
- C++ coverage: gcov, lcov; SonarQube C/C++ coverage docs
- Tech debt: CodeScene, Sonar technical debt ratio; CodeAnt tracking metrics
- Build hygiene: Reproducible Builds, content-hash Make patterns
