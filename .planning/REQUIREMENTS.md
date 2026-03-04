# Requirements: Q-Series REPL

**Defined:** 2025-02-24  
**Core Value:** Rogers-Ramanujan must work — prodmake recovers product form with denominators at exponents ≡ ±1 (mod 5)

## v1 Requirements

Requirements for initial release. Each maps to roadmap phases.

### Core (Foundation)

- [ ] **CORE-01**: BigInt: arbitrary precision integer, base 10⁹, signed, vector<uint32_t>
- [ ] **CORE-02**: BigInt: +, -, *, divmod, gcd; long division with binary-search quotient
- [ ] **CORE-03**: Frac: exact rational (BigInt num, den); reduce() after every construction and operation
- [ ] **CORE-04**: Series: truncated power series (std::map<int,Frac>, int trunc)
- [ ] **CORE-05**: Series: +, -, *, /, pow, inverse (recurrence j=1..n), subs_q; truncation propagation
- [ ] **CORE-06**: Number theory helpers: divisors, mobius, legendre, sigma, euler_phi

### Q-Functions

- [ ] **QFUNCS-01**: aqprod — rising q-factorial (a;q)_n
- [ ] **QFUNCS-02**: qbin — Gaussian polynomial [n;m]_q
- [ ] **QFUNCS-03**: etaq — eta product Π(1-q^{kn})
- [ ] **QFUNCS-04**: theta2, theta3, theta4 — theta functions
- [ ] **QFUNCS-05**: tripleprod, quinprod — triple and quintuple products
- [ ] **QFUNCS-06**: winquist — Winquist identity

### Conversion

- [ ] **CONVERT-01**: prodmake — Andrews' algorithm, series → infinite product
- [ ] **CONVERT-02**: etamake — eta product identification
- [ ] **CONVERT-03**: jacprodmake — Jacobi product identification (80% periodicity, verify)
- [ ] **CONVERT-04**: qfactor — finite q-product factorization
- [ ] **CONVERT-05**: sift — coefficient extraction a_{ni+k}

### Linear Algebra

- [ ] **LINALG-01**: Gaussian elimination over Q
- [ ] **LINALG-02**: kernel computation for rational matrices

### Relations

- [ ] **REL-01**: findhom — homogeneous polynomial relations
- [ ] **REL-02**: findnonhom — nonhomogeneous polynomial relations
- [ ] **REL-03**: findhomcombo, findnonhomcombo — express f as polynomial in list
- [ ] **REL-04**: findpoly — polynomial relation between two series

### Parser and REPL

- [ ] **REPL-01**: Parser: tokenizer + recursive-descent expression parser (Maple-like syntax)
- [ ] **REPL-02**: Variable environment, assignment (:=)
- [ ] **REPL-03**: sum/add — summation over index
- [ ] **REPL-04**: Built-in function dispatch (aqprod, etaq, prodmake, etc.)
- [ ] **REPL-05**: series/coeffs display, set_trunc
- [ ] **REPL-06**: REPL loop with basic history

### Acceptance Tests

- [ ] **TEST-01**: Rogers-Ramanujan — prodmake yields denominators at ±1 mod 5
- [ ] **TEST-02**: Partition function — 1/etaq(1,50) coefficients correct
- [ ] **TEST-03**: Theta as eta — etamake(theta3), etamake(theta4) match SPEC
- [ ] **TEST-04**: Jacobi product — jacprodmake on Rogers-Ramanujan → JAC(0,5,∞)/JAC(1,5,∞)
- [ ] **TEST-05**: Gauss AGM — findhom yields X₁²+X₂²-2X₃², -X₁X₂+X₄²
- [ ] **TEST-06**: Sift + eta — Rødseth identity
- [ ] **TEST-07**: qfactor — T(8,8) factorization
- [ ] **TEST-08**: Watson's modular equation — findnonhomcombo
- [ ] **TEST-09**: Euler pentagonal dissection — jacprodmake on sifted etaq(1)

## v1.1 Requirements (Garvan Demo)

Requirements for milestone v1.1 — demo using Frank Garvan's qseriesdoc examples.

### Demo

- [ ] **DEMO-01**: Demo artifact exists — script or doc that runs qseries commands to reproduce qseriesdoc examples
- [ ] **DEMO-02**: Rogers-Ramanujan example — sum + prodmake, matches Output (1) and (2) style from qseriesdoc §3.1
- [ ] **DEMO-03**: Product conversion examples — qfactor T(8,8), etamake(theta2/3/4), jacprodmake on Rogers-Ramanujan (qseriesdoc §3.2, §3.3, §3.4)
- [ ] **DEMO-04**: Relations examples — findhom (Gauss AGM), findhomcombo, findnonhomcombo (qseriesdoc §4)
- [ ] **DEMO-05**: Sifting and product identities — sift, triple product, quintuple product, Winquist (qseriesdoc §5, §6)

## v1.2 Requirements (More QoL)

Requirements for milestone v1.2 — quality-of-life improvements.

### Script Mode

- [ ] **QOL-01**: User can run `qseries < script.qs` to execute a file of commands non-interactively

### Help

- [ ] **QOL-02**: User can type `help` to see general help and list of built-ins
- [ ] **QOL-03**: User can type `help(func)` to see documentation for a specific built-in function

### Timing

- [ ] **QOL-04**: User sees elapsed time for each command (ms or s)

### Multi-line Input

- [ ] **QOL-05**: User can use backslash continuation to split long expressions across lines

### Tab Completion

- [ ] **QOL-06**: User can press Tab to autocomplete identifiers and built-in function names

### Error Messages

- [ ] **QOL-07**: Parse and runtime errors show clearer diagnostics (e.g., line/column, expected token, function name)

### Demo Packaging

- [ ] **QOL-08**: User can obtain a single folder (binary + demo script + README) for sharing or distribution

## v1.4 Requirements (Close more gaps)

Requirements for milestone v1.4 — additional Maple qseries gap-closure from FEATURE-GAPS.md.

### Product Conversion

- [ ] **GAP-01**: User can call `mprodmake(f,T)` to convert q-series f to product (1+q^n1)*(1+q^n2)*...

### Validation Utilities

