# Phase 66: Exercise Solutions & Regression - Research

**Researched:** 2026-03-02
**Domain:** Q-series exercise verification — eta quotients, Eisenstein series, cubic modular relations
**Confidence:** HIGH

## Summary

Phase 66 is a **verification and documentation phase**, not a feature-building phase. All three exercises (4, 9, 10) can be solved using existing REPL functionality — no new functions or code changes are expected. The work consists of writing test scripts that execute the exercise computations, verifying correctness against known mathematical results, documenting outcomes, and confirming zero regressions.

Exercise 4 (b(q) = η(τ)³/η(3τ)) is a straightforward eta quotient computation. Exercise 10 (findpoly cubic relation) uses the q-shift-fixed findpoly from Phase 63. Exercise 9 (N(q) via findnonhomcombo) is the most complex — it requires constructing the Eisenstein series E₆ and the cubic AGM functions a(q), x(q), then running findnonhomcombo — but analysis shows this IS feasible at T=100, contrary to the "may be infeasible" caveat.

**Primary recommendation:** No code changes needed. Write exercise test scripts, run them, document results, verify all regression suites pass.

## Architecture Patterns

### Exercise Computation Patterns

Each exercise follows the same pattern: (1) construct the mathematical objects as REPL expressions, (2) run the identifying function (etamake/findpoly/findnonhomcombo), (3) verify the output matches the known mathematical result.

### Exercise 4: b(q) = η(τ)³/η(3τ)

**What it is:** From qseriesdoc.md Exercise 4, the cubic AGM function b(q) is one of the Borwein-Borwein-Garvan cubic theta functions. Two of {a(q), b(q), c(q)} can be written as eta products. The known answers from [12] are:
- b(q) = η(τ)³/η(3τ)
- c(q) = 3·η(3τ)³/η(τ)

**REPL computation:**
```
b := etaq(1,50)^3 / etaq(3,50)
series(b, 20)
etamake(b, 50)
```

**Why it works (no code changes):**
- `etaq(k,T)` computes Π(1-q^{kn}) — existing function
- η(τ)³/η(3τ) = (q^{1/24}·etaq(1))³ / (q^{1/8}·etaq(3)) = q^{1/8-1/8}·etaq(1)³/etaq(3) = etaq(1)³/etaq(3) — the q-powers cancel exactly
- The etamake algorithm will identify this as η(τ)³/η(3τ)
- No q-shift complications (everything is integer-exponent)

**Expected coefficients (b(q) first terms):** b(q) = 1 - 3q + 0q² + 6q³ - 3q⁴ + ... (to be verified computationally). These arise from Π(1-q^n)³/Π(1-q^{3n}) using Jacobi's identity Π(1-q^n)³ = Σ(-1)^k(2k+1)q^{k(k+1)/2}.

**Confidence:** HIGH — uses only existing, well-tested functions.

### Exercise 9: N(q) via findnonhomcombo

**What it is:** From qseriesdoc.md Exercise 9, express the Eisenstein series E₆(q) = N(q) = 1 - 504·Σ σ₅(n)q^n in terms of a(q) and x(q) = c³/a³ using findnonhomcombo.

**Mathematical background:**
- N(q) is a weight-6 modular form
- a(q) = θ₃(q)·θ₃(q³) + θ₂(q)·θ₂(q³) is a weight-1 cubic AGM function
- c(q) = 3·q^{1/3}·etaq(3,T)³/etaq(1,T), so c³ = 27q·etaq(3,T)⁹/etaq(1,T)³ (integer shift)
- x = c³/a³ is weight 0 (modular function)
- Expected result from [8, Theorem 12.3]: N = a⁶·(1 + 20x - 8x²)

**REPL computation:**
```
set_trunc(100)
a := theta3(q,100)*theta3(q^3,100) + theta2(q,100)*theta2(q^3,100)
c3 := 27*q*etaq(3,100)^9/etaq(1,100)^3
x := c3/a^3
N := 1 - 504*sum(sigma(n,5)*q^n, n, 1, 99)
findnonhomcombo(N, [a, x], [6, 2], 0)
```

