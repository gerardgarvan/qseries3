# Phase 86: RR Identity Search — Research

**Researched:** 2026-03-02  
**Domain:** Rogers-Ramanujan/Göllnitz-Gordon identity search, eta/theta product identification  
**Confidence:** HIGH (codebase + Maple reference) / MEDIUM (identity parameterizations)

## Summary

Phase 86 implements Rogers-Ramanujan and Göllnitz-Gordon functions (RRG, RRH, RRGstar, RRHstar), a checkid routine to detect if an expression is an eta/theta product, and findids(type, T) for systematic identity search. The design maps directly from Frank Garvan's Maple ramarobinsids package (gaps/ramarobinsids.txt). The C++ codebase already has jac2series, jacprodmake, etamake, prodmake, newprodmake, eta2jac, jac2eprod, provemodfuncid, QP2, etaprodtoqseries, and eta_cusp infrastructure.

**Primary recommendation:** Implement RRG/RRH/RRGstar/RRHstar as q-series from sum definitions; implement checkid as Maple CHECKRAMIDF (jac2series → normalize → prodmake → etamake); implement findids by translating findtype1..findtype10 search loops with their type-specific SYMF templates and parameter constraints.

## User Constraints

No CONTEXT.md exists for phase 86. Scope is defined by the ROADMAP and REQUIREMENTS:

- **RRID-01:** RRG(n), RRH(n), RRGstar(n), RRHstar(n)
- **RRID-02:** checkid(expr, T) — check if expression is an eta/theta product
- **RRID-03:** findids(type, T) — systematic identity search (types 1–10)

Success criteria from ROADMAP:
1. RRG(1) and RRH(1) match standard Rogers-Ramanujan products
2. checkid(RRG(1)*RRH(1), 50) identifies the product as an eta-quotient
3. findids(1, 20) discovers ≥3 known type-1 identities
4. findids(2, 20) discovers ≥2 known type-2 identities

---

## Standard Stack

### Core (Existing C++ Types)

| Component       | Location    | Purpose                                        |
|----------------|-------------|------------------------------------------------|
| Series         | series.h    | Truncated q-series                             |
| jac2series     | convert.h   | Jacobi product → Series                         |
| jacprodmake    | convert.h   | Series → Jacobi product                         |
| etamake        | convert.h   | Series → eta-product list                       |
| prodmake       | convert.h   | Andrews' algorithm, series → product exponents  |
| newprodmake    | convert.h   | prodmake with arbitrary leading term            |
| QP2            | theta_ids.h | Second Bernoulli polynomial (Geta exponent)     |
| etaprodtoqseries | eta_cusp.h | eta-product → Series                           |
| provemodfuncid | theta_ids.h | Prove Jacobi identity on Gamma_1(N)             |

### Supporting (qfuncs, number theory)

| Function  | Location  | Purpose                              |
|-----------|-----------|--------------------------------------|
| aqprod    | qfuncs.h  | Rising q-factorial (a;q)_n           |
| etaq      | qfuncs.h  | η(kτ) as q-series                    |
| divisors  | qfuncs.h  | Divisors of n                        |
| gcd       | —         | GCD for coprime checks               |

**Alternatives considered:** None. The Maple ramarobinsids package is the authoritative reference; the C++ stack mirrors it.

---

## Architecture Patterns

### RR Function Definitions

Rogers-Ramanujan (G, H) and Göllnitz-Gordon (GM, HM) have sum and product forms:

**Rogers-Ramanujan (standard):**
```
G(q) = Σ_{n≥0} q^(n²) / (q;q)_n = 1/((q;q⁵)_∞(q⁴;q⁵)_∞) = JAC(0,5,∞)/JAC(1,5,∞)
H(q) = Σ_{n≥0} q^(n²+n) / (q;q)_n = 1/((q²;q⁵)_∞(q³;q⁵)_∞) = JAC(0,5,∞)/JAC(2,5,∞)
```

