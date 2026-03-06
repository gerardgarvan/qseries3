# Phase 99-01 Summary: findlincombomodp

## Status: COMPLETE

## What was done

1. **Added `solve_modp` to `linalg.h`** — Solves M·x = b over F_p. Returns `std::optional<std::vector<int64_t>>` (solution or nullopt if inconsistent). Extended `gauss_to_rref_modp` with `maxCols` parameter so augmented [M|b] pivots only on M's columns.

2. **Added `findlincombomodp` to `relations.h`** — Expresses f as linear combination of L mod p. Builds mod-p matrix from L, transposes, and solves `solve_modp(MT, f_coeffs, p)`. Returns `std::optional<std::vector<int64_t>>` (coefficients c_1..c_k or nullopt). Signature: `findlincombomodp(f, L, p, topshift=0)`.

3. **REPL integration** — Added `FindlincombomodpResult` struct, `formatFindlincombomodp`, dispatch for `findlincombomodp(f,L,p[,topshift])`, help entry, and display branch. Output format: `"c1 L1 + c2 L2 + ... (mod p)"` or `"no linear combination mod p"`.

4. **Acceptance tests** — Added two tests to `tests/acceptance-modp.sh`:
   - `findlincombomodp(etaq(1,30)+etaq(2,30), [etaq(1,30), etaq(2,30)], 7)` → `1 L1 + 1 L2 (mod 7)`
   - `findlincombomodp` runs without error for valid inputs

## Verification

- Build: `g++ -std=c++20 -O2 -o qseries main.cpp` (or `make`)
- `findlincombomodp(etaq(1,30)+etaq(2,30), [etaq(1,30), etaq(2,30)], 7)` outputs `1 L1 + 1 L2 (mod 7)`
- `help(findlincombomodp)` documents usage
- `bash tests/acceptance-modp.sh` — all tests pass

## Files modified

- `src/linalg.h` — added `solve_modp`, extended `gauss_to_rref_modp` with maxCols
- `src/relations.h` — added `findlincombomodp`
- `src/repl.h` — FindlincombomodpResult, formatFindlincombomodp, dispatch, help, display
- `tests/acceptance-modp.sh` — findlincombomodp tests
