# Stack Additions for t-Core Partition Package

## 1. New Data Structures

### 1.1 Partition Type

The REPL currently has no first-class "list of integers" data type. The `EvalResult` variant supports `Series`, `int64_t`, `std::vector<int>` (mprodmake only), various product/relation result types, and `DisplayOnly`. A partition needs to be a **sorted non-increasing list of positive integers** like `[6,6,6,5,4,4,2,1,1]`.

**Recommended representation:**

```cpp
struct Partition {
    std::vector<int> parts;  // sorted non-increasing, all > 0

    int size() const {
        int s = 0;
        for (int p : parts) s += p;
        return s;
    }
    int numParts() const { return static_cast<int>(parts.size()); }
    int largestPart() const { return parts.empty() ? 0 : parts[0]; }

    std::string str() const {
        std::string out = "[";
        for (size_t i = 0; i < parts.size(); ++i) {
            if (i) out += ", ";
            out += std::to_string(parts[i]);
        }
        return out + "]";
    }
};
```

**Key design decisions:**
- Store parts in **non-increasing** order (Maple convention, matches `t-core.txt` which reverses for internal use). The Maple `combinat[partition]` returns partitions in non-increasing order.
- Use `int` not `int64_t` — partition parts are small (bounded by n). Even p(200) = 3.97×10^12 partitions; we won't enumerate those.
- No need for BigInt/Frac — parts are always positive integers.

### 1.2 Partition List Type

Functions like `partition(n)` (enumerate) and `tcores(p,n)` return **lists of partitions**. The t-quotient `tquot(ptn,t)` returns a **list of partitions** (one per residue class). The GSK bijection `PHI1` returns a `[core, [q0,q1,...,q_{t-1}]]` pair.

**Recommended approach:** Add two new `EvalResult` alternatives:

```cpp
using EvalResult = std::variant<
    Series,
    std::map<int, Frac>,                // prodmake
    std::vector<int>,                    // mprodmake
    std::vector<std::pair<int, Frac>>,   // etamake
    std::vector<JacFactor>,              // jacprodmake
    RelationKernelResult,
    RelationComboResult,
    QFactorResult,
    CheckprodResult,
    CheckmultResult,
    FindmaxindResult,
    int64_t,
    Partition,                           // NEW: single partition
    std::vector<Partition>,              // NEW: list of partitions
    DisplayOnly,
    std::monostate
>;
```

For the GSK bijection output `[core, quotient]`, use `DisplayOnly` with direct printing (like `partition_number` does now), since it's a compound structure.

### 1.3 Environment Extension

The `EnvValue` variant (currently `Series | vector<JacFactor>`) needs to also hold `Partition`:

```cpp
using EnvValue = std::variant<Series, std::vector<JacFactor>, Partition>;
```

This allows `ptn := [1,2,4,4,5,6,6,6]` assignment and later use in `tcoreofptn(ptn, 5)`.

## 2. Parser Changes

### 2.1 List Literal → Partition

The parser **already supports** `[...]` list syntax via `Expr::Tag::List` with `Expr::makeList()`. The tokenizer already handles `LBRACK`, `RBRACK`. The `parsePrimary()` function at line 338-343 of `parser.h` already parses `[expr, expr, ...]` into a `List` node.

**Current state:** `eval()` for `Tag::List` throws `"list evaluation not yet implemented"` (repl.h line 1096).

**What to change:** Add evaluation logic for `Tag::List`:

```cpp
case Expr::Tag::List: {
    std::vector<int> elems;
    for (const auto& elem : e->elements) {
        elems.push_back(static_cast<int>(evalToInt(elem.get(), env, sumIndices)));
    }
    std::sort(elems.begin(), elems.end(), std::greater<int>());
    // Remove zeros
    while (!elems.empty() && elems.back() <= 0) elems.pop_back();
    Partition ptn;
    ptn.parts = std::move(elems);
    return ptn;
}
```

**No parser changes needed.** The existing `Expr::Tag::List` + `LBRACK/RBRACK` tokens handle the syntax. Only the evaluator needs the new case.

### 2.2 Function Argument Handling

Functions like `tcoreofptn(ptn, 5)` need to accept a `Partition` argument. Add a helper analogous to `evalAsSeries`:

