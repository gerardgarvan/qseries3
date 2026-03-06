# Phase 62: Maple Checklist Satisfied — Research

**Researched:** 2026-03-01
**Domain:** q-series REPL correctness vs. Maple reference tutorial (qseriesdoc.md)
**Confidence:** HIGH

## Summary

This phase requires verifying all 41 Maple code blocks and 13 exercises from `maple_checklist.md` against qseries3's REPL. After a thorough audit of the current source code (`parser.h`, `repl.h`, `qfuncs.h`, `convert.h`, `relations.h`, `series.h`) and the expected outputs in `qseriesdoc.md`, I identified the exact status of every block.

Roughly 18 of 41 blocks should work with **syntax translation only** (dropping the `q` argument from Maple calls, adjusting arg counts). Another 8 blocks need **Maple proc/for-loop translation** into `sum()` expressions. The remaining ~15 blocks require **code fixes** — the most critical being: (1) theta2/3/4 functions ignore their `q` argument so `theta3(q^2, 100)` returns the wrong result, (2) `tripleprod`/`quinprod` use `T` as truncation but Maple treats it as factor count, giving radically different output for small T, and (3) `quinprod(z,q,prodid)` / `seriesid` symbolic modes are unimplemented.

**Primary recommendation:** Fix the three critical code bugs (theta q-arg, tripleprod/quinprod T semantics, quinprod modes), then systematically translate all 41 blocks to qseries3 syntax and verify. Add small features (for-loops or translate manually) as needed. One block (Block 10: `RootOf`) is architecturally infeasible.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Scope:** Both parts of `maple_checklist.md` — 41 Maple code blocks (Blocks 1–41) and 13 exercises (Exercises 1–13). Every block and exercise must be attempted.

**Failure Handling:** Fix what's reasonable. Document truly hard gaps as known limitations. Add small syntax features (for-loops, indexed variables) in this phase.

**Output Matching:** Mathematically equivalent is sufficient. Formatting differences are acceptable.

**Exercises:** Verify the math computationally. Provide qseries3 output. Proofs/generalizations out of scope. Exercises go in a separate `exercises_solutions.md`.

**Artifacts:**
1. Automated test script (`.sh`) running all 41 blocks — primary artifact
2. Updated `maple_checklist.md` with `[x]` marks and failure notes
3. `exercises_solutions.md` with qseries3 commands and output

**Compile/Test Strategy:** Implementer's choice (batch vs. fix-and-retest-each).

### Claude's Discretion
(None explicitly listed — freedom areas relate to implementation approach)

### Deferred Ideas (OUT OF SCOPE)
(None captured yet)
</user_constraints>

## Block-by-Block Status Assessment

### Classification Key

| Status | Count | Meaning |
|--------|-------|---------|
| (A) Syntax translation only | 18 | Drop `q` arg, adjust arg count, rename vars |
| (B) Maple proc → sum() translation | 6 | Translate for-loops/procs to `sum()` expressions |
| (C) Code fix required | 14 | Bug fix or small feature addition needed |
| (D) Infeasible | 1 | Architecturally impossible (algebraic numbers) |
| (E) Display-only (symbolic) | 2 | Requires symbolic z variable; skip or adapt |

### Part 1: Blocks 1–41

#### Section 3.1 — prodmake

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 1 | `add(q^(n^2)/aqprod(q,q,n), n=0..8)` + `series(x,q,50)` | `x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)` then `series(x, 50)` | **(A)** | Maple `add` → our `sum`; aqprod needs 4th arg (T); `series(x,q,50)` → `series(x,50)` |
| 2 | `prodmake(x,q,40)` | `prodmake(x, 40)` | **(A)** | Drop `q` arg |

