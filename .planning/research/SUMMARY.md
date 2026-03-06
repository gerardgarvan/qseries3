# Project Research Summary

**Project:** Q-Series REPL (qseries3)
**Domain:** C++20 single-binary REPL; code health audit and remediation
**Researched:** 2026-03-06
**Confidence:** HIGH

## Executive Summary

qseries3 is a zero-dependency C++20 REPL for q-series arithmetic and modular forms. Code health improvement should add **compiler flags** (`-Wshadow`), **static analysis** (cppcheck primary; clang-tidy optional), **gcov/lcov** for coverage, and **Makefile targets** for lint, coverage, and sanitizer runs—all without new runtime deps. The recommended approach is incremental: fix warnings before suppressing, align CXXFLAGS across Makefile/CI, add cppcheck first (no compile DB), then coverage/sanitize targets. Key risks: **suppression creep** (pragmas/NOLINT hiding bugs), **scope creep** (“fix all” becoming a rewrite), and **wrong coverage scope** (headers vs single-TU). Mitigation: fix-before-suppress policy, explicit phase scope and time-boxing, and defined analysis boundaries (main.cpp root; `-header-filter` for project sources only).

---

## Key Findings

### Recommended Stack (from STACK.md)

**Compiler flags:**
- Add `-Wshadow` — catches variable shadowing; not in -Wall; low noise.
- Keep `-Wall -Wextra -Wpedantic`.
- Defer `-Wconversion` — noisy in math code; add later if feasible.
- Align `.github/workflows/release.yml` with Makefile (add `-Wpedantic`).

**Static analysis:**
- **cppcheck 2.x** — primary; `--enable=warning,style,performance`; standalone, no compile DB.
- **clang-tidy 18+** — optional; via Bear for `compile_commands.json`; Option B: `clang-tidy src/main.cpp -- -std=c++20 -Isrc` if Bear unavailable.

**Coverage:**
- **gcov** + **lcov 2.x** — `-fprofile-arcs -ftest-coverage -g -O0`; single-TU yields one .gcno/.gcda pair.

**Sanitizers:**
- Keep `-fsanitize=address,undefined`; optional: add `-fsanitize=leak`. Do not combine ASan with MSan/TSan.

**Versions:**
- GCC 13.3+; cppcheck 2.12+ (C++20); clang-tidy 18+; lcov 2.0+; Bear 3.0+.

### Expected Features (from FEATURES.md)

**Table stakes (must have):**
- Zero compiler warnings — fix tcore.h, eta_cusp.h; professional baseline
- Acceptance tests pass — regression baseline; SPEC/ROADMAP gates
- Single-command build — `make` produces working binary
- Basic test harness — deterministic pass/fail via Make targets
- No silent regressions — changes caught by acceptance tests

**Differentiators (should have):**
- Warning audit report — systematic inventory; per-file remediation plan
- `-Werror` in CI — block new warnings
- Test coverage metrics — gcov/lcov; quantify gaps
- Tech debt inventory — complexity, duplication, hotspots; map to phases
- Build hygiene audit — reproducible builds, correct incremental deps
- Sanitizers in debug — already present; keep
- Static analysis (clang-tidy) — beyond compiler; optional integration

**Defer (v2+):**
- Coverage gate at 80%+ — arbitrary for REPL; acceptance coverage matters more
- SonarQube or heavy SaaS
- Coverage tooling if ROI low for single-TU

### Architecture Approach (from ARCHITECTURE.md)

**Code health integration:**
- **Integration points:** Makefile targets (lint, tidy, sanitize, check), CI (optional lint job), no change to test script logic.
- **New targets:** `make lint` (cppcheck), `make tidy` (clang-tidy), `make sanitize` (debug + acceptance), `make check` (umbrella), `make coverage` (gcov build + lcov report).

**Build order:**
1. cppcheck — easiest; no Bear
2. .clang-tidy + make tidy — Option B if no Bear
3. make sanitize — reuse `debug` target + acceptance
4. make check — aggregate
5. CI lint job — optional; runner must have tools

**Invariant:** Static analysis runs on source; acceptance tests need binary. Order: lint/tidy (optional, no build) → build → acceptance.

**REPL/data-flow (reference):** Single catch in runRepl; parse/eval/dispatch throw; getHelpTable shared by help + tab completion.

### Critical Pitfalls (from PITFALLS.md)

1. **Pragmas without push/pop** — Suppression leaks to rest of file/headers. Always use balanced `#pragma GCC diagnostic push` / `pop`. Prefer fix over pragma. Never bare `ignored` in headers.

2. **Suppression creep (NOLINT / -Wno-\*)** — Hides bugs; no governance. Prefer fix. If NOLINT: require justification comment. Cap NOLINTs per file. Avoid global `-Wno-unused-parameter`.

