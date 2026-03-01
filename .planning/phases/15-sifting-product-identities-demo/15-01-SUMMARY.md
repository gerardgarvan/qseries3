# Phase 15-01 Summary: Sifting and Product Identities Demo

**Completed:** 2026-02-25  
**Plan:** 15-01-PLAN.md  
**Status:** Complete

## What Was Built

Added four run blocks to `demo/garvan-demo.sh` replacing the Phase 15 placeholder. Each block reproduces qseriesdoc §5 and §6 examples.

### Run Blocks Added

1. **§5 Sifting (Rødseth):** `set_trunc(200)`, `PD := etaq(2,200)/etaq(1,200)`, `PD1 := sift(PD,5,1,199)`, `etamake(PD1, 30)` — uses T=30 per Test 6 to avoid coefficient overflow.

2. **§6.1 Triple product (Euler pentagonal):** `set_trunc(60)`, `series(tripleprod(q,q^3,10), 60)` — Euler pentagonal expansion.

3. **§6.2 Quintuple + Euler dissection:** `set_trunc(500)`, `series(quinprod(q,q^5,20), 100)`, `EULER := etaq(1,500)`, `E0 := sift(EULER,5,0,499)`, `jp := jacprodmake(E0,50)`, `jac2prod(jp)` — Test 9 pattern with jac2prod.

4. **§6.3 Winquist identity:** `set_trunc(200)`, `series(winquist(q^5,q^3,q^11,20), 60)`.

## Files Modified

- `demo/garvan-demo.sh` — added four run blocks after `# === Sifting and product identities (qseriesdoc §5, §6) ===`
- `dist/demo/garvan-demo.sh` — synced with source

## Verification

- Plan tasks: Add four run blocks ✓
- Must-haves: sift+etamake (Rødseth), triple product, quintuple+Euler, Winquist ✓

**Manual verification:** Run `cd dist && bash demo/garvan-demo.sh` or `make demo` (from project root) to exercise all blocks. Commands match acceptance tests 6 (Rødseth) and 9 (Euler).

## Patterns Used

- `q^n` syntax for product functions (q^3, q^5, q^11) — parsed as `Series::qpow(n, env.T)`
- Variables defined in same `run()` as usage (each run is a separate process)
- `jac2prod(jp)` expects variable name — `jp` assigned in same run
