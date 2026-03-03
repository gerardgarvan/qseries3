# Pitfalls Research — Adding RootOf (Algebraic Numbers) to Q-Series REPL

**Domain:** Adding algebraic number support (RootOf for ω = exp(2πi/3), cyclotomic Q(ω)) to an existing exact-arithmetic q-series REPL
**Researched:** 2026-03-03
**Confidence:** HIGH (verified against codebase, Maple docs, domain literature)

---

## Critical Pitfalls

### Pitfall 1: Coefficient Growth in Q(ω) Without Normalization

**What goes wrong:**
Elements of Q(ω) are represented as a + b·ω with ω² + ω + 1 = 0. Multiplication gives (a+bω)(c+dω) = (ac − bd) + (ad + bc − bd)ω. When used as Series coefficients, repeated arithmetic (especially Series multiplication, inverse, division) causes numerators and denominators to grow without bound. BigInt already handles Frac growth via GCD reduction; Q(ω) has no built-in reduction.

**Why it happens:**
- Series multiplication: O(T²) coefficient multiplications, each Q(ω) multiply involves 4 Frac ops
- Series inverse: recurrence g[n] = −(1/c₀) Σ cⱼ·g[n−j]; each step divides by c₀ and accumulates sums
- No equivalent of Frac::reduce() for algebraic numbers — denominators in the (a,b) pair can grow independently

**How to avoid:**
1. Define a normal form: store elements as (a, b) ∈ Q×Q with a common denominator d, then reduce (a,b,d) so gcd(a,b,d)=1
2. After every Q(ω) arithmetic op, apply reduction (e.g., extract gcd of numerators and divide through)
3. Consider lazy normalization with periodic cleanup (e.g., every N operations) — but beware intermediate overflow

**Warning signs:**
- Coefficient display shows huge integers (e.g., 10+ digit num/den) for simple inputs like a(q) or b(q)
- Memory or runtime spikes when computing b(q) via double sum with ω

**Phase to address:** RootOf type design phase — must be addressed before Series<RootOf> arithmetic

---

### Pitfall 2: Normal Form Ambiguity — Which Root of z²+z+1=0?

**What goes wrong:**
The polynomial z²+z+1=0 has two roots: ω = exp(2πi/3) and ω² = exp(4πi/3) = ω̄. They satisfy ω + ω² = −1, ω·ω² = 1. Maple’s `RootOf(z^2+z+1=0)` without index uses an implementation-defined choice. If your normal form differs from Maple’s, outputs will not match Block 10 or cross-check with Maple.

**Why it happens:**
Algebraically, Q(ω) ≅ Q(ω²) — the two roots define the same field. But for I/O, debugging, and Maple parity, you need a deterministic choice.

**How to avoid:**
1. Document the canonical choice explicitly (e.g., “we use the root with positive imaginary part, i.e. ω = exp(2πi/3)”)
2. If matching Maple: Maple orders roots by smallest |arg|, then positive arg, then smallest modulus. For z²+z+1, both roots have |z|=1; the one with arg = 2π/3 is typically index=1. Verify with Maple’s `evalf(RootOf(z^2+z+1, index=1))` → −0.5 + 0.866…I
3. Ensure all internal operations use the same convention; never mix ω and ω² as distinct unless explicitly modeling conjugates

**Warning signs:**
- b(q) coefficients match Maple’s up to a global sign or conjugate
- findhom/findpoly results differ from Maple only by ω ↔ ω² substitution

**Phase to address:** RootOf design phase — define and document canonical root choice before implementation

---

### Pitfall 3: Conjugate Handling — Silent Wrong Answers When ω and ω̄ Are Confused

**What goes wrong:**
In many identities, b(q) involves ω^(m−n). The conjugate series b̄(q) uses ω² instead of ω. If code paths treat ω and ω² as interchangeable (e.g., “both satisfy z²+z+1”), you can get b(q) and b̄(q) confused. For real outputs (e.g., a(q) = b(q)+c(q)+…), this may cancel; for complex series it produces wrong results.

