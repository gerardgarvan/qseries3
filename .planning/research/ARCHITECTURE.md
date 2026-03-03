# Architecture Research: RootOf / Q(ω) Integration

**Domain:** q-series REPL — algebraic number extension
**Researched:** 2026-03-03
**Confidence:** HIGH

## Executive Summary

RootOf and Q(ω) integrate with the existing q-series architecture by (1) adding a new `Algebraic` value type representing elements of cyclotomic fields Q(ζ_n), (2) introducing a Series variant with algebraic coefficients (`SeriesOmega`), (3) treating `RootOf(...)` as a builtin call (no parser grammar change), and (4) storing `Algebraic` in `EnvValue` when `omega := RootOf(3)` is evaluated. The build order is: **algebraic type first** → **Series extension** → **parser/REPL integration**. This preserves header-only, single-binary constraints.

---

## System Overview

### Current vs Extended Type Stack

```
CURRENT:
  BigInt → Frac → Series (std::map<int,Frac>) → qfuncs → convert → parser/REPL

EXTENDED:
  BigInt → Frac → Algebraic (cyclotomic Q(ζ_n))
                 ↘ Series (unchanged for Q coefficients)
                 ↘ SeriesOmega (std::map<int,Algebraic>) — NEW
```

### Integration Points

| Component | Responsibility | Modification |
|-----------|----------------|--------------|
| **algebraic.h** (NEW) | Cyclotomic field element: representation, arithmetic, cyclotomic polynomial | New header |
| **series.h** | Power series over Frac | Unchanged for Q-only paths |
| **series.h** or **series_omega.h** (NEW) | Series with coefficients in Q(ω) | New type or template |
| **frac.h** | Rationals | No change |
| **parser.h** | Parse `RootOf(3)` as `Call("RootOf", [IntLit(3)])` | No grammar change — Call handles it |
| **repl.h** | EvalResult, EnvValue, evalStmt, Var lookup, display | Add Algebraic, SeriesOmega variants; add RootOf dispatch; extend getSeriesFromEnv |
| **qfuncs.h** / **convert.h** | q-series arithmetic, prodmake, etamake | Extend only where omega appears (e.g. a(q), b(q), c(q)) |

---

## 1. Algebraic Number Type (Q(ω))

### Representation

Elements of Q(ζ_n) are represented as polynomials in ζ_n with rational coefficients, reduced modulo the nth cyclotomic polynomial Φ_n. Degree is strictly less than φ(n).

**Reference:** Regina Cyclotomic class, SymPy `AlgebraicNumber`, Wolfram `AlgebraicNumber[θ, {c₀, c₁, ...}]`.

```cpp
// algebraic.h — conceptual
struct Algebraic {
    int n;                           // cyclotomic order (Q(ζ_n))
    std::vector<Frac> coeffs;        // length φ(n), coeffs of 1, ζ, ζ², ..., ζ^(φ(n)-1)
    // Invariant: coeffs.size() == phi(n)
};
```

- **n = 3**: Q(ζ_3), φ(3)=2. Element a₀ + a₁·ζ, ζ² + ζ + 1 = 0.
- **n = 5**: φ(5)=4. Element a₀ + a₁·ζ + a₂·ζ² + a₃·ζ³.

### Arithmetic

- **Add/Sub**: Component-wise on coeffs (same n).
- **Mul**: Polynomial multiplication in ζ, then reduce mod Φ_n.
- **Inv**: Extended GCD or norm-based inversion.

Cyclotomic polynomial Φ_n is computed once (or cached). Regina provides `cyclotomic(n)`; we need a small implementation (Euler’s formula or recursive construction).

### RootOf Semantics

For Maple parity `omega := RootOf(z^2+z+1=0)`:

- **Option A (cyclotomic shortcut):** `RootOf(n)` → primitive nth root ζ_n = exp(2πi/n). Then `RootOf(3)` gives ζ_3 (satisfies z²+z+1=0).
- **Option B (general):** `RootOf(poly, index)` — more complex; defer.

