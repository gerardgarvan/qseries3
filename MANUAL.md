	# Q-Series REPL — User Manual

A standalone REPL for q-series computation. Reimplements core functionality of Frank Garvan's qseries Maple package for number theory, partition theory, and modular forms. All arithmetic is exact (no floating point).

---

## 1. Running the Program

### Interactive mode

```bash
./qseries.exe
```
or `./qseries` on Linux/macOS.

The REPL displays a banner and prompt `qseries> `. Type expressions and press Enter.

### Script mode

```bash
qseries < script.qs
```

Runs commands from a file. No banner; output only. Use for batch work.

### Version

```bash
qseries --version
```

Prints the package version (e.g. `qseries 2.0`) and exits. In the REPL, type `version()` for the same.

### Unit tests

```bash
./qseries.exe --test
```

Runs internal tests and exits.

---

## 2. Basic Syntax

### Expressions

- **Numbers:** `42`, `-7`
- **Variable q:** `q` (the formal power series variable)
- **Operators:** `+`, `-`, `*`, `/`, `^` (power)
- **Unary minus:** `-q`, `-(1+q)`

### Assignment

```
var := expression
```

Example: `rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)`

### Lists

```
[expr1, expr2, expr3]
```

Used as arguments to `findhom`, `findhomcombo`, etc.

### Comments

```
# this is a comment
```

Anything after `#` to end of line is ignored.

---

## 3. Built-in Functions

### 3.1 Building q-Series

| Function | Syntax | Description |
|----------|--------|-------------|
| **aqprod** | `aqprod(a,q,n,T)` | Rising q-factorial (a;q)_n |
| **qbin** | `qbin(m,n,T)` or `qbin(q,m,n,T)` | Gaussian polynomial [m;n]_q |
| **etaq** | `etaq(k,T)` or `etaq(q,k,T)` | Eta product Π(1-q^{kn}) |
| **theta** | `theta(z,T)` or `theta(z,q,T)` | Generalized theta Σ z^n q^(n²) |
| **theta2** | `theta2(T)` or `theta2(q,T)` | θ₂(q) |
| **theta3** | `theta3(T)` or `theta3(q,T)` | θ₃(q) |
| **theta4** | `theta4(T)` or `theta4(q,T)` | θ₄(q) |
| **tripleprod** | `tripleprod(z,q,T)` | Jacobi triple product |
| **quinprod** | `quinprod(z,q,T)` | Quintuple product |
| **winquist** | `winquist(a,b,q,T)` | Winquist identity |
| **T** | `T(r,n)` or `T(r,n,T)` | Finite q-product T_{r,n} |

### 3.2 Product Conversion

| Function | Syntax | Description |
|----------|--------|-------------|
| **prodmake** | `prodmake(f,T)` | Andrews' algorithm: series → infinite product |
| **etamake** | `etamake(f,T)` | Identify f as eta product |
| **jacprodmake** | `jacprodmake(f,T)` | Identify f as Jacobi product |
| **qfactor** | `qfactor(f)` or `qfactor(f,T)` | Factorize finite q-product |

### 3.3 Coefficient Extraction and Sifting

| Function | Syntax | Description |
|----------|--------|-------------|
| **series** | `series(f)` or `series(f,T)` | Display series coefficients |
| **coeffs** | `coeffs(f,from,to)` | List coefficients from exponent *from* to *to* |
| **sift** | `sift(f,n,k,T)` | Extract coefficients a_{ni+k} |
| **qdegree** | `qdegree(f)` | Highest exponent with nonzero coefficient |
| **lqdegree** | `lqdegree(f)` | Lowest exponent with nonzero coefficient |

### 3.4 Relations

