# Coding Conventions

**Analysis Date:** 2025-03-04

## Naming Patterns

**Files:**
- Headers: `snake_case.h` (e.g., `series.h`, `theta_ids.h`, `series_omega.h`)

**Functions:**
- camelCase for REPL-facing built-ins: `prodmake`, `etamake`, `jacprodmake`, `findhom`, `provemodfunc`
- lowercase with underscores for helpers: `bigGcd`, `offsetToLineCol`, `kindToExpected`
- Inline free functions: `aqprod`, `etaq`, `theta3`, `sift`, `dilly`

**Variables:**
- camelCase or snake_case (mixed; e.g., `minExp`, `maxExp`, `q_shift`, `fail_count`)

**Types:**
- PascalCase for structs: `BigInt`, `Frac`, `Series`, `Omega3`, `SeriesOmega`, `Tokenizer`, `Parser`
- Result structs: `ProveModfuncResult`, `CheckprodResult`, `NewProdmakeResult`

## Code Style

**Formatting:**
- No formatter config detected (no .clang-format, .editorconfig)
- Indentation: 4 spaces
- Braces: K&R style (`if (cond) {`)

**Linting:**
- Compiler flags: `-Wall -Wextra -Wpedantic`
- No separate linter (clang-tidy, etc.)

## Import Organization

**Order:**
1. Project headers (bigint, frac, series, qfuncs, convert, etc.)
2. Standard library (`<iostream>`, `<string>`, `<vector>`, etc.)
3. Platform (termios, windows.h) guarded by `#ifdef`

**Path:**
- All includes: `#include "filename.h"` (no path prefix; headers in same `src/`)

## Error Handling

**Patterns:**
- Throw on invalid input: `throw std::runtime_error("func: message")`
- Division by zero: `throw std::invalid_argument("...")`
- REPL catches and prints to stderr

```cpp
// Example from qfuncs.h / convert.h
if (n <= 0) throw std::runtime_error("sift: modulus n must be positive, got " + std::to_string(n));
```

## Logging

**Framework:** None (stdout/stderr)

**Patterns:**
- `std::cerr` for warnings (e.g., `etamake: zero series`)
- `std::cout` for REPL output and unit-test PASS/FAIL

## Comments

**When to Comment:**
- Algorithm steps (e.g., prodmake recurrence, divisor extraction)
- Edge cases (e.g., `// j starts at 1, not 0` in inverse)
- Block headers in unit tests (`// --- Group 1: 0 * anything = 0 ---`)

**JSDoc/TSDoc:**
- Not applicable (C++)

## Function Design

**Size:** Functions vary; `repl.h` is very large (~2900 lines) with many built-in handlers.

**Parameters:** Pass by const reference for Series/Frac; truncation `T` as `int`.

**Return Values:**
- Result structs for multi-value returns: `NewProdmakeResult`, `ProveModfuncResult`
- `inline` free functions return by value

## Module Design

**Exports:**
- Headers export structs and inline functions; no explicit export lists
- Everything in headers is effectively global within the translation unit

**Barrel Files:**
- `repl.h` aggregates many domain headers (eta_cusp, theta_ids, modforms, bailey, mock, crank_tables, rr_ids, tcore)

## Layout Rules (from .cursorrules)

- Each layer = single .h file (header-only, `inline`)
- No virtual methods; use structs with methods
- Operator overloading for Series arithmetic: `f * g + h.pow(3)`
- Always propagate truncation: `result.trunc = min(a.trunc, b.trunc)`
- Always call `Frac::reduce()` after construction
- Use `std::map<int, Frac>` for Series (ordered iteration)

---

*Convention analysis: 2025-03-04*