**Recommendation:** Implement `RootOf(n)` returning the canonical primitive nth root (index 2 in Maple’s convention, i.e. e^(2πi/n)). This covers qseriesdoc’s omega and Block 10.

---

## 2. Value Type Extensions

### EnvValue

```cpp
// Current
using EnvValue = std::variant<Series, std::vector<JacFactor>, Partition, Phi1Result, RelationKernelResult>;

// Extended
using EnvValue = std::variant<Series, std::vector<JacFactor>, Partition, Phi1Result, RelationKernelResult, Algebraic>;
```

### EvalResult

```cpp
// Add
Algebraic,              // RootOf(n), omega, or algebraic expression
SeriesOmega             // Series with coefficients in Q(ω) — when q-series involves omega
```

`SeriesOmega` is a distinct type (or a template specialization) to avoid mixing Q and Q(ω) in one variant and to keep Series logic clear.

---

## 3. Series Over Q(ω)

### Options

| Option | Description | Pros | Cons |
|--------|-------------|------|------|
| **A. SeriesOmega** | `struct SeriesOmega { std::map<int, Algebraic> c; int trunc; Frac q_shift; }` | Matches Series layout; explicit type | Duplicate arithmetic code |
| **B. Template Series\<Coef\>** | `Series<Frac>` vs `Series<Algebraic>` | Shared algorithms | Larger refactor; Coef concepts |
| **C. Union Coef** | `using Coef = std::variant<Frac, Algebraic>` | Single Series type | Complex dispatch; Algebraic + Frac mixed |

**Recommendation:** Option A — `SeriesOmega` with the same structure as Series. Copy the arithmetic pattern from Series, with coefficient type `Algebraic`. Keeps changes localized and avoids template bloat.

### When to Use Which

- **Series (Frac):** All existing paths; q-series over Q.
- **SeriesOmega (Algebraic):** When the expression involves `omega` (or any Algebraic), e.g. `omega * q + q^2` or `add(omega^n * q^(n^2), n, 0, 5)`.

Conversion: `Series` → `SeriesOmega` by promoting each `Frac` to `Algebraic` (Frac embeds as constant in Q(ζ_n)).

---

## 4. Parser Support for RootOf

### No Grammar Change

`RootOf(3)` and `RootOf(z^2+z+1=0)` parse as ordinary function calls:

```
rootof_expr := IDENT "(" expr_list ")"
```

The tokenizer recognizes `RootOf` as IDENT. The parser produces:

```cpp
Expr::makeCall("RootOf", { Expr::makeInt(3) })
// or
Expr::makeCall("RootOf", { polynomial_expr, index_expr })  // if general form later
```

**Implementation:** Add a `RootOf` branch in `dispatchBuiltin`, not a new `Expr::Tag`.

### Maple Syntax `RootOf(z^2+z+1=0)`

This is an equation. Options:

- **Phase 1:** Support only `RootOf(n)` for cyclotomic orders. Reject or defer `RootOf(poly)`.
- **Phase 2:** Parse `RootOf(expr=0)` as equation, extract polynomial, detect cyclotomic (e.g. z²+z+1 → n=3) and dispatch to cyclotomic path.

For v8.0, `RootOf(3)` is sufficient and matches the use case (omega = ζ_3).

---

## 5. Data Flow: omega := RootOf(3)

### End-to-End Flow

```
1. Input: "omega := RootOf(3)"
2. Parse:  Stmt::Assign("omega", Expr::Call("RootOf", [IntLit(3)]))
3. evalStmt:
   a. eval(assignRhs) → dispatchBuiltin("RootOf", [IntLit(3)])
   b. RootOf(3) → construct Algebraic for ζ_3 (coeffs [0,1] in Q(ζ_3))
   c. Returns EvalResult = Algebraic
4. evalStmt (assign branch):
   if (holds_alternative<Algebraic>(res))
       env.env["omega"] = get<Algebraic>(res);
5. Variable lookup (later use of omega):
   evalExpr(Var("omega")) →
   ev = env.env.find("omega") → holds_alternative<Algebraic>
   Return value: Can't return Algebraic to arithmetic expecting Series!
   → Need: when omega appears in series context, either
      - promote to SeriesOmega with constant coefficient omega, or
      - have BinOp/Sum handle Algebraic explicitly.
```