**Why it happens:**
- Display routines that “simplify” ω^3 → 1 without tracking power mod 3 can lose the distinction between ω and ω²
- Copy-paste from formulae that use ω̄ vs ω inconsistently

**How to avoid:**
1. Store ω explicitly; reduce ω^k to ω^(k mod 3) with k ∈ {0,1,2} and never collapse ω^1 and ω^2
2. Provide a separate `conjugate()` function if needed; do not overload equality
3. For a(q), b(q), c(q): a(q) is real (no ω), b(q) and c(q) involve ω — ensure the double-sum indexing matches the intended root

**Warning signs:**
- b(q) + conjugate(b(q)) does not equal a real series
- etamake(b(q)) fails or gives unexpected product (b(q) should be η(τ)³/η(3τ))

**Phase to address:** RootOf phase — implement and test conjugate handling; Block 10 verification

---

### Pitfall 4: findhom / findnonhom Assume Rational Coefficients — Kernel Over Q, Not Q(ω)

**What goes wrong:**
`findhom` builds a matrix M whose rows are coefficient vectors of monomials (from `build_matrix` → `coeffList`). `kernel(MT)` in `linalg.h` performs Gaussian elimination over **Q** (Frac). If series have coefficients in Q(ω), the matrix entries are algebraic numbers. The existing `gauss_to_rref` and `kernel` divide by pivots and perform linear combinations — they assume a field, but the field must be Q(ω), not Q. Treating Q(ω) elements as Frac (e.g., by projecting to Q) is incorrect.

**Why it happens:**
Current design: `Series` has `std::map<int, Frac> c`. `build_matrix` extracts `Frac` via `coeffList`. The kernel is over Q. There is no `kernel_over_Qomega`.

**How to avoid:**
1. **Option A:** Implement `Linalg<RootOf>` — Gaussian elimination and kernel over Q(ω). Represent matrix entries as (a,b) ∈ Q×Q; pivoting and row operations require division in Q(ω), which is well-defined (field).
2. **Option B:** For relations among series in Q(ω), first split into real/imaginary or 1/ω/ω² components. Each component is a Q-series. Run findhom on the Q-linear span of these components. More complex but reuses existing Q-linalg.
3. **Option C:** Restrict findhom to inputs with rational coefficients; document that algebraic series are not supported for relation finding in V1.

**Warning signs:**
- findhom([a(q), b(q)], 2, 0) crashes, returns empty, or gives spurious relations
- Kernel vectors contain non-rational entries when they should be in Q(ω)

**Phase to address:** RootOf integration phase — extend linalg or add findhom_qomega; or explicitly defer

---

### Pitfall 5: Series Inverse Requires Invertible Constant Term in Q(ω)

**What goes wrong:**
`Series::inverse()` computes g[0]=1/c₀ and g[n]=−(1/c₀) Σ cⱼ·g[n−j]. It assumes c₀ is invertible in the coefficient ring. For Frac, c₀≠0 suffices. For Q(ω), c₀≠0 also suffices (field). The algorithm is correct in principle, but implementation must use Q(ω) division, not Frac division.

**Why it happens:**
If you templatize or generalize Series to `Series<C>` with coefficient type C, the inverse implementation will call `C(1)/c0` and `scale * v` where scale is in C. The current code uses Frac; replacing with RootOf requires RootOf division. If RootOf is not a field (e.g., buggy zero-divisor), inverse can produce garbage.

**How to avoid:**
1. Ensure RootOf forms a field: every nonzero element has an inverse
2. Implement `operator/` for RootOf: (a+bω)^{-1} = (a+bω̄)/(a²−ab+b²) using the conjugate to clear the denominator
3. Add a fast path: when the series has rational coefficients (all ω-parts zero), delegate to existing Frac-based inverse for performance

**Warning signs:**
- `(etaq(1,50)*omega).inverse()` or `1/(aq*omega)` crashes or returns wrong series
- Division by a series with constant term 1+ω produces incorrect coefficients

**Phase to address:** RootOf + Series integration phase — RootOf division, then Series inverse with RootOf coefficients

---

### Pitfall 6: prodmake Assumes Rational Leading Coefficient and Normalization

