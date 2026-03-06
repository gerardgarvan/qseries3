# Architecture Patterns: qseries3 Gap Closure

**Project:** qseries3
**Domain:** q-series REPL, modular forms, eta/theta identity proving
**Researched:** 2025-03-06
**Milestone Context:** v11.1 — gaps and problems to resolve. Existing: modforms.h, eta_cusp.h, theta_ids.h, convert.h, linalg.h. Integration test passes.

---

# REPL UX Integration Architecture

**Domain:** REPL ergonomics, error diagnostics, help/docs, input convenience  
**Researched:** 2026-03-06  
**Milestone Context:** SUBSEQUENT — Add REPL ergonomics, error diagnostics, help/docs, input convenience. Existing: repl.h, parser.h, readline, dispatch, display. Zero external deps.  
**Confidence:** HIGH

---

## System Overview: REPL Layer

```
┌─────────────────────────────────────────────────────────────────────────────┐
│                        USER INPUT LAYER                                       │
├─────────────────────────────────────────────────────────────────────────────┤
│  TTY: readLineRaw (RawModeGuard, readOneChar)   │  Script: std::getline       │
│  handleTabCompletion ◄── getCompletionCandidates ◄── getHelpTable + env       │
│  History (↑/↓), Backslash continuation, Ctrl+L (clear)                        │
├─────────────────────────────────────────────────────────────────────────────┤
│                        REPL LOOP (runRepl)                                    │
│  trim, suppress_output (trailing :), save/load (pre-parse), clear             │
├─────────────────────────────────────────────────────────────────────────────┤
│  parse(trimmed)  ──►  evalStmt  ──►  dispatchBuiltin / eval  ──►  display    │
│       │                    │                      │                    │     │
│       │                    │                      │                    │     │
│       ▼                    ▼                      ▼                    ▼     │
│  [ERROR PATH]        [ERROR PATH]           [ERROR PATH]          [SUCCESS]  │
│  parser throws       evalStmt throws        dispatchBuiltin       format*    │
│  runtime_error       runtime_error         runtime_error          ansi::*    │
├─────────────────────────────────────────────────────────────────────────────┤
│  catch → std::cerr ansi::red() "error: " e.what()   │  timing (t0..t1)        │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Integration Points: Feature → Component Mapping

| Feature Area | Integration Point | Component | New vs Modified |
|--------------|-------------------|-----------|-----------------|
| **REPL ergonomics** | Prompt/prompt continuation, timing, suppress output | `runRepl`, `redrawLineRaw` | **Modified** |
| **Error messaging** | Parse errors, runtime errors, script line numbers | `parser.h` (Tokenizer, Parser), `runRepl` catch, `dispatchBuiltin` | **Modified** |
| **Help system** | `help`, `help(func)`, tab completion candidates | `getHelpTable`, `dispatchBuiltin("help", ...)`, `handleTabCompletion` | **Modified** (extend table, add `?` shortcut) |
| **Input features** | Raw line edit, tab completion, history, continuation | `readLineRaw`, `handleTabCompletion`, `getCompletionCandidates` | **Modified** (extend completion rules) |

---

## Component Responsibilities

| Component | Responsibility | Communicates With |
|-----------|----------------|-------------------|
| `runRepl` | Main loop: read → trim → parse → eval → display; error catch; timing; history load/save | `readLineRaw`, `parse`, `evalStmt`, `display`, `loadHistory`, `saveHistory` |
| `readLineRaw` | Char-by-char input, ESC sequences, Tab, Backspace; history navigation; Ctrl+L | `handleTabCompletion`, `redrawLineRaw`, `getCompletionCandidates` |
| `handleTabCompletion` | Prefix match, LCP, single/multi completion | `getCompletionCandidates`, `getHelpTable`, `redrawLineRaw` |
| `getCompletionCandidates` | Union of built-in names + env keys | `getHelpTable`, `Environment` |
| `getHelpTable` | Static map name → (signature, description) | `dispatchBuiltin("help")`, `handleTabCompletion` |
| `dispatchBuiltin` | Route built-in calls; `help` returns DisplayOnly; unknown → suggestions | `getHelpTable`, `eval`, all qfuncs/convert/relations |
| `parse` | Tokenize → parseStmt; throws with line/col | `Tokenizer`, `Parser` |
| `display` | Visit EvalResult → formatEtamake, formatProdmake, etc. | All format* helpers |

---

## Data Flow

### Input → Eval → Display (Success Path)

```
User types line
    ↓