#### Section 3.2 — qfactor

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 3 | Defines `T(r,j)` proc, calls `T(8,8)` | `t8 := T(8, 8)` | **(A)** | T(r,n) already built-in |
| 4 | `factor(t8)` | Skip or use `qfactor(t8)` | **(A)** | No `factor()` but not needed; qfactor subsumes |
| 5 | `qfactor(t8,20)` | `qfactor(t8, 20)` | **(A)** | Direct equivalent |
| 6 | Defines `dixson(a,b,c,q)` proc with for-loop, calls `dixson(5,5,5,q)` + `qfactor(dx,20)` | `dx := sum((-1)^k * q^(k*(3*k+1)/2) * qbin(5+k,10,50) * qbin(5+k,10,50) * qbin(5+k,10,50), k, -5, 5)` then `qfactor(dx, 20)` | **(B)** | Translate proc to `sum()`. The three qbin args are all `qbin(5+k, 10, T)` since a=b=c=5. |

#### Section 3.3 — etamake

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 7 | `t2:=theta2(q,100)/q^(1/4)` then `etamake(t2,q,100)` | `t2 := theta2(100)/q^(1/4)` then `etamake(t2, 100)` | **(A)** | theta2 returns q_shift=1/4; dividing by q^(1/4) cancels it. Drop `q` from etamake. |
| 8 | `t3:=theta3(q,100)` then `etamake(t3,q,100)` | `t3 := theta3(100)` then `etamake(t3, 100)` | **(A)** | Direct translation |
| 9 | `t4:=theta4(q,100)` then `etamake(t4,q,100)` | `t4 := theta4(100)` then `etamake(t4, 100)` | **(A)** | Direct translation |
| 10 | `omega:=RootOf(z^2+z+1=0)` | N/A | **(D)** | **Infeasible.** No algebraic number support. Document as known limitation. |

#### Section 3.4 — jacprodmake

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 11 | For-loop building RR sum, `jacprodmake(x,q,40)` | `x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)` then `y := jacprodmake(x, 40)` | **(B)** | Translate for-loop to sum(). |
| 12 | `jac2prod(y)` | `jac2prod(y)` | **(A)** | Works if y holds JacFactor from Block 11. |
| 13 | For-loop building sum, `jacprodmake(x,q,50)` | `x := sum(q^(n*(n+1)/2)*aqprod(-q,q,n,50)/aqprod(q,q,2*n+1,50), n, 0, 10)` then `jp := jacprodmake(x, 50)` | **(B)** | Translate for-loop. `-q` as aqprod arg should work (unary minus on q). |
| 14 | `jac2series(jp,500)` | `jac2series(jp, 500)` | **(A)** | Direct equivalent. |

#### Section 4.1 — findhom

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 15 | `findhom([theta3(q,100),theta4(q,100),theta3(q^2,100),theta4(q^2,100)],q,1,0)` | `findhom([theta3(100), theta4(100), theta3(q^2,100), theta4(q^2,100)], 1, 0)` | **(C)** | **BUG: theta3/4 ignore q argument.** `theta3(q^2,100)` returns `theta3(q,100)` — WRONG. Fix needed. Workaround: `subs_q(theta3(100),2)`. |
| 16 | Same pattern, degree 2 | Same issue | **(C)** | Same theta q-arg bug. |

#### Section 4.2 — findhomcombo

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 17 | Defines UE proc with double for-loop + `legendre(m,p)` | `f := sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)` | **(B)** | Double `sum()` nesting. Inner bound `trunk/m` → use large bound, rely on truncation. `legendre()` already exists. |
| 18 | `findhomcombo(f,[B1,B2],q,3,0,yes)` | `findhomcombo(f, [B1,B2], 3, 0)` | **(A)** | Drop `q` and `yes` args. `yes` (etaopt) not implemented but not needed for math correctness. Output uses X₁X₂ form instead of eta products — formatting difference only. |