3. **Coverage scope wrong** — Too narrow: only .cpp (misses headers). Too broad: third-party noise. For qseries3: analyze main.cpp (headers pulled in). Set `-header-filter='.*/src/.*'`; document scope up front.

4. **Plan scope creep** — “Fix all” becomes rewrite. Incremental adoption: enable 2–5 high-value checks first. Define baseline; fail only on *new* findings. Phase scope: which checks, which files, time-box.

5. **GCC vs Clang flag incompatibility** — clang-tidy with GCC compile_commands.json can choke. Filter GCC-only flags or use Option B (no Bear).

6. **-Werror blocking on compiler upgrade** — Pin compiler in CI; have `-Wno-error=<new-warning>` escape hatch.

---

## Implications for Roadmap

Based on research, suggested phase structure for code health (v11.3):

### Phase 1: Warning audit + remediation
**Rationale:** Foundation; errors/warnings cross-cutting; helps debug later changes.
**Delivers:** Zero warnings; fix tcore.h, eta_cusp.h; document any remaining.
**Addresses:** Zero compiler warnings (table stakes), warning audit report (differentiator).
**Avoids:** Suppression creep — use omit name, `[[maybe_unused]]`, or `(void)x`; no pragmas without push/pop.

### Phase 2: CXXFLAGS alignment + cppcheck
**Rationale:** Consistency across build surfaces; cppcheck is easiest static analysis.
**Delivers:** Same -Wall -Wextra -Wpedantic in Makefile, build.sh, release.yml; `make lint` target.
**Uses:** cppcheck, -Wshadow.
**Avoids:** Scope creep — only align flags and add one tool; no “fix all cppcheck findings.”

### Phase 3: Coverage target (optional)
**Rationale:** Quantify gaps; low ROI for single-TU but provides baseline.
**Delivers:** `make coverage`, `make coverage-report`; gcov/lcov workflow.
**Uses:** gcov, lcov.
**Avoids:** Coverage scope wrong — analyze main.cpp; headers included; exclude system/third-party.

### Phase 4: clang-tidy + sanitize targets (optional)
**Rationale:** Deeper analysis; sanitizer validation in CI.
**Delivers:** `make tidy`, `make sanitize`; optional `.clang-tidy`.
**Avoids:** GCC/Clang flag mismatch — use Option B if Bear unavailable; compile DB filtering if using Bear.

### Phase 5: Tech debt inventory (lightweight)
**Rationale:** Informs future phases; attach to phase work, not separate “debt sprint.”
**Delivers:** One-pass: complexity hotspots, duplication, known brittle areas.
**Avoids:** Scope creep — inventory only; fixes in context of feature phases.

### Phase ordering rationale

- Warning cleanup first — establishes baseline; other tools report against clean build.
- CXXFLAGS + cppcheck second — minimal setup; immediate value.
- Coverage/sanitize next — reuses existing tests and debug target.
- clang-tidy optional — higher integration cost.
- Debt inventory last — feeds planning; no build changes.

### Research flags

**Phases needing deeper research during planning:**
- **Phase 3 (coverage):** single-TU line attribution; acceptance vs unit coverage trade-off.
- **Phase 4 (clang-tidy):** Bear availability on Cygwin/Windows; check selection for incremental adoption.

**Phases with standard patterns (skip research-phase):**
- **Phase 1 (warnings):** Well-documented; project already uses `[[maybe_unused]]`.
- **Phase 2 (CXXFLAGS, cppcheck):** Straightforward; cppcheck standalone.

---

## Confidence Assessment

| Area | Confidence | Notes |
|------|------------|-------|
| Stack | HIGH | GCC/LLVM docs; cppcheck, gcov/lcov standard; sanitizers built-in |
| Features | HIGH | Clean-as-you-code consensus; FEATURES aligned with codebase |
| Architecture | HIGH | Existing Makefile, CI, tests; integration points clear |
| Pitfalls | HIGH | Official docs; community post-mortems; project context |

**Overall confidence:** HIGH

### Gaps to Address

- Bear/clang-tidy on Cygwin/Windows — validate during Phase 4 planning; fallback to Option B.
- Coverage ROI for single-TU — track trends only; no strict gate.
- -Wconversion — deferred; revisit if math code cleaned enough.

---

## Sources

### Primary (HIGH confidence)
- GCC Warning Options, Diagnostic Pragmas
- cppcheck manual
- clang-tidy docs (NOLINT, header-filter, compile_commands)
- Bear, gcov/lcov
- C++ Core Guidelines F.9 (unused parameters)

### Secondary (MEDIUM confidence)
- SonarQube clean-as-you-code
- PVS-Studio incremental adoption
- Stack Overflow (pragma push/pop, template pragma scope)

### Project context
- STACK.md, FEATURES.md, ARCHITECTURE.md, PITFALLS.md
- Makefile, .github/workflows/release.yml, tests/*.sh

---
*Research completed: 2026-03-06*
*Ready for roadmap: yes*