- [ ] **GAP-02**: User can call `checkprod(f,T)` to check whether q-series f is a "nice" product
- [ ] **GAP-03**: User can call `checkmult(f,T)` to check whether coefficients of f are multiplicative

### Relations

- [ ] **GAP-04**: User can call `findmaxind(L,n,topshift)` to find a maximal independent subset of q-series in list L

---

## v1.5 Requirements (REPL improvements)

Requirements for milestone v1.5 — REPL UX: suppress output, arrow-key navigation, optional-arg variants.

### Output Suppression

- [ ] **REPL-OPTS-01**: User can suppress output by ending a statement with semicolon (e.g. `rr := sum(...);` stores without printing)

### Line Navigation

- [ ] **REPL-OPTS-02**: User can move cursor left/right within the input line using arrow keys (when stdin is TTY)

### Optional Arguments

- [ ] **REPL-OPTS-03**: All 2-arg and 3-arg function variants work; omitted optional args use defaults (env.T, etc.) — audit series, etaq, qfactor, jac2series, checkprod, checkmult, findmaxind

---

## v2 Requirements

Deferred to future release.

### Deferred

- **OPT-01**: Memoization of etaq(k,T)
- **OPT-02**: Euler pentagonal optimization for etaq(1,T)
- **OPT-03**: LaTeX output

## Out of Scope

| Feature | Reason |
|---------|--------|
| Floating point in math pipeline | Exact rationals only; corrupts coefficients |
| External libraries (GMP, Boost) | Zero-dependency constraint |
| General-purpose CAS | Scope: q-series, partition theory, modular forms only |
| Full readline/libedit | External dependency; simple history sufficient |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| CORE-01 | Phase 1 | Pending |
| CORE-02 | Phase 1 | Pending |
| CORE-03 | Phase 2 | Pending |
| CORE-04 | Phase 3 | Pending |
| CORE-05 | Phase 3 | Pending |
| CORE-06 | Phase 4 | Pending |
| QFUNCS-01 | Phase 4 | Pending |
| QFUNCS-02 | Phase 4 | Pending |
| QFUNCS-03 | Phase 4 | Pending |
| QFUNCS-04 | Phase 4 | Pending |
| QFUNCS-05 | Phase 4 | Pending |
| QFUNCS-06 | Phase 4 | Pending |
| CONVERT-01 | Phase 5 | Pending |
| CONVERT-02 | Phase 6 | Pending |
| CONVERT-03 | Phase 6 | Pending |
| CONVERT-04 | Phase 6 | Pending |
| CONVERT-05 | Phase 6 | Pending |
| LINALG-01 | Phase 7 | Pending |
| LINALG-02 | Phase 7 | Pending |
| REL-01 | Phase 8 | Pending |
| REL-02 | Phase 8 | Pending |
| REL-03 | Phase 8 | Pending |
| REL-04 | Phase 8 | Pending |
| REPL-01 | Phase 9 | Pending |
| REPL-02 | Phase 10 | Pending |
| REPL-03 | Phase 10 | Pending |
| REPL-04 | Phase 10 | Pending |
| REPL-05 | Phase 10 | Pending |
| REPL-06 | Phase 10 | Pending |
| TEST-01 | Phase 5 | Pending |
| TEST-02 | Phase 4 | Pending |
| TEST-03 | Phase 6 | Pending |
| TEST-04 | Phase 6 | Pending |
| TEST-05 | Phase 8 | Pending |
| TEST-06 | Phase 6 | Pending |
| TEST-07 | Phase 6 | Pending |
| TEST-08 | Phase 8 | Pending |
| TEST-09 | Phase 6 | Pending |
| DEMO-01 | Phase 11 | Pending |
| DEMO-02 | Phase 12 | Pending |
| DEMO-03 | Phase 13 | Pending |
| DEMO-04 | Phase 14 | Pending |
| DEMO-05 | Phase 15 | Pending |
| QOL-01 | Phase 16 | Pending |
| QOL-02 | Phase 17 | Pending |
| QOL-03 | Phase 17 | Pending |
| QOL-04 | Phase 18 | Pending |
| QOL-05 | Phase 19 | Pending |
| QOL-06 | Phase 20 | Pending |
| QOL-07 | Phase 21 | Pending |
| QOL-08 | Phase 22 | Pending |
| GAP-01 | Phase 24 | Pending |
| GAP-02 | Phase 25 | Pending |
| GAP-03 | Phase 25 | Pending |
| GAP-04 | Phase 26 | Pending |
| REPL-OPTS-01 | Phase 27 | Pending |
| REPL-OPTS-02 | Phase 28 | Pending |
| REPL-OPTS-03 | Phase 29 | Pending |
| PERF-01 | Phase 32 | Pending |
| PERF-02 | Phase 33 | Pending |
| PERF-03 | Phase 32, 33 | Pending |

**Coverage:**
- v1 requirements: 38 total
- v1.1 (DEMO) requirements: 5 total
- v1.2 (QOL) requirements: 8 total
- v1.4 (GAP) requirements: 4 total
- v1.5 (REPL-OPTS) requirements: 3 total
- v1.6 (PERF) requirements: 3 total
- Mapped to phases: 3/3 v1.6 PERF requirements ✓

---

## v1.6 Requirements (etaq Performance)

Requirements for milestone v1.6 — etaq memoization and pentagonal optimization.

### Performance

- [ ] **PERF-01**: `etaq(k,T)` results are memoized — repeated calls with the same `(k,T)` return cached Series without recomputation
- [ ] **PERF-02**: `etaq(1,T)` uses Euler's pentagonal number theorem recurrence, reducing complexity from O(T^2) to O(T*sqrt(T))
- [ ] **PERF-03**: All existing acceptance tests still pass after optimizations (no regressions)

---

## v1.7 Requirements (Polish & Documentation)

Requirements for milestone v1.7 — compiler warning cleanup and user-facing documentation.

### Polish

- [ ] **POLISH-01**: Build with `-Wall -Wextra -Wpedantic` produces zero warnings
- [ ] **POLISH-02**: All existing acceptance tests still pass after warning fixes (no regressions)

### Documentation

- [ ] **DOC-01**: `MANUAL.md` exists with getting-started guide, all built-in functions documented with signatures, descriptions, and examples
- [ ] **DOC-02**: `MANUAL.md` includes workflow tutorials (Rogers-Ramanujan walkthrough, product identification, relation finding)

