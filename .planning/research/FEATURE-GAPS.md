# Feature Gaps Analysis: Q-Series REPL vs Maple qseries

**Purpose:** Identify what the qseries REPL is missing compared to Frank Garvan's Maple qseries package and related tooling. Informs prioritization for future milestones.

**Sources:** qseries.org functions index, qseriesdoc.md, FEATURES.md, MANUAL.md, SPEC.md  
**Date:** 2026-02-25

---

## Summary

The REPL implements the **core mathematical and UX features** from the Maple package. Gaps fall into four buckets:

1. **Additional Maple functions** — mprodmake, qetamake, findlincombo, etc.
2. **Utility and meta** — version, qdegree, checkprod, checkmult
3. **Mod-p variants** — findhommodp, findlincombomodp (modular arithmetic)
4. **Output and polish** — LaTeX, optional formats, richer display

---

## Maple qseries Functions — Coverage

### Implemented (28 built-ins)

| Category | Functions |
|----------|-----------|
| q-Series building | aqprod, qbin, etaq, theta, theta2, theta3, theta4, T |
| Products | tripleprod, quinprod, winquist |
| Conversion | prodmake, etamake, jacprodmake, qfactor |
| Coefficients | series, coeffs, sift |
| Relations | findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly |
| Other | subs_q, jac2prod, set_trunc, legendre, sigma, sum/add |

**Note:** `jac2series` exists in convert.h for verification but is not exposed as a user-callable built-in. `jac2prod(var)` displays the Jacobi product stored in a variable.

---

## Gaps — Maple Functions We Don't Have

### 1. Product conversion variants

| Function | Maple description | Use case | Complexity |
|----------|-------------------|----------|------------|
| **mprodmake** | Convert q-series to product (1+q^n1)*(1+q^n2)*... | Different product form; complements prodmake | Medium |
| **qetamake** | Variant of etamake | Alternative eta conversion | Low (if etamake exists) |
| **zqfactor** | (z,q)-series to (z,q)-product | Bivariate series; theta-like | High |

### 2. Relation-finding extensions

| Function | Maple description | Use case | Complexity |
|----------|-------------------|----------|------------|
| **findlincombo** | Express q-series as linear combination of list | Simpler than findhomcombo (degree 1) | Low |
| **findlincombomodp** | Same, but coefficients mod p | Modular arithmetic; congruences | Medium |
| **findhommodp** | Homogeneous relations mod p | Modular relations | Medium |
| **findhomcombomodp** | Homogeneous polynomial mod p | Same for combo variant | Medium |
| **findcong** | Find linear congruence | Congruence relations | Medium |
| **findmaxind** | Maximal independent subset of q-series | Determine rank of list | Medium |
| **findprod** | Products that are linear combination of q-series | Different relation form | Medium |

### 3. Utility / degree / checking

| Function | Maple description | Use case | Complexity |
|----------|-------------------|----------|------------|
| **qdegree** | Degree in q (highest exponent) | Inspect series | Low |
| **lqdegree** | Lowest degree in q (q-polynomial) | Inspect series | Low |
| **lqdegree0** | Lowest degree (q-monomial) | Fine-grained inspection | Low |
| **checkprod** | Check q-series is a "nice" product | Validation | Low–Medium |
| **checkmult** | Check coefficients are multiplicative | Modular form property | Medium |
| **packageversion** | Package version | Meta | Trivial |
| **changes** | List of recent changes | Changelog / meta | Trivial |

### 4. Jacobi / sum conversion

| Function | Maple description | Use case | Complexity |
|----------|-------------------|----------|------------|
| **qs2jaccombo** | Convert sum of q-series to sum of jacprods | Combinatorial identities | High |
| **jac2series** (user API) | Convert jacprod to series | We have impl, not exposed | Trivial |

---

## Gaps — Output and UX

| Gap | Description | Priority |
|-----|-------------|----------|
| **LaTeX output** | Optional `latex(f)` or `--latex` mode | Deferred (v2) |
| **version** | `version` or `--version` command | Low |
| **Rich coefficient display** | Optional formats (maple vs compact) | Low |
| **Batch export** | Write results to file from REPL | Low |

---

## Gaps — qseriesdoc Examples We Can't Fully Reproduce

From qseriesdoc.md, these examples use Maple capabilities we lack:

1. **UE (Eisenstein series)** — User-defined proc in Maple; we have legendre but no built-in `UE(q,k,p,T)`. Users would need to express via sum/add. **Possible built-in:** `eisenstein(q,k,p,T)`.

2. **radsimp** — Maple symbolic simplification. We use exact rationals; no symbolic algebra. **Gap:** Minor; our outputs are already reduced.

3. **RootOf / omega** — Complex roots (e.g. ω = exp(2πi/3)). We work over Q. **Gap:** Fundamental; a(q), b(q), c(q) with ω are outside our scope without extension to cyclotomic fields.

4. **theta(q^10, q^25, 5)** — Two-parameter theta with different bases. Our `theta(z,q,T)` uses z^n q^(n²); different bases need `subs_q` composition. **Gap:** May need `theta2(z, q, base1, base2, T)` or similar.

5. **Dixon sum (dixson proc)** — Sum with (-1)^k and qbin. Users can write via sum/add. **Gap:** No built-in; acceptable.

---

## Prioritized Gap Recommendations

### High value, moderate effort

| Gap | Why | Effort |
|-----|-----|--------|
| **findlincombo** | Simpler than findhomcombo; common use case | Low |
| **qdegree / lqdegree** | Basic inspection; helps debugging and scripts | Low |
| **version** | Meta; users expect it | Trivial |
| **jac2series(f, T)** | Expose existing impl; useful for verification | Trivial |

### Medium value, higher effort

| Gap | Why | Effort |
|-----|-----|--------|
| **mprodmake** | Different product form; extends prodmake family | Medium |
| **checkprod / checkmult** | Validation utilities | Medium |
| **findmaxind** | Useful for understanding dependency structure | Medium |

### Lower priority / defer

| Gap | Why |
|-----|-----|
| Mod-p family (findhommodp, etc.) | Specialized; modular forms research |
| zqfactor | Bivariate; scope creep |
| qetamake | etamake covers main use; variant is nice-to-have |
| qs2jaccombo | Complex; niche |
| LaTeX | Deferred in FEATURES.md |

---

## Anti-Features (Explicitly Out of Scope)

- **RootOf / cyclotomic fields** — Requires extension beyond Q; major scope change
- **Symbolic simplification (radsimp-style)** — We are numeric/series, not CAS
- **General q-difference equations** — Mathematica qFunctions domain; not Garvan package

---

## Suggested Next-Milestone Feature Set

For **v1.3** (incremental):

1. **version** — `version` built-in or `--version` flag
2. **qdegree, lqdegree** — Degree inspection
3. **jac2series(f, T)** — Expose as built-in
4. **findlincombo** — Linear combination relation finder

For **v1.4+** (optional):

5. **mprodmake** — (1+q^n) product form
6. **checkprod, checkmult** — Validation helpers
7. **findmaxind** — Maximal independent subset

---

*Research completed: 2026-02-25*