**Göllnitz-Gordon:**
```
G*(q) = Σ_{n≥0} q^(n²)(-q;q²)_n / (q²;q²)_n = 1/((q;q⁸)_∞(q⁴;q⁸)_∞(q⁷;q⁸)_∞)
H*(q) = Σ_{n≥0} q^(n(n+2))(-q;q²)_n / (q²;q²)_n = 1/((q³;q⁸)_∞(q⁴;q⁸)_∞(q⁵;q⁸)_∞)
```

For RRG(n), RRH(n) with n>1: the Maple package uses G(n), H(n) as Rogers-Ramanujan functions with parameter n. These are expressed via Geta(g,d,n) and JAC products. G(n) = Geta-based formula with modulus 5n (or similar). The ramarobinsids uses jac2series on expressions built from G, H, GM, HM — so the REPL must either:
- (a) Define G(n), H(n) as Jacobi products and evaluate via jac2series, or
- (b) Define them as q-series from sums.

For **RRG(1)** and **RRH(1)**: use sum form (aqprod) or product form (jac2series of JAC(0,5,∞)/JAC(1,5,∞) and JAC(0,5,∞)/JAC(2,5,∞)). The sum form is self-contained; the product form reuses jac2series. Recommended: implement RRG(1), RRH(1) via sums for clarity; RRG(n), RRH(n) for n>1 via Geta/JAC (requires Geta infrastructure).

### Geta(g,d,n) from ramarobinsids

```
Geta(g,d,n) = q^(n·QP2(g/d)·d/2) · JAC(n·g, n·d, ∞) / JAC(0, n·d, ∞)
GetaEXP(g,d,n) = n·QP2(g/d)·d/2
```

QP2 exists in theta_ids.h. JAC(a,b,∞) maps to JacFactor. Geta can be implemented as a helper that returns a Jacobi product (or Series via jac2series). The Maple G, H, GM, HM are built from Geta with specific (g,d) pairs for modulus 5 (RR) and 8 (GG).

### CHECKRAMIDF → checkid

Maple CHECKRAMIDF(SYMF, ACC, T):
1. _JFUNC = subs(G,H,GM,HM into SYMF)
2. S = jac2series(_JFUNC, T1)
3. LDQ = lqdegree(S)
4. S2 = S / q^LDQ, C2 = coeff(S2, q, 0)
5. If C2=0, return (fail)
6. P2 = S2/C2 (normalize to constant 1)
7. PL = prodmake(S2/C2, T3)  // list of exponents a_n
8. PLX = max(|PL|)
9. If PLX < ACC: RID = etamake(S2, T3) * q^LDQ; return RID (eta quotient)
10. Else return (fail)

**checkid(expr, T)** in C++:
- expr is a Series (evaluated from REPL). If expr involves G,H,GM,HM, the REPL must evaluate those to Series first.
- For a single Series f: (1) LDQ = lqdegree(f), (2) g = f/q^LDQ, (3) lead = g.coeff(0), (4) if lead==0 return empty, (5) h = g/lead, (6) a = prodmake(h, T), (7) if max|a_n| > ACC return empty, (8) return etamake(g, T) with q^LDQ factored in.

The C++ etamake expects Series with constant term 1 (or handles leading term). The convert.h etamake divides by lead first. So checkid can: normalize f to f/q^LDQ, divide by constant term, run prodmake on normalized series, check exponent bound, run etamake. Return either the eta quotient (as GP list or formatted string) or empty/false.

### findtype1 / findtype2 → findids

**findtype1(T):** Search over n=2..T, divisors p of n, c1 ∈ {-1,1}, a = n/p:
- Constraints: a≠p, a≤p, gcd(a,p)=1
- ABCOND: GE(p)+HE(a)-(GE(a)+HE(p)) ∈ Z
- SYMF = G(p)·H(a) + c1·G(a)·H(p)
- For each candidate: run CHECKRAMIDF; if pass, record [p,a,c1]

