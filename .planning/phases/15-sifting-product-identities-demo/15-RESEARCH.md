# Phase 15: Sifting and Product Identities Demo — Research

**Researched:** 2026-02-25  
**Domain:** Demo scripting, q-series REPL commands (sift, tripleprod, quinprod, winquist)  
**Confidence:** HIGH

## Summary

Phase 15 extends the garvan-demo.sh with run blocks that reproduce qseriesdoc §5 (sifting) and §6 (product identities). All required built-ins exist in repl.h; the work is assembling the correct REPL commands and matching demo structure to prior phases (12, 13, 14).

**Primary recommendation:** Add four run blocks to the Phase 15 placeholder: §5 sift+etamake (Rødseth), §6.1 triple product (Euler pentagonal), §6.2 quintuple + Euler dissection, §6.3 Winquist. Use exact Test 6 / Test 9 commands as reference; for qseriesdoc-style product functions, use `q^n` syntax (parsed as `Series::qpow(n, T)`).

## User Constraints

No CONTEXT.md exists for Phase 15. All scope comes from ROADMAP, qseriesdoc, and success criteria.

## Standard Stack

| Component | Purpose |
|-----------|---------|
| `demo/garvan-demo.sh` | Single script, `run()` per subsection |
| `repl.h` | sift, tripleprod, quinprod, winquist dispatch |
| `convert.h` | sift(f,n,k,T) implementation |
| `qfuncs.h` | tripleprod(z,q,T), quinprod(z,q,T), winquist(a,b,q,T) |

No new libraries or tools. Demo pattern follows Phases 12–14.

## Architecture Patterns

### Demo Run Block Structure

Each subsection uses:

```bash
echo ""
echo "--- [Section Name] (qseriesdoc §X.Y) ---"
run "cmd1" "cmd2" ...
```

- `run()` pipes all commands to `"$BIN"`; each `run()` is a separate process (no shared variables across blocks).
- Variables must be defined within the same `run()` where they are used.
- `set_trunc(N)` must be in the same `run()` as commands that depend on it.

### Pattern: Redefining Variables per Block

Because each `run()` is a fresh pipe, variables from prior blocks are not available. Define series (e.g. `EULER`, `PD`) inside the block that uses them.

### Pattern: q^n vs subs_q

- **q^n**: When the parser sees `q^3`, `q^5`, `q^11`, etc., it evaluates to `Series::qpow(n, env.T)`. Use this for product function arguments (tripleprod, quinprod, winquist).
- **subs_q(f,k)**: Used when you have a series `f` and want `f(q^k)`. For product functions, passing `q^n` as the second/third argument is equivalent to “q replaced by q^n” in the formula.

## Exact REPL Commands

### §5 Sifting (Rødseth) — Test 6 Reference

qseriesdoc §5: `sift(PD,q,5,1,199)` then `etamake(PD1,q,38)`.

**Our sift signature:** `sift(f,n,k,T)` — no `q` argument.

| Step | REPL Command |
|------|--------------|
| Build PD | `PD := etaq(2,200)/etaq(1,200)` |
| Sift | `PD1 := sift(PD,5,1,199)` |
| Identify | `etamake(PD1, 30)` |

**Note:** Test 6 uses `etamake(PD1, 30)` because T=38 can trigger coefficient overflow. Demo should use T=30 for robustness (acceptance.sh line 73).

**Expected output (etamake):** η(5τ)³ η(2τ)² / (η(10τ) η(τ)⁴) (Rødseth identity).

### §6.1 Triple Product (Euler Pentagonal)

qseriesdoc §6.1: `tripleprod(q,q^3,10)`.

| Command | Purpose |
|---------|---------|
| `tripleprod(q, q^3, 10)` | Euler pentagonal: Π(1-q^n) = Σ (-1)^n q^{n(3n-1)/2} |

**Syntax:** `q` and `q^3` are valid; `q^3` parses as `q` raised to power 3.

**Expected (Output 30):**  
`q⁵⁷ + q⁵¹ - q⁴⁰ - q³⁵ + q²⁶ + q²² - q¹⁵ - q¹² + q⁷ + q⁵ - q² - q + 1`

**Truncation:** Default T=50 is sufficient. Optionally `set_trunc(60)` for clarity.

### §6.2 Quintuple Product + Euler Dissection — Test 9 Reference