#### Section 4.3 — findnonhom

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 19 | `F := q -> theta3(q,500)/theta3(q^5,100)` and `U := 2*q*theta(q^10,q^25,5)/theta3(q^25,20)` | Manual variable definitions: `Fq := theta3(500)/theta3(q^5,100)` and `U := 2*q*theta(q^10,q^25,5)/theta3(q^25,20)` | **(C)** | **Multiple issues:** (1) No lambda/function definitions. (2) theta3(q^5,...) and theta3(q^25,...) have the ignored-q bug. (3) `theta(q^10, q^25, 5)` — the 3-arg theta DOES use its args, so that part works. Fix theta first. |
| 20 | `findnonhom([F(q),F(q^5),U],q,3,20)` | `findnonhom([Fq, Fq5, U], 3, 20)` | **(C)** | Depends on Block 19 fixes. Need to compute F(q^5) manually too. |
| 21 | `ANS:=EQNS[1]` | N/A — read relation from findnonhom output | **(C)** | No result indexing. Can verify mathematically by constructing the polynomial manually. |
| 22 | `subs({X[1]=F(q),...},ANS)` + `series(CHECK,q,500)` | Manual polynomial evaluation | **(C)** | No `subs()` or `X[1]` indexed vars. Workaround: construct the expression `-Fq*Fq5*U + Fq5^2 + U^2 + U - 1` directly and verify it's O(q^500). |

#### Section 4.4 — findnonhomcombo

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 23 | `findnonhomcombo(T^2,[T,xi],q,[1,7],0,no)` | Rename T variable: `TT := q*(etaq(7,100)/etaq(1,100))^4` then `findnonhomcombo(TT^2, [TT, xi], [1, 7], 0)` | **(C)** | **Name collision:** `T` is a built-in function (`T(r,n)`). Must use different variable name. Drop `q` and `no` args. |
| 24 | `collect(%[1],[X[1]])` | N/A — view Block 23 output directly | **(A)** | `collect` is a formatting function. The polynomial is visible in Block 23's output. Skip. |

#### Section 4.5 — findpoly

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 25 | Uses `radsimp()`, `theta2(q^3,...)`, `theta3(q^3,...)`, `q^(1/3)`, `findpoly(x,y,q,3,1,60)` | Complex translation needed | **(C)** | **Multiple issues:** (1) theta q-arg bug for q^3 variants. (2) `radsimp` not needed if we compute directly — series arithmetic handles simplification implicitly. (3) `q^(1/3)` fractional power — supported via q_shift (Phase 60-61). (4) findpoly: drop `q` arg → `findpoly(x, y, 3, 1, 60)`. |

#### Section 5 — Sifting

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 26 | `sift(PD,q,5,1,199)` | `sift(PD, 5, 1, 199)` | **(A)** | Drop `q` arg. 4-arg sift already correct. |
| 27 | `etamake(PD1,q,38)` | `etamake(PD1, 38)` | **(A)** | Drop `q` arg. |

#### Section 6.1 — Triple Product

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 28 | `tripleprod(z,q,10)` | N/A | **(E)** | **Symbolic z.** Maple displays a Laurent series in symbolic `z` with q-polynomial coefficients. qseries3 operates on concrete series. Skip with note: "requires symbolic variable support." |
| 29 | `tripleprod(q,q^3,10)` | `tripleprod(q, q^3, 100)` then `series(result, 60)` | **(C)** | **T-semantics bug.** Our `tripleprod` uses T as truncation cutoff. With T=10, result is truncated to O(q^10) — loses terms. Expected output has terms to q^57. **Fix:** T in tripleprod should control number of loop iterations, not truncation. Internal trunc should be derived from environment T or a larger value. |

