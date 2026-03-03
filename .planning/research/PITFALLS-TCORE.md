# PITFALLS-TCORE.md — Research: Common Mistakes Adding t-Core Partitions to qseries REPL

## 1. Type System Explosion

### The Problem

The current type system uses two variant types:

```
EvalResult = variant<Series, map<int,Frac>, vector<int>, vector<pair<int,Frac>>,
                     vector<JacFactor>, RelationKernelResult, RelationComboResult,
                     QFactorResult, CheckprodResult, CheckmultResult, FindmaxindResult,
                     int64_t, DisplayOnly, monostate>
EnvValue   = variant<Series, vector<JacFactor>>
```

Adding a partition type (e.g. `vector<int64_t>`) means:

- **EnvValue**: Must become `variant<Series, vector<JacFactor>, vector<int64_t>>`. Every `std::holds_alternative` / `std::get` call on EnvValue must handle the third case. Currently there are at least 6 such sites: `getSeriesFromEnv`, `saveSession`, `loadSession`, `evalStmt` (assignment), `jac2prod` dispatch, `jac2series` dispatch.

- **EvalResult**: Already has 14 alternatives. Adding `vector<int64_t>` as a 15th is fine syntactically but:
  - `display()` uses `std::visit` with `if constexpr` chains — must add a branch or the visit will silently discard partitions (compile error in some compilers, silent no-op in others with `auto&&` catch-alls).
  - `evalStmt` assignment path: currently handles `Series`, `vector<JacFactor>`, `int64_t` — must add partition case or assignments like `p := partition_list(5)` will throw "assignment requires Series or Jacobi product".
  - `toSeries()` helper: called from binary ops — will throw on partitions with cryptic "expected series expression" instead of clear "cannot do arithmetic on partitions".

### Mitigation

- Use a type alias `using Partition = std::vector<int64_t>;` and a display wrapper struct `struct PartitionResult { std::vector<int64_t> parts; };` to make the variant alternative distinct from `std::vector<int>` (already used for mprodmake).
- Grep all `std::holds_alternative`, `std::get`, and `std::visit` calls before merging. There are exactly these sites to update:
  1. `getSeriesFromEnv` — add clear error: "variable holds partition, not series"
  2. `saveSession` / `loadSession` — serialize partitions as `P varname part1,part2,...`
  3. `evalStmt` assignment — store partition in env
  4. `display()` — print partition as `[5, 4, 4, 2, 1, 1]`
  5. `toSeries()` — clear error message

### Critical Risk: `vector<int64_t>` vs `vector<int>`

`EvalResult` already contains `std::vector<int>` (for mprodmake). If you add `std::vector<int64_t>`, these are **distinct types** in the variant, so that's fine. But if you accidentally use `vector<int>` for partitions, it will match the mprodmake case in `display()` and format the partition as a product `(1+q)(1+q²)...` — completely wrong, and hard to debug because it won't crash, just produce garbage output.

---

## 2. Partition Ordering Convention

### The Problem

The Maple code uses **non-decreasing** order internally (smallest part first):

```maple
testptn := [1, 1, 2, 4, 4, 5, 6, 6, 6, 7, 7, 7, 7, 7, 8, 9, 9, 10, 10, 13, 16, 17];
```

But standard mathematical convention and typical user expectation is **non-increasing** (largest part first):
`[17, 16, 13, 10, 10, 9, 9, 8, 7, 7, 7, 7, 7, 6, 6, 6, 5, 4, 4, 2, 1, 1]`

The Maple code explicitly converts between conventions at I/O boundaries:

- `invphi1` (line 221): reverses input t-core from non-increasing to non-decreasing: `ptn:=[seq(tc[ntc-j+1],j=1..ntc)]`
- `invphi1` (line 239): reverses output back to non-increasing: `optn:=[seq(ptn[nn-j+1],j=1..nn)]`
- `tquot` (line 606): reverses to non-decreasing: `ptnz:=[seq(ptn[nps-j+1],j=1..nps),seq(0,...)]`
- `tresdiag` (line 634): iterates `for j from n by -1 to 1` — processes in non-decreasing order
- `makebiw` (line 291): reverses: `ip:=[seq(ptn[np-k],k=0..(np-1)),...]`
- `rvec` (line 521): uses `j:=nn-m+1` — iterates non-decreasingly through a non-increasing input

### The Specific Bug