**Why it works (no code changes):**
- `sigma(n,5)` returns int64_t, which `toSeries()` converts to `Series::constant(Frac(val), T)` during binary multiplication. Values up to σ₅(100) ≈ 10^{10} fit in int64_t.
- theta2(q,T) has q_shift=1/4 and theta2(q^3,T) has q_shift=3/4. Their product has q_shift=1, which normalize_q_shift() absorbs to q_shift=0 by shifting indices by +1. Then addition with theta3 product (q_shift=0) works.
- c³ = 27q·etaq(3)⁹/etaq(1)³ avoids fractional q-shifts entirely (cubing eliminates q^{1/3}).
- n_list=[6,2] generates 7×3 = 21 monomials. Matrix is 100×21. Gaussian elimination trivial.

**Feasibility assessment:** FEASIBLE at T=100 (~30 seconds estimated). The computation involves:
- 99 iterations for N(q) construction (trivial)
- ~6 Series multiplications for a⁶ at T=100 (~100ms each)
- ~21 monomial products for findnonhomcombo matrix (~2s total)
- Gaussian elimination on 100×21 matrix (~instant)

**Alternative N(q) construction (if sigma approach fails):**
```
N := 1 - 504*sum(sum(n^5*q^(m*n), m, 1, 99), n, 1, 99)
```
This double-sum pattern is proven to work (Block 17 in maple-checklist uses identical pattern for UE Eisenstein series).

**Confidence:** HIGH for construction, MEDIUM for exact expected coefficients in findnonhomcombo output format.

### Exercise 10: findpoly cubic relation

**What it is:** From qseriesdoc.md Exercise 10, define m = (θ₃(q)/θ₃(q³))² and find y = c³/a³ as a rational function of m using findpoly.

**REPL computation:**
```
set_trunc(100)
m := theta3(q,100)^2 / theta3(q^3,100)^2
a := theta3(q,100)*theta3(q^3,100) + theta2(q,100)*theta2(q^3,100)
c3 := 27*q*etaq(3,100)^9/etaq(1,100)^3
y := c3/a^3
findpoly(m, y, 6, 1)
```

**Expected polynomial:** P(X,Y) = Y·(X²+6X-3)³ - 27·(X-1)·(X+1)⁴ = 0, which gives:
y = 27(m-1)(m+1)⁴/(m²+6m-3)³. This is [8, Eq.(12.8)].

**Why it works (no code changes):**
- m = theta3(q,T)²/theta3(q^3,T)² — both have q_shift=0, purely integer exponents
- y = c³/a³ — integer shift as shown above
- findpoly(m, y, 6, 1) generates (6+1)×(1+1) = 14 monomials X^i·Y^j
- With T=100, matrix is 100×14 — massively overdetermined, kernel found easily
- Phase 63's q-shift fix already enabled findpoly with theta quotients (Block 25)

**Q-shift relevance:** Phase 63 fixed q-shift normalization which unblocked Block 25 (findpoly on theta quotients involving θ₂). Exercise 10 only uses θ₃ (no fractional shifts), so it's even simpler. But the intermediate a(q) computation uses θ₂ products, which now work correctly due to Phase 63's normalize_q_shift().

**Confidence:** HIGH — uses proven findpoly infrastructure, purely integer-shift series.

## Common Pitfalls

### Pitfall 1: Wrong a(q) Construction
**What goes wrong:** Using `q * theta2(q,T) * theta2(q^3,T)` instead of just `theta2(q,T) * theta2(q^3,T)` for the theta2 product in a(q).
**Why it happens:** Misunderstanding that theta2(q,T) already includes q^{1/4} via q_shift. The REPL's theta2 returns the FULL θ₂(q) with q_shift=1/4, NOT θ₂/q^{1/4}.
**How to avoid:** Use `a := theta3(q,T)*theta3(q^3,T) + theta2(q,T)*theta2(q^3,T)` — the q^{1/4} and q^{3/4} shifts combine to q^1 which normalize_q_shift() absorbs.
**Verification:** a(q) should start 1 + 6q + 0q² + 6q³ + 6q⁴ + ...

### Pitfall 2: Using c(q) directly instead of c³
**What goes wrong:** Computing c(q) = 3·q^{1/3}·etaq(3)³/etaq(1) introduces fractional q-shift (1/3). Using c in findnonhomcombo would cause "cannot add series with different q-shifts" errors.
**How to avoid:** Always compute c³ = 27q·etaq(3)⁹/etaq(1)³ directly, bypassing the q^{1/3} factor. The cube eliminates the fractional shift.