---

## Traceability (v1.7)

| Requirement | Phase | Status |
|-------------|-------|--------|
| POLISH-01 | Phase 34 | Pending |
| POLISH-02 | Phase 34 | Pending |
| DOC-01 | Phase 35 | Pending |
| DOC-02 | Phase 35 | Pending |

---

## v1.8 Requirements (More Math Functions)

Requirements for milestone v1.8 — expose internal helpers, add convenience functions, and math enrichment.

### Expose Internal Helpers

- [ ] **MATH-01**: `divisors(n)` exposed as REPL built-in — returns sorted list of positive divisors
- [ ] **MATH-02**: `mobius(n)` exposed as REPL built-in — returns Mobius function μ(n)
- [ ] **MATH-03**: `euler_phi(n)` exposed as REPL built-in — returns Euler's totient φ(n)

### Convenience Functions

- [ ] **MATH-04**: `coeff(f, n)` — extract single coefficient of q^n from series f
- [ ] **MATH-05**: `dissect(f, m, T)` — return list of all m siftings [sift(f,m,0,T), ..., sift(f,m,m-1,T)]
- [ ] **MATH-06**: `jacobi(a, n)` — Jacobi symbol (a/n) generalizing Legendre to composite odd n
- [ ] **MATH-07**: `kronecker(a, n)` — Kronecker symbol generalizing Jacobi to all integers n

### Math Enrichment

- [ ] **MATH-08**: `eisenstein(k, T)` — normalized Eisenstein series E_{2k}(q) = 1 - (4k/B_{2k}) Σ σ_{2k-1}(n) q^n
- [ ] **MATH-09**: `partition(n)` — direct computation of partition number p(n)
- [ ] **MATH-10**: `qdiff(f)` — formal q-derivative of series f

### Regression

- [ ] **MATH-11**: All existing acceptance tests still pass after additions

---

## Traceability (v1.8)

| Requirement | Phase | Status |
|-------------|-------|--------|
| MATH-01, MATH-02, MATH-03 | Phase 36 | Pending |
| MATH-04, MATH-05, MATH-06, MATH-07 | Phase 37 | Pending |
| MATH-08, MATH-09, MATH-10 | Phase 38 | Pending |
| MATH-11 | Phase 36, 37, 38 | Pending |

---

## v1.9 Requirements (Manual Update & Testing)

Requirements for milestone v1.9 — update documentation and add acceptance tests for v1.8 functions.

### Documentation

- [ ] **DOC-03**: MANUAL.md updated with all 10 new v1.8 built-ins (coeff, dissect, divisors, eisenstein, euler_phi, jacobi, kronecker, mobius, partition, qdiff) — each with signature, description, and example
- [ ] **DOC-04**: MANUAL.md version references updated to 1.9

### Testing

- [ ] **TEST-01**: Acceptance test script covering all 10 new v1.8 functions with correct expected output
- [ ] **TEST-02**: All existing acceptance tests still pass after additions

---

## Traceability (v1.9)

| Requirement | Phase | Status |
|-------------|-------|--------|
| DOC-03, DOC-04 | Phase 39 | Pending |
| TEST-01, TEST-02 | Phase 40 | Pending |

---

## v2.0 Requirements (Robustness & Tutorial Coverage)

Requirements for milestone v2.0 — fix critical bugs, harden input validation, improve Garvan tutorial coverage.

### Robustness

- [ ] **ROBUST-01**: `etaq(k, T)` validates k > 0; throws clear error for k <= 0
- [ ] **ROBUST-02**: `sift(f, n, k, T)` validates n > 0; throws clear error for n <= 0
- [ ] **ROBUST-03**: Integer division by zero in expressions throws "division by zero" error (no UB/crash)
- [ ] **ROBUST-04**: Parser has recursion depth limit (~500); throws error on excessively nested input
- [ ] **ROBUST-05**: Integer literals exceeding int64_t range detected and reported as error
- [ ] **ROBUST-06**: `Series::pow(INT_MIN)` handled safely; no signed integer overflow UB
- [ ] **ROBUST-07**: `std::bad_variant_access` never leaks to user; replaced with descriptive error messages
- [ ] **ROBUST-08**: Unknown built-in suggests closest match (Levenshtein distance or prefix match)
- [ ] **ROBUST-09**: Integer-valued built-in results (mobius, euler_phi, etc.) can be assigned to variables

### Tutorial Coverage

- [ ] **TUTORIAL-01**: `T_rn(r, n, T)` uses memoization; T(0, 30, 50) completes in reasonable time
- [ ] **TUTORIAL-02**: `min(a, b, ...)` and `max(a, b, ...)` built-ins available for convenience

---

## Traceability (v2.0)

| Requirement | Phase | Status |
|-------------|-------|--------|
| ROBUST-01 through ROBUST-09 | Phase 41 | Pending |
| TUTORIAL-01, TUTORIAL-02 | Phase 42 | Pending |

---

## v2.1 Requirements (Website)

Requirements for milestone v2.1 — documentation website with live WebAssembly playground.

### Wasm Build

- [ ] **WASM-01**: C++ compiles to WebAssembly via Emscripten with `evaluate(expr) → string` API (Embind)
- [ ] **WASM-02**: C++ exception handling works in Wasm — errors return messages, not module abort
- [ ] **WASM-03**: Wasm binary is <1.5 MB compressed (gzip/brotli)
- [ ] **WASM-04**: Computations run in a Web Worker — UI never freezes during long operations

### Documentation Site

- [ ] **SITE-01**: Astro Starlight project in `website/` folder with responsive layout, dark mode, sidebar navigation
- [ ] **SITE-02**: Landing page with feature highlights, download link, project overview
- [ ] **SITE-03**: MANUAL.md converted to HTML documentation sections
- [ ] **SITE-04**: Garvan tutorial examples with KaTeX math rendering and commentary
- [ ] **SITE-05**: Full-text search via Starlight's built-in Pagefind

### Playground

- [ ] **PG-01**: Dedicated playground page with terminal-style UI (xterm.js)
- [ ] **PG-02**: Preloaded example dropdown (Rogers-Ramanujan, prodmake, theta functions, etc.)
- [ ] **PG-03**: Loading indicator while Wasm downloads, computing indicator while executing