```cpp
inline Partition evalAsPartition(const Expr* e, Environment& env,
    const std::map<std::string, int64_t>& sumIndices) {
    EvalResult r = eval(e, env, sumIndices);
    if (std::holds_alternative<Partition>(r))
        return std::get<Partition>(r);
    throw std::runtime_error("expected partition (list of integers)");
}
```

Also need `getPartitionFromEnv()` analogous to `getSeriesFromEnv()`:

```cpp
inline Partition getPartitionFromEnv(const EnvValue& v) {
    if (std::holds_alternative<Partition>(v))
        return std::get<Partition>(v);
    throw std::runtime_error("variable does not hold a partition");
}
```

The `evalListToInt` lambda (repl.h line 741-748) already exists for findhom and can be reused internally, but a proper `evalAsPartition` is cleaner for the t-core functions.

## 3. Partition Enumeration Algorithm

### 3.1 `partition(n)` — Enumerate All Partitions

Currently `partition(n)` computes p(n) the partition **number**. We need to either:
- (A) Rename existing to `partnum(n)` and make `partition(n)` return all partitions, OR
- (B) Add `partitions(n)` (plural) for enumeration, keep `partition(n)` as the count.

**Recommendation: Option (B)** — matches Maple convention where `combinat[partition](n)` enumerates but our existing `partition(n)` computes p(n). Use `partitions(n)` for the list.

### 3.2 Generation Algorithm