### What `omega` Stores in the Environment

`env.env["omega"]` holds an `EnvValue` of type `Algebraic`:

```cpp
Algebraic{
    .n = 3,
    .coeffs = {Frac(0), Frac(1)}   // 0 + 1·ζ_3 = ζ_3
}
```

So `omega` is the primitive 3rd root of unity.

### Use in Expressions

When `omega` appears in an expression like `omega * q`:

1. **Var("omega"):** Evaluates to `Algebraic` (stored in EnvValue).
2. **BinOp(Mul, omega, q):** Left is Algebraic, right is Series. Need a rule: `Algebraic * Series → SeriesOmega` (scale each coefficient of Series by the Algebraic).

The evaluator must handle `Algebraic` in binary ops: either through overloads or by promoting to `SeriesOmega` before arithmetic.

---

## 6. Build Order

### Phase 1: Algebraic Type (algebraic.h)

1. Cyclotomic polynomial Φ_n (small n: 1–24 or so).
2. `struct Algebraic` with `n`, `coeffs`.
3. Arithmetic: `+`, `-`, `*`, `inverse`, `==`.
4. Factory: `Algebraic primitiveRoot(int n)` for ζ_n.
5. No dependency on Series. Depends only on Frac, BigInt.

### Phase 2: Series Extension (SeriesOmega)

1. `struct SeriesOmega` in series.h or series_omega.h.
2. Arithmetic mirroring Series (add, mul, truncTo, etc.) with `Algebraic` coefficients.
3. Conversion: `Series::constant(Frac, T)` → `SeriesOmega::constant(Algebraic, T)` via `Algebraic(Frac)`.
4. Scaling: `Algebraic * Series → SeriesOmega`.

### Phase 3: Parser/REPL Integration

1. `dispatchBuiltin("RootOf", args)`: if one integer arg, return `Algebraic::primitiveRoot(n)`.
2. Add `Algebraic` to `EvalResult` and `EnvValue`.
3. `evalStmt` assign: store `Algebraic` in env.
4. `evalExpr` Var: return `Algebraic` when variable holds Algebraic.
5. BinOp: extend `toSeries` / arithmetic to handle `Algebraic`:
   - `Algebraic` × `Series` → `SeriesOmega`
   - `SeriesOmega` + `SeriesOmega` → `SeriesOmega`
   - `Series` + `SeriesOmega` → promote Series to SeriesOmega (Frac→Algebraic) then add
6. Display: `formatAlgebraic`, `display` for `SeriesOmega`.
7. Session save/load: add tag for Algebraic (e.g. `A varname n c0 c1 ...`).

### Phase 4 (Optional): qfuncs / convert

- Functions like `a(q)`, `b(q)`, `c(q)` from qseriesdoc that use omega: implement as SeriesOmega if needed.
- prodmake, etamake: stay over Q unless we explicitly add omega-aware variants.

---

## 7. Component Boundary Summary

### New Components

| Component | Location | Depends On |
|-----------|----------|------------|
| Algebraic | algebraic.h | frac.h, bigint.h |
| SeriesOmega | series.h or series_omega.h | frac.h, algebraic.h |
| RootOf dispatch | repl.h | algebraic.h |
| Cyclotomic polynomial | algebraic.h | bigint.h, frac.h |

### Modified Components

| Component | Changes |
|-----------|---------|
| repl.h | EvalResult +Algebraic,+SeriesOmega; EnvValue +Algebraic; evalStmt; Var lookup; BinOp handling; display; session |
| parser.h | None (RootOf is Call) |
| series.h | Optionally SeriesOmega in same file, or new header |

### Unchanged

- bigint.h, frac.h, qfuncs.h (except omega-aware functions later)
- convert.h (prodmake, etamake over Q)
- linalg.h, relations.h

---