### Deployment

- [ ] **DEPLOY-01**: Site deploys to Cloudflare Pages with correct `.wasm` MIME type
- [ ] **DEPLOY-02**: GitHub Actions CI/CD pipeline: push → build Wasm → build site → deploy

### Future (deferred from v2.1)

- Shareable permalinks (URL hash encoding)
- Inline "Try It" buttons in documentation
- Custom domain DNS setup
- Terminal mode upgrade (xterm-pty for full interactive REPL)
- LocalStorage draft persistence

---

## Traceability (v2.1)

| Requirement | Phase | Status |
|-------------|-------|--------|
| WASM-01 | Phase 43 | Pending |
| WASM-02 | Phase 43 | Pending |
| WASM-03 | Phase 43 | Pending |
| WASM-04 | Phase 46 | Pending |
| SITE-01 | Phase 44 | Pending |
| SITE-02 | Phase 45 | Pending |
| SITE-03 | Phase 45 | Pending |
| SITE-04 | Phase 45 | Pending |
| SITE-05 | Phase 44 | Pending |
| PG-01 | Phase 46 | Pending |
| PG-02 | Phase 46 | Pending |
| PG-03 | Phase 46 | Pending |
| DEPLOY-01 | Phase 47 | Pending |
| DEPLOY-02 | Phase 47 | Pending |

**Coverage:** 14/14 v2.1 requirements mapped ✓

---

## v3.0 Requirements (Simple Website & Example Audit)

Requirements for milestone v3.0 — replace complex website with single HTML page, audit all examples.

### Cleanup

- [ ] **CLEAN-01**: Delete entire `website/` directory (Astro Starlight, playground, node_modules, etc.)
- [ ] **CLEAN-02**: Delete `.github/workflows/deploy.yml` and `website/public/_headers` (Cloudflare CI/CD no longer needed)
- [ ] **CLEAN-03**: Remove `wasm-website` Makefile target (no website to copy Wasm into)

### Single-Page Website