| Function | Syntax | Description |
|----------|--------|-------------|
| **findhom** | `findhom(L,n,topshift)` | Homogeneous polynomial relations between series in list L |
| **findnonhom** | `findnonhom(L,n,topshift)` | Nonhomogeneous polynomial relations |
| **findhomcombo** | `findhomcombo(f,L,n,topshift[,etaopt])` | Express f as polynomial in L |
| **findnonhomcombo** | `findnonhomcombo(f,L,n_list,topshift[,etaopt])` | Express f as polynomial in L (nonhom) |
| **findlincombo** | `findlincombo(f,L,topshift)` | Express f as linear combination of series in L |
| **findpoly** | `findpoly(x,y,deg1,deg2[,check])` | Polynomial relation between two series x, y |

### 3.5 Summation

| Function | Syntax | Description |
|----------|--------|-------------|
| **sum** | `sum(expr, var, lo, hi)` | Σ expr as var runs from lo to hi |
| **add** | `add(expr, var, lo, hi)` | Same as sum |

### 3.6 Other

| Function | Syntax | Description |
|----------|--------|-------------|
| **subs_q** | `subs_q(f,k)` | Substitute q^k for q |
| **jac2prod** | `jac2prod(var)` | Display Jacobi product stored in variable |
| **jac2series** | `jac2series(var)` or `jac2series(var,T)` | Convert Jacobi product (in var) to series |
| **version** | `version()` | Print package version |
| **set_trunc** | `set_trunc(N)` | Set default truncation |
| **legendre** | `legendre(a,p)` | Legendre symbol (a/p) |
| **sigma** | `sigma(n)` or `sigma(n,k)` | Divisor sum σ_k(n) |

---

## 4. Help

- **`help()`** — Brief usage and list of all built-ins
- **`help(func)`** — Signature and description for a function, e.g. `help(prodmake)`

---

## 5. Interactive Features

### Tab completion

In interactive mode (TTY), press **Tab** to autocomplete:

- **Unique match** — Completes the identifier
- **Multiple matches** — Lists options
- **No match** — No change

Completes both built-in names and user-defined variables.

### Multi-line input

End a line with backslash `\` to continue on the next line:

```
qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), \
  > n, 0, 8)
```

### Timing

When a command finishes, the REPL prints elapsed time (e.g. `0.123s`) on the next line.

### History

The last 100 commands are kept in history. (Use Up/Down for navigation if your terminal supports it.)

---

## 6. Default Truncation

The default truncation is 50. Use `set_trunc(N)` to change it. `q` and all series use this truncation.

---

## 7. Examples

### Rogers–Ramanujan identity

```
rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)
prodmake(rr, 40)
```

The product form has denominators only at exponents ≡ ±1 (mod 5).

### Partition function

```
p := 1/etaq(1,50)
series(p, 20)
```

### Theta as eta product

```
etamake(theta3(q,100), 100)
etamake(theta4(q,100), 100)
```

### qfactor T(8,8)

```
set_trunc(64)
t8 := T(8,8)
qfactor(t8, 20)
```

### Find relations (Gauss AGM)

```
findhom([theta3(q,100), theta4(q,100), theta3(subs_q(q,2),100), theta4(subs_q(q,2),100)], 2, 0)
```

---

## 8. Error Messages

- **Parse errors** — Show line and column, and what was expected
- **Runtime errors** — Include the function name, e.g. `etaq: expected etaq(k), etaq(k,T), or etaq(q,k,T), got 0 arguments`
- **Script mode** — Errors include the command line number in the script

---

## 9. Demo

A self-contained demo is available via `make package-demo`. This creates `qseries-demo/` with:

- The qseries binary
- `garvan-demo.sh` — runs examples from Frank Garvan's qseriesdoc tutorial
- README

Run:

```bash
cd qseries-demo
bash garvan-demo.sh
```

---

## 10. Reference

- **qseriesdoc.md** — Frank Garvan's qseries tutorial (Maple) with worked examples
- **SPEC.md** — Technical specification and acceptance tests

---

*Q-Series REPL — Maple-like, zero dependencies, exact rational arithmetic*