In `rvec`, the formula is:
```
x := x + trunc( (ptn[j] + modp(p - m - k, p)) / p )
```
where `m` ranges from 1 to nn and `j = nn - m + 1`. This means:
- `m=1` → `j=nn` (last element, smallest part in non-increasing order)  
- `m=nn` → `j=1` (first element, largest part)

If we store partitions in non-increasing order (natural for users), we must be careful: `m` is the **row index from the bottom** of the Young diagram. Getting this backward (m from top) will produce wrong r-vectors and cascade into wrong n-vectors, wrong t-cores, and wrong t-quotients.

### Mitigation

**Choose one internal convention and document it.** Recommendation: store internally in non-increasing order (matches user I/O, mathematical convention). Then `rvec` becomes:

```cpp
// ptn is non-increasing: ptn[0] >= ptn[1] >= ... >= ptn[nn-1]
// m = row index from bottom (1-based), maps to ptn[nn - m] (0-based)
for (int m = 1; m <= nn; ++m) {
    int lambda_m = ptn[nn - m];  // m-th row from bottom
    x += (lambda_m + ((p - m % p - k % p + 2*p) % p)) / p;
}
```

Any function that reverses the partition (as Maple does) must be identified and the reversal removed if we use non-increasing throughout. Functions that need auditing:
- `rvec` ✓ (index reversal built into loop)
- `tquot` ✓ (explicit reversal at line 606)
- `invphi1` ✓ (reversal at lines 221, 239)
- `makebiw` ✓ (reversal at line 291)
- `nvec2ptn` — outputs in Maple's internal order, needs reversal for our convention
- `tcoreofptn` — calls rvec, which handles its own indexing

---

## 3. `partition(n)` Performance — Enumeration vs Counting

### The Problem

The REPL already has `partition(n)` that returns `p(n)` the count (via `partition_number()`). The t-core package needs actual enumeration: `combinat[partition](n)` returns the list of all partitions of n.

| n   | p(n)         | Practical? |
|-----|-------------|------------|
| 10  | 42          | instant    |
| 20  | 627         | instant    |
| 30  | 5,604       | fast       |
| 40  | 37,338      | ~0.1s      |
| 50  | 204,226     | ~1s        |
| 60  | 966,467     | ~5s        |
| 80  | 15,796,476  | minutes    |
| 100 | 190,569,292 | impossible |

The Maple `tcores(p,n)` function (line 584) calls `combinat[partition](n)` and filters, which is O(p(n)) in both time and space.

### Where Enumeration is Actually Needed

- `tcores(p, n)` — list all p-cores of n. Calls partition(n) then filters.
- `numnepo(n)` — count partitions with odd number of even parts. Calls partition(n).
- Testing/validation — verifying bijections on all partitions of small n.

### Mitigation

1. **Hard limit**: Cap enumeration at n ≤ 60 (p(60) ≈ 10^6, manageable). Error message for larger n: "partition enumeration: n too large (max 60), use partition(n) for the count".
2. **Direct t-core enumeration**: For `tcores(p, n)`, instead of enumerating ALL partitions and filtering, enumerate t-cores directly via n-vectors. A p-core of n has n-vector (n₀,...,n_{p-1}) with Σnᵢ = 0 and Σᵢ (formula involving nᵢ) = n. This is vastly more efficient — the number of p-cores of n grows polynomially, not exponentially.
3. **Name collision**: The REPL already has `partition(n)` meaning p(n). The tcore package needs `partition(n)` meaning "list all partitions of n". Must use a different name, e.g. `partitions(n)` or `partition_list(n)`.

---

## 4. `rvec` Formula Correctness

### The Core Formula (Maple line 515-525)

```maple
rvec := proc(ptn, p, k)
    x := 0;
    nn := np(ptn);  # number of parts
    for m from 1 to nn do
        j := nn - m + 1;
        x := x + trunc((ptn[j] + modp(p - m - k, p)) / p);
    od;
    RETURN(x);
end;
```

### What This Computes

For a partition λ = (λ₁ ≥ λ₂ ≥ ... ≥ λ_nn) (non-increasing input, accessed in reverse), `rvec(λ, p, k)` = number of cells in the p-residue diagram of λ colored k.

The formula for each row m (counting from bottom, 1-based):
```
floor( (λ_m + ((p - m - k) mod p)) / p )
```
where λ_m is the m-th part from the bottom.

### Pitfall 1: Integer Division vs Floor

