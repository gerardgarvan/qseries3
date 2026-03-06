# Domain Pitfalls: qseries3 Gap Closure

**Domain:** Maple qseries parity in a zero-deps C++ REPL
**Researched:** 2026-03-06
**Context:** v11.1 — Close gaps, fix problems. Common mistakes when adding Maple parity features.

---

## Critical Pitfalls

### Pitfall 1: Series inverse is the #1 source of bugs

**What goes wrong:** Incorrect inverse recurrence, wrong truncation after shift, or wrong recurrence bounds.

**Why it happens:** The recurrence `g[n] = -(1/c₀) Σ c_j g[n-j]` must use `j` from 1 to `n`, not 0. For series with non-zero leading exponent (e.g. `q^m * f`), shift-first logic must correctly adjust truncation to `T - m`.

**Consequences:** Rogers-Ramanujan fails; prodmake, etamake, and any division produce garbage; b(q) = η³/η(3τ) can diverge or give wrong coefficients.

**Prevention:** Unit-test inverse on `(1-q)`, `q*(1-q)`, and series with leading exponents 1, 2. Verify `f * (1/f)` = 1 to truncation.

**Detection:** Rogers-Ramanujan prodmake yields denominators at exponents other than ±1 (mod 5).

**Phase to address:** Phase 3 (Series); regression in any phase that touches inverse/shift.

---

### Pitfall 2: Truncation propagation and q-shift alignment

**What goes wrong:** Two series with different truncations or different q-shifts get combined (added, findpoly inputs), producing wrong coefficients or silent truncation errors.

**Why it happens:** Maple silently normalizes; C++ requires explicit `min(T₁,T₂)` and q-shift alignment before addition/relation finding.

**Consequences:** findpoly on theta quotients fails (Block 25); addAligned or equivalent must normalize q_shift before combining. Wrong truncation in sum-of-products yields garbage beyond effective truncation.

**Prevention:** Always align q_shift before addition; propagate truncation as `min` in *, /, +; document truncation semantics in every new q-series function.

**Detection:** Block 25 fails; findpoly on theta2/theta3 quotients returns no relation or wrong relation.

**Phase to address:** Phase 63 (q-shift fix), Phase 97 (Block 25); any phase adding new arithmetic on Series.

---

### Pitfall 3: Maple-only semantics assumed in C++

**What goes wrong:** Implementing a Maple feature without verifying exact semantics: argument order, optional-arg defaults, indexing (1-based vs 0-based), output formatting.

**Why it happens:** Maple qseries package is the reference; C++ reimplementation assumes rather than verifies semantics.

**Consequences:** Maple-checklist blocks fail; user expectations mismatch; “it works in Maple” debugging loops.

**Prevention:** For each Maple parity feature: (1) run the exact Maple command, (2) document expected output, (3) implement to match, (4) add a real test (no skip) in maple-checklist.sh.

**Detection:** New feature passes locally but maple-checklist block still skips or fails; help text contradicts Maple behavior.

**Phase to address:** Every phase that adds Maple parity (Blocks 1–41, exercises); Phase 62 (maple-checklist).

---

### Pitfall 4: Double-sum / omega arithmetic truncation (EX-04c)

**What goes wrong:** `b(q) = Σ_n Σ_m ω^(n-m) q^(n²+nm+m²)` fails to produce `1 - 3q` when truncation or summation range is too small.

**Why it happens:** Double sum converges slowly; T=60 and range ±12 may be insufficient. SeriesOmega accumulation can also lose precision or mis-handle omega^k reduction if truncation is wrong.

**Consequences:** EX-04c fails; b(q) double-sum does not match eta identity `η(τ)³/η(3τ)`.

**Prevention:** Increase truncation (e.g. 80–100) and range (e.g. ±15) for b(q) double-sum test; verify omega^k reduction and SeriesOmega truncation semantics.

**Detection:** `acceptance-exercises.sh` EX-04c fails; `b_sum` does not start with `1 - 3q`.

