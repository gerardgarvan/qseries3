# Architecture Research: t-core Partition Integration

## 1. Current Type System

### EvalResult (repl.h:300–315)
```cpp
using EvalResult = std::variant<
    Series,                                    // default arithmetic result
    std::map<int, Frac>,                       // prodmake
    std::vector<int>,                          // mprodmake
    std::vector<std::pair<int, Frac>>,         // etamake
    std::vector<JacFactor>,                    // jacprodmake
    RelationKernelResult,                      // findhom/findnonhom
    RelationComboResult,                       // findhomcombo/findnonhomcombo
    QFactorResult,                             // qfactor
    CheckprodResult,                           // checkprod
    CheckmultResult,                           // checkmult
    FindmaxindResult,                          // findmaxind
    int64_t,                                   // legendre/sigma/mod/etc
    DisplayOnly,                               // already printed
    std::monostate                             // set_trunc (no output)
>;
```

### EnvValue (repl.h:137)
```cpp
using EnvValue = std::variant<Series, std::vector<JacFactor>>;
```

### Key observation
EvalResult already has 14 variant alternatives. Adding 2–3 more (Partition, list of Partitions) is consistent with the pattern. EnvValue is minimal (2 alternatives) — adding Partition is straightforward.

---

## 2. Parser: No Changes Required

### List literals already parse correctly
The parser (parser.h:338–343) handles `[...]` syntax:
```cpp
if (peek().kind == Token::Kind::LBRACK) {
    consume();
    auto elems = parseCommaSeparatedExprs();
    expect(Token::Kind::RBRACK);
    return Expr::makeList(std::move(elems));
}
```

This means `[1, 2, 3, 4, 5]` already produces an `Expr` with `tag = Tag::List` and `elements` containing five `IntLit` expressions. No parser changes needed.

### Current list evaluation gap
In evalExpr (repl.h:1095–1096):
```cpp
case Expr::Tag::List:
    throw std::runtime_error("list evaluation not yet implemented");
```

This is the integration point. A bare `[1,2,3]` expression currently errors out. We need to make it evaluate to a Partition.

### How lists are consumed today
Functions that take list arguments (findhom, findnonhomcombo, etc.) never hit the List eval case — they use helper lambdas that extract elements directly from the AST node:
```cpp
auto evalListToSeries = [&](const Expr* listExpr) {
    if (!listExpr || listExpr->tag != Expr::Tag::List)
        throw std::runtime_error(...);
    std::vector<Series> out;
    for (const auto& e : listExpr->elements)
        out.push_back(evalAsSeries(e.get(), env, sumIndices));
    return out;
};
```

This pattern works because `args[0]` is the raw AST node, and the lambda checks `tag == List` directly without calling eval(). The tcore functions can use the same pattern with a new `evalToPartition` helper.

---

## 3. Proposed Type Extensions

### Partition type
```cpp
using Partition = std::vector<int64_t>;
```

A simple typedef is sufficient. No need for a struct with methods — partitions are just integer vectors. The typedef gives semantic clarity in function signatures.

### EvalResult additions
```cpp
using EvalResult = std::variant<
    Series,
    std::map<int, Frac>,
    std::vector<int>,
    std::vector<std::pair<int, Frac>>,
    std::vector<JacFactor>,
    RelationKernelResult,
    RelationComboResult,
    QFactorResult,
    CheckprodResult,
    CheckmultResult,
    FindmaxindResult,
    int64_t,
    DisplayOnly,
    std::monostate,
    Partition,                    // NEW: single partition [1,1,2,4,4,5]
    std::vector<Partition>        // NEW: list of partitions (partitions(n), tcores(p,n))
>;
```

### EnvValue additions
```cpp
using EnvValue = std::variant<Series, std::vector<JacFactor>, Partition>;
```

This allows `p := [1,1,2,4,4,5]` and `p := tcoreofptn([1,2,3,4], 5)` to store partitions in variables.