qseriesdoc §6.2: `quinprod(q,q^5,20)`, `EULER:=etaq(q,1,500)`, `E0:=sift(EULER,q,5,0,499)`, `jacprodmake(E0,q,50)`.

**Our signatures:** etaq(k,T) or etaq(q,k,T); sift(f,n,k,T); jacprodmake(f,T).

| Step | REPL Command |
|------|--------------|
| Quintuple product | `quinprod(q, q^5, 20)` |
| Euler product | `EULER := etaq(1,500)` |
| Sift residue 0 mod 5 | `E0 := sift(EULER, 5, 0, 499)` |
| Identify | `jacprodmake(E0, 50)` |
| Expand (optional) | `jac2prod(jp)` (if jp assigned) |

**Test 9 (acceptance.sh):** `set_trunc(500)` then `EULER := etaq(1,500)` and `E0 := sift(EULER,5,0,499)`, then `jacprodmake(E0,50)`.

**Expected (jacprodmake):** JAC(2,5,∞)·JAC(0,5,∞)/JAC(1,5,∞).

### §6.3 Winquist Identity

qseriesdoc §6.3: `winquist(q^5,q^3,q^11,20)`.

| Command | Purpose |
|---------|---------|
| `winquist(q^5, q^3, q^11, 20)` | Winquist identity RHS with a=q^5, b=q^3, q=q^11 |

**Syntax:** `q^5`, `q^3`, `q^11` all parse as `Series::qpow(n, env.T)`.

**Expected (Output 41):**  
`1 - q² - 2q³ + q⁵ + q⁷ + q⁹ + q¹¹ + q¹² - q¹³ - q¹⁵ - q¹⁶ - q¹⁸ + O(q²⁰)`

**Truncation:** The third argument `q^11` means the identity’s base variable is q¹¹; output exponents are multiples of 11. The fourth argument T limits the inner exponent. Use `set_trunc` high enough (e.g. 100+) so the displayed series is not truncated away.

## q^3, q^5, q^11 Syntax

| Expression | Parser | Result |
|------------|--------|--------|
| `q^3` | BinOp(Pow, Q, 3) | `Series::qpow(3, env.T)` |
| `q^5` | BinOp(Pow, Q, 5) | `Series::qpow(5, env.T)` |
| `q^11` | BinOp(Pow, Q, 11) | `Series::qpow(11, env.T)` |

Source: repl.h eval, lines 391–393 — when left side of `^` is `q`, returns `Series::qpow(expVal, env.T)`.

**Do not use** `subs_q(q, 3)` for product functions; that returns `f(q^3)` for some `f`, not the monomial `q^3`. Use `q^3` directly.

## Demo Run Block Structure Options

### Option A: One Block per Subsection (Recommended)

```bash
# §5 Sifting
echo ""
echo "--- Sifting: Rødseth (qseriesdoc §5) ---"
run "set_trunc(200)" "PD := etaq(2,200)/etaq(1,200)" "PD1 := sift(PD,5,1,199)" "etamake(PD1, 30)"

# §6.1 Triple product
echo ""
echo "--- Triple product: Euler pentagonal (qseriesdoc §6.1) ---"
run "set_trunc(60)" "series(tripleprod(q,q^3,10), 60)"

# §6.2 Quintuple + Euler dissection
echo ""
echo "--- Quintuple product and Euler dissection (qseriesdoc §6.2) ---"
run "set_trunc(500)" "series(quinprod(q,q^5,20), 100)" "EULER := etaq(1,500)" "E0 := sift(EULER,5,0,499)" "jp := jacprodmake(E0,50)" "jac2prod(jp)"

# §6.3 Winquist
echo ""
echo "--- Winquist identity (qseriesdoc §6.3) ---"
run "set_trunc(200)" "series(winquist(q^5,q^3,q^11,20), 60)"
```

### Option B: Combined §6 Block

Single block for §6.1–§6.3. Tradeoff: longer block, but fewer `run()` calls.

**Recommendation:** Option A — matches Phases 13–14 (one block per subsection) and keeps each subsection self-contained.

## Test 6 and Test 9 as Reference

### Test 6 (Rødseth) — tests/acceptance.sh lines 72–79

```bash
run "PD := etaq(2,200)/etaq(1,200)" "PD1 := sift(PD,5,1,199)" "etamake(PD1, 30)"
```

- Uses `etamake(PD1, 30)` (not 38) to avoid coefficient overflow.
- Pass criterion: output contains η (Unicode \xCE\xB7).

