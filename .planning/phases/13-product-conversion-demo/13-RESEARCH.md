# Phase 13: Product Conversion Demo — Research

**Researched:** 2026-02-25  
**Domain:** Demo script, product conversion examples (qfactor, etamake, jacprodmake)  
**Confidence:** HIGH

## Summary

Phase 13 appends product conversion examples to the Garvan demo script to reproduce qseriesdoc §3.2, §3.3, §3.4. All required built-ins exist: qfactor, etamake, jacprodmake, jac2prod, T(r,n), theta2/3/4, set_trunc. The demo placeholder `# === Product conversion: qfactor, etamake, jacprodmake (qseriesdoc §3.2–3.4) ===` is in `demo/garvan-demo.sh` with `# (Phases 12–15 append content here)`. The work is to add the exact REPL commands and structure the run blocks. One run block per subsection (§3.2, §3.3, §3.4) is recommended for clarity and independent verification.

**Primary recommendation:** Add three run blocks under the Product conversion section: (1) qfactor T(8,8), (2) etamake on theta2/3/4, (3) jacprodmake on Rogers-Ramanujan + jac2prod. Use `set_trunc` where needed. theta2 needs no `/q^(1/4)` in our REPL — our theta2 already returns θ₂/q^{1/4} with integer exponents.

---

## User Constraints

No CONTEXT.md for Phase 13. No locked decisions. Proceed with standard research.

---

## Exact REPL Commands by Subsection

### §3.2 qfactor (Outputs 3–5)

| Step | qseriesdoc | Our REPL |
|------|------------|----------|
| Compute T(8,8) | `t8:=T(8,8)` | `set_trunc(64)` then `t8 := T(8,8)` |
| qfactor | `qfactor(t8,20)` | `qfactor(t8, 20)` |

**Note:** T(8,8) is degree 42 (qseriesdoc Output 3). `set_trunc(64)` ensures enough headroom. Acceptance Test 7 uses the same: `set_trunc(64)` `t8 := T(8,8)` `qfactor(t8, 20)`.

**Expected output (Output 5):**
```
q⁶·(1-q⁹)(1-q¹⁰)(1-q¹¹)(1-q¹⁶) / ((1-q)(1-q²)(1-q³)(1-q⁴))
```
(Order of factors may vary; formatQfactor uses `(1-q^n)` style.)

---

### §3.3 etamake (Outputs 8, 10, 12)

| Theta | qseriesdoc | Our REPL |
|-------|------------|----------|
| θ₂ | `t2:=theta2(q,100)/q^(1/4)` then `etamake(t2,q,100)` | `etamake(theta2(100), 100)` |
| θ₃ | `t3:=theta3(q,100)` then `etamake(t3,q,100)` | `etamake(theta3(100), 100)` |
| θ₄ | `t4:=theta4(q,100)` then `etamake(t4,q,100)` | `etamake(theta4(100), 100)` |

**theta2 handling (CRITICAL):** qseriesdoc divides `theta2(q,100)` by `q^(1/4)` because Maple’s theta2 returns θ₂ with half-integer exponents. Our theta2(q,T) already returns θ₂/q^{1/4} with integer exponents (qfuncs.h, Phase 4 CONTEXT). So we use `theta2(100)` or `theta2(q,100)` directly — no `/q^(1/4)`.

**Expected etamake outputs:**
- theta2: η(4τ)² / η(2τ) (qseriesdoc Output 8 shows `2 η(4τ)² / (q^(1/4) η(2τ))` — numeric factor and q^(1/4) in display may differ; eta structure should match)
- theta3: η(2τ)⁵ / (η(4τ)² η(τ)²) (Output 10)
- theta4: η(τ)² / η(2τ) (Output 12)

**Acceptance Test 3** checks `etamake(theta3(q,100), 100)` and `etamake(theta4(q,100), 100)` for η (Unicode) in output. theta2 is not in Test 3 — add `etamake(theta2(100), 100)` in demo for completeness.

---

### §3.4 jacprodmake (Outputs 13, 14)

| Step | qseriesdoc | Our REPL |
|------|------------|----------|
| Build RR sum | loop `x:=x+q^(n²)/aqprod(q,q,n)` | `rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)` |
| jacprodmake | `jacprodmake(x,q,40)` | `jacprodmake(rr, 40)` |
| jac2prod | `jac2prod(y)` with y from jacprodmake | `jp := jacprodmake(rr, 40)` then `jac2prod(jp)` |

**Note:** Rogers-Ramanujan sum may already be in env from Rogers-Ramanujan section (Phase 12). If the demo runs all sections in one pipe, `rr` persists. If separate blocks, repeat the sum before jacprodmake.

**Expected outputs:**
- jacprodmake: displayed via jac2prod → `1 / ((q,q⁵)_∞ (q⁴,q⁵)_∞)` or equivalent
- jac2prod(jp): same (q^a;q^b)_∞ form

**JAC notation:** qseriesdoc Output 13 shows `y := JAC(0, 5, ∞) / JAC(1, 5, ∞)`. Our display uses jac2prod (expanded (q,q^5)_∞ form), not JAC(a,b,∞). Success criteria say "Jacobi product form (JAC notation)" — if strict, a `formatJacProd` helper could be added to show JAC form; current jac2prod output satisfies the mathematical identification.

---

## Expected Output Format from Display Helpers

### formatQfactor (src/repl.h ~line 503)

- Output: `q⁶·(1-q⁹)(1-q¹⁰)... / ((1-q)(1-q²)...))`
- Uses `(1-q^n)` style, Unicode exponents, middle dot before numerator

### formatEtamake (src/repl.h ~line 478)