### Ambiguity: `vector<int>` vs `Partition`
EvalResult already has `std::vector<int>` (mprodmake). Partition is `std::vector<int64_t>`. These are distinct types in the variant, so no ambiguity. If we wanted `std::vector<int64_t>` for both, we'd need a wrapper — but `int` vs `int64_t` keeps them distinct naturally.

---

## 4. Evaluation Flow

### evalToPartition helper
Add alongside existing `evalListToSeries` / `evalListToInt`:
```cpp
auto evalToPartition = [&](const Expr* e) -> Partition {
    // Case 1: list literal [1,2,3,4]
    if (e && e->tag == Expr::Tag::List) {
        Partition p;
        for (const auto& elem : e->elements)
            p.push_back(evalToInt(elem.get(), env, sumIndices));
        return p;
    }
    // Case 2: variable holding a Partition
    if (e && e->tag == Expr::Tag::Var) {
        auto it = env.env.find(e->varName);
        if (it != env.env.end() && std::holds_alternative<Partition>(it->second))
            return std::get<Partition>(it->second);
    }
    // Case 3: function returning a Partition
    EvalResult r = eval(e, env, sumIndices);
    if (std::holds_alternative<Partition>(r))
        return std::get<Partition>(r);
    throw std::runtime_error(runtimeErr(name, "expected partition (integer list)"));
};
```

### Bare list evaluation
Change the `Expr::Tag::List` case in evalExpr (repl.h:1095–1096) from throwing to returning a Partition:
```cpp
case Expr::Tag::List: {
    Partition p;
    for (const auto& elem : e->elements)
        p.push_back(evalToInt(elem.get(), env, sumIndices));
    return p;
}
```

This is safe because existing functions that take list args (findhom etc.) extract from the AST directly and never call eval() on the list node.

### dispatchBuiltin additions
New if/else branches at end of dispatchBuiltin, following the pattern of existing functions:
```cpp
if (name == "istcore") {
    if (args.size() != 2)
        throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
    Partition ptn = evalToPartition(args[0].get());
    int p = static_cast<int>(evi(1));
    return static_cast<int64_t>(istcore(ptn, p) ? 1 : 0);
}
if (name == "tcoreofptn") {
    if (args.size() != 2)
        throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
    Partition ptn = evalToPartition(args[0].get());
    int p = static_cast<int>(evi(1));
    return tcoreofptn(ptn, p);
}
// ... etc for all ~15 functions
```

### Assignment flow
In evalStmt (repl.h:1500–1520), add Partition handling:
```cpp
if (std::holds_alternative<Partition>(res)) {
    env.env[s->assignName] = std::get<Partition>(res);
    return res;
}
if (std::holds_alternative<std::vector<Partition>>(res)) {
    // Lists of partitions are display-only; cannot assign to a single variable
    return res;
}
```

### Variable lookup
In evalExpr `Expr::Tag::Var` case, add handling for Partition-typed variables. Currently only Series and JacFactor are in EnvValue. When a Partition variable is used in an expression context that expects a series, throw a clear error. When used as argument to a tcore function, evalToPartition resolves it.

---

## 5. New Header: tcore.h

### Placement in include chain
```
qfuncs.h → tcore.h → convert.h → ...
```

tcore.h depends only on `<vector>`, `<cstdint>`, `<algorithm>`, `<stdexcept>` — no dependency on Series/Frac. It operates purely on `vector<int64_t>`. Include it from repl.h.

### Function catalog (from Maple tcore package)