**What goes wrong:**
`prodmake` normalizes by `scale = 1/b0` so that b[0]=1, then runs Andrews’ recurrence. The recurrence uses Frac arithmetic throughout. If f has coefficients in Q(ω), then b0 ∈ Q(ω), and normalization requires division in Q(ω). The output exponents a[n] are computed as (c[n] − Σ d·a[d])/n. For integer-product identification, a[n] are expected to be rational (or half-integer for jacprodmake). With algebraic coefficients, a[n] can lie in Q(ω). The interpretation of “exponent of (1−q^n)” as an algebraic number is non-standard — prodmake’s output format assumes rational exponents.

**Why it happens:**
Andrews’ algorithm is defined over any field. The issue is the *output* semantics: prodmake returns a map from n to exponent. For eta products and Jacobi products, those exponents are rational. If the input is a(q) or b(q), the product form may involve roots of unity in the coefficients of the product factors, not in the exponents. Mixing the two causes confusion.

**How to avoid:**
1. **For Block 10:** The immediate goal is a(q), b(q), c(q) as series. prodmake on these series is used for product identification. If b(q)=η³/η(3τ), then prodmake(b(q)) should yield rational exponents. Ensure that when the series is actually an eta product with rational exponents, prodmake still works.
2. Generalize prodmake to accept Series<RootOf> and return `std::map<int, RootOf>` — but document that etamake/jacprodmake expect rational exponents.
3. Alternatively: keep prodmake as Frac-only; require users to extract real/rational parts before product conversion when needed.

**Warning signs:**
- prodmake(b(q)) returns empty or errors because b0 or intermediate values are algebraic
- prodmake output contains ω in exponents, and etamake refuses it

**Phase to address:** RootOf integration phase — prodmake over Q(ω) or documented restrictions

---

### Pitfall 7: etamake Explicitly Requires Integer Coefficients

**What goes wrong:**
`etamake` at `convert.h:137–148` checks `c.den != BigInt(1)` and rejects non-integer coefficients. It interprets coefficients as eta exponents. For series with coefficients in Q(ω), every coefficient has the form a+bω. There is no “integer” in Q(ω) in the same sense — the ring of integers of Q(ω) is Z[ω], with elements n+mω.

**Why it happens:**
etamake is designed for rational-coefficient series that are eta products. The algorithm finds the smallest k with nonzero coefficient and cancels by multiplying by etaq^k. For algebraic coefficients, “smallest k” still makes sense, but the coefficient is in Q(ω). The cancellation step `g = g * et.pow(c_int)` requires c_int to be an integer. You cannot use c_int from Q(ω) without a convention (e.g., “take the rational part” or “require coefficient in Z”).

**How to avoid:**
1. Keep etamake as Frac-only. Document that etamake does not support algebraic coefficients.
2. For b(q): use the eta identity b(q)=η(τ)³/η(3τ) and call etamake on the rational series that represents it (e.g., via the workaround in FEATURES.md). Block 10’s purpose is to define omega; the a,b,c series can be obtained without etamake on algebraic series.
3. If extending etamake: define “eta exponent” in Q(ω) or Z[ω] and implement cancellation — research required, likely out of scope for initial RootOf.

**Warning signs:**
- etamake(b(q)) fails with “non-integer coefficient” or similar
- Attempt to “rationalize” b(q) before etamake produces wrong product

**Phase to address:** RootOf phase — document etamake limitation; use b(q)=η³/η(3τ) workaround for Block 10 / Exercise 4

---

### Pitfall 8: Series Multiplication and Addition — Type Consistency

**What goes wrong:**
Mixing Series<Frac> and Series<RootOf> in one expression (e.g., `aq + omega * bq`) requires either (a) automatic promotion of Frac to RootOf (Frac as a+bω with b=0), or (b) a common coefficient type. If both coexist, addition/multiplication must handle mixed types. Ad hoc handling leads to ambiguity (e.g., is 1+omega a Frac or RootOf?).

**Why it happens:**
The REPL evaluates expressions like `aq + omega * bq`. If aq is Series<Frac> and omega is RootOf, then omega*bq is Series<RootOf>. Adding aq requires promoting aq to Series<RootOf> or defining operator+(Series<Frac>, Series<RootOf>).