#### Section 6.2 — Quintuple Product

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 30 | `quinprod(z,q,prodid)` | N/A | **(E)** | **Symbolic z + prodid mode.** Displays the product identity symbolically. Could add as a special display mode. |
| 31 | `quinprod(z,q,seriesid)` | N/A | **(E)** | **Symbolic z + seriesid mode.** Same as above. |
| 32 | `quinprod(z,q,3)` | N/A | **(E)** | **Symbolic z.** Same as Block 28. |
| 33 | `sift(EULER,q,5,0,499)` | `E0 := sift(EULER, 5, 0, 499)` | **(A)** | Direct translation. |
| 34 | `jacprodmake(E0,q,50)` | `jp := jacprodmake(E0, 50)` | **(A)** | Drop `q` arg. |
| 35 | `jac2prod(jp)` | `jac2prod(jp)` | **(A)** | Direct equivalent. |
| 36 | `quinprod(q,q^5,20)` + `series(qp,q,100)` | `qp := quinprod(q, q^5, 100)` then `series(qp, 100)` | **(C)** | **T-semantics issue** same as tripleprod. With T=20, only 20 terms survive but expected output goes to q^85+. Fix: use environment T for truncation. |

#### Section 6.3 — Winquist's Identity

| Block | Maple Command | qseries3 Translation | Status | Notes |
|-------|--------------|----------------------|--------|-------|
| 37 | `Q:=n->tripleprod(q^n,q^33,10)` then defines A0..B4 | Manual definitions: `A0 := tripleprod(q^15, q^33, 200)` etc. | **(C)** | **Multiple issues:** (1) No lambda/function definitions. (2) tripleprod T semantics — use large T. (3) All Q(k) calls must be expanded manually. |
| 38 | `jacprodmake(IDG,q,50)` | `jp := jacprodmake(IDG, 50)` | **(A)** | Drop `q` arg. Depends on Block 37. |
| 39 | `jac2prod(jp)` | `jac2prod(jp)` | **(A)** | Direct equivalent. |
| 40 | `winquist(q^5,q^3,q^11,20)` + `series(...)` | `series(winquist(q^5, q^3, q^11, 20), 20)` | **(C)** | Winquist's T parameter: currently T controls both loop bound and truncation. Need to verify output matches. May need larger T. |
| 41 | `series(IDG-winquist(...),q,60)` | `series(IDG - winquist(q^5, q^3, q^11, 200), 60)` | **(C)** | Depends on Block 37 + Block 40 fixes. Use large T for winquist. |

### Summary Count

| Status | Blocks |
|--------|--------|
| (A) Works with syntax translation | 1, 2, 3, 4, 5, 7, 8, 9, 12, 14, 18, 24, 26, 27, 33, 34, 35, 38, 39 (19 blocks) |
| (B) Needs proc→sum translation | 6, 11, 13, 17 (4 blocks) |
| (C) Needs code fix | 15, 16, 19, 20, 21, 22, 23, 25, 29, 36, 37, 40, 41 (13 blocks) |
| (D) Infeasible | 10 (1 block) |
| (E) Symbolic z / display-only | 28, 30, 31, 32 (4 blocks) |

## Critical Code Fixes Required

### Fix 1: theta2/3/4 Must Use q Argument (HIGH priority)

**What's wrong:** `theta2(const Series& /*q*/, int T)` — the q parameter is commented out and ignored. All three theta functions compute with the implicit standard q regardless of what's passed.

**Impact:** Blocks 15, 16, 19, 20, 25, and Exercises 6, 10 all use `theta3(q^k, T)` with k > 1. Currently produces WRONG results silently.

**Fix approach:** Detect the q-power from the first argument. If the argument is a q-power (single term at exponent k with coefficient 1), compute the theta function with exponents scaled by k:

```cpp
inline Series theta3(const Series& q_arg, int T) {
    int k = 1;  // default: standard q
    if (q_arg.c.size() == 1) {
        auto it = q_arg.c.begin();
        if (it->second == Frac(1) && it->first > 0)
            k = it->first;
    }
    Series result = Series::one(T);
    result.trunc = T;
    for (int n = 1; n * n * k < T; ++n) {
        int exp = n * n * k;
        result.setCoeff(exp, result.coeff(exp) + Frac(2));
    }
    return result;
}
```

Similarly for theta2 (with q_shift scaled: `q_shift = Frac(k, 4)`) and theta4.

