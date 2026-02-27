# Q-Series REPL — Project Specification

## Project Overview

Build a **standalone, single-binary REPL** (read-eval-print loop) for q-series computation in **C++ (g++ 13.3, C++20)**. The REPL reimplements the core functionality of Frank Garvan's `qseries` Maple package — the standard package used by researchers in number theory, partition theory, and modular forms.

**Non-negotiable constraints:**

- **Single static binary** — compiles with `g++ -static -O2`, zero runtime dependencies
- **No external libraries** — no GMP, no Boost, no package managers. All bignum arithmetic built from scratch
- **Exact rational arithmetic** throughout — no floating point anywhere in the mathematical pipeline
- **Must handle the computations in the reference document** — see Acceptance Tests section below

---

## What This Program Does (For Someone Who Doesn't Know q-Series)

A *q-series* is a formal power series in a variable called `q`, like:

```
1 + q + q² + 2q³ + 3q⁴ + 5q⁵ + ...
```

The coefficients are often combinatorially meaningful (e.g., partition counts). This REPL lets a mathematician:

1. **Build** these series from standard constructions (eta products, theta functions, q-factorials)
2. **Convert** a series into an infinite product form — the key operation (Andrews' algorithm)
3. **Identify** whether a series is an eta-product, theta-product, or Jacobi-product
4. **Find algebraic relations** between lists of series (by reducing to linear algebra)
5. **Sift** coefficients — extract every n-th coefficient to get new generating functions

The classic example: given the Rogers-Ramanujan series `Σ q^(n²) / (q;q)_n`, the REPL should recover the product `1 / ((q;q⁵)_∞ · (q⁴;q⁵)_∞)`.

---

## Architecture

```
┌─────────────────────────────────────────────────────┐
│                     REPL + Parser                   │  ← User-facing layer
│  - Line editor with history (readline-style)        │
│  - Expression parser (recursive descent)            │
│  - Variable bindings, function calls                │
├─────────────────────────────────────────────────────┤
│                  q-Series Functions                  │  ← Mathematical operations
│  aqprod, qbin, etaq, theta2/3/4, tripleprod,       │
│  quinprod, winquist, prodmake, etamake,             │
│  jacprodmake, sift, findhom, findnonhom,            │
│  findhomcombo, findnonhomcombo, findpoly            │
├─────────────────────────────────────────────────────┤
│              Series (Truncated Power Series)         │  ← Core data type
│  - map<int, Frac> coefficients, int truncation      │
│  - Arithmetic: +, -, *, /, power, inverse, compose  │
├─────────────────────────────────────────────────────┤
│              Frac (Exact Rationals a/b)              │  ← Exact arithmetic
│  - Auto-reducing, GCD on every operation             │
├─────────────────────────────────────────────────────┤
│              BigInt (Arbitrary Precision)             │  ← Foundation
│  - Base 10^9 vector<uint32_t>, signed               │
│  - +, -, *, divmod, gcd                              │
└─────────────────────────────────────────────────────┘
```

### File Structure

```
src/
  bigint.h         — BigInt: arbitrary precision signed integer
  frac.h           — Frac: exact rational (auto-reduced, uses BigInt)
  series.h         — Series: truncated formal power series in q
  qfuncs.h         — All q-series functions (aqprod through winquist)
  convert.h        — Product conversion: prodmake, etamake, jacprodmake
  linalg.h         — Rational matrix kernel (for findhom/findnonhom)
  relations.h      — findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly
  parser.h         — Tokenizer + recursive-descent expression parser
  repl.h           — REPL loop, variable environment, help system
  main.cpp         — Entry point
Makefile
```

---

## Layer 1: BigInt

Arbitrary precision signed integer. Base 10⁹ (each digit is `uint32_t` in `[0, 999999999]`), stored least-significant-first. This base is chosen because (a) two digits fit in `uint64_t` for multiplication, and (b) conversion to/from decimal strings is trivial.

### API

```cpp
struct BigInt {
    bool neg = false;
    std::vector<uint32_t> d; // digits base 10^9, d[0] = least significant

    BigInt();                    // zero
    BigInt(int64_t v);           // from machine integer
    BigInt(const std::string& s); // from decimal string

    bool isZero() const;
    BigInt abs() const;

    // Comparison
    static int cmpAbs(const BigInt& a, const BigInt& b);
    bool operator==(const BigInt&) const;
    bool operator!=(const BigInt&) const;
    bool operator<(const BigInt&) const;
    bool operator>(const BigInt&) const;
    bool operator<=(const BigInt&) const;
    bool operator>=(const BigInt&) const;

    // Arithmetic
    BigInt operator-() const;     // unary negate
    BigInt operator+(const BigInt&) const;
    BigInt operator-(const BigInt&) const;
    BigInt operator*(const BigInt&) const;
    BigInt operator/(const BigInt&) const;
    BigInt operator%(const BigInt&) const;
    static std::pair<BigInt,BigInt> divmod(const BigInt& a, const BigInt& b);

    std::string str() const;      // decimal representation
};

BigInt bigGcd(BigInt a, BigInt b); // Euclidean algorithm
```

### Implementation Notes

- **Addition**: Walk both digit vectors, propagate carry. O(max(n,m)).
- **Subtraction**: Requires `|a| >= |b|` helper; route through sign logic in `operator-`.
- **Multiplication**: Schoolbook O(n·m). This is fine — our numbers rarely exceed ~100 digits. No need for Karatsuba.
- **Division**: Long division in base 10⁹. For each quotient digit, binary search `[0, BASE-1]` for the largest `k` where `k * divisor <= remainder`. This is the trickiest part; get the borrowing right.
- **GCD**: Standard Euclidean using `divmod`.

**Critical**: Every operation must handle sign correctly. Zero is always non-negative. Test edge cases: `0 * anything = 0`, `(-a) * (-b) = a*b`, etc.

---

## Layer 2: Frac

Exact rational number. Auto-reduces on construction via GCD.

### API

```cpp
struct Frac {
    BigInt num, den; // den always > 0 after reduce()

    Frac();                      // 0/1
    Frac(int64_t n);             // n/1
    Frac(int64_t n, int64_t d);
    Frac(BigInt n, BigInt d);

    void reduce();               // normalize: den > 0, gcd(|num|,den) = 1
    bool isZero() const;
    bool isOne() const;
    Frac abs() const;

    Frac operator-() const;
    Frac operator+(const Frac&) const;
    Frac operator-(const Frac&) const;
    Frac operator*(const Frac&) const;
    Frac operator/(const Frac&) const;
    bool operator==(const Frac&) const;
    bool operator<(const Frac&) const;
    // ... other comparisons

    std::string str() const;     // "3" or "-7/12"
};
```

### Implementation Notes

- `reduce()` is called in every constructor and should: (1) handle `den < 0` by flipping both signs, (2) compute `gcd(|num|, den)` and divide both, (3) set `num = 0, den = 1` if numerator is zero.
- Arithmetic is the standard: `a/b + c/d = (a*d + c*b) / (b*d)` then reduce. This generates intermediate BigInts but that's fine for our size range.
- Comparison: `a/b < c/d` iff `a*d < c*b` (when `b,d > 0`).

---

## Layer 3: Series

Truncated formal power series: `f(q) = Σ c_n q^n + O(q^T)`.

### Internal Representation

```cpp
struct Series {
    std::map<int, Frac> c; // sparse: exponent → coefficient (only nonzero entries)
    int trunc;              // series is valid to O(q^trunc)
};
```

Using `std::map<int, Frac>` (ordered) rather than `unordered_map` because:
- Iteration in exponent order is needed constantly (display, minExp, prodmake)
- Series are sparse for most q-series computations
- The ordered keys make `display()` trivial

### API

```cpp
struct Series {
    // Constructors
    static Series zero(int T);
    static Series one(int T);
    static Series q(int T);          // the variable q itself
    static Series qpow(int e, int T); // q^e
    static Series constant(Frac f, int T);

    // Coefficient access
    Frac coeff(int n) const;         // returns 0 for missing exponents
    void setCoeff(int n, Frac v);
    int minExp() const;              // smallest exponent with nonzero coeff
    int maxExp() const;

    // Truncation
    void clean();                    // remove zeros and terms >= trunc
    Series truncTo(int T) const;

    // Arithmetic
    Series operator-() const;
    Series operator+(const Series&) const;
    Series operator-(const Series&) const;
    Series operator*(const Series&) const;
    Series operator*(const Frac&) const;
    Series operator/(const Series&) const;  // uses inverse()
    Series pow(int n) const;               // binary exponentiation, handles negatives
    Series inverse() const;                // 1/f via term-by-term recurrence

    // Composition
    Series subs_q(int k) const;     // f(q) → f(q^k): multiply all exponents by k

    // Display
    std::string str(int maxTerms = 30) const;
    void display(int maxTerms = 40) const;

    // Utility
    std::vector<Frac> coeffList(int from, int to) const;
};
```

### Key Implementation Details

**Multiplication** (`operator*`): Double loop over both sparse maps. For each pair `(e1,c1)` and `(e2,c2)`, accumulate `c1*c2` at exponent `e1+e2`. Skip if `e1+e2 >= min(trunc_a, trunc_b)`. This is the hot path — everything depends on it.

**Inverse** (`inverse()`): This is mathematically crucial and error-prone. Given `f = c₀ + c₁q + c₂q² + ...` with `c₀ ≠ 0`, compute `g = 1/f` via the recurrence:

```
g[0] = 1/c₀
g[n] = -(1/c₀) * Σ_{j=1}^{n} c_j * g[n-j]     for n ≥ 1
```

If the series doesn't start at exponent 0 (say it starts at exponent `m`), shift first: compute `h = f / q^m`, invert `h`, then the result is `g / q^m` (i.e., `g` with exponents shifted by `-m`). Make sure to handle the effective truncation `T - m`.

**Power** (`pow(n)`): Binary exponentiation for positive `n`. For negative `n`, compute `inverse().pow(-n)`.

**subs_q(k)**: Replace every exponent `e` with `e*k`. Update truncation to `trunc * k`. This is used everywhere — e.g., `theta3(q^2, T)` is implemented as `theta3(q, T).subs_q(2)`.

---

## Layer 4: Q-Series Functions

These are the "building block" functions that construct series from mathematical definitions.

### 4.1 aqprod — Rising q-Factorial

```
aqprod(a, q, n, T)  →  (a; q)_n = Π_{k=0}^{n-1} (1 - a·q^k)
```

**Implementation**: Start with `result = 1`. For each `k` from `0` to `n-1`, compute `factor = 1 - a·q^k` as a Series, multiply `result *= factor`, truncate to `T`. The `a` and `q` parameters are Series objects (not just the variable q — a could be `q^2`, `q^3`, etc.).

**Example**: `aqprod(q, q, 5, 20)` computes `(1-q)(1-q²)(1-q³)(1-q⁴)(1-q⁵)`.

### 4.2 qbin — Gaussian Polynomial (q-Binomial)

```
qbin(q, m, n, T)  →  [n choose m]_q = (q;q)_n / ((q;q)_m · (q;q)_{n-m})
```

**Implementation**: Use the product formula directly to avoid series division:
```
[n;m]_q = Π_{i=1}^{m} (1 - q^{n-m+i}) / (1 - q^i)
```
Each step is one multiplication and one division by a short polynomial.

**Edge cases**: Return `0` if `m < 0` or `m > n`. Return `1` if `m == 0` or `m == n`.

### 4.3 etaq — Eta Product

```
etaq(q, k, T)  →  Π_{n=1}^{∞} (1 - q^{kn}),  truncated to O(q^T)
```

**Implementation**: Start with `result = 1`. For `n = 1, 2, 3, ...` while `k*n < T`, multiply `result *= (1 - q^{kn})`, truncate to `T` after each step.

**This is the workhorse function** — nearly everything in the package reduces to products of `etaq` at the end.

### 4.4 Theta Functions

**theta2(q, T)**: Returns the series for `θ₂(q) / q^{1/4}` (integer exponents only):

```
θ₂(q)/q^{1/4} = 2·Σ_{n=0}^{∞} q^{n(n+1)}
```

Set coefficient of `q^{n(n+1)}` to `2` for each `n ≥ 0` while `n(n+1) < T`.

**theta3(q, T)**:
```
θ₃(q) = Σ_{n=-∞}^{∞} q^{n²} = 1 + 2·Σ_{n=1}^{∞} q^{n²}
```

Set `coeff[0] = 1`, then `coeff[n²] += 2` for `n = 1, 2, ...` while `n² < T`.

**theta4(q, T)**:
```
θ₄(q) = Σ_{n=-∞}^{∞} (-1)^n q^{n²} = 1 + 2·Σ_{n=1}^{∞} (-1)^n q^{n²}
```

Same structure as theta3 but with alternating signs.

**theta(z, q, T)** — General theta series with two Series arguments:
```
θ(z, q) = Σ_{n=-∞}^{∞} z^n · q^{n²}
```

### 4.5 Triple Product

```
tripleprod(z, q, T) = Π_{n≥1} (1 - z·q^{n-1})(1 - z⁻¹·q^n)(1 - q^n)
```

z and q are both Series. Compute iteratively: for each n, multiply in the three factors and truncate. The loop runs while `n < T` (since `q^n` contributes exponent ≥ n).

**Usage example from the document**: `tripleprod(q^n, q^33, 10)` computes `Q(n)` used in the Winquist identity examples.

### 4.6 Quintuple Product

```
quinprod(z, q, T) = (-z;q)_∞ · (-q/z;q)_∞ · (z²q;q²)_∞ · (q/z²;q²)_∞ · (q;q)_∞
```

**Implementation**: Accumulate the five infinite products factor by factor:
- For n = 1, 2, ... while n < T:
  - Multiply by `(1 + z·q^{n-1})` — from `(-z;q)_∞`
  - Multiply by `(1 + q^n/z)` — from `(-q/z;q)_∞`
  - Multiply by `(1 - q^n)` — from `(q;q)_∞`
- For m = 1, 2, ... while 2m-1 < T:
  - Multiply by `(1 - z²·q^{2m-1})` — from `(z²q;q²)_∞`
  - Multiply by `(1 - q^{2m-1}/z²)` — from `(q/z²;q²)_∞`

### 4.7 Winquist's Identity

```
winquist(a, b, q, T) = Σ_{n≥0} Σ_m (-1)^{n+m} · [
    (a^{-3n} - a^{3n+3})(b^{-3m} - b^{3m+1})
  + (a^{1-3m} - a^{3m+2})(b^{3n+2} - b^{-3n-1})
] · q^{3n(n+1)/2 + m(3m+1)/2}
```

Sum over `n ≥ 0` and all `m` (positive and negative) such that the q-exponent is `< T`. The a, b, q arguments are all Series objects.

---

## Layer 5: Product Conversion Algorithms

### 5.1 prodmake — Andrews' Algorithm

This is the **most important algorithm in the package**. Given a q-series `f`, it finds exponents `a_n` such that:

```
f = (leading) · Π_{n=1}^{T-1} (1 - q^n)^{-a_n}
```

**Algorithm** (from Andrews, "q-series", section 10.7):

1. Extract coefficients `b[0..T-1]` from `f`. Normalize so `b[0] = 1`.

2. Compute `c[n]` by the recurrence:
   ```
   c[n] = n·b[n] - Σ_{j=1}^{n-1} b[n-j]·c[j]
   ```

3. Extract `a[n]` via Möbius-like inversion:
   ```
   a[n] = (c[n] - Σ_{d|n, d<n} d·a[d]) / n
   ```

4. Return the array `a[1..T-1]`.

**Output format**: Display as a fraction of products:
```
numerator:    Π (1 - q^n)^{e_n}   for n where -a[n] > 0
denominator:  Π (1 - q^n)^{e_n}   for n where -a[n] < 0
```

### 5.2 etamake — Eta Product Identification

Given a q-series `f`, express it as a product of Dedekind eta functions: `Π_k η(k·τ)^{e_k}`.

**Algorithm**:

1. Start with `g = f` (normalized to leading coefficient 1).
2. Find the smallest exponent `k > 0` where `g` has a nonzero coefficient `c`.
3. Multiply `g` by `etaq(q, k, T)^c`. This cancels the `q^k` term.
4. Record that we used `etaq(q, k, T)^c` — so the representation picks up `η(k·τ)^{-c}`.
5. Repeat until `g = 1` (or until convergence fails, indicating it's not an eta product).

**Output format**: Display like `η(2τ)⁵ / (η(τ)² · η(4τ)²)`.

### 5.3 jacprodmake — Jacobi Product Identification

Identify a q-series as a product of theta-type products.

**Notation**: `JAC(a, b, ∞)` denotes:
- If `0 < a < b`: `(q^a; q^b)_∞ · (q^{b-a}; q^b)_∞ · (q^b; q^b)_∞`
- If `a = 0`: `(q^b; q^b)_∞`

**Algorithm**:
1. Run `prodmake` to get exponents `a[n]`.
2. The product exponents `e[n] = -a[n]` give the power of `(1-q^n)`.
3. Search for a period `b`: check if `e[n+b] = e[n]` for all `n` in a suitable range.
4. Once period `b` is found, decompose the one-period pattern into `JAC(r, b, ∞)` factors for residues `r = 0, 1, ..., b/2`.

**Output functions**:
- `jacprodmake(f, T)` → list of `JAC(a, b, ∞)^e` factors
- `jac2prod(jac_expr)` → expand into `(q^a; q^b)_∞` notation
- `jac2series(jac_expr, T)` → compute the q-series from the JAC factors

### 5.4 qfactor — Finite q-Product Factorization

Like `prodmake` but for *polynomials* (rational functions in q). Tries to write a polynomial as `q^e · Π (1-q^i)^{b_i}`.

---

## Layer 6: Coefficient Sifting

### sift(f, n, k, T)

Extract the subsequence `a_{n·i+k}` from the series `f = Σ a_j q^j`:

```
sift(f, n, k, T) = Σ_{i≥0} a_{n·i+k} · q^i + O(q^{T/n})
```

**Implementation**: Simply read off `f.coeff(n*i + k)` for `i = 0, 1, 2, ...` and build a new Series.

**Example from the document**: Sifting Euler's product by residue mod 5 reveals the quintuple product identity.

---

## Layer 7: Relation Finding (Linear Algebra)

All relation-finding functions reduce to: "build a matrix of q-expansion coefficients, then compute its rational kernel."

### Rational Gaussian Elimination

```cpp
// Compute the kernel (null space) of matrix M over Q
// M is a vector of rows, each row is vector<Frac>
// Returns a basis for the kernel
std::vector<std::vector<Frac>> kernel(std::vector<std::vector<Frac>>& M);
```

Standard row-reduction over exact rationals. No pivoting heuristics needed (exact arithmetic doesn't accumulate error), but do choose a nonzero pivot in each column.

### 7.1 findhom(L, n, topshift)

Find **homogeneous** polynomial relations of degree `n` among the q-series in list `L`.

**Algorithm**:
1. Let `L = [f₁, f₂, ..., f_k]`.
2. Generate all monomials of total degree `n` in `k` variables: `f₁^{a₁}·f₂^{a₂}·...·f_k^{a_k}` where `a₁+...+a_k = n`.
3. For each monomial, compute the q-series product to some order `T`.
4. Build a matrix: each row is the coefficient vector of one monomial's q-expansion.
5. Compute the kernel of this matrix.
6. Each kernel vector gives a relation: `Σ c_i · monomial_i = 0`.

**Monomial generation**: Use combinations with replacement (stars-and-bars). For `k` variables and degree `n`, the number of monomials is `C(n+k-1, k-1)`.

**topshift**: If spurious relations appear, increase `topshift` to use more q-coefficients (more rows in the matrix → harder for spurious kernel vectors to survive).

### 7.2 findhomcombo(f, L, n, topshift, etaoption)

Like `findhom` but finds a specific homogeneous polynomial of degree `n` in the members of `L` that equals `f`.

Set up the same matrix but with `f`'s expansion as an additional column, then solve the system `f = Σ c_i · monomial_i`.

If `etaoption = true`, convert each monomial into eta-product form for display.

### 7.3 findnonhom(L, n, topshift)

Find **nonhomogeneous** polynomial relations of degree ≤ `n` among the series in `L`.

Same as `findhom` but include ALL monomials of degree 0 through `n` (not just degree exactly `n`). The degree-0 monomial is the constant `1`.

### 7.4 findnonhomcombo(f, L, n_list, topshift, etaoption)

Like `findnonhom` but writes `f` as a polynomial in `L`. The `n_list` parameter specifies the maximum degree for each variable independently (useful when the variables have different modular weights).

### 7.5 findpoly(x, y, deg1, deg2, check)

Find a polynomial relation `P(X, Y) = 0` between two q-series `x` and `y`, where `X` has degree ≤ `deg1` and `Y` has degree ≤ `deg2`.

Generate all monomials `X^i · Y^j` with `i ≤ deg1`, `j ≤ deg2`. Set up the linear system and find the kernel.

---

## Layer 8: Number Theory Helpers

```cpp
std::vector<int> divisors(int n);        // sorted positive divisors
int mobiusFn(int n);                     // Möbius function μ(n)
int legendreSymbol(int a, int p);        // Legendre symbol (a/p)
int64_t sigmaFn(int n, int k = 1);      // σ_k(n) = Σ_{d|n} d^k
int64_t euler_phi(int n);               // Euler's totient φ(n)
```

These are needed for:
- `prodmake` uses `divisors(n)` in the Möbius extraction step
- `legendreSymbol` is used to compute Eisenstein series `U_{p,k}`
- `sigmaFn` appears in the partition recurrence

---

## Layer 9: REPL and Parser

### Expression Language

The REPL should parse an expression language that closely mirrors the Maple syntax from the document:

```
# Variable assignment
x := etaq(q, 1, 50)

# Arithmetic
y := x^5 / etaq(q, 5, 50)

# Function calls
prodmake(y, 40)
etamake(y, 50)
jacprodmake(y, 40)

# Building the Rogers-Ramanujan series
rr := sum(q^(n^2) / aqprod(q, q, n), n, 0, 8)
series(rr, 50)
prodmake(rr, 40)

# Sifting
PD := etaq(q, 2, 200) / etaq(q, 1, 200)
sift(PD, 5, 1, 199)

# Relation finding
findhom([theta3(q,100), theta4(q,100), theta3(q^2,100), theta4(q^2,100)], 2, 0)

# Display
series(x, 20)
coeffs(x, 0, 10)
```

### Parser Design

Recursive-descent parser with these precedences (low to high):
1. `:=` (assignment)
2. `+`, `-` (additive)
3. `*`, `/` (multiplicative)
4. unary `-`
5. `^` (exponentiation, right-associative)
6. function calls: `name(args...)`
7. atoms: integers, `q` (the formal variable), variable names, `(expr)`

### Built-in Functions (REPL commands)

| Function | Signature | Description |
|----------|-----------|-------------|
| `aqprod` | `aqprod(a, q_expr, n, T)` | Rising q-factorial `(a;q)_n` |
| `qbin` | `qbin(m, n, T)` | Gaussian polynomial `[n;m]_q` |
| `etaq` | `etaq(k, T)` | Eta product `Π(1-q^{kn})` |
| `theta2` | `theta2(T)` | `θ₂(q)/q^{1/4}` |
| `theta3` | `theta3(T)` | `θ₃(q)` |
| `theta4` | `theta4(T)` | `θ₄(q)` |
| `theta` | `theta(z_expr, T)` | General `Σ z^n q^{n²}` |
| `tripleprod` | `tripleprod(z_expr, q_expr, T)` | Triple product |
| `quinprod` | `quinprod(z_expr, q_expr, T)` | Quintuple product |
| `winquist` | `winquist(a_expr, b_expr, q_expr, T)` | Winquist identity RHS |
| `prodmake` | `prodmake(f, T)` | → infinite product |
| `etamake` | `etamake(f, T)` | → eta product |
| `jacprodmake` | `jacprodmake(f, T)` | → Jacobi product |
| `jac2prod` | (on jacprodmake result) | Expand JAC notation |
| `qfactor` | `qfactor(f, T?)` | Factor polynomial as q-product |
| `sift` | `sift(f, n, k, T)` | Extract `a_{ni+k}` |
| `findhom` | `findhom([...], n, topshift)` | Homogeneous relations |
| `findhomcombo` | `findhomcombo(f, [...], n, topshift)` | Express f as hom. poly. |
| `findnonhom` | `findnonhom([...], n, topshift)` | Nonhomogeneous relations |
| `findnonhomcombo` | `findnonhomcombo(f, [...], n_list, topshift)` | Express f as poly. |
| `findpoly` | `findpoly(x, y, deg1, deg2, check?)` | Polynomial relation |
| `series` | `series(f, T)` | Display truncated to O(q^T) |
| `coeffs` | `coeffs(f, from, to)` | List coefficients |
| `sum` | `sum(expr, var, lo, hi)` | Summation |
| `add` | alias for `sum` | |
| `legendre` | `legendre(a, p)` | Legendre symbol |
| `sigma` | `sigma(n)` or `sigma(n,k)` | Divisor sum σ_k(n) |
| `help` | `help` or `help(func)` | Help system |

### Variable Environment

- Variables are stored in a `std::map<std::string, Series>`.
- `q` is always pre-defined as `Series::q(T)` where `T` is the current default truncation.
- Default truncation starts at 50, adjustable via `set_trunc(N)`.

### REPL Features

- **Line editing**: Implement basic readline-style input (using POSIX `termios` for arrow keys, history). Or just use `std::getline` with a simple history buffer — don't over-engineer this.
- **History**: Store last 100 commands, accessible with up/down arrows.
- **Multi-line**: If a line ends with `\`, continue on the next line.
- **Timing**: Print computation time for each command (helps user choose truncation levels).
- **Startup banner**: Show version, brief help hint.

---

## Acceptance Tests

These tests come directly from the Garvan document. **The REPL must produce these results.**

### Test 1: Rogers-Ramanujan Identity (Doc §3.1, Output 2)

```
qseries> x := sum(q^(n^2) / aqprod(q, q, n, 50), n, 0, 8)
qseries> prodmake(x, 40)
```

Expected: the product `1 / ((1-q)(1-q^4)(1-q^6)(1-q^9)(1-q^11)...(1-q^39))` — denominators at positions ≡ ±1 mod 5.

### Test 2: Partition Function (Doc §2.2.1)

```
qseries> p := 1 / etaq(1, 50)
qseries> series(p, 20)
```

Expected: `1 + q + 2*q^2 + 3*q^3 + 5*q^4 + 7*q^5 + 11*q^6 + 15*q^7 + 22*q^8 + 30*q^9 + 42*q^10 + ...`

### Test 3: Theta as Eta Products (Doc §3.3, Outputs 8-12)

```
qseries> etamake(theta3(q, 100), 100)
```

Expected: `η(2τ)⁵ / (η(4τ)² · η(τ)²)`

```
qseries> etamake(theta4(q, 100), 100)
```

Expected: `η(τ)² / η(2τ)`

### Test 4: Jacobi Product Identification (Doc §3.4, Output 13)

```
qseries> x := sum(q^(n^2) / aqprod(q, q, n, 50), n, 0, 8)
qseries> jacprodmake(x, 40)
```

Expected: `JAC(0,5,∞) / JAC(1,5,∞)` — which expands to `1 / ((q;q⁵)_∞ · (q⁴;q⁵)_∞)`.

### Test 5: Gauss AGM (Doc §4.1, Output 18)

```
qseries> findhom([theta3(q,100), theta4(q,100), theta3(q^2,100), theta4(q^2,100)], 2, 0)
```

Expected: Two relations: `{X₁² + X₂² - 2·X₃², -X₁·X₂ + X₄²}`

### Test 6: Sifting + Eta Identification (Doc §5, Output 28)

```
qseries> PD := etaq(2, 200) / etaq(1, 200)
qseries> PD1 := sift(PD, 5, 1, 199)
qseries> etamake(PD1, 38)
```

Expected: Rødseth's identity — `η(5τ)³ · η(2τ)² / (η(10τ) · η(τ)⁴)`

### Test 7: Qfactor (Doc §3.2, Output 5)

```
qseries> T := proc(r, j) ... end   # (or built-in helper)
qseries> t8 := T(8, 8)
qseries> qfactor(t8, 20)
```

Expected: `q⁶ · (1-q⁹)(1-q¹⁰)(1-q¹¹)(1-q¹⁶) / ((1-q)(1-q²)(1-q³)(1-q⁴))`

### Test 8: Watson's Modular Equation (Doc §4.4, Output 24)

```
qseries> xi := q^2 * etaq(49, 100) / etaq(1, 100)
qseries> T := q * (etaq(7, 100) / etaq(1, 100))^4
qseries> findnonhomcombo(T^2, [T, xi], [1, 7], 0)
```

Expected: `T² = (49ξ³ + 35ξ² + 7ξ)T + 343ξ⁷ + 343ξ⁶ + 147ξ⁵ + 49ξ⁴ + 21ξ³ + 7ξ² + ξ`

### Test 9: Euler Pentagonal Dissection (Doc §6.2, Outputs 34-36)

```
qseries> EULER := etaq(1, 500)
qseries> E0 := sift(EULER, 5, 0, 499)
qseries> jacprodmake(E0, 50)
```

Expected: `JAC(2,5,∞) · JAC(0,5,∞) / JAC(1,5,∞)`

---

## Performance Targets

| Operation | Truncation | Target Time |
|-----------|-----------|-------------|
| `etaq(1, 200)` | 200 | < 0.5s |
| `prodmake(f, 40)` | 40 | < 0.1s |
| `prodmake(f, 100)` | 100 | < 2s |
| `findhom` (4 series, degree 2) | 100 | < 5s |
| `theta3(q, 500)` | 500 | < 0.01s |
| Series multiplication | T=200 | < 1s |

For a benchmark reference: the Maple package handles T=500 comfortably on a modern machine. Our C++ version should be at least as fast.

---

## Build Instructions

```makefile
CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra
LDFLAGS = -static

qseries: main.cpp
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o qseries main.cpp

clean:
	rm -f qseries
```

The final binary should be < 2MB.

To produce the Windows build, cross-compile with `x86_64-w64-mingw32-g++ -static`.

---

## Mathematical Reference Summary

### Notation

| Symbol | Meaning |
|--------|---------|
| `(a;q)_n` | Rising q-factorial: `Π_{k=0}^{n-1} (1 - a·q^k)` |
| `(a;q)_∞` | Infinite rising q-factorial |
| `[n;m]_q` | Gaussian polynomial: `(q;q)_n / ((q;q)_m · (q;q)_{n-m})` |
| `η(τ)` | Dedekind eta: `q^{1/24} · Π(1-q^n)` |
| `θ₂(q)` | `Σ q^{(n+1/2)²}` |
| `θ₃(q)` | `Σ q^{n²}` |
| `θ₄(q)` | `Σ (-1)^n q^{n²}` |
| `JAC(a,b,∞)` | `(q^a;q^b)_∞ · (q^{b-a};q^b)_∞ · (q^b;q^b)_∞` |
| `O(q^T)` | Truncation: terms of degree ≥ T are discarded |

### Key Algorithm: Andrews' Series → Product Conversion

Given `f = 1 + Σ_{n≥1} b_n q^n`, find `a_n` such that `f = Π (1-q^n)^{-a_n}`.

**Step 1 — Logarithmic derivative recurrence:**
```
c[n] = n·b[n] - Σ_{j=1}^{n-1} b[n-j] · c[j]
```

**Step 2 — Divisor extraction:**
```
a[n] = (1/n) · (c[n] - Σ_{d|n, d<n} d · a[d])
```

This works because if `f = Π(1-q^n)^{-a_n}`, then `q·f'/f = Σ_n a_n · n·q^n/(1-q^n) = Σ_n (Σ_{d|n} d·a_d) q^n`.

---

## Common Pitfalls and Warnings

1. **Series inverse is the #1 source of bugs**: The recurrence `g[n] = -(1/c₀) Σ c_j g[n-j]` must be computed for `j` from 1 to `n`, not 0 to `n`. And the shift/unshift for non-zero leading exponent must correctly adjust truncation bounds.

2. **Truncation propagation**: When multiplying two series with different truncations, the result's truncation is `min(T₁, T₂)`. Failing to propagate this correctly will produce garbage.

3. **BigInt division**: The long-division algorithm is the hardest part of BigInt to get right. Test thoroughly: `1000000000 / 1`, `999999999 / 1000000000`, `123456789012345 / 123`, negative dividends, etc.

4. **Frac reduction**: If you forget to reduce fractions, the BigInt numerators and denominators will grow exponentially. Reduce after EVERY arithmetic operation.

5. **etamake convergence**: If the input isn't an eta product, the algorithm will loop forever. Add a maximum iteration count and report failure.

6. **jacprodmake periodicity detection**: Don't require exact periodicity — check that `e[n+b] == e[n]` holds for at least 80% of the test range, then verify the identified pattern reconstructs the original series.

7. **findhom matrix size**: For `k` series and degree `n`, the number of monomials is `C(n+k-1, k-1)`. With k=4, n=3 that's 20 monomials. With k=6, n=4 it's 126. The matrix can get large — make sure the kernel computation handles this.

8. **Memory in series multiplication**: Two dense series of order T each produce up to T² intermediate products. For T=500 this is 250K multiplications of Frac — each involving BigInt multiplication and GCD. This is where most time is spent.

---

## Stretch Goals (Not Required for V1)

- **Memoization**: Cache `etaq(k, T)` results (same `k` and `T` often reused).
- **Euler product optimization**: Use Euler's pentagonal number theorem to compute `etaq(1, T)` in O(T) instead of O(T · √T).
- **LaTeX output**: Optional `\LaTeX` formatting for products and relations.
- **Script mode**: Read commands from a file (`qseries < script.qs`).
- **Tab completion**: In the REPL, complete function names and variable names.
