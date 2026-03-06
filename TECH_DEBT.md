# Tech Debt Inventory

One-pass lightweight inventory for future remediation. No fixes in this document.

## Hotspots

- **repl.h** (~3k LOC): REPL loop, eval, dispatch, display, TTY, history, help in one header; consider splitting
- **repl.h:770–2260** (dispatchBuiltin): ~100 `if (name == "foo")` branches; consider registration table
- **repl.h:2261** (evalExpr): ~180 lines, variant-heavy; many holds_alternative/std::get branches
- **qfuncs.h** (~724): Many q-series primitives and number-theory helpers; 100+ if/else-if patterns
- **convert.h** (~586): prodmake, etamake, jacprodmake; etamake ~150+ lines iterative loop
- **main.cpp** (~736): Mixed test driver + REPL entry; mixed concerns
- **mock.h:mockqs_term** (~280 lines): ~48 nested if/else-if by order (2,3,5,6,7,8,10) and name
- **theta_ids.h**: Complex math (getacuspord, srep, newxy, Scmake); gcd/frac logic
- **repl.h** includes 16 headers; any new math type touches Environment, evalExpr, evalStmt, display, getSeriesFromEnv

## Duplication

- **repl.h:770+** — Built-in arg checking: `args.size() != N` + `ev(i)`/`evi(i)` repeated across ~100 built-ins
- **repl.h:evalExpr** — Variant unwrapping: 6+ holds_alternative/get branches for EnvValue/EvalResult
- **repl.h:evalStmt** — Assign target: 8 variant branches for assignment targets
- **repl.h:getSeriesFromEnv** — 5 branches throwing different "holds X, not series" messages
- **mock.h:mockqs_term** — Same `if (order == N) { if (name == "x") {...} }` structure across 6 orders
- **repl.h:2343–2500** — formatProdmake, formatEtamake: similar exponent extraction and string logic
- **repl.h:runRepl** — TTY vs non-TTY: `if (stdin_is_tty())` duplicated for prompt, readLine, continuation, history
- **repl.h:~3075** — save/load parsing: manual `trimmed.substr(0,5)=="save("`; could use parser

## Brittle Areas

- **repl.h:773** — `env.env.at("q")` in dispatchBuiltin: assumes q exists; unset q throws
- **repl.h** — Magic constants: pow limit 10000, maxHistory 1000, maxContinuations 100, Levenshtein threshold 3
- **Platform** — `#ifdef __EMSCRIPTEN__`, `#ifdef _WIN32`, Cygwin conditionals; RawModeGuard only for POSIX
- **repl.h:66–82** — parseParserMessage: parses `"parser: line N, col M: message"`; format change would break
- **repl.h** — Help table vs dispatchBuiltin: getHelpTable and dispatchBuiltin both encode names/arity; two edits per new built-in
- **repl.h** — No registration table: dispatch is one long if-chain; typos fail as "unknown built-in"