In Maple, `trunc(x)` truncates toward zero, which equals `floor(x)` for non-negative x. The expression `(ptn[j] + modp(p-m-k, p))` is always ≥ 0 (since ptn[j] ≥ 0 and modp returns 0..p-1), so `trunc` = `floor` here. In C++, integer division of non-negative values is fine:

```cpp
int val = ptn_m + ((p - m%p - k%p + 2*p) % p);
x += val / p;  // OK: both operands non-negative
```

**But**: if `p - m - k` is negative, `modp(p-m-k, p)` in Maple always returns a value in [0, p-1]. C++ `%` operator with negative operands is implementation-defined (before C++11) or truncates toward zero (C++11+), so `-7 % 5 = -2`, not 3. Must use:
```cpp
int rem = ((p - (m % p) - (k % p)) % p + p) % p;  // force into [0, p-1]
```

### Pitfall 2: Index Direction

With non-increasing storage `ptn[0] >= ptn[1] >= ...`:
- m=1 (bottom row) → λ₁ is the **smallest** part → `ptn[nn-1]` (0-based) = `ptn[nn-m]`
- m=nn (top row) → λ_nn is the **largest** part → `ptn[0]` = `ptn[nn-m]`

So the mapping is `ptn_m = ptn[nn - m]` (0-based). Getting this wrong (using `ptn[m-1]` instead) will compute wrong r-vectors for any non-symmetric partition.

### Test Cases

From the Maple source test data, with `testptn = [1,1,2,4,4,5,6,6,6,7,7,7,7,7,8,9,9,10,10,13,16,17]` (non-increasing: `[17,16,13,10,10,9,9,8,7,7,7,7,7,6,6,6,5,4,4,2,1,1]`), p=5:

These r-vector values can be verified against Maple output and used as regression tests.

---

## 5. `nvec2ptn` Reconstruction — Frobenius Coordinates and Conjugation

### The Algorithm (Maple lines 391-447)

`nvec2ptn` reconstructs a partition (t-core) from its n-vector:

1. Compute X = sum of positive entries in nvec (Durfee square size)
2. Build Frobenius arm lengths `ppartsA`: for each positive `nvec[j]` (with residue i=j-1), add `t*(k-1)+i` for k=1..nvec[j]
3. Build conjugate n-vector: `cnvec[j] = -nvec[t-j+1]`
4. Build Frobenius leg lengths `ppartsB`: same procedure using cnvec
5. Construct upper part: `partsA = [ppartsA[j] + X - j + 1 for j=1..X]`
6. Construct lower part: `pp2a = [ppartsB[j] - j + 1 for j=1..X]`, remove zeros
7. **Conjugate** pp2a using `combinat[conjpart]` → `partsB`
8. Return `[op(partsB), op(partsA)]`

### Pitfall: Implementing Partition Conjugation

Step 7 calls `combinat[conjpart]` which computes the conjugate (transpose) of a partition. We must implement this from scratch. The conjugate of λ = (λ₁ ≥ λ₂ ≥ ... ≥ λ_k) is λ' where:
```
λ'_j = |{i : λ_i ≥ j}|
```

Implementation:
```cpp
inline std::vector<int64_t> conjpart(const std::vector<int64_t>& ptn) {
    if (ptn.empty()) return {};
    int64_t maxPart = *std::max_element(ptn.begin(), ptn.end());
    std::vector<int64_t> conj(maxPart, 0);
    for (int64_t part : ptn)
        for (int64_t j = 0; j < part; ++j)
            conj[j]++;
    // conj is non-increasing by construction
    return conj;
}
```

This is O(n * max_part) which is fine for reasonable partitions but could be slow for partitions with very large parts (e.g. [1000000]). Alternatively use a histogram approach:

```cpp
inline std::vector<int64_t> conjpart(const std::vector<int64_t>& ptn) {
    if (ptn.empty()) return {};
    int64_t maxPart = ptn[0];  // assumes non-increasing
    std::vector<int64_t> conj(maxPart, 0);
    // For sorted partition, conj[j] = number of parts >= j+1
    for (size_t i = 0; i < ptn.size(); ++i) {
        if (ptn[i] > 0)
            conj[0]++;  // not useful alone
    }
    // Better: since ptn is sorted, conj[j] = index of first part < j+1
    // This is O(max_part * log(n)) with binary search
    for (int64_t j = 0; j < maxPart; ++j) {
        // count parts >= j+1: binary search for j+1 in reversed array
        auto it = std::lower_bound(ptn.rbegin(), ptn.rend(), j + 1);
        conj[j] = std::distance(it, ptn.rend());
    }
    return conj;
}
```