**Phase to address:** Phase 66 (exercise regression), Phase 94 (b(q) / Block 10); omega/SeriesOmega phases (91–93).

---

### Pitfall 5: Sturm-bound precision in provers

**What goes wrong:** `provemodfuncGAMMA0id` and `provemodfuncid` use Sturm bounds derived from `mintot`; if `mintot` is a fraction or exceeds machine-int range, conversion to `mintot_int` fails or truncates.

**Why it happens:** eta_cusp.h and theta_ids.h cast `mintot` to `int` when `mintot.den == 1`; large or fractional `mintot` is not handled. Cap at 500 can mask insufficient verification depth.

**Consequences:** Identity provers report “proven” when verification depth was wrong; or fail to prove valid identities; false negatives on high-level identities.

**Prevention:** Use BigInt-based Sturm computation when mintot is large; avoid hard cap without documenting; add explicit “verification depth” in output.

**Detection:** provemodfuncGAMMA0id/provemodfuncid return `proven=1` for identities that fail at higher depth; or fail for identities that should hold.

**Phase to address:** Phase 82 (ETA identity prover), Phase 85 (theta identity prover); any phase touching Sturm bounds.

---

### Pitfall 6: Block 24 / collect-style formatting trap

**What goes wrong:** Attempting to implement Maple’s `collect(%[1],[X[1]])` as a core feature, or treating output order as significant.

**Why it happens:** Block 24 is Maple formatting-only; mathematically equivalent output differs only in term grouping. Implementing full collect is out of scope for a C++ REPL.

**Consequences:** Scope creep; wasted effort; confusion when Block 24 remains N/A.

**Prevention:** Document Block 24 as N/A with explicit rationale (Maple formatting-only; output equivalent). Do not implement collect unless explicitly scoped.

**Detection:** Phase 98 scope expands to “full collect”; Block 24 test expects exact Maple formatting.

**Phase to address:** Phase 98 (Block 24 N/A rationale); do not add collect in v11.1.

---

### Pitfall 7: Frac reduction and BigInt growth

**What goes wrong:** Forgetting to reduce fractions after operations; BigInt numerators/denominators grow exponentially.

**Why it happens:** Frac operations produce unreduced intermediates; reduce() must run on every constructor and op.

**Consequences:** Slowdown; overflow in long chains (findhom, prodmake, etaq).

**Prevention:** reduce() in every Frac constructor and after every +, -, *, /.

**Detection:** Benchmarks degrade on long computations; very large coefficient strings in output.

**Phase to address:** Phase 2 (Frac); regression in any phase doing heavy rational arithmetic.

---

### Pitfall 8: etamake / jacprodmake convergence and periodicity

**What goes wrong:** etamake loops forever on non–eta-product input; jacprodmake requires exact periodicity and fails on near-periodic patterns.

**Why it happens:** No iteration cap in etamake; jacprodmake uses strict equality instead of threshold.

**Prevention:** Cap etamake iterations; use ~80% periodicity threshold in jacprodmake; verify reconstruction matches original series.

**Detection:** etamake hangs; jacprodmake fails on Rogers-Ramanujan or Euler pentagonal dissection.

**Phase to address:** Phase 6 (convert-extended); any phase extending etamake/jacprodmake.

---

## Moderate Pitfalls

### Pitfall 9: Optional-argument and signature drift

**What goes wrong:** New function added with different optional-arg semantics than Maple; e.g. `etaq(k)` vs `etaq(k,T)`, `findmaxind(L)` vs `findmaxind(L,n,topshift)`.

**Prevention:** Audit each new built-in for 1/2/3-arg variants; document defaults in help(); add acceptance-optional-args test.

**Phase to address:** Phase 29 (optional-arg audit); every phase adding built-ins.

---

### Pitfall 10: Output format mismatch (Unicode vs ASCII)

**What goes wrong:** Tests grep for `X[1]` but output uses `X₁`; tests expect ASCII `eta` but output uses `η`.