**How to avoid:**
1. Use a single coefficient type that subsumes Q: e.g., `Algebraic` = Frac | RootOf(ω). Frac promotes to Algebraic (as 0·ω + frac).
2. Or: make Series always store the “largest” coefficient type in use. Once any coefficient is RootOf, the whole series is Series<RootOf>.
3. Avoid ad hoc overloads; centralize promotion in one place.

**Warning signs:**
- `aq + omega` fails to compile or gives wrong result
- Some code paths treat omega as Frac (e.g., 0) and others as RootOf

**Phase to address:** RootOf type design — define promotion and common type before REPL integration

---

## Moderate Pitfalls

### Pitfall 9: Double-Sum b(q) — O(T²) Series Ops with RootOf Coefficients

**What goes wrong:**
b(q) = Σ_{m,n} ω^(m−n) q^(n²+nm+m²). A naive implementation creates a Series for each (m,n), multiplies by ω^(m−n) (a RootOf), and adds. Each addition merges coefficient maps; each coefficient add involves Q(ω) arithmetic. The double sum has O(√T × √T) = O(T) terms, but each term triggers Series addition. With RootOf coefficients, each add is more expensive than Frac. Performance can degrade significantly.

**How to avoid:**
1. Accumulate into a single `std::map<int, RootOf>` instead of creating intermediate Series per term
2. Precompute ω^k for k∈{0,1,2} once; use mod 3 for exponent
3. Prefer the eta identity b(q)=η³/η(3τ) when possible — avoids ω entirely and is O(T) in eta product computation

**Phase to address:** RootOf performance phase — optimize double-sum evaluation for a(q), b(q), c(q)

---

### Pitfall 10: Display and Parsing — ω vs ω² vs -1-ω

**What goes wrong:**
Users expect to see `omega` or `ω` in output. Internal form may be (a,b) for a+bω. Reducing ω² to −1−ω is correct algebraically but may confuse if output shows `-1-omega` instead of `omega^2`. Parsing `omega^2` or `ω²` must map to the correct internal representation.

**How to avoid:**
1. Define a canonical display: e.g., always reduce to a+bω with a,b ∈ Q, and display as `a + b*omega` (or `a + bω`)
2. Parser: accept `omega`, `ω`, `RootOf(z^2+z+1)` and normalize to the canonical RootOf
3. For ω², display as `omega^2` or `-1-omega` consistently — document the choice

**Phase to address:** RootOf display/parse phase

---

### Pitfall 11: jacprodmake and prodmake with Algebraic Exponents

**What goes wrong:**
jacprodmake consumes prodmake output and decomposes exponents into JAC factors. JAC factors have exponents that are currently Frac (including half-integers). If prodmake returns exponents in Q(ω), jacprodmake’s decomposition logic (e.g., `x[b/2] = e[b/2]/2`) assumes real rationals. Decomposition into JAC(a,b)^e with e ∈ Q(ω) is not standard.

**How to avoid:**
1. Restrict jacprodmake to rational-exponent prodmake output
2. If prodmake over Q(ω) produces exponents in Q(ω), run jacprodmake only when the exponents can be projected to Q (e.g., when the series is actually an eta/Jacobi product with rational exponents)

**Phase to address:** RootOf integration — document jacprodmake restriction

---

## Technical Debt Patterns

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Skip normalization in Q(ω) ops | Faster arithmetic | Coefficient explosion, overflow | Never — causes wrong/slow results |
| Use Frac for ω (e.g., approximate) | Reuse existing code | Wrong answers; ω²≠1 in Q | Never |
| Defer findhom over Q(ω) | Ship RootOf without linalg changes | Cannot find relations among a,b,c | Acceptable for MVP if Block 10 only needs a,b,c as series |
| etamake Frac-only, use eta identity for b(q) | No etamake changes | b(q) must use workaround | Acceptable — identity is standard |

---