### Test 9 (Euler) — tests/acceptance.sh lines 99–106

```bash
run "set_trunc(500)" "EULER := etaq(1,500)" "E0 := sift(EULER,5,0,499)" "jacprodmake(E0,50)"
```

- Pass criterion: output contains "JAC" or "(q,q" (Jacobi product form).

## Common Pitfalls

### Pitfall 1: etamake T=38 Overflow

**What goes wrong:** `etamake(PD1, 38)` can hit coefficient overflow for Rødseth.  
**How to avoid:** Use `etamake(PD1, 30)` as in acceptance tests.

### Pitfall 2: Variable Scope Across run() Calls

**What goes wrong:** `PD` defined in one `run()` is not available in the next.  
**How to avoid:** Define all variables in the same `run()` as the commands that use them.

### Pitfall 3: Winquist Truncation

**What goes wrong:** With `q^11` as base, output exponents are multiples of 11. If `set_trunc` is too low, higher terms are truncated.  
**How to avoid:** Use `set_trunc(200)` or similar before winquist; `series(..., 60)` for display.

### Pitfall 4: Confusing q^n with subs_q

**What goes wrong:** Using `subs_q(q, 3)` where `q^3` is intended.  
**How to avoid:** For product identities, use `q^3`, `q^5`, `q^11` directly.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead |
|---------|-------------|-------------|
| Coefficient extraction | Custom loop | `sift(f,n,k,T)` |
| Triple product expansion | Manual product | `tripleprod(z,q,T)` |
| Quintuple product | Manual product | `quinprod(z,q,T)` |
| Winquist series | Manual double sum | `winquist(a,b,q,T)` |

## Code Examples

### Sift + Etamake (Rødseth)

```bash
set_trunc(200)
PD := etaq(2,200)/etaq(1,200)
PD1 := sift(PD,5,1,199)
etamake(PD1, 30)
```

### Triple Product (Euler Pentagonal)

```bash
set_trunc(60)
series(tripleprod(q,q^3,10), 60)
```

### Quintuple + Euler Dissection

```bash
set_trunc(500)
series(quinprod(q,q^5,20), 100)
EULER := etaq(1,500)
E0 := sift(EULER,5,0,499)
jp := jacprodmake(E0,50)
jac2prod(jp)
```

### Winquist Identity

```bash
set_trunc(200)
series(winquist(q^5,q^3,q^11,20), 60)
```

## Open Questions

1. **Winquist truncation semantics:** Does the fourth argument T in `winquist(a,b,q,T)` limit the inner exponent or the output series truncation? The implementation uses T as loop bound; when q=q^11, output exponents are 11×inner. Verify against qseriesdoc Output 41.
2. **jac2prod variable:** Test 9 only checks jacprodmake output. For demo, we may assign `jp := jacprodmake(E0,50)` then `jac2prod(jp)`. jac2prod expects a variable name; ensure the assigned variable is in scope.

## Sources

### Primary (HIGH confidence)

- **qseriesdoc.md** — §5 sift/etamake Rødseth, §6.1 triple product, §6.2 quinprod + Euler dissection, §6.3 Winquist
- **src/repl.h** — sift, tripleprod, quinprod, winquist dispatch and signatures
- **src/convert.h** — sift implementation
- **src/qfuncs.h** — tripleprod, quinprod, winquist implementations
- **demo/garvan-demo.sh** — current structure, Phase 15 placeholder
- **tests/acceptance.sh** — Test 6 (Rødseth), Test 9 (Euler) exact commands

### Secondary (MEDIUM confidence)

- **.planning/ROADMAP.md** — Phase 15 success criteria
- **SPEC.md** — Acceptance tests 6, 9
- **.planning/phases/13-product-conversion-demo/13-01-SUMMARY.md** — run block pattern
- **.planning/phases/14-relations-demo/14-01-SUMMARY.md** — run block pattern, set_trunc usage

## Metadata

**Confidence breakdown:**
- REPL commands: HIGH — verified in repl.h, qfuncs.h, convert.h
- q^n syntax: HIGH — verified in repl.h eval (BinOp Pow, isQLike)
- Demo structure: HIGH — matches Phases 13–14
- Test 6/9 reference: HIGH — acceptance.sh and SPEC.md

**Research date:** 2026-02-25  
**Valid until:** 30 days (stable demo format)