**Confidence:** HIGH — straightforward computation change.

### Fix 2: tripleprod/quinprod T Parameter Semantics (HIGH priority)

**What's wrong:** Our `tripleprod(z, q, T)` uses T as both the loop iteration count AND the series truncation. With T=10, the result is truncated at O(q^10), losing all terms beyond q^9. Maple uses T as the iteration count but does NOT truncate the result — the output can have terms up to q^(O(T²)).

**Impact:** Block 29 (`tripleprod(q,q^3,10)` should produce polynomial up to q^57, we produce up to q^9). Block 36 (`quinprod(q,q^5,20)` should show terms to ~q^85). Block 37 (all Q(k) definitions). Blocks 40-41 (winquist).

**Fix approach:** Separate the loop count from the truncation. Use the environment's T (or a dynamically computed bound) for truncation, and the function's T argument for the number of product factors:

```cpp
inline Series tripleprod(const Series& z, const Series& q, int T) {
    // T = number of product factors; compute appropriate truncation
    int trunc = std::max(z.trunc, q.trunc);
    // ... use trunc for Series::one(trunc) and truncTo(trunc)
    // ... use T for loop: for (int n = 1; n <= T; ++n)
    ...
}
```

Alternatively, make T always mean "truncation order of the output" (matching our general semantics) and compute enough loop iterations internally. This is more consistent with our other functions. In that case, the user would call `tripleprod(q, q^3, 100)` to get output up to O(q^100).

**Recommendation:** Keep T as truncation order (consistent with all other functions) but fix the loop to compute enough factors. Determine max n by checking when all three factor terms exceed T. For tripleprod(z,q,T) where z=q^a, q=q^b: the factors are (1-q^(a+b(n-1))), (1-q^(-a+bn)), (1-q^(bn)). The loop should continue while any factor has terms below T, i.e., while `b*n < T`.

**Confidence:** HIGH — clear fix with well-understood semantics.

### Fix 3: Variable Name Collision with T (MEDIUM priority)

**What's wrong:** Block 23 uses `T` as a variable name (`T:=series(q*(etaq(q,7,100)/etaq(q,1,100))^4,q,101)`), but `T` is a built-in function `T(r,n)`.

**Impact:** Block 23, and any user who wants to name a variable `T`.

**Fix approach:** Two options:
1. **Minimal (recommended):** Tell users to use a different name in the test script (e.g., `TT`).
2. **Full fix:** Make the parser check if an identifier followed by `:=` shadows a built-in, and allow it (user variables take precedence). This requires the parser/evaluator to distinguish `T(8,8)` (function call) from `T := expr` (assignment) and `T^2` (variable access). Currently, `T` would be parsed as an IDENT and if followed by `(` would be treated as a Call. If followed by `:=` it would be an assignment. The issue is after assignment, `T^2` would try to look up `T` as a variable, not as a function. This would work UNLESS the user later wants to call `T(r,n)` again. Since the checklist uses `T` as a variable without needing the function, option 1 suffices.

**Confidence:** HIGH.

### Fix 4: quinprod prodid/seriesid Modes (LOW priority)

**What's wrong:** Blocks 30-31 use `quinprod(z,q,prodid)` and `quinprod(z,q,seriesid)` where `prodid`/`seriesid` are symbolic identifiers requesting different display modes. Our quinprod only accepts integer T.

**Impact:** Blocks 30, 31. These are symbolic identity displays, not computational results. They can be documented as known limitations (display-only features) without affecting mathematical correctness.

**Decision:** Mark Blocks 30-31 as display-only features. Not critical for the checklist. If time permits, could add these as special print modes that output the identity text.

**Confidence:** HIGH — clear limitation, acceptable to skip.

## Architecture Patterns

### Test Script Pattern

The existing acceptance tests follow a consistent pattern. Use the same approach for the maple checklist test:

```bash
#!/usr/bin/env bash
set -e
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found"; exit 1; }

run() {
    printf '%s\n' "$@" | "$BIN" 2>&1
}

PASS=0; FAIL=0

# Block 1: Rogers-Ramanujan series
if run \
    "x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" \
    "series(x, 50)" \
    | grep -q "961"; then
    echo "PASS: Block 1"
    PASS=$((PASS+1))
else
    echo "FAIL: Block 1"
    FAIL=$((FAIL+1))
fi
```

Each block test pipes multiple commands into the REPL (so variable state persists between lines) and checks for a key value in the output using `grep`.

### Maple-to-qseries3 Translation Rules

| Maple Syntax | qseries3 Equivalent | Automated? |
|-------------|---------------------|------------|
| `with(qseries):` | (implicit) | N/A |
| `add(expr, n=lo..hi)` | `sum(expr, n, lo, hi)` | Manual |
| `series(f, q, T)` | `series(f, T)` | Drop middle arg |
| `prodmake(f, q, T)` | `prodmake(f, T)` | Drop middle arg |
| `etamake(f, q, T)` | `etamake(f, T)` | Drop middle arg |
| `jacprodmake(f, q, T)` | `jacprodmake(f, T)` | Drop middle arg |
| `sift(f, q, n, k, T)` | `sift(f, n, k, T)` | Drop 2nd arg |
| `findhom(L, q, n, ts)` | `findhom(L, n, ts)` | Drop 2nd arg |
| `findnonhom(L, q, n, ts)` | `findnonhom(L, n, ts)` | Drop 2nd arg |
| `findhomcombo(f,L,q,n,ts,opt)` | `findhomcombo(f, L, n, ts)` | Drop q,opt |
| `findnonhomcombo(f,L,q,N,ts,opt)` | `findnonhomcombo(f, L, N, ts)` | Drop q,opt |
| `findpoly(x,y,q,d1,d2,chk)` | `findpoly(x, y, d1, d2, chk)` | Drop q |
| `aqprod(a,q,n)` | `aqprod(a, q, n, T)` | Add T arg |
| `theta3(q^k, T)` | `theta3(q^k, T)` (after fix) | None needed |
| `qbin(q, m, n)` | `qbin(m, n, T)` | Drop q, add T |
| `F := q -> expr` | Compute F(q) and F(q^k) as separate variables | Manual expansion |
| `Q := n -> expr` | Expand each Q(k) call manually | Manual |

### Exercises Translation Approach

For each exercise, create a sequence of qseries3 commands that:
1. Define the necessary series
2. Apply the relevant function (prodmake, etamake, jacprodmake, findhom, etc.)
3. Verify the result

Exercises involving proofs or generalizations are out of scope — only the computational verification.

## Common Pitfalls

### Pitfall 1: theta Functions with q-Powers
**What goes wrong:** `theta3(q^2, 100)` silently returns theta3(q, 100) — wrong by a factor of subs_q(..., 2). All downstream computations using this result are incorrect.
**Why it happens:** theta2/3/4 implementations comment out the q parameter (`/*q*/`). The functions directly construct series using integer arithmetic on exponents, never referencing the actual q series passed.
**How to avoid:** Fix theta functions first, then test Block 15 (findhom with q^2 variants) as a smoke test. If the quadratic relations match Gauss AGM identities, the fix is correct.
**Warning signs:** findhom returning empty set or spurious relations when theta functions with q^k are involved.

### Pitfall 2: Truncation vs. Factor Count
**What goes wrong:** `tripleprod(q, q^3, 10)` produces a series truncated at O(q^10) instead of a polynomial with terms up to q^57.
**Why it happens:** T parameter is used for both `Series::one(T)` initialization and `truncTo(T)` in the inner loop.
**How to avoid:** After fixing, verify Block 29 output matches the pentagonal number terms: 1 - q - q² + q⁵ + q⁷ - q¹² - q¹⁵ + q²² + q²⁶ - q³⁵ - q⁴⁰ + q⁵¹ + q⁵⁷.
**Warning signs:** tripleprod/quinprod results missing high-degree terms that should be present.