readLineRaw (TTY) or getline (script)
    ↓
Backslash continuation loop (runRepl) → line concatenated
    ↓
trim, suppress_output (trailing :), save/load shortcut
    ↓
history.push_back(trimmed)
    ↓
parse(trimmed) → StmtPtr
    ↓
evalStmt(stmt, env) → eval → dispatchBuiltin
    ↓
display(res, env, T)
    ↓
timing (if TTY)
```

### Error Path (Single Funnel)

```
parse() throws runtime_error  ──┐
evalStmt() throws             ──┼──► catch in runRepl
dispatchBuiltin throws        ──┘         ↓
eval/evalToInt throws         ──┐    std::cerr << ansi::red() << "error: "
                                └──► if (!stdin_is_tty() && inputLineNum > 0)
                                         std::cerr << "line " << inputLineNum << ": ";
                                     std::cerr << e.what() << std::endl;
```

All errors converge at `runRepl`'s single `catch`; no separate error-handling component.

### Help System Data Flow

```
help           → dispatchBuiltin("help", {}) → getHelpTable() → list all names
help(etaq)     → dispatchBuiltin("help", [Var("etaq")]) → table["etaq"] → print sig + desc
unknown built-in → dispatchBuiltin fallback → levenshtein suggestions
Tab completion → handleTabCompletion → getCompletionCandidates() → getHelpTable + env.env
```

---

## New vs Modified Components (Explicit)

### Modified (No New Files)

| Component | Current State | Modification for Ergo/Error/Help/Input |
|-----------|---------------|----------------------------------------|
| `parser.h` | Tokenizer throws "parser: line N, col C: expected X" | Improve messages (underline offending token, suggest fix); keep offsetToLineCol |
| `repl.h` runRepl | Single catch, line number only in script mode | Add caret/underline for parse errors if TTY; optional verbose mode |
| `repl.h` getHelpTable | Static map, ~90 entries | Add entries for new built-ins; optional `help("topic")` search |
| `repl.h` handleTabCompletion | Prefix match on identifier; functions get `(` appended | Add subcommand completion (e.g. `help eta` → `etaq`, `etamake`); optional path-style |
| `repl.h` readLineRaw | Tab, arrows, Backspace, Ctrl+L | Optional: Home/End, Ctrl+A/E (if not already); optional `?` as help shortcut |
| `repl.h` ansi | gold, red, dim, reset, bold | Optional: cyan for hints, yellow for warnings |

### New (Optional / Future)

| Component | Purpose | Integration |
|-----------|---------|-------------|
| `formatParseError(input, offset, msg)` | Pretty-print parse error with caret | Called from runRepl catch when exception message contains "parser:" |
| `helpSearch(query)` | Fuzzy/search over getHelpTable | New branch in dispatchBuiltin("help", [Var("?")]) or help("query") |
| Completion context (optional) | After `(`, complete arg types | Would require parse-in-progress; high complexity, defer |

---

## Suggested Build Order (Dependencies)

```
1. Error diagnostics (parser + REPL catch)
   ├── Parser: ensure all throws use offsetToLineCol + kindToExpected
   ├── REPL: formatParseError(input, offset, msg) for TTY
   └── REPL: script mode "line N: " prefix (already present)
   Rationale: Errors are cross-cutting; improving them first helps debug later changes.

2. Help system extensions
   ├── Add missing getHelpTable entries
   ├── Optional: help("substring") search
   └── Optional: ? as help shortcut in readLineRaw
   Rationale: getHelpTable is shared by help + tab completion; extend once.

3. Input convenience
   ├── Tab: extend completion (e.g. after . or for multi-token)
   ├── Optional: Home/End in readLineRaw
   └── Optional: ? → help
   Rationale: Depends on getHelpTable; no parser changes.

4. Ergonomics polish
   ├── Continuation prompt consistency
   ├── Timing format tweaks
   └── Clear/suppress_output behavior
   Rationale: Cosmetic; depends on nothing.
```

**Build order rationale:** Error handling is foundational for debugging. Help and completion share `getHelpTable`, so extend that next. Input conveniences touch `readLineRaw` and completion. Ergonomics are last as they are mostly cosmetic.

---

## Internal Boundaries

| Boundary | Communication | Notes |
|----------|---------------|-------|
| runRepl ↔ readLineRaw | `std::optional<std::string>` | readLineRaw returns nullopt on EOF; runRepl breaks loop |
| runRepl ↔ parse | `parse(string)` → `StmtPtr`; throws on error | Parser owns Tokenizer; no streaming |
| runRepl ↔ evalStmt | `evalStmt(Stmt*, Env)` → `EvalResult`; throws | evalStmt calls eval → dispatchBuiltin |
| dispatchBuiltin ↔ getHelpTable | const ref to map | getHelpTable is static; no mutation |
| handleTabCompletion ↔ getCompletionCandidates | set of strings | Completion is synchronous; env read-only |
| Emscripten (main_wasm.cpp) | `evaluate(string)` → string | No runRepl; uses parse, evalStmt, display; redirects cout/cerr to oss |

---

## Anti-Patterns for REPL UX

1. **Don’t add a separate error module** — Keep one catch in runRepl and improve message content. A separate ErrorReporter would add indirection without benefit.
2. **Don’t duplicate getHelpTable** — Help and tab completion must share the same source of truth.
3. **Don’t block readLineRaw on I/O** — Completion and redraw must stay synchronous; no async help fetch.
4. **Don’t change parser API for errors** — Parser should keep throwing `runtime_error`; REPL is responsible for presentation (colors, caret, line numbers).

---

## Scalability (REPL Context)

| Concern | TTY Mode | Script Mode | Emscripten |
|---------|----------|-------------|------------|
| History size | 1000 lines, file persist | N/A | N/A |
| Completion candidates | ~90 built-ins + env | N/A | N/A |
| Error context | Full line + caret possible | Line number only | Captured in oss |
| Timing | Per-command | Suppressed | Not shown |

---

## Sources

- `src/repl.h` — runRepl, readLineRaw, handleTabCompletion, getHelpTable, display, evalStmt, dispatchBuiltin
- `src/parser.h` — Tokenizer, Parser, offsetToLineCol, kindToExpected
- `src/main_wasm.cpp` — evaluate, error capture
- `SPEC.md` — Architecture diagram, file structure
- `.planning/ROADMAP.md` — Phases 17–22 (help, timing, continuation, tab, errors)

---

## Executive Summary

Missing features (provemodfuncid enhancements, RR identity search, Block 25 fix, etc.) integrate into a well-defined data-flow pipeline. **provemodfuncid** and **RR identity search** are already implemented; the main gaps are extensions (provemodfuncidBATCH, findids types 3–10), robustness (Block 25 q-shift normalization), and deferred stubs (U_p operator). No new top-level components are required—only modifications and extensions to existing headers.

---

## Current Architecture Overview

```
┌─────────────────────────────────────────────────────────────────────────────┐
│  REPL + Parser (repl.h, parser.h)                                            │
├─────────────────────────────────────────────────────────────────────────────┤
│  Built-ins dispatch: provemodfuncGAMMA0id, provemodfuncid, RRG, RRH,        │
│  checkid, findids, jac2eprod, jac2series, prodmake, etamake, jacprodmake     │
├─────────────────────────────────────────────────────────────────────────────┤
│  modforms.h  │ eta_cusp.h      │ theta_ids.h  │ convert.h  │ rr_ids.h       │
│  DELTA12     │ provemodfunc    │ provemodfunc │ prodmake   │ RRG, RRH       │
│  makebasisM  │ GAMMA0id        │ id           │ etamake    │ checkid        │
│  makeALTbasisM│ etaprodtoqseries│ jac2eprod   │ jacprodmake│ findids        │
│              │ cuspmake        │ jac2series   │ jac2series │ geta           │
├─────────────────────────────────────────────────────────────────────────────┤
│  Series (std::map<int,Frac>)  │  Frac  │  BigInt                            │
└─────────────────────────────────────────────────────────────────────────────┘
```

---

## Data Flow: Identity Proving Pipeline

### ETA identity (Gamma_0(N))

```
etaid: [(coeff, GP), ...]  →  etaprodtoqseries(GP)  →  Series
                                                         ↓
                                              Sturm bound from mintotGAMMA0ORDS
                                                         ↓
                                              Verify sum = 0 to O(q^sturm)
                                                         ↓
                                              provemodfuncGAMMA0id → proven=0/1
```

- **Component:** eta_cusp.h
- **Input:** `[[c, GP], ...]` where GP = `[t1,r1,t2,r2,...]` (eta-quotient)
- **Output:** `ProveModfuncResult { proven, sturm_bound, message }`

### Theta/Jacobi identity (Gamma_1(N))

```
jacid: [(coeff, JacFactor[]), ...]  →  jac2eprod(jac)  →  GETA [(n,a,c),...]
       |                                                    ↓
       └────────────────→ jac2series(jac) ─────────→ Series (q-expansion)
                                                         ↓
                                              Gamma1ModFunc(gl,N), getaprodcuspORDS
                                                         ↓
                                              Sturm bound from mintotORDS
                                                         ↓
                                              provemodfuncid → proven=0/1
```

- **Component:** theta_ids.h (uses convert.h jac2series)
- **Input:** `[[coeff, [[a,b,c],...]], ...]` (Jacobi product list)
- **Output:** `ProveModfuncIdResult { proven, sturm_bound, message }`

### Rogers-Ramanujan → Proof chain

```
Series (RR sum)  →  jacprodmake  →  JacFactor  →  jac2eprod  →  GETA
                                                         ↓
                                              provemodfuncid(jacid, N)
```

- **Integration test flow:** `rr := sum(...) → j := jacprodmake(rr,50) → jac2eprod(j) → provemodfuncid([...], 25)`

---

## Integration Points: Missing Features vs Existing

| Missing / Gap Feature            | Integrates With              | Data Flow Change | New vs Modified    |
|----------------------------------|------------------------------|------------------|--------------------|
| provemodfuncid enhancements      | theta_ids.h                  | None             | **Modified**       |
| provemodfuncidBATCH              | theta_ids.h                  | Batch jacid list | **New function**   |
| RR identity search (findids)     | rr_ids.h                     | Uses RRG/RRH, checkid | **Implemented** |
| checkid                          | convert.h (prodmake, etamake)| Series → CheckidResult | **Implemented** |
| Block 25 fix (q-shift)           | Series::addAligned, findpoly | Normalize q_shift in + | **Modified** |
| U_p operator (provemodfuncGAMMA0UpETAid) | eta_cusp.h          | Stub only        | **New impl**       |
| findlincombomodp                 | linalg.h, relations.h        | modp + solve     | **New function**   |

---

## Component Dependencies (Build Order)

```
BigInt → Frac → Series
         ↓
    qfuncs (aqprod, etaq, theta, EISENq, Phiq)
         ↓
    convert (prodmake, etamake, jacprodmake, jac2series, sift)
         ↓
    linalg (kernel, solve)
         ↓
    relations (findhom, findnonhom, findpoly)
         ↓
┌───────┴───────┬───────────────┬──────────────┐
│               │               │              │
eta_cusp    theta_ids      modforms       rr_ids
(Gamma_0)   (Gamma_1)      (M_k bases)    (RR, checkid, findids)
    │               │               │              │
    └───────────────┴───────────────┴──────────────┘
                           ↓
                      repl.h (dispatch)
```

---

## Recommended Build Order for Phases

### Tier 1: Foundation (no new components)

| Phase | What | Depends On | Output |
|-------|------|------------|--------|
| 97 Block 25 fix | q-shift alignment in Series::+ before findpoly | Series | `addAligned` used; Block 25 passes |
| 98 Block 24 | collect-style formatter or N/A docs | — | Test rationale documented |
| 99 findlincombomodp | f ≡ Σ c_i L_i (mod p) | modp, linalg F_p | New built-in |

**Rationale:** Block 25 fix unblocks findpoly on theta quotients; findlincombomodp completes modular linear-algebra story. No new headers.

### Tier 2: Theta IDs Extensions

| Phase | What | Depends On | Output |
|-------|------|------------|--------|
| 85 provemodfuncid | provemodfuncidBATCH; verify theta_aids | theta_ids, jac2eprod | Batch proof; 2+ theta_aids verified |

**Rationale:** provemodfuncid is implemented; Phase 85 adds BATCH and theta_aids regression coverage. Depends only on theta_ids (already integrated).

### Tier 3: RR Identity Search (Implemented; Verification)

| Phase | What | Depends On | Output |
|-------|------|------------|--------|
| 86 RR identity | findids types 3–10; acceptance tweaks | rr_ids, checkid, RRG/RRH | Extended findids; acceptance-rr-id passes |

**Rationale:** RRG, RRH, checkid, findids (types 1–2) exist. Gaps: findids types 3–10, discovery count tuning, q-shift handling in addSeriesAligned.

### Tier 4: Deferred / Lower Priority

| Phase | What | Notes |
|-------|------|-------|
| provemodfuncGAMMA0UpETAid | U_p sifting proof | Stub in eta_cusp.h; complex, defer |
| findids types 3–10 | Extended RR search | Types 1–2 suffice for v11.1 |

---

## Data Flow Changes for Gap Closure

### Block 25 (q-shift alignment)

**Current:** `findpoly(x, y, ...)` fails when x and y have different `q_shift` (e.g. theta2/theta3 quotients).

**Change:** Ensure `Series::operator+` (or a normalized path into findpoly) aligns q_shifts via `addAligned` or equivalent. `addAligned` exists in rr_ids.h and is used by findids; findpoly's input path must use similar normalization.

**Modified files:** `series.h` (add path), `relations.h` or findpoly caller.

### provemodfuncidBATCH

**Current:** `provemodfuncid(jacid, N)` processes one identity.

**Change:** Add `provemodfuncidBATCH(jacids, N)` that returns `vector<ProveModfuncIdResult>`. Same logic per identity.

**Modified files:** `theta_ids.h`, `repl.h`.

### findlincombomodp

**Current:** `findhommodp` exists; no degree-1 linear combo mod p.

**Change:** Add `findlincombomodp(f, L, p, T)` → coefficients in F_p. Uses `modp`, F_p Gaussian elimination (existing), solve step.

**Modified files:** `relations.h` or new `modp_linalg` section, `repl.h`.

---

## Anti-Patterns to Avoid

1. **Don't duplicate Sturm logic** — provemodfuncGAMMA0id and provemodfuncid each have Sturm-bound computation; keep them separate (Gamma_0 vs Gamma_1 cusp formulas differ).
2. **Don't bypass jac2eprod for provemodfuncid** — provemodfuncid requires GETA (generalized eta) for Gamma_1 cusp orders; JacFactor → jac2eprod → getalist is mandatory.
3. **Don't add new REPL types for identity results** — `ProveModfuncResult` and `ProveModfuncIdResult` are sufficient; display as strings.
4. **Don't change provemodfuncid's jacid format** — It expects `[(Frac, vector<JacFactor>), ...]`; ramarobinsids.txt uses provemodfuncidBATCH with same per-identity format.

---

## Scalability / Performance Notes

| Concern | At T=50 | At T=200 | At T=500 |
|---------|---------|----------|----------|
| provemodfuncid Sturm bound | ~100 | ~400 (capped 500) | 500 cap |
| findids search space | Small | Moderate | O(n²) pairs |
| checkid (prodmake + etamake) | Fast | Slower | May need T limit |
| jac2series with JacFactor | Fast | Depends on product size | — |

**Recommendation:** Cap Sturm at 500 (already done). For findids, consider T limit or early termination for large T.

---

---

# Code Health Tooling Integration

**Domain:** Code health audit, static analysis, sanitizers, formatting  
**Researched:** 2026-03-06  
**Milestone Context:** v11.3 — Code health audit and plan. Existing: Makefile, src/*.h, main.cpp, tests/*.sh. Single g++ invocation, header-only, bash acceptance tests.  
**Confidence:** HIGH

---

## Integration Points: Makefile + Bash Tests

| Integration Point | Purpose | Current State | Code Health Addition |
|-------------------|---------|---------------|----------------------|
| **Makefile target** | Run tooling before/after build | `all`, `test`, `acceptance*` | New targets: `lint`, `tidy`, `cppcheck`, `sanitize` |
| **Build dependency** | Ensure binary exists before tests | `acceptance: dist/qseries.exe` | Static analysis runs on source; no build dep. Tests still need `dist/qseries.exe` |
| **CI workflow** | Gate release on health checks | Release workflow: build → acceptance | Add lint/tidy step before or parallel to build; extend `.github/workflows/` |
| **Test scripts** | Assert behavior | `tests/*.sh` pipe to `$BIN` | No change to test logic. Optional: `tests/acceptance-sanitize.sh` for sanitizer build |
| **Pre-commit / local** | Developer feedback | Manual `make` | Optional: `make check` = lint + tidy + acceptance |

**Key invariant:** Code health tools operate on **source**, not the binary. Acceptance tests require the binary. Order: static analysis (optional, no build) → build → acceptance tests.

---

## Component Mapping: New vs Modified

### New Components (add, do not modify existing)

| Component | Type | Purpose |
|-----------|------|---------|
| `make lint` | Makefile target | Run cppcheck on `src/*.cpp src/*.h` (or `src/main.cpp`; headers included) |
| `make tidy` | Makefile target | Run clang-tidy on `src/main.cpp` |
| `make sanitize` | Makefile target | Build with `-fsanitize=address,undefined`, run acceptance subset |
| `make check` | Makefile target | Optional umbrella: lint + tidy + acceptance |
| `.clang-tidy` | Config file | Checks to enable/disable; format style |
| `compile_commands.json` | Generated | Required for clang-tidy `-p`; produced by Bear or manual |
| `.github/workflows/lint.yml` | CI job | Run lint/tidy on push/PR (optional; Release stays tag-triggered) |

### Modified Components

| Component | Current | Modification |
|-----------|---------|--------------|
| **Makefile** | `.PHONY` list, targets | Add `lint`, `tidy`, `sanitize`, `check`; extend `.PHONY` |
| **Makefile `debug`** | Already uses `-fsanitize=address,undefined` | Keep as-is; `sanitize` can wrap `debug` + run tests |
| **Release workflow** | Build + acceptance (ubuntu) | Optional: add `make lint` before build; keep acceptance as gate |

**Optional modification (for sanitize):** `tests/acceptance.sh` — add `BIN="${BIN:-./dist/qseries.exe}"` at top so `BIN=./qseries_debug make sanitize` runs acceptance against the sanitizer binary. Without this, use `tests/acceptance-sanitize.sh` (new file) instead.

**Unchanged:** Core `tests/*.sh` logic (pipe to $BIN, grep output), `dist/qseries.exe` build rule, `CXXFLAGS`, single-TU structure.

---

## Suggested Build Order (Dependencies)

```
1. cppcheck (no compile DB)
   └── Add: make lint
   └── cppcheck --enable=warning,performance,portability -Isrc src/main.cpp
   Rationale: Easiest; no Bear, no compile_commands.json. Run immediately.

2. .clang-tidy + make tidy
   ├── Option A: Bear -- make -B → compile_commands.json; clang-tidy -p . src/main.cpp
   ├── Option B: clang-tidy src/main.cpp -- -std=c++20 -Isrc (simpler, fewer checks)
   └── Add: make tidy (with --fix optional)
   Rationale: More checks than cppcheck; may need Bear on Cygwin. Option B sufficient for single TU.

3. make sanitize
   └── Reuse: make debug → ./qseries_debug; run acceptance.sh or subset
   └── Add: make sanitize = debug + bash tests/acceptance.sh
   Rationale: Existing debug target has sanitizers; wire to acceptance.

4. make check (umbrella)
   └── Depends on: lint, tidy, acceptance
   └── Add: make check = make lint && make tidy && make acceptance
   Rationale: Single command for full health pass.

5. CI lint job (optional)
   └── New: .github/workflows/lint.yml on push/PR
   └── Steps: checkout → make lint → make tidy (or skip tidy if Bear unavailable in runner)
   Rationale: Catches regressions before merge; Release workflow unchanged.
```

**Build-order rationale:**
- cppcheck first: zero setup, immediate value
- clang-tidy second: needs config; Bear may be unavailable on Windows/Cygwin
- sanitize third: reuses existing `debug` target
- check fourth: aggregates
- CI last: optional, depends on runner having tools

---

## Data Flow: Code Health Pipeline

```
Source (src/main.cpp, src/*.h)
    │
    ├──► cppcheck  ──► stdout/stderr (warnings)
    │
    ├──► clang-tidy (with compile_commands.json or -- -std=c++20 -Isrc)
    │         └──► stdout (diagnostics); --fix writes files
    │
    └──► g++ -fsanitize  ──► qseries_debug
                                └──► tests/acceptance.sh  ──► exit 0/1
```

**Separation:** Static analysis (cppcheck, clang-tidy) does not require building. Sanitizer run requires build + execution of tests. CI can run lint/tidy in parallel with build; acceptance must run after build.

---

## Makefile Integration Pattern

```makefile
# Add to existing .PHONY
.PHONY: ... lint tidy sanitize check

# Lint: cppcheck (no DB)
lint:
	cppcheck --enable=warning,performance,portability --quiet -Isrc src/main.cpp

# Tidy: clang-tidy (Option B: no Bear)
tidy:
	clang-tidy src/main.cpp -- -std=c++20 -Isrc

# Sanitize: build with sanitizers + run acceptance
sanitize: qseries_debug
	BIN=./qseries_debug bash tests/acceptance.sh
# Requires: acceptance.sh honors BIN from env (add BIN="${BIN:-./dist/qseries.exe}" at top)

# Check: full health pass
check: lint tidy dist/qseries.exe
	$(MAKE) acceptance
```

**Dependency rules:** `tidy` and `lint` have no dependencies. `sanitize` depends on `qseries_debug` (from `debug`). `check` depends on `lint`, `tidy`, and `dist/qseries.exe`; runs `acceptance` last.

---

## Bash Test Integration

**No changes to existing test scripts.** Code health tooling does not alter:
- How tests run: `printf '%s\n' "$@" | "$BIN"`
- BIN resolution: `./dist/qseries.exe` or `./dist/qseries`
- PASS/FAIL logic

**Optional addition:** `tests/acceptance-sanitize.sh` — same as `acceptance.sh` but invokes `./qseries_debug` if present, for sanitizer UBSan/ASan validation. Makefile `sanitize` would run this instead of `acceptance.sh` when using the debug binary.

---

## Pitfalls for Code Health Integration

1. **clang-tidy without compile_commands.json:** For single-TU, use `clang-tidy src/main.cpp -- -std=c++20 -Isrc`. Without `-Isrc`, headers in `src/` may not resolve if run from project root.
2. **Bear on Windows/Cygwin:** Bear intercepts compiler calls; availability on Windows varies. Prefer Option B (no Bear) for cross-platform.
3. **Sanitizers in CI:** `-fsanitize` can slow tests; consider running a subset (e.g. `acceptance` only) for sanitize in CI.
4. **cppcheck on headers:** For header-only libs, analyzing `src/main.cpp` pulls in all headers. Analyzing `src/*.h` directly can yield duplicate or noisy diagnostics; prefer single-TU analysis.

---

## Sources

- Makefile — existing targets, CXXFLAGS, debug
- `.github/workflows/release.yml` — build, acceptance
- tests/acceptance.sh, tests/run-all.sh — BIN resolution, test pattern
- [Clang-Tidy Integrations](https://clang.llvm.org/extra/clang-tidy/Integrations.html) — config, IDE
- Bear / compiledb — compile_commands.json for Makefile
- cppcheck — standalone, no compile DB
- Web search: clang-tidy Makefile, cppcheck integration, Bear compile_commands

---

## Sources (Identity Proving / Gap Closure)

### Primary (HIGH confidence)

- `src/eta_cusp.h` — provemodfuncGAMMA0id, cuspmake, etaprodtoqseries
- `src/theta_ids.h` — provemodfuncid, jac2eprod, Gamma1ModFunc, getaprodcuspORDS
- `src/convert.h` — prodmake, etamake, jacprodmake, jac2series
- `src/rr_ids.h` — RRG, RRH, checkid, findids
- `tests/integration-eta-theta-modforms.sh` — integration flow

### Secondary

- `.planning/ROADMAP.md` — Phases 82, 84, 85, 86
- `.planning/phases/102-cross-package-integration/102-RESEARCH.md` — single proof chain
- `gaps/etapackage.txt` — Maple provemodfuncGAMMA0id / provemodfuncidBATCH usage
