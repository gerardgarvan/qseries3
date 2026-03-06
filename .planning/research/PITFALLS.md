# Domain Pitfalls: Code Health Tooling

**Domain:** Code health assessment and tooling integration on a mature C++ codebase
**Project:** qseries3
**Researched:** 2026-03-06
**Confidence:** HIGH (official GCC/LLVM docs; community post-mortems; project context)

---

## Executive Summary

Adding code health tooling (static analysis, coverage, stricter warnings, CI checks) to a mature C++ codebase like qseries3 (~15 headers, single TU, existing -Wall -Wextra -Wpedantic) introduces three major failure modes: (1) **warning suppression** that hides real issues or leaks across scopes, (2) **coverage scope** that is too narrow (missing headers) or too broad (third-party, generated code), and (3) **plan scope creep** where “fix all warnings” or “100% coverage” balloons into rewrites. Prevention: fix before suppress, define scope up front, adopt incrementally.

---

## Critical Pitfalls

### Pitfall 1: Pragmas Without Balanced Push/Pop

**What goes wrong:**
Using `#pragma GCC diagnostic ignored "-Wfoo"` without a matching `#pragma GCC diagnostic pop` causes the suppression to persist for the rest of the file. Warnings in unrelated code are silently suppressed.

**Why it happens:**
Developers add `#pragma GCC diagnostic ignored` to silence a single warning and forget the pop. In header files, this leaks to every translation unit that includes the header.

**Consequences:**
- Real bugs hidden below the pragma
- Headers poison downstream code
- Cross-compiler builds (GCC vs Clang) behave differently; Clang may not honor GCC pragmas

**Prevention:**
- Always use balanced blocks:
  ```cpp
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-parameter"
  // only the problematic call/section
  #pragma GCC diagnostic pop
  ```
- Prefer fixing the warning (omit parameter name, `[[maybe_unused]]`, `(void)x`) over pragmas
- Never put bare `ignored` without `push`/`pop` in headers

**Detection:**
- Grep for `#pragma GCC diagnostic ignored` without a preceding `push` or following `pop`
- Run with `-Werror`; if pragmas leak, unrelated code may start failing

**Phase to address:** Code health / warning-cleanup phase. Add as acceptance: “no pragma diagnostic without matching push/pop.”

---

### Pitfall 2: Template Instantiation Ignores Pragma Scope

**What goes wrong:**
Pragmas placed around template *definitions* often do not affect diagnostics from template *instantiation*, which happens later or in a different TU. The “fix” appears to work locally but warnings reappear when the template is used elsewhere.

**Why it happens:**
The compiler emits diagnostics at instantiation time, not definition time. Pragma scope ends at the definition; instantiation is out of scope.

**Prevention:**
- Wrap the *call site* (the code that instantiates the template), not the template definition
- Or fix the template itself (e.g. use SFINAE, concepts, or correct types) instead of suppressing

**Phase to address:** Static analysis / clang-tidy phase. If using NOLINT on templates, verify on all instantiation sites.

---

### Pitfall 3: NOLINT / Suppression Creep and Technical Debt

**What goes wrong:**
Adding `// NOLINT` or `// NOLINTNEXTLINE` to silence clang-tidy (or similar) without fixing the underlying issue causes suppressions to accumulate. There is no easy way to re-enable checks or audit which suppressions are intentional vs oversight.

**Why it happens:**
Quick path to “clean” CI: suppress instead of fix. No governance on when NOLINT is acceptable.

**Consequences:**
- Technical debt: genuine bugs hidden
- Unclear intent: which NOLINTs are interface constraints vs laziness?
- Version drift: NOLINT behavior can change across clang-tidy versions; NOLINT on first line of file was historically buggy

**Prevention:**
- Prefer fix over suppress. For unused parameters: omit the name, use `/* param */`, or `[[maybe_unused]]`
- Require a short comment next to each NOLINT: `// NOLINT(misc-unused-parameters) — callback signature must match API`
- Cap NOLINTs per file in review (e.g. max 2) to force discussion
- Use `NOLINTBEGIN`/`NOLINTEND` for blocks, but ensure no orphaned markers

**Phase to address:** Clang-tidy integration phase. Add policy: “fix before NOLINT; NOLINT requires justification comment.”

---

### Pitfall 4: Fix vs Suppress — Wrong Choice for Unused Parameters

**What goes wrong:**
Using pragmas or `(void)x` for every unused parameter when the interface is under your control. Or, changing the interface (removing parameters) when the function must match an external callback signature.