### Pitfall 3: Truncation mismatch in theta quotients
**What goes wrong:** Using mismatched truncation values (e.g., theta3(q,100)/theta3(q^3,40)) gives limited effective terms. With T=40 for q^3, only 6 θ₃ terms available.
**How to avoid:** Use consistent T=100 for all base functions. The q^3 substitution naturally limits terms (n²·3 < 100 → n ≤ 5), but the overall truncation stays at 100.

### Pitfall 4: sigma overflow in N(q) construction  
**What goes wrong:** For large n, σ₅(n) can be very large. At n=100, σ₅(100) ≈ 10^{10}.
**Why it's actually OK:** int64_t handles up to ~9.2×10^{18}. The product 504·σ₅(100) ≈ 5×10^{12} fits easily. BigInt handles arbitrarily large values if overflow occurs.

### Pitfall 5: Not copying dist binary
**What goes wrong:** maple-checklist.sh prefers dist/qseries.exe which may be stale.
**How to avoid:** Always copy freshly-built binary to dist/ before running tests. Or update test scripts to use root binary first.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| b(q) from double sum with ω | Implement RootOf(z²+z+1=0) | `etaq(1,T)^3/etaq(3,T)` | Direct eta product identity avoids algebraic number arithmetic |
| N(q) from definition | Implement general Eisenstein series | `1 - 504*sum(sigma(n,5)*q^n, n, 1, T)` | Existing sigma + sum already work |
| c(q) with q^{1/3} | Try to use fractional q-shifts directly | `c3 := 27*q*etaq(3,T)^9/etaq(1,T)^3` | Compute c³ directly to avoid fractional shifts |

## Code Examples

### Exercise 4 Test Script Pattern
```bash
OUT=$(echo "b := etaq(1,50)^3/etaq(3,50)
series(b, 20)" | ./qseries.exe 2>&1)
# Verify b starts with 1 - 3q + ... (known coefficients)
if echo "$OUT" | grep -q "1 - 3q"; then
    pass "Exercise 4: b(q) coefficients"
fi
# Verify etamake identifies it
OUT2=$(echo "b := etaq(1,50)^3/etaq(3,50)
etamake(b, 50)" | ./qseries.exe 2>&1)
if echo "$OUT2" | grep -q "η(τ)³"; then
    pass "Exercise 4: etamake identification"
fi
```

### Exercise 9 REPL Commands
```
set_trunc(100)
a := theta3(q,100)*theta3(q^3,100) + theta2(q,100)*theta2(q^3,100)
c3 := 27*q*etaq(3,100)^9/etaq(1,100)^3
x := c3/a^3
N := 1 - 504*sum(sigma(n,5)*q^n, n, 1, 99)
findnonhomcombo(N, [a, x], [6, 2], 0)
```
Expected output: coefficients 1, 20, -8 for monomials a⁶, a⁶x, a⁶x² respectively.

### Exercise 10 REPL Commands
```
set_trunc(100)
m := theta3(q,100)^2 / theta3(q^3,100)^2
a := theta3(q,100)*theta3(q^3,100) + theta2(q,100)*theta2(q^3,100)
c3 := 27*q*etaq(3,100)^9/etaq(1,100)^3
y := c3/a^3
findpoly(m, y, 6, 1)
```
Expected output: polynomial Y(X²+6X-3)³ - 27(X-1)(X+1)⁴

## Key Mathematical Identities (Reference)

| Identity | Source | REPL Form |
|----------|--------|-----------|
| b(q) = η(τ)³/η(3τ) | [12] Borwein³-Garvan | `etaq(1,T)^3/etaq(3,T)` |
| c(q) = 3η(3τ)³/η(τ) | [12] | `3*etaq(3,T)^3/etaq(1,T)` (has q^{1/3} shift) |
| a(q) = θ₃(q)θ₃(q³)+θ₂(q)θ₂(q³) | [12] | `theta3(q,T)*theta3(q^3,T)+theta2(q,T)*theta2(q^3,T)` |
| E₆ = a⁶(1+20x-8x²) | [8] Berndt-Bhargava-Garvan Thm 12.3 | findnonhomcombo result |
| y = 27(m-1)(m+1)⁴/(m²+6m-3)³ | [8] Eq.(12.8) | findpoly result |

## Test Infrastructure