### Pitfall 3: Variable Name T Conflict
**What goes wrong:** Assigning `T := expr` shadows the built-in `T(r,n)` function. After assignment, `T(8,8)` would try to evaluate `T` as a variable and apply `(8,8)` as arguments to a series, causing a confusing error.
**How to avoid:** In the test script, use `TT` or another name for the eta ratio variable from Block 23.

### Pitfall 4: Double Sum Performance
**What goes wrong:** `sum(sum(legendre(m,5)*n^5*q^(m*n), n, 1, 50), m, 1, 50)` computes 50×50 = 2500 terms, each involving series multiplication. Can be slow.
**Why it happens:** The naive upper bound of 50 for the inner loop is wasteful — many terms produce q-powers beyond truncation.
**How to avoid:** Use truncation to discard high-power terms automatically. The computation is correct but slow. For the checklist, set reasonable truncation (50) and accept the runtime.

### Pitfall 5: etaopt Formatting vs. Mathematical Correctness
**What goes wrong:** `findhomcombo(f, [B1,B2], 3, 0)` outputs `X₁²X₂ + 40X₁X₂² + 335X₂³` but Maple with `yes` option outputs `η(5τ)³η(τ)⁹ + 40η(5τ)⁹η(τ)³ + 335η(5τ)¹⁵/η(τ)³`.
**Why it happens:** etaopt is parsed but not implemented (`(void)etaopt`).
**How to avoid:** Accept the X-variable format as correct. The coefficients (1, 40, 335) are what matter. Document the formatting difference.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Symbolic simplification (radsimp) | Symbolic algebra engine | Direct series arithmetic | Series arithmetic implicitly "simplifies" by computing exact coefficients. No need for symbolic simplification. |
| Factor detection (Maple factor) | Polynomial factoring | qfactor | qfactor already converts to q-product form which is what all blocks need. |
| Algebraic number arithmetic | Complex/algebraic number support | Known limitation note | Would require entirely new number type. Exercise 4's b(q) can use real-valued workaround: `b(q) = 3*S0 - a(q)` where S0 avoids ω. |

## Exercise Assessment

### Exercises Feasibility

| Exercise | Feasibility | Key Commands | Gaps |
|----------|-------------|-------------|------|
| 1 | **Feasible** | `sum(q^(n^2)/aqprod(q,q,2*n,50), n, 0, 8)` then `prodmake(x,40)` | None |
| 2 | **Feasible** | `T(r,n)` + `qfactor(T(r,n), 20)` for various r,n | None |
| 3 | **Feasible** | `sum(...)` like dixson then `qfactor` | Same pattern as Block 6 |
| 4 | **Partial** | a(q) and c(q) computable via double sums. b(q) needs ω (RootOf). | b(q) workaround: `b(q) = 3*Σ Σ q^(3(n²+3nk+3k²)) - a(q)` — but needs double sum with non-standard exponent formula. Both a(q) and c(q) identifiable as eta products. |
| 5 | **Feasible** | `sum(q^(n*(3*n+1)/2)*aqprod(-q,q,n,50)/aqprod(q,q,2*n+1,50), n, 0, 10)` then jacprodmake+jac2series | None |
| 6 | **Partial** | findhom with a,b,c,a(q^3),b(q^3),c(q^3) | Depends on Exercise 4 (b(q) computation). a(q) and c(q) work fine. |
| 7 | **Feasible** | Define C1,C2,C3 as eta products, compute UE(q,3,7,50), use findhomcombo | Double sum for UE |
| 8 | **Feasible** | Same pattern as Block 23 | Variable name T conflict (use TT) |
| 9 | **Feasible** | Define N(q) via sum, compute c(q)^3/a(q)^3, findnonhomcombo | Needs a(q), c(q) from Exercise 4 |
| 10 | **Feasible** | `findpoly(m, y, 3, 1, 60)` after defining m and y | Needs theta q-arg fix for theta3(q^3,...) |
| 11 | **Feasible** | sift + jacprodmake for each r=0..4 | None |
| 12 | **Feasible** | etaq + sift + jacprodmake for mod 5 and mod 7 dissections | quinprod symbolic modes skipped |
| 13 | **Feasible** | tripleprod + winquist + jacprodmake | Needs tripleprod fix |