**findtype2(T):** n=1..T, divisors p of n, c1 ∈ {-1,1}, a = n/p:
- Constraints: a≤p, gcd(a,p)=1
- ABCOND: GE(a)+GE(p)-(HE(a)+HE(p)) ∈ Z
- SYMF = G(a)·G(p) + c1·H(a)·H(p)
- Record [a,p,c1]

GE and HE are the "exponent" functions (GetaEXP or similar) — they ensure the combination has integer q-power. The Maple package defines GE, HE in the qseries package (not in ramarobinsids). For C++: GE(a), HE(a) can be derived from the Geta exponent formula for the RR modulus (5). Typical: GE(a) = (5a²−5a+2)/10, HE(a) = (5a²+5a+2)/10 or similar — need to verify from qseries/ramarobinsids documentation.

**Known type-1 identities** (from Slater): e.g. G(2)H(1) − G(1)H(2) = q·η(5τ)²/η(τ), etc. For T=20, n runs 2..20, giving many (p,a,c1) candidates; at least 3 should pass CHECKRAMIDF.

**Known type-2 identities:** e.g. G(1)G(1)+H(1)H(1), G(1)G(2)−H(1)H(2), etc. At least 2 should appear for T=20.

---

## Don't Hand-Roll

| Problem              | Don't Build                | Use Instead              | Why                             |
|----------------------|----------------------------|--------------------------|---------------------------------|
| Series → product     | Custom recurrence          | prodmake / newprodmake   | Andrews' algorithm is standard  |
| Product → eta form   | Custom factorization       | etamake                  | Already implemented             |
| Jacobi → series      | Manual theta expansion     | jac2series               | Handles half-integer exponents  |
| Geta exponent        | Ad hoc formula             | QP2 from theta_ids.h     | Matches Maple                   |
| Cusp/Sturm proof     | Custom bound               | provemodfuncid           | Gamma_1(N) Sturm already there  |

---

## Common Pitfalls

### Pitfall 1: prodmake expects constant 1
**What goes wrong:** prodmake assumes f has constant term 1. Expressions like q^k·(eta product) have leading power q^k.  
**How to avoid:** Use newprodmake, or divide by lead and q^LDQ before prodmake. CHECKRAMIDF does this explicitly.

### Pitfall 2: ACC threshold too tight
**What goes wrong:** Maple uses ACC=10 for most types, ACC=24 for type 6. Too small ACC rejects valid eta products; too large accepts false positives.  
**How to avoid:** Use ACC=10 (or make it a parameter). Type 6 uses 24.

### Pitfall 3: GE/HE not integer
**What goes wrong:** The ABCOND checks ensure the combined exponent is integral. If GE, HE are wrong, valid identities are skipped or invalid ones enumerated.  
**How to avoid:** Derive GE, HE from GetaEXP for RR modulus 5 and GG modulus 8. Cross-check with Maple output.

### Pitfall 4: G(n), H(n) for n>1
**What goes wrong:** G(1), H(1) are standard. G(n), H(n) for n>1 are parametric — likely Geta(1,5,n) or similar. Wrong parameterization breaks findids.  
**How to avoid:** Implement G(1), H(1) first; extend via Geta once the mapping from Maple is confirmed.

### Pitfall 5: Double-check with higher T
**What goes wrong:** Maple runs CHECKRAMIDF at TT1 and TT2 (e.g. 100 and 400) and only accepts if both pass. Low T can give false positives.  
**How to avoid:** Optionally run checkid at T and 9T/10; require both to pass before reporting an identity.

---

## Maple → C++ Mapping