**Prevention:** Use UTF-8 byte sequences in grep (`\xCE\xB7` for η); document output format in MANUAL.md; keep format consistent across relation finders.

**Phase to address:** Phase 66 (exercise tests); Phase 89 (list indexing); relation phases (08, 78).

---

### Pitfall 11: Integer vs rational coercion at boundaries

**What goes wrong:** REPL passes integer to function expecting Frac or Series; or built-in returns int when user expects Series.

**Prevention:** Eval layer must coerce appropriately; help() documents types; runtime errors for invalid types.

**Phase to address:** Parser/REPL phases; every new built-in.

---

## Minor Pitfalls

### Pitfall 12: findhom matrix size blowup

**What goes wrong:** With k series and degree n, monomial count is C(n+k-1, k-1). k=6, n=4 → 126 monomials; matrix can be large.

**Prevention:** Document complexity; consider topshift and T limits; avoid unbounded growth in kernel computation.

**Phase to address:** Phase 8 (relations); Phase 67 (findhommodp).

---

### Pitfall 13: Maple proc vs C++ function equivalence

**What goes wrong:** Maple `T:=proc(r,j)...` uses `option remember`; C++ T_rn must use memoization or equivalent.

**Prevention:** Implement memoization for recursive procs; verify T(8,8) and similar match Maple.

**Phase to address:** Phase 4 (qfuncs), Phase 42 (T_rn memoization).

---

## Phase-Specific Warnings

| Phase / Area           | Likely Pitfall                         | Mitigation                                                   |
|------------------------|----------------------------------------|--------------------------------------------------------------|
| EX-04c / b(q)          | Double-sum truncation, omega arithmetic| Increase T and range; verify SeriesOmega semantics           |
| Block 24               | Implementing collect                   | Document N/A; do not implement                               |
| Sturm-bound provers    | mintot→int overflow, 500 cap           | BigInt Sturm; document cap; explicit depth in output         |
| Block 25 / findpoly    | q-shift misalignment                   | addAligned before findpoly; verify theta quotients           |
| Maple checklist        | Skip instead of real test              | Add real test or explicit N/A rationale                      |
| New built-ins          | Optional-arg drift                     | Audit 1/2/3-arg; add optional-arg test                       |
| Series arithmetic      | Truncation, inverse                    | Unit tests; Rogers-Ramanujan canary                          |

---

## Known Issues (v11.1 Context)

| Issue            | Severity | Status  | Phase to Fix       |
|------------------|----------|---------|--------------------|
| EX-04c failure   | Medium   | Open    | 66, 94, omega      |
| Block 24 skip    | Low      | N/A     | 98 (rationale)     |
| Sturm-bound cap  | Medium   | Document| 82, 85             |

---

## Warning Signs When Closing Maple Parity Gaps

1. **"Works in Maple"** — If the only verification is "Maple does it", add a concrete test with expected output.
2. **New skip in maple-checklist** — Prefer real test; if N/A, document rationale explicitly.
3. **Optional args omitted** — New built-in should support 1-arg, 2-arg, 3-arg where Maple does.
4. **Output format change** — Relation finders use X₁, X₂; etamake uses η. Do not regress to ASCII without updating tests.
5. **Truncation increase** — If a test needs higher T to pass, document why; consider if algorithm is wrong.
6. **Sturm_bound=0 or proven=0** — Prover may have mintot conversion issue; check eta_cusp.h, theta_ids.h.

---

## Sources

- SPEC.md (Common Pitfalls and Warnings, §576+)
- .planning/PROJECT.md, ROADMAP.md, STATE.md
- tests/acceptance-exercises.sh, maple-checklist.sh
- src/eta_cusp.h, src/theta_ids.h (Sturm bounds)
- .planning/phases/66-exercise-solutions-regression, 97-block-25-fix, 102-cross-package-integration
- Maple CodeGeneration limitations (WebSearch: Maple to C++ porting)