## Implementation Strategy

### Recommended Order

1. **Fix theta2/3/4 q-argument** (in `qfuncs.h`) — unblocks 7 blocks + 3 exercises
2. **Fix tripleprod/quinprod T semantics** (in `qfuncs.h`) — unblocks 5 blocks + 2 exercises
3. **Write test script** with all (A) and (B) blocks first — establishes pass/fail baseline
4. **Translate proc/for-loop blocks** (6, 11, 13, 17) to sum() expressions
5. **Fix variable T collision** — rename in test script (Block 23)
6. **Handle Block 19-22 series** — define F(q), F(q^5) manually, construct polynomial check
7. **Handle Block 25** — translate radsimp away with direct computation
8. **Handle Blocks 37-41** — expand Q(k) lambda manually, use large T for tripleprod/winquist
9. **Mark infeasible/symbolic blocks** — Block 10 (RootOf), Blocks 28/30/31/32 (symbolic z)
10. **Write exercises_solutions.md** — one exercise at a time

### Task Size Estimate

| Category | Effort |
|----------|--------|
| Code fixes (theta, tripleprod, quinprod) | ~2 hours |
| Test script (41 blocks) | ~3 hours |
| Block translations (proc→sum, workarounds) | ~2 hours |
| Exercise solutions | ~3 hours |
| Documentation + checklist update | ~1 hour |
| **Total** | **~11 hours** |

This is a large phase. Consider splitting into two sub-plans:
- **62-01:** Code fixes + blocks 1-27 (the easier half)
- **62-02:** Blocks 28-41 + all exercises + final checklist update

## Open Questions

1. **winquist T semantics:** Does our `winquist(a,b,q,T)` have the same T-semantics issue as tripleprod? The implementation uses `for (int n = 0; 3*n*(n+1)/2 < T; ++n)` for the outer loop and truncates at T. Need to verify if T=20 in Block 40 produces correct output.

2. **Symbolic z blocks (28, 30, 31, 32):** Should these be marked as known limitations, or should we add a minimal symbolic display mode for tripleprod/quinprod? Recommendation: mark as display-only limitations. The mathematical content is verified by the concrete-argument blocks (29, 36).

3. **Exercise 4 b(q) workaround:** The identity `b(q) = 3*S0 - a(q)` where `S0 = Σ_{n,k} q^(3(n²+3nk+3k²))` avoids ω. But does our double `sum()` handle this efficiently? The exponent `3(n²+3nk+3k²)` grows fast, so many terms contribute. Need to check truncation performance.

## Sources

### Primary (HIGH confidence)
- Direct source code audit: `src/parser.h`, `src/repl.h`, `src/qfuncs.h`, `src/convert.h`, `src/relations.h`, `src/series.h`
- Reference document: `qseriesdoc.md` (Garvan's tutorial with expected outputs)
- Checklist document: `maple_checklist.md`

### Secondary (HIGH confidence)
- Existing acceptance tests: `tests/acceptance.sh` and related scripts (verified working patterns)
- Phase context: CONTEXT.md decisions (user constraints)

## Metadata

**Confidence breakdown:**
- Block status assessment: HIGH — based on direct source code audit
- Code fix specifications: HIGH — bugs clearly identified with concrete fixes
- Exercise feasibility: HIGH — traced through each computation path
- Effort estimates: MEDIUM — depends on implementation approach and edge cases encountered

**Research date:** 2026-03-01
**Valid until:** Stable — source code changes only through planned phases