## Integration Gotchas

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| findhom + RootOf | Using existing kernel(M) over Q | Implement kernel over Q(ω) or decompose into Q-components |
| prodmake + RootOf | Assuming b[0] is Frac | Normalize with 1/b0 in Q(ω); output may have algebraic exponents |
| etamake + RootOf | Passing b(q) directly | Use eta identity b(q)=η³/η(3τ); etamake remains Frac-only |
| Series inverse + RootOf | Using Frac division for 1/c0 | Use RootOf division; ensure RootOf is a field |
| sift + RootOf | No change needed | sift extracts coefficients; works if coeff() returns RootOf |

---

## Performance Traps

| Trap | Symptoms | Prevention | When It Breaks |
|------|----------|------------|----------------|
| Coefficient growth in Q(ω) | Huge num/den in coeffs | Normalize after every op; periodic gcd reduce | T≈100+ with nested products |
| Double-sum b(q) | Slow evaluation | Accumulate into map; use eta identity | T≈200 with naive loop |
| findhom over Q(ω) | Slow kernel | Consider Q-linear decomposition first | Many series, degree ≥2 |

---

## "Looks Done But Isn't" Checklist

- [ ] **RootOf arithmetic:** Often missing reduction/normalization — verify coeff magnitude stays bounded for a(q), b(q)
- [ ] **Maple parity:** Often missing index convention — verify RootOf(z^2+z+1) matches Maple’s choice
- [ ] **findhom:** Often left as Q-only — verify whether Block 10 or exercises need findhom on algebraic series
- [ ] **prodmake/etamake:** Often assumes Frac — verify documented limits and workarounds
- [ ] **Series inverse:** Often works for Frac but not RootOf — test 1/(f*omega) for generic f

---

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Coefficient growth | RootOf type design | Run a(q), b(q) to T=100; inspect coeff size |
| Normal form choice | RootOf design | Compare b(q) coeffs with Maple |
| Conjugate confusion | RootOf + a,b,c | b(q)+conjugate(b(q)) is real |
| findhom over Q(ω) | RootOf + relations | findhom([aq, bq], 2) if in scope |
| Series inverse | RootOf + Series | 1/(etaq(1,50)*omega) correct |
| prodmake algebraic | RootOf + convert | prodmake(b(q)) or document N/A |
| etamake integer check | RootOf integration | etamake(b(q)) — expect use eta identity instead |
| Type consistency | RootOf design | aq + omega*bq type-checks and runs |

---

## Phase-Specific Warnings

| Phase Topic | Likely Pitfall | Mitigation |
|-------------|----------------|------------|
| RootOf type | Coefficient growth | Implement normalize() and call after every op |
| RootOf type | ω vs ω² | Document canonical root; never equate |
| Series<RootOf> | inverse, prodmake | Generalize or restrict; document |
| findhom | Q vs Q(ω) kernel | Extend linalg or defer |
| Block 10 | etamake on b(q) | Use b(q)=η³/η(3τ); do not pass algebraic series to etamake |
| a,b,c sums | Performance | Accumulate into map; prefer eta identity for b(q) |

---

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Coefficient growth | MEDIUM | Add normalization pass; re-run; may need to increase normal form precision |
| Wrong root choice | LOW | Swap ω ↔ ω² globally; re-verify against Maple |
| findhom wrong | HIGH | Implement Q(ω) kernel or restrict to Q inputs |
| etamake rejects b(q) | LOW | Use eta identity; no code change needed |

---

## Sources

- Maple RootOf indexed: https://www.maplesoft.com/support/help/maple/view.aspx?path=RootOf%2Findexed (ordering convention)
- `src/relations.h`, `src/linalg.h` — findhom, kernel (Q-only)
- `src/series.h` — inverse, operator* (Frac-based)
- `src/convert.h` — prodmake, etamake (Frac, integer checks)
- `maple_checklist.md` Block 10 — RootOf(z^2+z+1=0)
- `.planning/research/FEATURES.md` — b(q) eta identity, a(q), c(q) definitions
- Cyclotomic Q(ω): ω²+ω+1=0, ω³=1; elements a+bω; inverse (a+bω)^{-1} = (a+bω̄)/(a²−ab+b²)