Use **reverse lexicographic** generation (Kelleher-O'Sullivan "Generating All Partitions" algorithm, or the simpler iterative approach). This is the standard for ordered partition generation:

```cpp
inline std::vector<Partition> enumerate_partitions(int n) {
    if (n < 0) return {};
    if (n == 0) return {Partition{{}}};

    std::vector<Partition> result;
    std::vector<int> a(n + 1, 0);
    int k = 1;
    a[1] = n;

    while (k > 0) {
        // Output current partition a[1..k]
        Partition p;
        p.parts.assign(a.begin() + 1, a.begin() + k + 1);
        result.push_back(std::move(p));

        // Generate next partition in reverse lex order
        int rem = 0;
        while (k > 0 && a[k] == 1) {
            rem += a[k];
            --k;
        }
        if (k == 0) break;
        --a[k];
        ++rem;
        // Fill remaining with copies of a[k]
        while (rem > a[k]) {
            a[k + 1] = a[k];
            rem -= a[k];
            ++k;
        }
        a[k + 1] = rem;
        ++k;
    }
    return result;
}
```

### 3.3 Complexity Considerations

| n   | p(n)         | Time    | Memory   |
|-----|-------------|---------|----------|
| 10  | 42          | instant | trivial  |
| 20  | 627         | instant | ~10 KB   |
| 30  | 5604        | <1ms    | ~100 KB  |
| 50  | 204226      | ~10ms   | ~5 MB    |
| 100 | 190,569,292 | seconds | ~4 GB    |

**Practical limit: n ≤ 60-70.** Beyond that, the number of partitions grows exponentially (p(n) ~ exp(π√(2n/3)) / (4n√3)). The Maple t-core package also implicitly has this limit since `combinat[partition](n)` builds the full list.

**Add a safety guard:**
```cpp
if (n > 80) throw std::runtime_error("partitions: n=" + std::to_string(n) +
    " too large (p(n) ≈ " + partition_number(n).str() + "); limit is 80");
```

## 4. New Source File: `tcore.h`

All t-core algorithms go in a single new header, following the project convention. Dependencies: `series.h` (for `Frac`/`BigInt` if needed, but most operations are pure integer).

### 4.1 Core Functions to Implement

From `t-core.txt`, the public API (mapped from Maple `tcore[...]` procs):

| Maple function          | C++ function            | Signature                                     | Returns        |
|------------------------|------------------------|-----------------------------------------------|----------------|
| `combinat[partition]`  | `enumerate_partitions` | `(int n)`                                     | `vector<Partition>` |
| `tcore[tcoreofptn]`    | `tcoreofptn`           | `(Partition, int t)`                          | `Partition`    |
| `tcore[istcore]`       | `istcore`              | `(Partition, int t)`                          | `bool` (int64_t 0/1) |
| `tcore[tcores]`        | `tcores`               | `(int t, int n)`                              | `vector<Partition>` |
| `tcore[tquot]`         | `tquot`                | `(Partition, int t)`                          | `vector<Partition>` |
| `tcore[PHI1]`          | `phi1`                 | `(Partition, int t)`                          | DisplayOnly    |
| `tcore[invphi1]`       | `invphi1`              | `(Partition core, vector<Partition> quot, int t)` | `Partition` |
| `tcore[ptn2nvec]`      | `ptn2nvec`             | `(Partition, int t)`                          | `vector<int>` → Partition |
| `tcore[nvec2ptn]`      | `nvec2ptn`             | `(vector<int>)`                               | `Partition`    |
| `tcore[ptn2rvec]`      | `ptn2rvec`             | `(Partition, int t)`                          | `vector<int>` → Partition |
| `tcore[tcrank]`        | `tcrank`               | `(Partition, int t)`                          | `int64_t`      |
| `tcore[tresdiag]`      | `tresdiag`             | `(Partition, int t)`                          | DisplayOnly    |
| `tcore[makebiw]`       | `makebiw`              | `(Partition, int t, int mj)`                  | DisplayOnly    |

### 4.2 Internal Helper Functions (not exposed to REPL)

From `t-core.txt`:
- `rvec(ptn, p, k)` — count residue-k nodes in p-residue diagram. This is the fundamental building block.
- `np(ptn)` — number of parts (just `ptn.numParts()`)
- `addrimthook(ptn, j, L, t)` — add rim t-hook starting from jth part
- `findhookinpos(ptn, t, w, p)` — find position to insert rim hook
- `veccombo(C, VL)` — linear combination of vectors

### 4.3 Key Algorithm: `rvec` (Residue Vector)

This is the most important internal function. From the Maple source (line 515-524):

```cpp
inline int rvec(const Partition& ptn, int p, int k) {
    int x = 0;
    int nn = ptn.numParts();
    for (int m = 1; m <= nn; ++m) {
        int j = nn - m + 1;  // 1-indexed in Maple
        int part_val = ptn.parts[j - 1];  // 0-indexed in C++
        x += (part_val + ((p - m - k) % p + p) % p) / p;
    }
    return x;
}
```

Note: The `modp(p-m-k, p)` in Maple always returns non-negative. In C++, `%` can be negative, so use `((p - m - k) % p + p) % p`.

## 5. C++ Standard Library Features Useful for Partition Algorithms

### 5.1 Already Available (C++20)

- **`std::vector<int>`** — primary storage for partition parts. Fast, cache-friendly.
- **`std::sort` with `std::greater<int>()`** — normalize partitions to non-increasing order.
- **`std::accumulate`** — sum of parts (partition size).
- **`<algorithm>`: `std::remove_if`, `std::erase_if`** — remove zero/negative parts. C++20 adds `std::erase_if` for containers.
- **`std::format`** (C++20) — for formatted display output. Note: may not be available in all g++ versions. Stick with `std::to_string` + string concat for portability.

### 5.2 Useful Patterns

- **`std::span<const int>`** (C++20) — pass partition parts to functions without copying. Useful for internal helpers that just need a read-only view.
- **`std::ranges::sort`** (C++20) — cleaner sort syntax: `std::ranges::sort(parts, std::greater{})`.
- **Integer modular arithmetic** — wrap the C++ `%` operator for always-positive mod:
  ```cpp
  inline int mod(int a, int p) { return ((a % p) + p) % p; }
  ```
  This is needed pervasively in residue computations.

### 5.3 No External Dependencies Needed

All partition algorithms are pure integer arithmetic:
- No BigInt needed (parts are small ints)
- No Frac needed (no rational arithmetic in t-core computations, except `nvec2alphavec` which uses rationals for t=5,7 — but this is a display-only function)
- No Series needed (except for the GF connection functions that already exist)

## 6. REPL Integration Points

### 6.1 `dispatchBuiltin` Additions

Add cases in `dispatchBuiltin()` (repl.h) for each new function. Example pattern:

```cpp
if (name == "tcoreofptn") {
    if (args.size() != 2)
        throw std::runtime_error(runtimeErr(name, "expects 2 arguments"));
    Partition ptn = evalAsPartition(args[0].get(), env, sumIndices);
    int t = static_cast<int>(evi(1));
    return tcoreofptn(ptn, t);
}
```

### 6.2 Display Function Extension

Add to the `display()` visitor:

```cpp
} else if constexpr (std::is_same_v<T, Partition>) {
    std::cout << arg.str() << std::endl;
} else if constexpr (std::is_same_v<T, std::vector<Partition>>) {
    for (const auto& p : arg)
        std::cout << p.str() << std::endl;
}
```

### 6.3 Assignment Support

In `evalStmt()`, add:

```cpp
if (std::holds_alternative<Partition>(res)) {
    env.env[s->assignName] = std::get<Partition>(res);
    return res;
}
```

### 6.4 Variable Lookup in `eval()`

Currently `Expr::Tag::Var` returns the EnvValue directly as a Series. Need to handle the Partition case:

```cpp
case Expr::Tag::Var: {
    auto it = env.env.find(e->varName);
    if (it == env.env.end()) throw ...;
    if (std::holds_alternative<Partition>(it->second))
        return std::get<Partition>(it->second);
    return getSeriesFromEnv(it->second);
}
```

### 6.5 Help Table Additions

```cpp
{"istcore",     {"istcore(ptn, t)",         "test if partition is a t-core"}},
{"invphi1",     {"invphi1([core, quot], t)", "inverse GSK bijection"}},
{"makebiw",     {"makebiw(ptn, t, mj)",     "display bi-infinite words W_0,...,W_{t-1}"}},
{"nvec2ptn",    {"nvec2ptn(nvec)",          "partition (t-core) with given n-vector"}},
{"partitions",  {"partitions(n)",           "enumerate all partitions of n"}},
{"phi1",        {"phi1(ptn, t)",            "GSK bijection: ptn → [t-core, t-quotient]"}},
{"ptn2nvec",    {"ptn2nvec(ptn, t)",        "n-vector of t-core partition"}},
{"ptn2rvec",    {"ptn2rvec(ptn, t)",        "r-vector (residue counts) of partition mod t"}},
{"tcoreofptn",  {"tcoreofptn(ptn, t)",      "compute the t-core of a partition"}},
{"tcores",      {"tcores(t, n)",            "list all t-core partitions of n"}},
{"tcrank",      {"tcrank(ptn, t)",          "t-core crank value (0 to t-1)"}},
{"tquot",       {"tquot(ptn, t)",           "t-quotient of a partition"}},
{"tresdiag",    {"tresdiag(ptn, t)",        "display t-residue diagram"}},
```

### 6.6 Session Save/Load

Add serialization for `Partition` in save/load:

```
P varname 6,6,6,5,4,4,2,1,1
```

### 6.7 Tab Completion

The existing tab completion scans `getHelpTable()` keys, so new functions are automatically included.

## 7. Summary of Changes by File

| File        | Changes |
|-------------|---------|
| `tcore.h`   | **NEW FILE.** All partition algorithms: enumerate_partitions, rvec, tcoreofptn, istcore, tcores, tquot, phi1, invphi1, ptn2nvec, nvec2ptn, ptn2rvec, tcrank, tresdiag, makebiw, addrimthook, findhookinpos |
| `repl.h`    | Add `Partition` to `EvalResult` and `EnvValue`. Add `evalAsPartition()`. Handle `Tag::List` evaluation. Add dispatchBuiltin cases for all t-core functions. Extend `display()`. Extend `evalStmt()` assignment. Extend save/load. Add help entries. |
| `parser.h`  | **No changes needed.** List syntax `[a,b,c]` already parsed. |
| `series.h`  | **No changes needed.** |
| `qfuncs.h`  | **No changes needed.** |
| `convert.h` | **No changes needed.** |
| `main.cpp`  | Add `#include "tcore.h"` (before repl.h, or included by repl.h). |

## 8. Risk Assessment

| Risk | Mitigation |
|------|-----------|
| `EvalResult` variant getting large (15+ types) | Already has 13 alternatives; 2 more is fine. `std::variant` handles up to 256 types. |
| Partition enumeration blowing up memory for large n | Hard limit at n=80 with informative error message. |
| Maple's 1-indexed arrays vs C++ 0-indexed | Careful index translation in every algorithm. The `rvec` computation is particularly tricky — test thoroughly. |
| `invphi1` argument parsing (nested structure `[core, [q0,...]]`) | Parse as a special 3-argument form `invphi1(core, quot, t)` where quot is a list of lists. Or accept `invphi1(phi1_result, t)` where phi1_result is stored in a variable. Simplest: make it 3-arg `invphi1(core, quot, t)`. |
| `modp` in Maple always returns ≥ 0; C++ `%` doesn't | Use `((a % p) + p) % p` wrapper everywhere in tcore.h. |

## RESEARCH COMPLETE