### Pitfall: 1-Based vs 0-Based in Frobenius Coordinates

The Maple code uses 1-based indexing throughout. In the Frobenius construction:
- `ppartsA[j] + X - j + 1` where j ranges 1..X
- `ppartsB[j] - j + 1` where j ranges 1..X

Converting to 0-based: if `ppartsA` is a 0-based vector of length X:
- `partsA[j] = ppartsA[j] + X - (j+1) + 1 = ppartsA[j] + X - j` for j=0..X-1

Off-by-one here will produce wrong partitions that are close enough to look plausible but fail identity checks.

### Pitfall: Empty n-vector entries / Zero Durfee square

If all nvec entries are ≤ 0, then X = 0 and the Frobenius construction produces the empty partition. This must be handled: the returned partition should be `[]`, not an error.

---

## 6. `invphi1` — Rim Hook Insertion

### The Algorithm

`invphi1` inverts the GSK (Garvan-Kim-Stanton) Φ₁ bijection:
1. Start with t-core in non-increasing order → reverse to non-decreasing
2. For each quotient partition tq[j] (j from t down to 1):
   - For each part p of tq[j] (from largest to smallest):
     - Find position via `findhookinpos(ptn, t, r, p)` where r = j-1
     - Insert rim t-hook via `addrimthook(ptn, posi, p, t)`
3. Reverse output back to non-increasing

### Pitfall: `addrimthook` Array-List Conversion (Maple Lines 79-86, 105-106)

The Maple code explicitly warns:
```maple
# 11/11/03: Convert newptn to array. Apparently maple complains
# if list length > 100.
```

In Maple, lists are immutable and indexed assignment on lists > 100 elements fails. Arrays are mutable. The code converts to array, does the rim-hook insertion, then converts back.

**In C++, this is a non-issue** — `std::vector` supports indexed mutation of any size. But the Maple workaround reveals that large partitions (>100 parts) are expected in practice. This means:
- Padding with dummy zeros (`seq(0, k=1..(j-nmp))` at line 74, `seq(0, k=1..(p*t))` at line 186) creates vectors with hundreds of trailing zeros
- The rim-hook insertion loop (`while len < L*t`) iterates up to L*t times per hook
- For t=5 with quotient parts up to 20, that's 100 iterations per hook, times number of parts across all quotients

### Pitfall: `findhookinpos` Correctness

```maple
findhookinpos := proc(ptn, t, w, p)
    # find part number where can insert rim pt-hook from word W_w
    prenewptn := [op(ptn), seq(0, k=1..(p*t))];  # pad with p*t zeros
    newptn := convert(prenewptn, array);
    find := 0; j := 1; fj := 0;
    while find < p do
        if modp(newptn[j] - j, t) = w then
            fj := j;
            find := find + 1;
        fi;
        j := j + 1;
    od;
    RETURN(fj);
end;
```

This scans for the p-th position where `(λ_j - j) mod t = w`. The padding with `p*t` zeros ensures we'll always find enough positions. In C++:
- The partition is in non-decreasing order (internally in Maple)
- `j` is 1-based in Maple; convert to 0-based: `newptn[j-1] - j` vs `newptn[j] - (j+1)` — easy off-by-one
- The `w` (residue) ranges 0..t-1; `modp` returns non-negative result. C++ `%` with negative numerator: MUST use `((val % t) + t) % t`

### Pitfall: Iteration Order in invphi1

The quotients are processed from j=t down to 1, and within each quotient, parts are processed from largest to smallest (k from ntqj down to 1). Reversing this order will produce a different (wrong) partition because rim-hook positions depend on the current state of the partition being built up.

---

## 7. `tcrank` Formula — Integer Overflow

### The Formula (Maple lines 590-602)

```maple
tcrank := proc(ptn, p)
    h := t -> (t - (p-1)/2)^(p-3);
    x := 0;
    m := np(ptn);
    for i from 1 to m do
        j := m - i + 1;
        lam := ptn[j];
        x := x + modp(h(lam - i) - h(i - 1), p);
    od;
    RETURN(modp(x, p));
end;
```

### Pitfall 1: The `h` Function Uses Rational Arithmetic in Maple

`h(t) = (t - (p-1)/2)^(p-3)` — in Maple, `(p-1)/2` is exact rational when p is odd (which it always is for interesting primes). So `h(t)` computes `(t - (p-1)/2)^(p-3)` in exact rationals, then takes `modp`.