- [ ] **PAGE-01**: Single self-contained HTML file with kangaroo-banner hero section (sunset gradient, hopping kangaroo SVG, stars, boomerang, dust particles)
- [ ] **PAGE-02**: Build instructions section: prerequisites, one-liner compile command, platform notes
- [ ] **PAGE-03**: Key examples section: Rogers-Ramanujan, partition function, theta functions, relation finding — with REPL input/output
- [ ] **PAGE-04**: Full function reference: all 40+ built-in functions with signature, description, and example
- [ ] **PAGE-05**: Design follows kangaroo-banner aesthetic throughout: dark background (#0d0617), sunset accents, gold (#ffe066) headings, Bebas Neue + Outfit fonts

### Example Audit

- [ ] **AUDIT-01**: Every command in `demo/garvan-demo.sh` runs and produces correct output
- [ ] **AUDIT-02**: Every example in `MANUAL.md` runs and produces correct output
- [ ] **AUDIT-03**: Every code snippet in the single-page website runs and produces correct output
- [ ] **AUDIT-04**: Fix any examples that fail or produce incorrect output

---

## Traceability (v3.0)

| Requirement | Phase | Status |
|-------------|-------|--------|
| CLEAN-01 | Phase 48 | Pending |
| CLEAN-02 | Phase 48 | Pending |
| CLEAN-03 | Phase 48 | Pending |
| PAGE-01 | Phase 49 | Pending |
| PAGE-02 | Phase 49 | Pending |
| PAGE-03 | Phase 49 | Pending |
| PAGE-04 | Phase 49 | Pending |
| PAGE-05 | Phase 49 | Pending |
| AUDIT-01 | Phase 50 | Pending |
| AUDIT-02 | Phase 50 | Pending |
| AUDIT-03 | Phase 50 | Pending |
| AUDIT-04 | Phase 50 | Pending |

**Coverage:** 12/12 v3.0 requirements mapped ✓

---

## v4.0 Requirements (Core Improvements)

Requirements for milestone v4.0 — REPL UX polish and performance optimizations.

### REPL UX

- [ ] **UX-01**: ANSI colored output — prompt in gold, errors in red, timing in dim; auto-disabled for pipes/script mode; respects `NO_COLOR` env var
- [ ] **UX-02**: Clear screen — Ctrl+L in raw terminal mode and `clear` command; works on Unix and Windows
- [ ] **UX-03**: History persistence — command history saved to `~/.qseries_history` on exit, loaded on startup; max 1000 lines
- [ ] **UX-04**: Tab completion: longest-common-prefix fill — multiple candidates share common prefix, fill to common prefix before listing
- [ ] **UX-05**: Tab completion: auto-parentheses — completing a function name appends `(` automatically
- [ ] **UX-06**: Save/load session — `save(name)` serializes environment to `.qsession` file; `load(name)` restores variables; human-readable text format with version header

### Performance

- [ ] **PERF-04**: Series multiplication early-break — skip inner loop iterations where exponent sum exceeds truncation; 2-4x speedup
- [ ] **PERF-05**: Karatsuba BigInt multiplication — hybrid schoolbook/Karatsuba with threshold at ~32 limbs; O(n^1.585) for large operands
- [ ] **PERF-06**: Benchmarking suite — `bench_main.cpp` with micro-benchmarks for BigInt multiply, Series multiply, etaq, prodmake; median reporting; `DoNotOptimize` barriers

### Regression

- [ ] **REG-01**: All existing acceptance tests pass after all changes
- [ ] **REG-02**: Script mode output unchanged (no ANSI codes in piped output)

---

## Traceability (v4.0)

| Requirement | Phase | Status |
|-------------|-------|--------|
| UX-01 | Phase 51 | Pending |
| UX-02 | Phase 51 | Pending |
| UX-03 | Phase 56 | Pending |
| UX-04 | Phase 55 | Pending |
| UX-05 | Phase 55 | Pending |
| UX-06 | Phase 56 | Pending |
| PERF-04 | Phase 53 | Pending |
| PERF-05 | Phase 52 | Pending |
| PERF-06 | Phase 54 | Pending |
| REG-01 | Phase 56 | Pending |
| REG-02 | Phase 51 | Pending |

**Coverage:** 11/11 v4.0 requirements mapped ✓

---

## v4.1 Requirements (Distribution)

Requirements for milestone v4.1 — Docker image, install script, and CI/CD release pipeline.

### Docker

- [ ] **DIST-01**: Dockerfile with multi-stage build — gcc build stage, scratch runtime, image < 5MB
- [ ] **DIST-02**: Image published to ghcr.io — `docker run -it ghcr.io/gerardgarvan/qseries3` launches REPL
- [ ] **DIST-03**: Makefile docker targets — `make docker-build` and `make docker-run` for local workflow

### Install Script

- [ ] **DIST-04**: One-liner install — `curl -fsSL <url> | sh` detects platform (Linux/macOS), downloads binary to ~/.local/bin
- [ ] **DIST-05**: SHA256 checksum verification — install script verifies download integrity against published checksums
- [ ] **DIST-06**: Version selection — `install.sh --version v2.1` installs specific version (defaults to latest)

### CI/CD

- [ ] **DIST-07**: Tag-triggered release builds — pushing `v*` tag builds binaries and creates GitHub Release with assets
- [ ] **DIST-08**: Cross-platform build matrix — produces Linux x86_64 and macOS arm64 binaries
- [ ] **DIST-09**: Automated Docker push — CI builds and pushes Docker image to ghcr.io on release

### Regression

- [ ] **REG-03**: All existing acceptance tests pass in Docker container

---

## Traceability (v4.1)

| Requirement | Phase | Status |
|-------------|-------|--------|
| DIST-01 | Phase 58 | Pending |
| DIST-02 | Phase 58 | Pending |
| DIST-03 | Phase 58 | Pending |
| DIST-04 | Phase 59 | Pending |
| DIST-05 | Phase 59 | Pending |
| DIST-06 | Phase 59 | Pending |
| DIST-07 | Phase 57 | Pending |
| DIST-08 | Phase 57 | Pending |
| DIST-09 | Phase 58 | Pending |
| REG-03 | Phase 58 | Pending |

**Coverage:** 10/10 v4.1 requirements mapped ✓

---

## v4.2 Requirements (Fix Block Failures)

Requirements for milestone v4.2 — fix 3 Maple checklist block failures and 3 dependent exercises.

### Q-Shift Arithmetic

- [ ] **QSHIFT-01**: Series addition allows mismatched q_shifts when the difference is an integer — coefficients are index-shifted to align
- [ ] **QSHIFT-02**: Block 25 (findpoly with theta2/theta3 quotients) passes in the Maple checklist test

### Fractional Jacobi Exponents

- [ ] **JAC-01**: JacFactor exponents support Frac values (not just int) — e.g., JAC(7,14,∞)^(1/2)
- [ ] **JAC-02**: Series::powFrac(Frac) computes fractional powers via generalized binomial series
- [ ] **JAC-03**: jac2series correctly reconstructs series from half-integer JAC exponents
- [ ] **JAC-04**: Blocks 13-14 (jacprodmake on Slater's identity) pass in the Maple checklist test

### Exercise Fixes

- [ ] **EX-01**: Exercise 4 b(q) computed using eta product identity b(q) = η(τ)³/η(3τ)
- [ ] **EX-02**: Exercise 9 N(q) computation attempted with findnonhomcombo
- [ ] **EX-03**: Exercise 10 findpoly works with q-shift-fixed theta series

### Regression

- [ ] **REG-04**: All existing acceptance tests and maple-checklist tests still pass

---

## Traceability (v4.2)

| Requirement | Phase | Status |
|-------------|-------|--------|
| QSHIFT-01 | Phase 63 | Pending |
| QSHIFT-02 | Phase 63 | Pending |
| JAC-01 | Phase 65 | Pending |
| JAC-02 | Phase 64 | Pending |
| JAC-03 | Phase 65 | Pending |
| JAC-04 | Phase 65 | Pending |
| EX-01 | Phase 66 | Pending |
| EX-02 | Phase 66 | Pending |
| EX-03 | Phase 66 | Pending |
| REG-04 | Phase 66 | Pending |

**Coverage:** 10/10 v4.2 requirements mapped ✓

---

## v4.3 Requirements (Modular Arithmetic)

Requirements for modular (mod p) series arithmetic and relation finding.

### Modular Series

- [ ] **MOD-01**: `modp(f, p)` reduces all series coefficients modulo prime p, returning a Series with integer coefficients in {0, ..., p-1}
- [ ] **MOD-02**: `nterms(f)` returns the count of non-zero coefficients in a series

### Modular Linear Algebra

- [ ] **MOD-03**: Gaussian elimination over F_p (modular inverse via extended GCD, kernel computation in Z/pZ)
- [ ] **MOD-04**: `findhommodp(L, p, n, topshift)` finds homogeneous polynomial relations among series with all arithmetic mod p

### Worksheet Verification

- [ ] **MOD-05**: Garvan's mod-7 eta dissection worksheet reproduces all numbered outputs (1)-(7) in the qseries3 REPL

## Traceability (v4.3)

| Requirement | Phase | Status |
|-------------|-------|--------|
| MOD-01 | Phase 67 | Pending |
| MOD-02 | Phase 67 | Pending |
| MOD-03 | Phase 67 | Pending |
| MOD-04 | Phase 67 | Pending |
| MOD-05 | Phase 68 | Pending |

**Coverage:** 5/5 v4.3 requirements mapped ✓

---

---

## v4.4 Requirements (Partition Statistics)

Requirements for partition rank and crank generating functions.

### Rank

- [ ] **STAT-01**: `rankgf(m, T)` returns Σ N(m,n) q^n where N(m,n) is the number of partitions of n with rank m
- [ ] **STAT-02**: Rank symmetry holds: rankgf(m, T) = rankgf(-m, T)

### Crank

- [ ] **STAT-03**: `crankgf(m, T)` returns Σ M(m,n) q^n where M(m,n) is the number of partitions of n with crank m
- [ ] **STAT-04**: Crank symmetry holds: crankgf(m, T) = crankgf(-m, T)
- [ ] **STAT-05**: Andrews-Garvan convention: M(0,1) = -1

### Partition identity

- [ ] **STAT-06**: Σ_m N(m,n) = p(n) for all n (rank partition identity)
- [ ] **STAT-07**: Σ_m M(m,n) = p(n) for all n (crank partition identity)

## Traceability (v4.4)

| Requirement | Phase | Status |
|-------------|-------|--------|
| STAT-01 | Phase 69 | Pending |
| STAT-02 | Phase 69 | Pending |
| STAT-03 | Phase 69 | Pending |
| STAT-04 | Phase 69 | Pending |
| STAT-05 | Phase 69 | Pending |
| STAT-06 | Phase 69 | Pending |
| STAT-07 | Phase 69 | Pending |

**Coverage:** 7/7 v4.4 requirements mapped ✓

---

---

## v5.0 Requirements (t-core Package)

Requirements for implementing Garvan's Maple tcore package — partition t-cores, t-quotients, GSK bijection, vector representations, and display.

### Infrastructure

- [ ] **INFRA-01**: Partition type (`struct Partition`) as new variant in `EvalResult` and `EnvValue`; assignable to variables
- [ ] **INFRA-02**: List literals `[1,2,3,4]` evaluate to Partition values (non-decreasing order); empty list `[]` supported
- [ ] **INFRA-03**: `partitions(n)` enumerates all partitions of n, returning a displayable list (with reasonable n limit)
- [ ] **INFRA-04**: Partition display as `[λ₁, λ₂, ..., λ_k]`; partition conjugate utility function

### Core t-core

- [ ] **TCORE-01**: `rvec(ptn, t, k)` counts nodes in t-residue diagram colored k
- [ ] **TCORE-02**: `istcore(ptn, t)` tests whether a partition is a t-core (returns boolean)
- [ ] **TCORE-03**: `tcoreofptn(ptn, t)` computes the t-core of a partition
- [ ] **TCORE-04**: `tquot(ptn, t)` computes the t-quotient (list of t partitions)
- [ ] **TCORE-05**: `tcores(t, n)` lists all t-cores of n

### GSK Bijection

- [ ] **GSK-01**: `phi1(ptn, t)` returns [t-core, t-quotient] pair
- [ ] **GSK-02**: `invphi1([core, quotient], t)` reconstructs the original partition
- [ ] **GSK-03**: Size identity holds: |λ| = |core| + t × Σ|quotient_i|

### Vectors

- [x] **VEC-01**: `ptn2nvec(ptn, t)` computes the n-vector of a t-core
- [x] **VEC-02**: `nvec2ptn(nvec)` reconstructs a t-core from its n-vector
- [x] **VEC-03**: `ptn2rvec(ptn, t)` computes the r-vector
- [x] **VEC-04**: `nvec2alphavec(nvec)` converts n-vector to alpha-vector (t=5 and t=7)

### Statistics

- [x] **STAT-08**: `tcrank(ptn, t)` computes the t-core crank (mod t), handling overflow via modular exponentiation

### Display

- [x] **DISP-01**: `tresdiag(ptn, t)` prints the t-residue diagram
- [x] **DISP-02**: `makebiw(ptn, t, m)` prints the bi-infinite words W₀ through W_{t-1}

## Traceability (v5.0)

| Requirement | Phase | Status |
|-------------|-------|--------|
| INFRA-01 | Phase 70 | Done |
| INFRA-02 | Phase 70 | Done |
| INFRA-03 | Phase 70 | Done |
| INFRA-04 | Phase 70 | Done |
| TCORE-01 | Phase 71 | Done |
| TCORE-02 | Phase 71 | Done |
| TCORE-03 | Phase 71 | Done |
| TCORE-04 | Phase 72 | Done |
| TCORE-05 | Phase 71 | Done |
| GSK-01 | Phase 72 | Done |
| GSK-02 | Phase 72 | Done |
| GSK-03 | Phase 72 | Done |
| VEC-01 | Phase 73 | Done |
| VEC-02 | Phase 73 | Done |
| VEC-03 | Phase 73 | Done |
| VEC-04 | Phase 73 | Done |
| STAT-08 | Phase 74 | Done |
| DISP-01 | Phase 74 | Done |
| DISP-02 | Phase 74 | Done |

**Coverage:** 19/19 v5.0 requirements mapped ✓

---

## Milestone v6.0: Gaps Packages (Phases 76–87)

### Partition Statistics & Overpartitions
- [x] **PSTAT-01**: `drank(ptn)` — Dyson rank (largest part minus number of parts) — Done
- [x] **PSTAT-02**: `agcrank(ptn)` — Andrews-Garvan crank (µ(π) or l(π) - µ(π) form) — Done
- [x] **PSTAT-03**: Partition predicates: `ptnDP`, `ptnOP`, `ptnRR`, `ptnCC`, `ptnSCHUR`, `ptnOE` — Done
- [x] **PSTAT-04**: `overptns(n)` — enumerate all overpartitions of n as [dptn, ptn] pairs — Done
- [x] **PSTAT-05**: `overptnrank(optn)`, `overptncrank(optn)` — overpartition rank and crank — Done
- [x] **PSTAT-06**: Counting functions: `PDP(n)`, `POE(n)`, `PRR(n)`, `PSCHUR(n)` — Done
- [x] **PSTAT-07**: `kapPD(ptn)`, `lamPD(ptn)`, `numLE(ptn)` — partition statistics helpers — Done

### Mock Theta Functions
- [x] **MOCK-01**: `mockqs(name, order, T)` — q-series expansion of any mock theta function — Done
- [x] **MOCK-02**: Registry of all 44 Ramanujan mock theta functions (orders 2,3,5,6,7,8,10) — Done
- [x] **MOCK-03**: `mockdesorder(m)` — list mock theta function names by order — Done

### Misc Utilities
- [x] **MISC-01**: `newprodmake(f, T)` — enhanced prodmake handling arbitrary leading terms — Done
- [x] **MISC-02**: `EISENq(k, T)` — Eisenstein series q-expansion via Bernoulli/sigma — Done
- [x] **MISC-03**: `Phiq(j, T)` — divisor sum generating function Σ σ_j(n) q^n — Done
- [x] **MISC-04**: `dilly(f, d)` — q-dilation (substitute q → q^d in series) — Done
- [x] **MISC-05**: `sieveqcheck(f, p)` — check if all exponents in one residue class mod p — Done
- [x] **MISC-06**: `siftfindrange(f, p, T)` — find residue class with fewest/most terms — Done
- [x] **MISC-07**: `polyfind(n0, n1, n2, T)` — fit quadratic through three coefficient values — Done

### Crank/Rank Tables
- [ ] **CRANK-01**: SPT-crank table: `NS(m,n)`, `sptcrankresnum(k,r,n)`
- [ ] **CRANK-02**: Overpartition crank table: `MBAR(m,n)`, `ocrankresnum(k,r,n)`
- [ ] **CRANK-03**: `compute_overpartition_numbers(T)` — pentagonal recurrence variant
- [ ] **CRANK-04**: M2-rank/crank/orank tables via partition enumeration: `M2N`, `M2M`, `M2NBAR`
- [ ] **CRANK-05**: Residue GFs: `GFDM2N(k1,k2,t,r)`, `GFDM2M(...)`, `GFDM2NBAR(...)`

### Bailey Chains
- [ ] **BAILEY-01**: `betafind(alpha, n)` and `alphafind(beta, n)` — Bailey pair computation
- [ ] **BAILEY-02**: `alphaup`, `alphadown`, `betaup`, `betadown` — Bailey chain operations
- [ ] **BAILEY-03**: Catalog of known Bailey pairs (unit pair, Rogers-Ramanujan, etc.)

### ETA Cusp Theory (Gamma_0)
- [ ] **ETA-01**: `cuspmake(N)` — inequivalent cusps for Gamma_0(N)
- [ ] **ETA-02**: `cuspord(GP, cusp)` — Ligozat's order formula for eta-quotients at cusps
- [ ] **ETA-03**: `gammacheck(GP, N)` — Newman's 5 conditions for modular functions on Gamma_0(N)
- [ ] **ETA-04**: `cuspORDS(GP, N)` — orders at all cusps, `mintotGAMMA0ORDS`
- [ ] **ETA-05**: `etaprodtoqseries(GP, T)` — q-expansion of eta-quotient from GP list
- [ ] **ETA-06**: `vp(n, p)` — p-adic valuation

### ETA Identity Prover
- [ ] **ETA-07**: `provemodfuncGAMMA0id(etaid, N)` — prove eta-quotient identity on Gamma_0(N) via Sturm bound
- [ ] **ETA-08**: U_p operator: `provemodfuncGAMMA0UpETAid` — prove identity involving U_p sifting

### Modular Forms
- [x] **MF-01**: `makebasisM(k, T)` — basis of M_k(SL_2(Z)) using E4, E6
- [ ] **MF-02**: `makeALTbasisM(k, T)` — alternative basis using Delta_12 (deferred)
- [x] **MF-03**: `DELTA12(T)` — the cusp form q·η(τ)^24
- [x] **MF-04**: `makebasisPX(k, T)` — basis using partition function P and sigma sums

### Theta IDs (Gamma_1 Cusps)
- [x] **THETA-01**: `cuspmake1(N)` — cusps for Gamma_1(N)
- [x] **THETA-02**: `getacuspord(n, r, a, c)` — generalized eta cusp order (Biagioli)
- [x] **THETA-03**: `Gamma1ModFunc(L, N)` — modular function check for Gamma_1(N)
- [x] **THETA-04**: `QP2(num, den)` — second periodic Bernoulli
- [x] **THETA-05**: `eta2jac`, `jac2eprod` — notation conversions (jac2getaprod deferred)

### Theta IDs Identity Prover
- [ ] **THETA-06**: `provemodfuncid(jacid, N)` — prove identity on Gamma_1(N) via Sturm bound

### Ramanujan-Robins Identity Search
- [ ] **RRID-01**: `RRG(n)`, `RRH(n)`, `RRGstar(n)`, `RRHstar(n)` — Rogers-Ramanujan/Göllnitz-Gordon functions
- [ ] **RRID-02**: `checkid(expr, T)` — check if expression is an eta/theta product
- [ ] **RRID-03**: `findids(type, T)` — systematic identity search (types 1-10)

### Integration
- [ ] **INT-01**: Full regression: all existing tests pass
- [ ] **INT-02**: Cross-package integration: ETA+thetaids+modforms identity proofs work end-to-end

### v6.0 Requirements Traceability

| Req ID | Phase | Status |
|--------|-------|--------|
| PSTAT-01..07 | Phase 76 | Done |
| MOCK-01..03 | Phase 77 | Done |
| MISC-01..07 | Phase 78 | Done |
| CRANK-01..05 | Phase 79 | Done |
| BAILEY-01..03 | Phase 80 | Done |
| ETA-01..06 | Phase 81 | Done |
| ETA-07..08 | Phase 82 | Done |
| MF-01..04 | Phase 83 | Done (MF-02 makeALTbasisM deferred) |
| THETA-01..05 | Phase 84 | Done |
| THETA-06 | Phase 85 | Pending |
| RRID-01..03 | Phase 86 | Pending |
| INT-01..02 | Phase 87 | Pending |

**Coverage:** 44 v6.0 requirements across 12 phases

### v7.0 Maple Checklist Gaps

Requirements for closing remaining maple-checklist skips (in suggested order).

#### quinprod Identity Modes (Blocks 30–31)
- [ ] **GAP-QP-01**: `quinprod(z,q,prodid)` — returns quintuple product identity in product form (string formula)
- [ ] **GAP-QP-02**: `quinprod(z,q,seriesid)` — returns quintuple product identity in series form (string formula)

#### List Indexing (Block 21)
- [ ] **GAP-IDX-01**: User can index into list results: `x[1]` returns first element of findhom/findnonhom output

#### Symbolic z in Triple/Quintuple Products (Blocks 28, 32)
- [ ] **GAP-SYM-01**: `tripleprod(z,q,T)` with symbolic z — returns bivariate series in z and q (Laurent in z)
- [ ] **GAP-SYM-02**: `quinprod(z,q,T)` with symbolic z — returns bivariate series in z and q

### v7.0 Out of Scope (Deferred)

| Feature | Reason |
|---------|--------|
| factor(t8) — polynomial factorization | Maple's factor() into cyclotomics; requires polynomial factorization over Q. qfactor already handles q-product structure. |

### v7.0 Requirements Traceability

| Req ID | Phase | Status |
|--------|-------|--------|
| GAP-QP-01 | Phase 88 | Done |
| GAP-QP-02 | Phase 88 | Done |
| GAP-IDX-01 | Phase 89 | Done |
| GAP-SYM-01 | Phase 90 | Done |
| GAP-SYM-02 | Phase 90 | Done |

**Coverage:** 5/5 v7.0 requirements mapped ✓

---

## v8.0 RootOf Function

Requirements for RootOf / Q(ω) support — Maple Block 10 parity and qseriesdoc a(q), b(q), c(q).

### Algebraic Type (Omega3 / Q(ω))

- [ ] **ROOT-01**: Omega3 type — struct { Frac a, b } for a + b·ω with ω² = -ω-1
- [ ] **ROOT-02**: Omega3 arithmetic — +, -, *, / with normalization after each op
- [ ] **ROOT-03**: Omega3 division — (a+bω)⁻¹ = conjugate / norm; nonzero check
- [ ] **ROOT-04**: ω^k reduction — powers reduce to ω^(k mod 3)
- [ ] **ROOT-05**: Omega3 display — str() as `a + b*omega` or `omega`, `omega^2` for canonical forms

### Series with Q(ω) Coefficients

- [ ] **ROOT-06**: SeriesOmega — std::map<int, Omega3> parallel to Series
- [ ] **ROOT-07**: SeriesOmega arithmetic — add, mul, truncation propagation
- [ ] **ROOT-08**: Mixed ops — Omega3 * Series → SeriesOmega

### Parser and REPL Integration

- [ ] **ROOT-09**: omega symbol — built-in or RootOf(z²+z+1=0) parses to Omega3(0,1)
- [ ] **ROOT-10**: RootOf dispatch — RootOf(3) or RootOf(z²+z+1=0) returns Omega3(0,1)
- [ ] **ROOT-11**: sum(omega^expr) — evaluate ω^exponent → Omega3 in sum/add
- [ ] **ROOT-12**: EnvValue — omega := RootOf(...) stores Omega3 in variable environment

### Block 10 and b(q)

- [ ] **ROOT-13**: b(q) — compute via sum(omega^(n-m)*q^(n²+nm+m²)) or eta identity η³/η(3τ)
- [ ] **ROOT-14**: Block 10 parity — maple-checklist Block 10 (RootOf) no longer skips

### v8.0 Out of Scope (Deferred)

| Feature | Reason |
|---------|--------|
| General RootOf(poly) | Arbitrary algebraic extensions; polynomial ring and reduction needed |
| Cyclotomic ζₙ for n > 3 | Q(ζ₅) etc. have different degrees and reduction rules |
| findhom over Q(ω) | Kernel over Q(ω); linalg stays Q-only |
| prodmake/etamake on Q(ω) series | Andrews' algorithm assumes Q; use eta identity for b(q) product |
| convert(omega, radical) | Optional display; v1.x |

### v8.0 Requirements Traceability

| Req ID | Phase | Status |
|--------|-------|--------|
| ROOT-01..05 | Phase 91 | Pending |
| ROOT-06..08 | Phase 92 | Pending |
| ROOT-09..12 | Phase 93 | Pending |
| ROOT-13..14 | Phase 94 | Pending |

**Coverage:** 14/14 v8.0 requirements mapped ✓

---

## v9.0 factor(t8) + Close Block 4

Requirements for polynomial factorization (factor builtin) and maple-checklist Block 4 parity.

### factor Builtin

- [ ] **FACTOR-01**: factor(expr) — expr evaluates to Series; factor univariate polynomial in q into cyclotomic form
- [ ] **FACTOR-02**: FactorResult — struct with map<int,int> cyclotomic (n → exponent of Φ_n), Frac content; display string (Φ_n notation)
- [ ] **FACTOR-03**: Cyclotomic factorization — sufficient for T(8,8); via qfactor→cyclotomic expansion or direct Φ_n extraction

### Block 4 Parity

- [ ] **BLOCK4-01**: Block 4 no longer skips — factor(t8) where t8 = T(8,8) returns cyclotomic product matching qseriesdoc Output (4)

### v9.0 Out of Scope (Deferred)

| Feature | Reason |
|---------|--------|
| Berlekamp–Zassenhaus for non-cyclotomics | Block 4 only needs cyclotomics; general irreducibles deferred |
| factor over Q(ω) | Algebraic extension; out of scope for v9 |

### v9.0 Requirements Traceability

| Req ID | Phase | Status |
|--------|-------|--------|
| FACTOR-01..03 | Phase 95 | Pending |
| BLOCK4-01 | Phase 96 | Pending |

**Coverage:** 4/4 v9.0 requirements mapped ✓

---

## v10.0 Close Remaining Gaps

Requirements for Block 25 fix, Block 24, and mod-p relation finders.

### Block 25 Fix (findpoly q-shift)

- [ ] **BLOCK25-01**: User can run findpoly on theta2/theta3 quotients — series with different q-shifts are normalized before addition
- [ ] **BLOCK25-02**: maple-checklist Block 25 passes — findpoly(x, y, q, 3, 1, 60) for theta quotients produces polynomial relation

### Block 24 (collect)

- [ ] **BLOCK24-01**: User can group relation terms by variable (collect-style) — OR Block 24 documented as N/A and remains skip
- [ ] **BLOCK24-02**: maple-checklist Block 24 — either real test or explicit N/A rationale

### Mod-p Relations

- [ ] **MODP-01**: User can call findlincombomodp(f, L, p, T) — express f as linear combination of L mod p
- [ ] **MODP-02**: findhommodp exists (v4.2); findlincombomodp complements it for linear (degree 1) mod-p relations

### v10.0 Out of Scope (Deferred)

| Feature | Reason |
|---------|--------|
| findcong, findprod | Lower priority; mod-p family sufficient for v10 |
| LaTeX output | Deferred |
| zqfactor | Bivariate; high effort |

### v10.0 Requirements Traceability

| Req ID | Phase | Status |
|--------|-------|--------|
| BLOCK25-01..02 | Phase 97 | Pending |
| BLOCK24-01..02 | Phase 98 | Pending |
| MODP-01..02 | Phase 99 | Pending |

**Coverage:** 6/6 v10.0 requirements mapped ✓

---

*Requirements defined: 2025-02-24*
*Last updated: 2026-03-03 — v10.0 Close Remaining Gaps requirements added*