| Maple                   | C++                               |
|-------------------------|-----------------------------------|
| G(a), H(a)              | RRG(a), RRH(a) — series or JAC   |
| GM(a), HM(a)            | RRGstar(a), RRHstar(a)           |
| Geta(g,d,n)             | geta(g,d,n) — via QP2 + jac2series |
| jac2series(expr, T)     | jac2series(jac, T)               |
| prodmake(S, q, T)       | prodmake(series, T)              |
| etamake(S, q, T)        | etamake(series, T)               |
| jacprodmake(S, q, T)    | jacprodmake(series, T)           |
| CHECKRAMIDF(SYMF,ACC,T) | checkid(series, T, acc)          |
| findtype1(T)            | findids(1, T)                    |
| findtype2(T)            | findids(2, T)                    |
| lqdegree(S)             | series.minExp() or custom        |

---

## Type 1–10 Identity Parameterizations

From ramarobinsids.txt:

| Type | SYMF template | Constraints |
|------|----------------|-------------|
| 1    | G(p)H(a) ± G(a)H(p) | a≠p, a≤p, gcd(a,p)=1, ABCOND |
| 2    | G(a)G(p) ± H(a)H(p) | a≤p, gcd(a,p)=1, ABCOND |
| 3    | (G(a1)G(p1)±H(a1)H(p1)) / (G(a2)H(p2)±H(a2)G(p2)) | a1≤p1, a2>p2, etc. |
| 4    | GM(p)HM(a) ± GM(a)HM(p) | a≠p, a≤p, (a mod 2=0 or p mod 2=0), gcd=1 |
| 5    | GM(a)GM(p) ± HM(a)HM(p) | a≤p, (a mod 2=0 or p mod 2=0), gcd=1 |
| 6    | G(a)HM(p) ± GM(a)H(p) | a≥p, gcd(a,p)=1, ACC=24 |
| 7    | GM(a)G(p) ± HM(a)H(p) | a≤p, a odd, p odd, gcd=1 |
| 8    | G(1)^p H(p) ± H(1)^p G(p) | p>1 |
| 9    | G(1)^a H(1)^b − H(1)^a G(1)^b − x | isolve(GE(a)+HE(b)=0), x∈{0,1} |
| 10   | (G(a1)H(p1)±H(a1)G(p1)) / (G(a2)HM(p2)±H(a2)GM(p2)) | a1>p1, a2>p2, etc. |

---

## Dependencies

| Dependency        | Status   | Use in Phase 86                                      |
|-------------------|----------|------------------------------------------------------|
| newprodmake       | Done     | Alternative path for checkid if lead ≠ 1             |
| prodmake          | Done     | Core of checkid                                      |
| etamake           | Done     | Convert to eta quotient in checkid                   |
| etaprodtoqseries  | Done     | If outputting eta form → series for verification     |
| jac2series        | Done     | Evaluating G, H, GM, HM from JAC form                |
| jacprodmake       | Done     | Identify Jacobi form (optional, Maple uses etamake)  |
| eta2jac, jac2eprod| Done     | GETA conversion if needed                            |
| provemodfuncid    | Partial  | Optional proof step (Maple proveit mode)             |
| eta_cusp          | Done     | etaprodtoqseries, gammacheck                         |
| QP2               | Done     | Geta exponent                                        |

---

## Code Examples

### RRG(1) and RRH(1) from sums

```cpp
// RRG(1) = Σ q^(n²)/(q;q)_n
Series RRG1(int T) {
    Series sum = Series::one(T);
    Series q = Series::q(T);
    for (int n = 1; n * n < T; ++n) {
        Series term = q.pow(n * n) / aqprod(q, q, n);
        sum = (sum + term).truncTo(T);
    }
    return sum;
}

// RRH(1) = Σ q^(n²+n)/(q;q)_n
Series RRH1(int T) {
    Series sum = Series::one(T);
    Series q = Series::q(T);
    for (int n = 1; n * n + n < T; ++n) {
        Series term = q.pow(n * n + n) / aqprod(q, q, n);
        sum = (sum + term).truncTo(T);
    }
    return sum;
}
```

### checkid sketch