## 8. Data Flow Diagrams

### Assignment Flow

```
"omega := RootOf(3)"
       │
       ▼
┌──────────────────┐     ┌─────────────────┐     ┌──────────────────┐
│ Parse Assign     │────▶│ eval(RootOf(3)) │────▶│ dispatchBuiltin  │
│ "omega" "RootOf" │     │                 │     │ RootOf → Algebraic│
└──────────────────┘     └─────────────────┘     └────────┬─────────┘
                                                          │
                                                          ▼
┌──────────────────┐     ┌─────────────────┐     ┌──────────────────┐
│ display(result)  │◀────│ return res      │◀────│ env["omega"]=alg │
│ or continue      │     │                 │     │ evalStmt assign  │
└──────────────────┘     └─────────────────┘     └──────────────────┘
```

### Expression Flow (omega * q)

```
"omega * q"
     │
     ▼
┌─────────────┐    ┌──────────────────┐    ┌─────────────────────┐
│ eval(BinOp) │───▶│ left=Algebraic   │───▶│ Algebraic*Series    │
│             │    │ right=Series(q)  │    │ → SeriesOmega       │
└─────────────┘    └──────────────────┘    └─────────────────────┘
```

---

## 9. Anti-Patterns to Avoid

### 1. Embedding Algebraic in Frac

**Bad:** Trying to extend Frac to hold algebraic numbers.
**Why:** Frac is for Q; Algebraic is Q(ω). They are different types.
**Instead:** Keep Frac and Algebraic separate; use conversion only at boundaries.

### 2. General RootOf(poly) in Phase 1

**Bad:** Full polynomial RootOf in the first phase.
**Why:** Requires polynomial parsing, root isolation, indexing. Much larger scope.
**Instead:** Implement only `RootOf(n)` for cyclotomic roots.

### 3. Template Series Without Need

**Bad:** Refactoring Series to `template<typename Coef>` immediately.
**Why:** Big refactor for marginal benefit; header-only compiles can slow down.
**Instead:** Add SeriesOmega as a parallel struct; consider templates only if duplication becomes severe.

### 4. Mixing Q and Q(ω) in One Series Type

**Bad:** `std::map<int, std::variant<Frac, Algebraic>>` as coefficients.
**Why:** Every operation needs variant dispatch; easy to create invalid mixes.
**Instead:** Separate Series (Frac) and SeriesOmega (Algebraic); convert explicitly when needed.

---

## 10. Phase Dependencies for Roadmapper

```
Phase A: algebraic.h (no dependencies on Series/REPL)
    └── Cyclotomic polynomial, Algebraic struct, arithmetic, primitiveRoot(n)

Phase B: SeriesOmega (depends on A)
    └── series_omega.h or extension in series.h

Phase C: REPL integration (depends on A, B)
    ├── RootOf dispatch
    ├── EnvValue/EvalResult
    ├── evalStmt, Var lookup
    ├── BinOp handling (Algebraic * Series → SeriesOmega)
    └── display, session

Phase D (optional): omega-aware qfuncs (depends on C)
    └── a(q), b(q), c(q) as SeriesOmega
```

**Critical path:** A → B → C. Phase D can follow or be interleaved with C if those functions are required for acceptance tests.

---

## 11. Session Format Extension

Add tag for Algebraic in save/load:

```
A varname n c0 c1 c2 ...
```

Example: `A omega 3 0 1` for ζ_3 (0 + 1·ζ).

---

## 12. Sources

- Regina Cyclotomic class: https://regina-normal.github.io/engine-docs/classregina_1_1Cyclotomic.html
- Maple RootOf: https://www.maplesoft.com/support/help/Maple/view.aspx?path=RootOf
- SymPy number fields: https://docs.sympy.org/latest/modules/polys/numberfields.html
- Wolfram AlgebraicNumber: https://reference.wolfram.com/language/ref/AlgebraicNumber.html
- Project: qseriesdoc.md (line 449), .planning/PROJECT.md, ARCHITECTURE-TCORE.md (integration pattern)