**Best practice (from C++ Core Guidelines F.9, GCC workarounds):**
- **You control the signature:** Omit the parameter name: `void callback(int, void*)` or `void f(int /*param*/)`. Cleanest.
- **Conditionally unused:** Use `[[maybe_unused]]` (C++17).
- **Must match external API:** Use `(void)param` or `[[maybe_unused]]`. Avoid pragmas.
- **Generated / third-party code you cannot edit:** Pragmas with push/pop as last resort.

**Anti-pattern:** Global `-Wno-unused-parameter` to get a “clean” build. Hides all unused-parameter warnings project-wide.

**Phase to address:** Warning-cleanup phase (qseries3 already did Phase 34 with `(void)q` and `[[maybe_unused]]` — good pattern).

---

### Pitfall 5: Coverage Scope — Headers and Single-TU Blind Spots

**What goes wrong:**
- **Too narrow:** Only `.cpp` files analyzed; headers (where most logic lives in header-only or template code) are excluded. qseries3 is effectively header-only (main.cpp pulls in headers); analyzing only main.cpp misses almost everything.
- **Too broad:** Third-party headers, generated code, or system headers get analyzed, producing noise and false positives.

**Why it happens:**
- Tools default to “compiled files only”; headers are often analyzed only in the context of the TUs that include them
- Without `compile_commands.json` or equivalent, tools may not know include paths or preprocessor definitions
- Unincluded headers are never analyzed (SonarQube, clang-tidy)