- Output: `η(2τ)⁵ / (η(4τ)² η(τ)²)` (numerator / denominator)
- Uses Unicode η, τ, superscripts

### jac2prod (src/convert.h ~line 273)

- Output: `1 / ((q,q⁵)_∞ (q⁴,q⁵)_∞)` for Rogers-Ramanujan
- Format: `(q^a,q^b)_∞` or `(q,q^b)_∞`; quotient as `1 / ((...))`

---

## Demo Script Structure

### Option A: One run block per subsection (recommended)

```bash
# === §3.2 qfactor ===
echo ""
echo "--- qfactor T(8,8) (§3.2) ---"
run "set_trunc(64)" "t8 := T(8,8)" "qfactor(t8, 20)"

# === §3.3 etamake ===
echo ""
echo "--- etamake theta2, theta3, theta4 (§3.3) ---"
run "set_trunc(100)" "etamake(theta2(100), 100)" "etamake(theta3(100), 100)" "etamake(theta4(100), 100)"

# === §3.4 jacprodmake ===
echo ""
echo "--- jacprodmake Rogers-Ramanujan (§3.4) ---"
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "jp := jacprodmake(rr, 40)" "jac2prod(jp)"
```

### Option B: Combined Product conversion block

Single `run` with all commands; relies on persistent env (set_trunc, rr, t8, jp). Less modular, harder to re-run subsets.

**Recommendation:** Option A — one block per subsection. Clear section headers, easy to validate each part, and `set_trunc` is explicit per block.

---

## Acceptance Test Alignment

| Test | Command | Demo |
|------|---------|------|
| Test 3 | etamake(theta3), etamake(theta4) | Include both + theta2 |
| Test 4 | jacprodmake(rr,40), jac2prod | jp := jacprodmake(rr,40); jac2prod(jp) |
| Test 7 | set_trunc(64), t8:=T(8,8), qfactor(t8,20) | Same |

---

## Common Pitfalls

### Pitfall 1: theta2 /q^(1/4) confusion

**What goes wrong:** Copying qseriesdoc literally and writing `theta2(100)/q^(1/4)` — our theta2 already has q^{1/4} factored out.  
**How to avoid:** Use `theta2(100)` or `theta2(q,100)` only.

### Pitfall 2: set_trunc too low for T(8,8)

**What goes wrong:** T(8,8) has degree 42; trunc < 43 truncates the polynomial.  
**How to avoid:** Use `set_trunc(64)` or higher before T(8,8).

### Pitfall 3: rr not in scope for jacprodmake

**What goes wrong:** If Product conversion runs in a separate script invocation, `rr` from Rogers-Ramanujan section does not exist.  
**How to avoid:** Either pipe all sections together (rr persists) or redefine rr in the jacprodmake block.

---

## Code Examples

### Exact demo snippet to append

Replace the placeholder `# (Phases 12–15 append content here)` with:

```bash
# §3.2 qfactor T(8,8)
echo ""
echo "--- qfactor T(8,8) (qseriesdoc §3.2) ---"
run "set_trunc(64)" "t8 := T(8,8)" "qfactor(t8, 20)"

# §3.3 etamake theta2, theta3, theta4
echo ""
echo "--- etamake on theta functions (qseriesdoc §3.3) ---"
run "set_trunc(100)" "etamake(theta2(100), 100)" "etamake(theta3(100), 100)" "etamake(theta4(100), 100)"

# §3.4 jacprodmake Rogers-Ramanujan
echo ""
echo "--- jacprodmake Rogers-Ramanujan (qseriesdoc §3.4) ---"
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "jp := jacprodmake(rr, 40)" "jac2prod(jp)"
```

**Alternative for §3.4 if rr already defined:** If Rogers-Ramanujan block runs first in same pipe, use:
```bash
run "jp := jacprodmake(rr, 40)" "jac2prod(jp)"
```

---

## Open Questions

1. **JAC notation display:** Success criteria mention "JAC notation". Current jacprodmake displays via jac2prod (q^a;q^b form). Do we need a `formatJacProd` to show JAC(0,5,∞)/JAC(1,5,∞)? Or is jac2prod output sufficient for DEMO-03?

2. **theta2 etamake output:** qseriesdoc Output 8 has `2 η(4τ)² / (q^(1/4) η(2τ))`. Our formatEtamake does not show q^(1/4) or leading constants. Verify etamake(theta2(100), 100) produces η structure consistent with 2η(4τ)²/η(2τ) (possibly scaled).

---

## Sources

### Primary (HIGH confidence)
- qseriesdoc.md §3.2 (qfactor, T(r,n), Outputs 3–5)
- qseriesdoc.md §3.3 (etamake theta2/3/4, Outputs 8, 10, 12)
- qseriesdoc.md §3.4 (jacprodmake, jac2prod, Outputs 13, 14)
- demo/garvan-demo.sh — current structure and placeholder
- tests/acceptance.sh — Test 3, 4, 7 commands

### Secondary (verified in codebase)
- src/repl.h — formatQfactor, formatEtamake, jac2prod display, theta2 dispatch
- src/convert.h — qfactor, etamake, jacprodmake, jac2prod
- src/qfuncs.h — theta2 returns θ₂/q^{1/4}

---

## Metadata

**Confidence breakdown:**
- REPL commands: HIGH — acceptance tests and qseriesdoc give exact syntax
- theta2 handling: HIGH — Phase 4 CONTEXT and qfuncs.h confirm θ₂/q^{1/4}
- Output format: HIGH — format helpers exist and are documented
- Demo structure: MEDIUM — Option A recommended; Option B possible

**Research date:** 2026-02-25  
**Valid until:** 30 days