But wait — the Maple `modp(expr, p)` works on rationals! For `modp(a/b, p)`, it computes `a * b^(-1) mod p`. In C++, we'd need modular inverse if we compute this way.

**However**, there's a simpler approach. Since we only need `h(t) mod p`, and `(p-1)/2` is an integer mod p (it's the modular inverse of 2), we can compute entirely in modular arithmetic:

```cpp
int64_t half_p_minus_1 = (p - 1) / 2;  // exact since p is odd
// h(t) = (t - half_p_minus_1)^(p-3) mod p
int64_t base = (((t - half_p_minus_1) % p) + p) % p;
int64_t result = mod_pow(base, p - 3, p);
```

### Pitfall 2: Overflow in `mod_pow`

For p=97 (a reasonable prime), `p-3 = 94`, and intermediate values in modular exponentiation are at most `96^2 = 9216` — well within int64_t. But for p=10007 (exotic), `(p-1)^2 = 10006^2 ≈ 10^8` — still fine for int64_t. So overflow isn't actually a problem for modular exponentiation with `int64_t`, but it IS a problem if you try to compute `(t - (p-1)/2)^(p-3)` directly without modular reduction:
- p=13: exponent is 10, base could be up to 10^6 → 10^60 — **overflows int64_t** (max ~9.2 × 10^18)
- Must use modular exponentiation, never direct power

### Pitfall 3: Variable Name Bug in Maple Source

Line 593: `h := t -> (t - (p-1)/2)^(p-3)` uses `t` as the lambda parameter, but `t` is also the parameter name for the modulus in many other tcore functions. In the `tcrank` function, the modulus is called `p`, so this is fine — but if someone copies this code and the modulus variable is named `t`, the lambda function would shadow it. In C++, name the parameter differently:

```cpp
auto h = [p](int64_t val) -> int64_t {
    int64_t base = (((val - (p-1)/2) % p) + p) % p;
    return mod_pow(base, p - 3, p);
};
```

---

## 8. Display Functions — `tresdiag` and `makebiw`

### The Problem

Both `tresdiag` (line 630) and `makebiw` (line 270) use `printf` to output formatted text and return nothing (`RETURN()`). In the REPL:

- Most builtins return an `EvalResult` that `display()` handles
- `DisplayOnly{}` exists for functions that print their own output
- But these functions produce multi-line tabular output that doesn't fit the normal pattern

### `tresdiag` Output Format

For partition [5, 4, 4, 2, 1, 1] with t=5:
```
4 0 1 2 3   (row for part 5)
3 4 0 1     (row for part 4)
2 3 4 0     (row for part 4)
1 2         (row for part 2)
0           (row for part 1)
4           (row for part 1)
```

### `makebiw` Output Format

Bi-infinite word display with column headers:
```
   -3-2-1 0 1 2 3
W0  E E E E N E N
W1  E E N N E N N
W2  E E E N N N N
W3  E E E E E N N
W4  E E N E E N N
```

### Mitigation

Return `DisplayOnly{}` and print directly to `std::cout`, same pattern as `coeffs`, `divisors`, `series`. For the Emscripten/web build, this works since stdout is captured.

For `tresdiag`, collect lines into a `std::string` and print once. For `makebiw`, same approach. Both should use ANSI color if available (highlight residues that match certain criteria).

---

## 9. Empty Partition `[]`

### The Problem

The empty partition `[]` (= unique partition of 0) must be handled correctly by every function:

| Function | Expected behavior for `[]` |
|----------|---------------------------|
| `np([])` | 0 |
| `ptnnorm([])` | 0 |
| `rvec([], p, k)` | 0 for all k |
| `ptn2nvec([], p)` | `[0, 0, ..., 0]` (length p) |
| `istcore([], p)` | true (for all p) |
| `tcoreofptn([], p)` | `[]` |
| `tquot([], t)` | `[[], [], ..., []]` (t empty quotients) |
| `tcrank([], p)` | 0 |
| `tresdiag([], t)` | (empty output) |
| `nvec2ptn([0,0,...,0])` | `[]` |
| `invphi1([[], [[],...,[]]], t)` | `[]` |
| `conjpart([])` | `[]` |

### Common Bugs

1. **`rvec` with empty partition**: The loop `for m from 1 to nn` doesn't execute when nn=0, so x=0. Correct. But `nn = np(ptn) = nops(ptn)` — if the partition is stored as an empty vector, `nops` returns 0. In C++, `ptn.size()` returns 0. No issue IF the loop uses `<` not `<=` with unsigned (size_t underflow: `size_t(0) - 1` wraps to huge value).

