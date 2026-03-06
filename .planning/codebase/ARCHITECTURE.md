# Architecture

**Analysis Date:** 2025-03-04

## Pattern Overview

**Overall:** Layered pipeline — bottom-up dependency chain from arbitrary-precision integers to REPL.

**Key Characteristics:**
- Header-only C++ (no separate .cpp except main/bench/wasm)
- Single translation unit (`src/main.cpp`) compiles all headers
- Data flows upward: BigInt → Frac → Series → qfuncs → convert/relations → parser → REPL
- All math is exact rational; no floating point in the pipeline

## Layers

**Layer 1 — BigInt:**
- Purpose: Arbitrary precision signed integer
- Location: `src/bigint.h`
- Contains: `struct BigInt`, base 10^9 digits, `bigGcd`
- Depends on: std::vector, std::string, <cstdint>
- Used by: `frac.h`, `qfuncs.h` (Eisenstein table), `omega3.h`

**Layer 2 — Frac:**
- Purpose: Exact rational numbers (auto-reduce via GCD)
- Location: `src/frac.h`
- Contains: `struct Frac`
- Depends on: `bigint.h`
- Used by: `series.h`, `qfuncs.h`, `convert.h`, `linalg.h`, `relations.h`, `omega3.h`

**Layer 3 — Series:**
- Purpose: Truncated formal power series in q (std::map<int,Frac>)
- Location: `src/series.h`
- Contains: `struct Series`, `struct BivariateSeries`
- Depends on: `frac.h`
- Used by: `qfuncs.h`, `convert.h`, `relations.h`, `repl.h`, `eta_cusp.h`, `theta_ids.h`, `rr_ids.h`, `mock.h`, `bailey.h`, `modforms.h`, `crank_tables.h`
- Critical: `q_shift` supports fractional q-power offset; truncation propagated in all ops

**Layer 4 — Omega3 / SeriesOmega:**
- Purpose: Eisenstein integers Q(ω), ω³=1
- Location: `src/omega3.h`, `src/series_omega.h`
- Depends on: `frac.h`, `series.h`
- Used by: `repl.h` (cyclotomic, RootOf, omega expressions)

**Layer 5 — qfuncs:**
- Purpose: Q-series building blocks (aqprod, etaq, theta2/3/4, tripleprod, quinprod, winquist, eisenstein, number theory helpers)
- Location: `src/qfuncs.h`
- Depends on: `series.h`
- Used by: `convert.h`, `relations.h`, `eta_cusp.h`, `theta_ids.h`, `rr_ids.h`, `mock.h`, `bailey.h`, `modforms.h`, `crank_tables.h`, `repl.h`

**Layer 6 — convert:**
- Purpose: Product conversion (prodmake, etamake, jacprodmake, qfactor, sift, dilly, factor, checkprod, checkmult)
- Location: `src/convert.h`
- Depends on: `series.h`, `qfuncs.h`
- Used by: `relations.h`, `theta_ids.h`, `rr_ids.h`, `repl.h`

**Layer 7 — linalg:**
- Purpose: Rational Gaussian elimination, kernel over Q
- Location: `src/linalg.h`
- Depends on: `frac.h`
- Used by: `relations.h`

**Layer 8 — relations:**
- Purpose: findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly, findmaxind, findlincombomodp
- Location: `src/relations.h`
- Depends on: `series.h`, `linalg.h`, `frac.h`
- Used by: `repl.h`

**Layer 9 — Domain extensions:**
- `src/eta_cusp.h` — Cusp orders, provemodfunc
- `src/theta_ids.h` — provemodfuncid, theta identity proving
- `src/modforms.h` — Modular form helpers
- `src/bailey.h` — Bailey-style identities
- `src/mock.h` — Mock theta
- `src/crank_tables.h` — Crank computations
- `src/tcore.h` — t-core arithmetic
- `src/rr_ids.h` — Rogers-Ramanujan identity checking

**Layer 10 — parser:**
- Purpose: Tokenizer + recursive-descent expression parser
- Location: `src/parser.h`
- Contains: `Tokenizer`, `Parser`, `Expr`, `Stmt`
- Depends on: std::string, std::vector
- Used by: `repl.h`

**Layer 11 — repl:**
- Purpose: REPL loop, variable environment, built-in dispatch, help system, line editing
- Location: `src/repl.h`
- Depends on: parser, series, omega3, series_omega, frac, convert, qfuncs, tcore, mock, crank_tables, bailey, eta_cusp, modforms, theta_ids, relations, rr_ids
- Used by: `main.cpp`, `main_wasm.cpp`

## Data Flow

**REPL eval:**
1. User input → Tokenizer → Parser → Expr/Stmt
2. Stmt evaluated in Environment (variable map)
3. Expr → evalExpr() → Series, Omega3, or other values
4. Result printed via display routines

**prodmake (Andrews' algorithm):**
1. Extract coeffs b[0..T-1] from Series
2. Recurrence c[n] = n*b[n] - Σ b[n-j]*c[j]
3. Extract a[n] via divisor sum
4. Output product representation

**State Management:**
- Environment: `std::map<std::string, std::variant<Series, SeriesOmega, ...>>`
- No persistent state; REPL is stateless across invocations (except in-pipe session)

## Key Abstractions

**Series:**
- Sparse `std::map<int, Frac>` coefficients, `int trunc`, `Frac q_shift`
- Arithmetic: +, -, *, /, pow, inverse, subs_q

**Result structs:**
- `NewProdmakeResult`, `CheckprodResult`, `CheckmultResult`, `FactorResult` — `src/convert.h`
- `ProveModfuncResult`, `CuspOrd` — `src/eta_cusp.h`
- `ProveModfuncIdResult` — `src/theta_ids.h`
- `RelationKernelResult`, `FindlincombomodpResult` — `src/repl.h`
- `FindmaxindResult` — `src/relations.h`

## Entry Points

**main.cpp:**
- Location: `src/main.cpp`
- Triggers: Binary execution
- Responsibilities: `--version`, `--test` (runUnitTests), else `runRepl()`

**main_wasm.cpp:**
- Location: `src/main_wasm.cpp`
- Triggers: Emscripten module load
- Responsibilities: Expose REPL via `emscripten::bind`

**bench_main.cpp:**
- Location: `src/bench_main.cpp`
- Triggers: `make bench`
- Responsibilities: Time-critical operations (etaq, prodmake, etc.)

## Error Handling

**Strategy:** Exceptions (`std::runtime_error`, `std::invalid_argument`) for math/parse errors; caught at REPL and printed.

**Patterns:**
- `throw std::runtime_error("func: message")`
- `runtimeErr(func, msg)` helper in `repl.h`
- Division by zero, invalid truncation, non-eta-product convergence → exception

## Cross-Cutting Concerns

**Logging:** None (stdout/stderr only)
**Validation:** Inline checks before operations; throw on invalid args
**Authentication:** None

---

*Architecture analysis: 2025-03-04*