```cpp
// CHECKRAMIDF: normalize to const 1, prodmake, check max|a_n|<acc, then etamake
// Returns eta quotient (GP) or empty on failure. lqdegree = f.minExp().
std::optional<std::vector<std::pair<int,Frac>>> checkid(const Series& f, int T, int acc = 10) {
    int ldq = f.minExp();
    if (ldq < 0) return std::nullopt;
    Series g = f.truncTo(T);
    Frac c0 = g.coeff(ldq);
    if (c0.isZero()) return std::nullopt;
    // h = g / (c0 * q^ldq) — constant 1, required by prodmake
    Series h = (g / (Series::constant(c0, T) * Series::q(T).pow(ldq))).truncTo(T);
    auto a = prodmake(h, T);
    if (a.empty()) return std::nullopt;
    int plx = 0;
    for (const auto& [n, an] : a) {
        if (an.den != BigInt(1)) return std::nullopt;
        int v = /* integer from an */;
        if (v < 0) v = -v;
        if (v > plx) plx = v;
    }
    if (plx >= acc) return std::nullopt;
    // etamake(g, T) handles leading; returns (k, e_k) pairs
    auto gp = etamake(g, T);
    if (gp.empty()) return std::nullopt;
    return gp;  // Caller formats with q^ldq factor
}
```

### findids(1, T) loop skeleton

```cpp
std::vector<std::tuple<int,int,int>> findids_type1(int T) {
    std::vector<std::tuple<int,int,int>> out;
    for (int n = 2; n <= T; ++n) {
        for (int p : divisors(n)) {
            int a = n / p;
            if (a == p || a > p || std::gcd(a, p) != 1) continue;
            for (int c1 : {-1, 1}) {
                if (!abcond_type1(p, a)) continue;
                Series symf = RRG(p) * RRH(a) + Frac(c1) * RRG(a) * RRH(p);
                if (checkid(symf, T, 10))
                    out.push_back({p, a, c1});
            }
        }
    }
    return out;
}
```

---

## Open Questions

1. **GE(a), HE(a) formulas**  
   - Need exact expressions for RR modulus 5 (and GG modulus 8 for GM, HM).  
   - Recommendation: Extract from Maple qseries package or Ramanujan-Robins-Sellers literature; implement as small lookup or closed form.

2. **G(n), H(n) for n>1**  
   - ramarobinsids builds expressions symbolically and evaluates via jac2series. The G, H are Jacobi products.  
   - Recommendation: Implement G(1), H(1) first; add Geta-based G(n), H(n) in a follow-up task once GE/HE are fixed.

3. **REPL integration**  
   - checkid(expr, T) requires expr to be evaluable. If expr = RRG(1)*RRH(1), the parser must recognize RRG, RRH and produce a Series.  
   - Recommendation: Add RRG, RRH, RRGstar, RRHstar as built-in functions returning Series; checkid accepts a Series.

---

## Sources

### Primary (HIGH confidence)
- gaps/ramarobinsids.txt — Maple package source (ramarobinsids 0.2, Jul 2018)
- src/convert.h — prodmake, etamake, jacprodmake, jac2series, newprodmake
- src/theta_ids.h — QP2, jac2eprod, eta2jac, provemodfuncid
- src/eta_cusp.h — etaprodtoqseries
- qseriesdoc.md — Rogers-Ramanujan sum/product forms

### Secondary (MEDIUM confidence)
- Web search — Rogers-Ramanujan G(q), H(q), Göllnitz-Gordon definitions
- ROADMAP.md — Phase 86 scope and success criteria

### Tertiary (LOW confidence)
- GE/HE closed forms — need verification against Maple qseries package

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all components exist and are referenced in code
- Architecture: HIGH — Maple→C++ mapping is explicit in ramarobinsids
- Pitfalls: HIGH — from code inspection and Maple logic
- GE/HE and G(n) for n>1: MEDIUM — require qseries package or literature check

**Research date:** 2026-03-02  
**Valid until:** 90 days (domain is stable)