2. **`nvec2ptn` with all-zero n-vector**: X = 0 (no positive entries). The Frobenius construction produces empty arm/leg vectors, and `combinat[conjpart]([])` returns `[]`. Result: `[op([]), op([])]` = `[]`. Must handle: don't call `max_element` on empty vector.

3. **`tcrank` with empty partition**: m = 0, loop doesn't execute, x = 0, `modp(0, p) = 0`. Correct.

4. **Parser/REPL**: An empty list `[]` currently isn't parseable — `parseCommaSeparatedExprs` expects at least one expression. Need to handle `[]` as valid syntax for the empty partition:
```cpp
// In parsePrimary:
if (peek().kind == Token::Kind::LBRACK) {
    consume();
    if (peek().kind == Token::Kind::RBRACK) {  // empty list
        consume();
        return Expr::makeList({});
    }
    auto elems = parseCommaSeparatedExprs();
    expect(Token::Kind::RBRACK);
    return Expr::makeList(std::move(elems));
}
```

---

## 10. Additional Pitfalls

### 10a. `avec2nvec` — Only t=5 Implemented

The Maple code (line 122-133) hardcodes a 5×5 matrix N4 and only supports t=5. For the REPL, we should either:
- Generalize to arbitrary t (compute the transformation matrix)
- Or clearly document the limitation and throw for t ≠ 5, 7

The `nvec2alphavec` (line 357-389) similarly only handles t=5 and t=7 with hardcoded coefficients.

### 10b. `tquot` — Lambda-bar Computation

The t-quotient algorithm constructs `lambdabar = [ptnz[j] - j for j=1..nps+t]` where ptnz is the reversed partition padded with t zeros. The subtraction `ptnz[j] - j` can produce negative values, and the grouping by `modp(lambdabar[j], t)` must use Maple-style modp (always non-negative).

Critical: the `floor(muibar[k]/t)` calls use negative dividends. Maple's `floor` returns true floor (toward -∞), while C++ integer division truncates toward zero. For negative values:
- Maple: `floor(-7/5) = -2`
- C++: `-7 / 5 = -1` (wrong!)

Must use:
```cpp
auto floor_div = [](int64_t a, int64_t b) -> int64_t {
    return a / b - (a % b != 0 && (a ^ b) < 0);
};
```

### 10c. `freqtab` — Pure Display Function

`freqtab` (line 201) builds a frequency table and prints it with `lprint`. This is a utility function that returns nothing useful. Implement as a DisplayOnly function.

### 10d. t-Quotient Structure

The t-quotient is a list of t partitions: `[[2,1], [3], [], [1,1], []]` for t=5. This is a **nested** data structure — a vector of vectors. Must decide how to represent this:
- New type `TQuotient = std::vector<std::vector<int64_t>>`?
- Or flatten into display string?

For `invphi1`, the input is `[tcore, tquotient]` — a pair of (partition, list-of-partitions). This requires either:
- A dedicated `PHI1Result` struct as an EvalResult alternative
- Or separate functions: `tcore(ptn, p)` returns partition, `tquot(ptn, p)` returns formatted display

### 10e. Session Save/Load

If partitions are stored in `EnvValue`, the session serializer needs a new tag (e.g., `P`) and deserializer. Currently handles `S` (Series) and `J` (JacFactor). Adding `P varname p1,p2,p3,...` is straightforward but must handle the empty partition: `P varname` (no parts after name).

---

## 11. Recommended Implementation Order

Based on dependency analysis:

1. **Partition type + display** — type alias, display formatting, parser support for `[]`
2. **conjpart** — needed by nvec2ptn
3. **rvec** — core formula, most functions depend on it. Test thoroughly with known examples
4. **ptn2rvec, ptn2nvec** — thin wrappers around rvec
5. **nvec2ptn** — uses conjpart and Frobenius coordinates
6. **istcore, tcoreofptn** — use rvec and nvec2ptn
7. **tquot** — independent algorithm, uses only basic partition operations
8. **tresdiag, makebiw** — display functions, low priority
9. **findhookinpos, addrimthook** — needed for invphi1
10. **invphi1, PHI1** — the full bijection, most complex
11. **tcrank** — standalone, needs mod_pow
12. **tcores, partitions** — partition enumeration, needs performance limits

---

## RESEARCH COMPLETE