**Prevention:**
- For qseries3 (single TU): ensure `main.cpp` is the analyzed root; headers will be pulled in. Generate `compile_commands.json` (e.g. via CMake `-DCMAKE_EXPORT_COMPILE_COMMANDS=ON`) or use Bear.
- Set `-header-filter` (clang-tidy) to project sources only: `-header-filter='.*/src/.*'` — exclude system and third-party
- Document scope explicitly: “src/*.h and src/main.cpp”; “exclude: build/, third_party/”

**Phase to address:** Static analysis / coverage setup phase. Define scope in the plan before running tools.

---

### Pitfall 6: GCC vs Clang Flag Incompatibility

**What goes wrong:**
Integrating clang-tidy (or other Clang-based tools) with a GCC-built project. GCC-specific flags (e.g. `-Wlogical-op`) in `compile_commands.json` cause clang-tidy to error. Missing defines (e.g. `_GNU_SOURCE`, `_POSIX_SOURCE`) make standard library identifiers unavailable to clang-tidy.

**Prevention:**
- Filter or patch `compile_commands.json` to remove GCC-only flags before feeding to clang-tidy
- Use `-isystem` to point to correct toolchain headers; consider `-nostdinc` if clang-tidy uses incompatible headers
- Prefer flags supported by both (e.g. `-Wall -Wextra`) when generating compile commands

**Phase to address:** Clang-tidy integration phase. Document GCC→Clang flag mapping.

---

### Pitfall 7: Plan Scope Creep — “Fix All” Becomes a Rewrite

**What goes wrong:**
Phase goal: “Add clang-tidy and achieve clean run.” Execution: 500 diagnostics, team spends weeks “fixing” style, naming, and refactors. The phase never closes; new features stall.

**Why it happens:**
- Big-bang adoption: enabling all checks at once on a mature codebase
- No baseline: every finding treated as must-fix
- Style vs correctness not distinguished: `readability-identifier-naming` is low-value for a working codebase

**Prevention:**
- **Incremental adoption:** Enable 2–5 high-value checks first (e.g. `bugprone-*`, `performance-*`). Add more over time.
- **Baseline:** Use `--warnings-as-errors` only for *new* code or specific paths. For existing code, record baseline and fail only on *new* findings.
- **Phase scope:** Define “Phase N: enable clang-tidy with checks X, Y, Z; fix only findings in files A, B.” Time-box. Defer the rest.

**Phase to address:** Planning phase for code health. Explicit scope: “which checks, which files, what’s in vs out.”

---

### Pitfall 8: -Werror Blocking Builds on New Compiler Versions

**What goes wrong:**
CI uses `-Werror`. New GCC/Clang version introduces a new warning or changes behavior; build fails. Team is blocked until someone patches the code or disables the new warning.

**Why it happens:**
Compiler vendors add and refine warnings. Code that was clean under GCC 12 may warn under GCC 13.

**Prevention:**
- Prefer fixing warnings over suppressing; but have a rollback: `-Wno-error=<new-warning>` for a release if needed
- Pin compiler version in CI for reproducibility; upgrade in a dedicated phase with time to fix new warnings
- Consider `-Werror` only for a subset of warnings (e.g. `-Werror=return-type`) rather than all

**Phase to address:** CI / build phase. Document compiler version and `-Werror` policy.

---

## Moderate Pitfalls

### Pitfall 9: Compile Database Wrong or Stale

**What goes wrong:**
`compile_commands.json` is missing, incomplete, or references non-existent paths (e.g. `.modmap` files in non-modules projects). Clang-tidy and similar tools fail or produce wrong results.

**Prevention:**
- Regenerate `compile_commands.json` on every configure/build
- For qseries3 (Makefile): use Bear (`bear -- make`) or a small CMake stub that exports compile commands
- Verify tool runs on at least one file before full rollout

### Pitfall 10: Excessive False Positives Leading to Check Disable

**What goes wrong:**
A check produces many false positives. Team disables the check entirely instead of narrowing scope or fixing the underlying pattern.

**Prevention:**
- Use `NOLINT` sparingly for genuine false positives, with a comment
- Use `-header-filter` to limit analysis to project code
- Disable only the specific sub-check that’s noisy, not the whole category

---

## Minor Pitfalls

### Pitfall 11: Performance — Full Analysis on Every Commit

**What goes wrong:**
Running full-project clang-tidy (or similar) on every commit is slow and blocks feedback.

**Prevention:**
- Use incremental analysis where supported (e.g. only changed files)
- Run full analysis in nightly CI; run on changed files in pre-commit or PR
- Use `-j$(nproc)` for parallelism

### Pitfall 12: Orphaned NOLINTBEGIN / NOLINTEND

**What goes wrong:**
`NOLINTBEGIN` without matching `NOLINTEND` (or vice versa) can suppress more than intended.

**Prevention:**
- Keep NOLINT blocks small and paired
- Grep for unbalanced NOLINTBEGIN/NOLINTEND in review

---

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation |
|-------------|----------------|------------|
| Warning cleanup | Pragmas without push/pop | Require balanced pragmas; prefer fix over suppress |
| Warning cleanup | Unused params | Use omit name, `[[maybe_unused]]`, or `(void)x`; avoid global -Wno-* |
| Clang-tidy integration | GCC flag incompatibility | Filter compile_commands.json; document flag mapping |
| Clang-tidy integration | NOLINT creep | Policy: fix first; NOLINT requires justification |
| Coverage / scope | Headers not analyzed | Single TU: analyze main.cpp; headers included. Set header-filter. |
| Coverage / scope | Third-party noise | Exclude via header-filter, sonar.sources, or path config |
| Plan / scope | Scope creep | Define: which checks, which files, baseline vs new-only. Time-box. |
| CI / -Werror | Build breaks on compiler upgrade | Pin compiler; have -Wno-error escape hatch |
| Templates | Pragma doesn’t apply | Wrap call site or fix template; don’t rely on definition-time pragmas |

---

## Pitfall-to-Phase Mapping (for Roadmap)

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Pragmas without push/pop | Code health / warning phase | Grep; no bare `ignored` without push/pop |
| Template pragma scope | Static analysis phase | Run tool on all TUs; no reappearing warnings |
| NOLINT / suppression creep | Clang-tidy phase | Policy + comment requirement; cap NOLINTs per file |
| Wrong fix vs suppress | Warning cleanup | Use omit name / [[maybe_unused]] / (void)x; no global -Wno-* |
| Coverage scope wrong | Coverage / analysis setup | Document scope; header-filter; single TU includes headers |
| GCC/Clang flag mismatch | Clang-tidy integration | compile_commands.json works with clang-tidy |
| Plan scope creep | Code health planning | Explicit phase scope; incremental checks; baseline |
| -Werror blocks build | CI phase | Pin compiler; document -Werror policy |
| Stale compile DB | Analysis setup | Regenerate on build; verify tool runs |

---

## Sources

- GCC Diagnostic Pragmas: https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
- Stack Overflow: GCC pragma push/pop, template instantiation, NOLINT behavior
- Clang-Tidy: compile_commands.json, header-filter, NOLINT — LLVM docs
- PVS-Studio: “Grandma’s recipe for mastering regular static analysis” (incremental adoption)
- C++ Core Guidelines F.9 (unused parameters)
- iifx.dev: GCC -Wunused-parameter workarounds for callbacks
- SonarQube: C++ analysis scope, compilation database mode
- qseries3: Makefile (CXXFLAGS), Phase 34 warning fixes, single-TU structure