### Current Test Scores (Phase 65 baseline)
| Suite | Score | Details |
|-------|-------|---------|
| acceptance.sh | 10/10 PASS | All 10 SPEC tests |
| acceptance-powfrac.sh | 6/6 PASS | Fractional power tests |
| maple-checklist.sh | 33/41 (33 PASS, 0 FAIL, 8 SKIP) | All testable blocks pass |

### Skipped Maple-Checklist Blocks (permanent)
These 8 blocks require Maple-only features and will never pass:
| Block | Reason |
|-------|--------|
| 4 | `factor(t8)` — Maple's polynomial factorization |
| 10 | `RootOf(z²+z+1=0)` — algebraic numbers |
| 21 | Result indexing `EQNS[1]` |
| 24 | `collect(%[1],[X[1]])` — formatting |
| 28 | `tripleprod(z,q,10)` — symbolic z variable |
| 30 | `quinprod(z,q,prodid)` — symbolic mode |
| 31 | `quinprod(z,q,seriesid)` — symbolic mode |
| 32 | `quinprod(z,q,3)` — symbolic z variable |

### New Test Requirements
Phase 66 needs a new test script (or additions to existing scripts) that:
1. Computes `etaq(1,T)^3/etaq(3,T)` and verifies coefficients (EX-01)
2. Attempts `findnonhomcombo(N, [a, x], [6, 2], 0)` and documents output (EX-02)
3. Runs `findpoly(m, y, 6, 1)` and verifies the cubic relation (EX-03)
4. Re-runs all existing suites and confirms zero regressions (REG-04)

## Open Questions

1. **Exact expected coefficients of b(q)**
   - What we know: b(q) = Π(1-q^n)³/Π(1-q^{3n}), starts with 1 - 3q + ...
   - What's unclear: Exact coefficients at q², q³, q⁴, etc. — not computed by hand
   - Recommendation: Compute in REPL, then verify against OEIS or [12]. The test should verify the first few known coefficients once determined.

2. **Exercise 9 expected findnonhomcombo output format**
   - What we know: Answer is N = a⁶ + 20a³c³ - 8c⁶ = a⁶(1+20x-8x²)
   - What's unclear: How findnonhomcombo formats the monomial coefficients in output (uses X[1], X[2] notation)
   - Recommendation: Run the computation, observe the output format, then write the test assertion accordingly.

3. **Exercise 10 polynomial normalization**
   - What we know: Relation is Y(X²+6X-3)³ - 27(X-1)(X+1)⁴ = 0
   - What's unclear: findpoly might output a scalar multiple of this polynomial, or factor differently
   - Recommendation: Check that findpoly output is proportional to the expected polynomial.

4. **Performance of Exercise 9 computation**
   - What we know: Estimated ~30 seconds for full computation at T=100
   - What's unclear: Could be faster or slower depending on BigInt sizes in intermediate products
   - Recommendation: If too slow at T=100, reduce to T=60 and increase topshift. Document timing.

## Sources

### Primary (HIGH confidence)
- `qseriesdoc.md` (project file) — Exercises 4, 9, 10 definitions, worked examples
- `src/series.h` — normalize_q_shift() implementation, powFrac, operator arithmetic
- `src/repl.h` — sum evaluation (Tag::Sum), sigma dispatch, toSeries() conversion
- `src/qfuncs.h` — theta2/theta3 implementations with q_shift, sigma(n,k)
- `src/relations.h` — findpoly, findnonhomcombo implementations
- `tests/maple-checklist.sh` — current test infrastructure, Block 17 UE pattern

### Secondary (MEDIUM confidence)
- [8] Berndt-Bhargava-Garvan "Ramanujan's theories of elliptic functions to alternative bases" — Exercise 9 answer (Theorem 12.3), Exercise 10 answer (Eq.(12.8))
- [12] Borwein-Borwein-Garvan "Some cubic modular identities of Ramanujan" — Exercise 4 eta product identities

## Metadata

**Confidence breakdown:**
- Exercise 4 feasibility: HIGH — standard eta quotient, no complexity
- Exercise 9 feasibility: HIGH — analysis shows T=100 sufficient, sigma*q^n works via toSeries
- Exercise 10 feasibility: HIGH — findpoly already works for similar Block 25 computation
- Regression: HIGH — no code changes expected, tests are established
- Expected mathematical results: MEDIUM — formulas from papers, not independently verified

**Research date:** 2026-03-02
**Valid until:** Indefinite (mathematical identities don't change; code API stable)