| Function | Signature | Returns | Description |
|----------|-----------|---------|-------------|
| `istcore` | `(ptn, p) → bool` | bool | test if partition is a p-core |
| `tcoreofptn` | `(ptn, p) → Partition` | Partition | compute t-core of partition |
| `tcores` | `(p, n) → vector<Partition>` | list | all p-cores of n |
| `tcrank` | `(ptn, p) → int` | int64_t | t-core crank (0..p-1) |
| `tquot` | `(ptn, t) → vector<Partition>` | list | t-quotient (list of t partitions) |
| `ptn2nvec` | `(ptn, p) → vector<int64_t>` | Partition | n-vector of p-core |
| `ptn2rvec` | `(ptn, p) → vector<int64_t>` | Partition | r-vector (mod p) |
| `nvec2ptn` | `(nvec) → Partition` | Partition | reconstruct p-core from n-vector |
| `invphi1` | `(bigvec, t) → Partition` | Partition | inverse GSK bijection |
| `partitions` | `(n) → vector<Partition>` | list | all partitions of n |
| `nep` | `(ptn) → int` | int64_t | number of even parts |
| `np` | `(ptn) → int` | int64_t | number of parts |
| `lp` | `(ptn) → int` | int64_t | largest part |
| `ptnnorm` | `(ptn) → int` | int64_t | sum of parts (norm) |
| `conjugate` | `(ptn) → Partition` | Partition | conjugate partition |

### Functions returning special types
- `tquot` returns a *list of partitions* (the t-quotient is [μ₀, μ₁, ..., μ_{t-1}]). This maps to `vector<Partition>` in C++.
- `PHI1` returns `[tcore, tquotient]` = `[Partition, vector<Partition>]`. This can be displayed as a formatted pair but is awkward to return as a single type. Simplest approach: print both and return DisplayOnly, or return only the pair as a special struct.
- `invphi1` takes `[core, quotient]` — could accept two args: `invphi1(core, quot, t)`.

### Internal helper functions (not exposed to REPL)
- `rvec(ptn, p, k)`: residue count (used by istcore, tcoreofptn, ptn2nvec, ptn2rvec)
- `addrimthook`, `findhookinpos`: rim hook operations (used by invphi1)
- `veccombo`: vector linear combination

---

## 6. Display Format

### Single partition
Maple style: `[1, 1, 2, 4, 4, 5, 6, 6, 6]`

```cpp
inline std::string formatPartition(const Partition& p) {
    std::string out = "[";
    for (size_t i = 0; i < p.size(); ++i) {
        if (i > 0) out += ", ";
        out += std::to_string(p[i]);
    }
    out += "]";
    return out;
}
```

### List of partitions
For `partitions(n)` and `tcores(p,n)`:
```
[[4], [3, 1], [2, 2], [2, 1, 1], [1, 1, 1, 1]]
```
One partition per line for large results:
```
[4]
[3, 1]
[2, 2]
[2, 1, 1]
[1, 1, 1, 1]
```

### display() additions
Add to the `std::visit` in display():
```cpp
} else if constexpr (std::is_same_v<T, Partition>) {
    std::cout << formatPartition(arg) << std::endl;
} else if constexpr (std::is_same_v<T, std::vector<Partition>>) {
    for (const auto& p : arg)
        std::cout << formatPartition(p) << std::endl;
}
```

### Boolean results
Functions like `istcore` return bool. Map to `int64_t` (1/0) for consistency with existing Legendre/Jacobi symbol returns that also produce integer results displayed directly.

---

## 7. Order Convention

### Decision: Non-decreasing (ascending) — matching Maple tcore

The Maple tcore package uses non-decreasing order throughout:
- Test example: `[1, 1, 2, 4, 4, 5, 6, 6, 6, 7, 7, 7, 7, 7, 8, 9, 9, 10, 10, 13, 16, 17]`
- `rvec()` iterates `j = nn-m+1` (reverses internally when needed)
- `invphi1` output reverses at end: `optn:=[seq(ptn[nn-j+1],j=1..nn)]`
- `makebiw` reverses: `ip:=[seq(ptn[np-k],k=0..(np-1)),...]`

This means the "parts list" convention is smallest-first: `[1, 1, 2, 3, 5]` for the partition 1+1+2+3+5=12.

For `partitions(n)`, generate each partition in non-decreasing order. The overall list ordering (lexicographic or by some other criterion) is secondary — Maple's `combinat[partition]` returns non-increasing, but tcore examples all show non-decreasing. We normalize to non-decreasing for all tcore functions.

### Conjugate partition
The conjugate of `[1, 1, 2, 3, 5]` (Ferrers diagram transpose) outputs in the same non-decreasing convention.

---

## 8. Session Save/Load

Add a "P" tag for partitions in the session format (following "S" for Series, "J" for JacFactor):
```
P varname 1,1,2,4,4,5,6,6,6
```

---

## 9. `partition(n)` Name Collision

Currently `partition(n)` computes the partition *number* p(n). The tcore package needs `partitions(n)` to enumerate all partitions of n. Two options:

**Option A (recommended)**: Use `partitions(n)` (plural) for enumeration. Keep `partition(n)` for the number. Clear, no breaking change.

**Option B**: Rename existing to `pnum(n)` and use `partition(n)` for enumeration. Breaking change.

Go with Option A.

---

## 10. Help Table Additions

Add entries to `getHelpTable()`:
```cpp
{"conjugate", {"conjugate(ptn)", "conjugate partition"}},
{"istcore", {"istcore(ptn,p)", "test if partition is a p-core"}},
{"invphi1", {"invphi1([core,quot],t)", "inverse GSK phi1 bijection"}},
{"lp", {"lp(ptn)", "largest part of partition"}},
{"nep", {"nep(ptn)", "number of even parts"}},
{"np_parts", {"np_parts(ptn)", "number of parts"}},
{"nvec2ptn", {"nvec2ptn(nvec)", "reconstruct p-core from n-vector"}},
{"partitions", {"partitions(n)", "all partitions of n as list of lists"}},
{"ptn2nvec", {"ptn2nvec(ptn,p)", "n-vector of p-core"}},
{"ptn2rvec", {"ptn2rvec(ptn,p)", "r-vector of partition (mod p)"}},
{"ptnnorm", {"ptnnorm(ptn)", "sum of parts of partition"}},
{"tcoreofptn", {"tcoreofptn(ptn,p)", "compute p-core of partition"}},
{"tcores", {"tcores(p,n)", "all p-cores of n"}},
{"tcrank", {"tcrank(ptn,p)", "t-core crank (value 0..p-1)"}},
{"tquot", {"tquot(ptn,t)", "t-quotient of partition"}},
```

Note: `np` collides with nothing currently but is very short. Could use `np_parts` or `nparts` for clarity since `np` alone might be confusing in the REPL.

---

## 11. Tab Completion

No changes needed — `getCompletionCandidates()` already pulls from `getHelpTable()` and `env.env`. New functions added to the help table automatically appear in tab completion.

---

## 12. Summary of Changes by File

| File | Changes |
|------|---------|
| **tcore.h** (NEW) | ~300 lines. Pure integer/vector algorithms. No Series/Frac dependency. |
| **repl.h** | Add Partition/vector<Partition> to EvalResult. Add Partition to EnvValue. Add evalToPartition helper. Add ~15 dispatch branches. Add display formatters. Add session P-tag. Implement List eval case. |
| **parser.h** | **No changes.** |
| **main.cpp** | Add `#include "tcore.h"` (or include from repl.h). |

### Risk assessment
- **Low risk**: Parser changes = zero. Existing list argument extraction (evalListToSeries, evalListToInt) is untouched.
- **Medium risk**: Expanding EvalResult variant adds compile-time cost and makes the `display()` visitor slightly larger. Manageable.
- **Potential issue**: The `Expr::Tag::List` eval change (returning Partition instead of throwing) could theoretically affect code that relied on the error. In practice, no code path currently evaluates bare lists — they're only consumed as function arguments via AST-level lambdas.

---

## RESEARCH COMPLETE
