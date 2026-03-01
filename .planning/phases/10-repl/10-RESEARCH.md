# Phase 10: REPL — Implementation Research

**Purpose:** Discover how to implement the full REPL: parse → eval → display with variable environment, built-in dispatch, and Garvan-style output. Downstream consumer: plan-phase (10-01-PLAN.md).

---

## Standard Stack → repl.h + main REPL loop; eval in same process

- **repl.h**: REPL loop, eval engine, display helpers, variable environment, built-in dispatch table
- **main.cpp**: Entry point; includes repl.h; calls `runRepl()` when built as interactive binary
- **In-process eval**: AST (from parser.h) is interpreted directly; no JIT, no separate process. All C++ APIs (qfuncs, convert, relations) are called synchronously.
- **Zero external deps**: No readline, no termios. Use `std::getline` for input; implement simple history buffer (100 entries) in pure C++.

---

## Architecture Patterns

### Data flow

```
std::getline → parse(input) → Stmt
                    ↓
              eval(stmt, env) → Result (Series | display-only | special)
                    ↓
              display(result) → cout
```

### Eval loop

1. **Read**: `std::getline(std::cin, line)`
2. **Skip empty**: If trimmed line is empty, continue
3. **Parse**: `auto stmt = parse(line)` — may throw
4. **Execute**: `eval(stmt, env)` — may throw (unknown var, bad args)
5. **Display**: Format result to cout
6. **Update env**: For Assign stmt, `env[name] = value`
7. **History**: Push non-empty line to history buffer (circular, max 100)

### Dispatch pattern

- Use a `std::map<std::string, BuiltInHandler>` or switch on call name
- Each handler: `(args, env, T) -> EvalResult`
- `EvalResult` = tagged union or variant:
  - `Series` (most built-ins)
  - `DisplayOnly` (series, coeffs — print and produce no stored value)
  - `ProdmakeResult` (map → format as product string)
  - `EtamakeResult` (vector<pair> → format as η notation)
  - `JacprodmakeResult` (vector<JacFactor> → jac2prod string)
  - `RelationsResult` (vector<vector<Frac>> → format as {X₁²+X₂²-2X₃², ...})
  - `FindhomcomboResult` (optional<vector<Frac>> → format as polynomial)

---

## Eval Design

### Expr → value

| Expr tag    | Eval behavior |
|-------------|---------------|
| IntLit      | Not a Series; used as int in args (e.g. etaq(1,50)) |
| Q           | `env["q"]` or `Series::q(T)` — q is always in env |
| Var         | `env[varName]` — throw if missing |
| BinOp       | Eval left, right → Series; apply +,-,*,/,\^ |
| UnOp        | -operand → -Series |
| Call        | Built-in dispatch; eval args first |
| List        | Eval each element → vector<Series> (for findhom(L,...)) |
| Sum         | Bind sumVar in local scope; for n=lo..hi: acc += eval(body) |

### Variable environment

- `std::map<std::string, Series> env`
- Pre-populate: `env["q"] = Series::q(T)` where T = default truncation (50)
- After `set_trunc(N)`: `T = N`; update `env["q"] = Series::q(T)`
- Assignment: `x := expr` → `env["x"] = eval(expr)`

### q vs T argument convention (CRITICAL)

| REPL call        | C++ API                  | Eval behavior |
|------------------|--------------------------|---------------|
| `etaq(1, 50)`    | `etaq(q, 1, 50)`         | 2 args: (k, T). Supply q from env |
| `etaq(q, 1, 50)` | `etaq(q, 1, 50)`         | 3 args: (q, k, T). Eval first arg as q |
| `theta3(100)`    | `theta3(q, 100)`         | 1 arg = T. Supply q from env |
| `theta3(q, 100)` | `theta3(q, 100)`         | 2 args = (q, T). Eval first as q |
| `aqprod(q, q, n, 50)` | `aqprod(a, q, n, T)` | Eval a, q from args; n from sum scope |
| `prodmake(f, 40)`| `prodmake(f, 40)`        | f = Series, T = 40 |

**Rule**: Functions that conceptually take `(q, T)` or `(q, k, T)` can have `q` omitted in REPL; eval supplies `env["q"]` when arg count is one less than C++.

### Sum scoping

- `sum(expr, var, lo, hi)`: var is bound in a **local scope** for expr
- Create `env_local = env` (copy or overlay)
- For n = eval(lo).toInt() .. eval(hi).toInt():
  - `env_local[var] = Series::constant(Frac(n), T)` — sum index as constant in expr? No.
- **Correct semantics**: The sum index `n` is an integer used **inside expr**. In `q^(n^2)`, `n` is the loop variable. So:
  - `env_local[var]` = we need to substitute `n` in the expression. The expression `q^(n^2)` has `n` as a Var. When we eval, we look up `n` → we need a "value" for n.
- **Approach**: The sum index is an **integer**, not a Series. Store `sumIndex[var] = n` in a separate map for the duration of expr eval. When we see `Var("n")` and `sumIndex.count("n")`, return `Series::constant(Frac(n), T)` so that `q^(n^2)` becomes `q^(n*n)` = `q^(n²)` as a Series (exponent = n²).
- Simpler: treat `Var("n")` in sum body as integer `n` from loop. Eval `q^(n^2)` where the `n` in the exponent is the integer. So we need to **evaluate expressions with integer variables** for exponent. The parser produces `q^(n^2)` as `BinOp(Pow, Q, BinOp(Pow, Var("n"), 2))`. The exponent `n^2` must reduce to an integer. So we need:
  - **Overlay**: `env_local` has `var → integer`. When eval sees Var(sumVar), return the integer as... but BinOp expects Series. So the exponent in `q^exp` is special: we need `exp` to be an integer.
- **Alternative**: Only allow integer-valued subexpressions for exponents. When we have `q^(n^2)`, the right side of Pow should eval to IntLit-like. Add an integer eval path: for pure integer expressions (Var with int binding, IntLit, BinOp on ints), return int. Then `Series::qpow(n*n, T)`.
- **Practical approach**: 
  - For `sum(body, var, lo, hi)`: lo, hi must eval to integers (or we get them from IntLit).
  - For each n in [lo, hi]: push `(var, n)` onto a "sum stack" or pass `sumIndices` map to eval.
  - When eval encounters `Var(var)` and var is in sumIndices, the value is the integer n. But Expr doesn't have IntLiteral in exponent — it has Var. So we need eval to produce different things: (a) Series for most uses, (b) int for exponent in q^exp.
  - **Simplest**: In sum body, when we see Var(sumVar), we treat it as the loop integer. For `q^(n^2)`: the exponent is `n^2`. We need to interpret that. Options: (1) special-case: if we have `BinOp(Pow, left, right)` and one side is Var(sumVar), compute integer exponent. (2) Add `evalInt(expr, sumIndices)` that only works for integer-valued exprs.
  - **Recommended**: `eval(expr, env, sumIndices)` where `sumIndices` is `map<string,int64_t>`. For Var(x) when x in sumIndices: return... we can't return int from eval if the rest expects Series. So we need **two evaluators** or a unified value type. 
  - **Unified approach**: `EvalResult` = variant of `Series | int64_t`. When we need Series and get int n, use `Series::constant(Frac(n), T)`. When we need int (e.g. exponent in q^e), and we have Series, take coeff(0) if it's constant, else error. Actually for `q^(n^2)`, the exponent must be integer. So in `eval` for BinOp(Pow, base, exp): if base is Q (or q-like), eval exp as integer (using sumIndices for Var), then return `Series::qpow(exponent, T)`.
  - **Implementation**: Add `evalInt(expr, env, sumIndices) -> int64_t` for expressions that must be integer (sum bounds, exponents, aqprod n, etc.). And `eval(expr, env, sumIndices) -> Series` for Series. When eval sees Var and it's in sumIndices, we don't have a Series — we have an int. So in `eval`, for Var(x): if sumIndices.count(x), we need to produce something usable. For `q^n` we need exponent. So in BinOp(Pow, left, right): if left is Q, then right must be integer-eval. Define `evalToInt(expr)` for that.
- **Conclusion**: Pass `std::map<std::string, int64_t> sumIndices` through eval. For Var(x) in sumIndices, we cannot return Series directly. The caller of eval (for binary ops) needs to know. The cleanest: have `eval` return a variant `std::variant<Series, std::monostate>` and for Var-in-sumIndices, the exponent case: we need to branch. Actually, the simplest is: **In sum body, replace Var(sumVar) conceptually with the integer**. The only use of the sum var is in integer contexts: exponent (n^2), aqprod(q,q,n,50). So when we eval aqprod(q,q,n,50), the fourth arg is Var("n"). We need that to be 4 (or whatever n is). So the third and fourth args of aqprod are integers. So we need `evalToInt(expr, sumIndices)` for those. And for `q^(n^2)`, the exponent `n^2` needs to be integer-eval'd. So: 
  - `eval(expr, env, sumIndices)` returns `Series`
  - For Var(x): if sumIndices.count(x), we have a problem — Var can't be Series. So we need the expr to be in a context. For `aqprod(q,q,n,50)`, the arg `n` is eval'd as int. So we need `evalArg(expr, env, sumIndices, expectInt)` — if expectInt and expr is Var in sumIndices, return the int as... we need to thread "expect integer" through. Simpler: **evalToInt(expr)** for expr that are IntLit, or Var (and Var in sumIndices), or BinOp on those. Call it for: sum lo, hi; any arg that must be int (n in aqprod, k in sift, etc.).
  - So: `int64_t evalToInt(Expr&, env, sumIndices)` and `Series eval(Expr&, env, sumIndices)`. For eval, when we have Var(x): if sumIndices.count(x), we cannot produce Series — the design is that in sum body, the var is used in integer position. So in practice, `q^(n^2)` is parsed as Pow(Q, Pow(Var(n), 2)). The exponent of outer Pow is Pow(Var(n),2). To eval that as Series we'd need Var(n) as Series. Instead, for Pow(base, exp): if base is Q, use evalToInt(exp) and return qpow. So we need recursive evalToInt for the exponent.
  - **Final**: `evalToInt(expr, env, sumIndices)` handles IntLit, Var (from sumIndices), BinOp(Add/Sub/Mul/Pow) on integers. `eval(expr, env, sumIndices)` returns Series; for BinOp(Pow, left, right), if left is Q, call `evalToInt(exp)` and return `Series::qpow(expVal, T)`.

### Built-in mapping (REPL name → C++)

| REPL          | Args (user)        | C++ call |
|---------------|--------------------|----------|
| aqprod        | (a, q, n, T)       | aqprod(eval(a), eval(q), evalToInt(n), evalToInt(T)) |
| qbin          | (m, n, T) or (q,m,n,T)| qbin(eval(q), evalToInt(m), evalToInt(n), T) — SPEC says qbin(m,n,T), qfuncs has qbin(q,m,n,T). Use 3-arg: q from env, qbin(env["q"], m, n, T). |
| etaq          | (k, T) or (q, k, T)| etaq(q, k, T) — q from env if 2 args |
| theta2/3/4    | (T) or (q, T)      | theta*(q, T) — q from env if 1 arg |
| theta         | (z, T) or (z, q, T)| theta(z, q, T) |
| tripleprod    | (z, q, T)          | tripleprod(eval(z), eval(q), T) |
| quinprod      | (z, q, T)          | quinprod(eval(z), eval(q), T) |
| winquist      | (a, b, q, T)       | winquist(eval(a), eval(b), eval(q), T) |
| prodmake      | (f, T)             | prodmake(eval(f), T) |
| etamake       | (f, T)             | etamake(eval(f), T) |
| jacprodmake   | (f, T)             | jacprodmake(eval(f), T) |
| jac2prod      | (var)              | jac2prod(lookup env[var]) — env must hold variant&lt;Series, vector&lt;JacFactor&gt;&gt; so jacprodmake result can be assigned and passed to jac2prod. |
| qfactor       | (f, T?)            | qfactor(eval(f), T) |
| sift          | (f, n, k, T)       | sift(eval(f), evalToInt(n), evalToInt(k), T) |
| findhom       | (L, n, topshift)   | findhom(eval(L) as vector<Series>, n, topshift) |
| findnonhom    | (L, n, topshift)   | findnonhom(...) |
| findhomcombo  | (f, L, n, topshift, etaopt?) | findhomcombo(eval(f), eval(L), n, topshift, etaopt) |
| findnonhomcombo | (f, L, n_list, topshift, etaopt?) | findnonhomcombo(f, L, n_list, topshift, etaopt) — n_list from list of ints |
| findpoly      | (x, y, deg1, deg2, check?) | findpoly(eval(x), eval(y), deg1, deg2, check) |
| series        | (f, T?)            | display only: f.str(T or default) |
| coeffs        | (f, from, to)      | display only: list coefficients |
| sum, add      | (body, var, lo, hi)| special: loop, accumulate Series |
| set_trunc     | (N)                | side effect: T = N; env["q"] = Series::q(T) |
| legendre      | (a, p)             | legendre(evalToInt(a), evalToInt(p)) |
| sigma         | (n) or (n, k)      | sigma(evalToInt(n)), sigma(evalToInt(n), evalToInt(k)) |

### Return types and display

| Built-in       | C++ return type               | REPL behavior |
|----------------|-------------------------------|---------------|
| aqprod, etaq, theta*, tripleprod, quinprod, winquist, qbin | Series | Eval returns Series; display Series::str() |
| prodmake       | map<int,Frac>                 | Format as product string; display |
| etamake        | vector<pair<int,Frac>>        | Format as η(2τ)⁵/(η(4τ)² η(τ)²); display |
| jacprodmake    | vector<JacFactor>             | Display jac2prod(jac); storable for jac2prod(var) |
| jac2prod       | string                        | Display string |
| qfactor        | QFactorResult                 | Format as q^e·num/den; display |
| sift           | Series                        | Display Series::str() |
| findhom, findnonhom, findpoly | vector<vector<Frac>> | Format each kernel vector as relation string; display |
| findhomcombo, findnonhomcombo | optional<vector<Frac>> | Format solution as polynomial; display |
| series         | (display only)                | f.str(maxTerms); no stored value |
| coeffs         | (display only)                | Print [c0, c1, ...]; no stored value |
| set_trunc      | (side effect)                 | Update T and env["q"]; no display |
| legendre, sigma | int                          | Print integer |

---

## Display Design

### Series

- Use `Series::str(maxTerms)` — already exists. Default maxTerms 30 or 40.
- `series(f, T)` can truncate f to T then str().

### prodmake

- Input: `std::map<int, Frac> a`
- Output format (qseriesdoc Output 2): `1 / ((1-q)(1-q^4)(1-q^6)...)` — denominator = product over n with a[n] > 0 of (1-q^n)^{a[n]}; numerator if any a[n] < 0.
- Helper: `formatProdmake(const std::map<int,Frac>& a) -> string`

### etamake

- Input: `vector<pair<int,Frac>>` for (k, e_k) meaning η(kτ)^{e_k}
- Output (qseriesdoc): `η(2τ)⁵ / (η(4τ)² η(τ)²)` — numerator: positive e; denominator: negative e.
- Helper: `formatEtamake(const vector<pair<int,Frac>>&) -> string`
- Use Unicode η, ², ⁵ etc. (Series::expToUnicode exists).

### jacprodmake

- Use existing `jac2prod(jac)` for string.
- Display that string.

### qfactor

- Input: QFactorResult {q_power, num_exponents, den_exponents}
- Output (qseriesdoc Output 5): `q⁶(1-q⁹)(1-q¹⁰)... / ((1-q)(1-q²)...)`

### Relations (findhom, findnonhom, findpoly)

- Each kernel vector = coefficients for monomials.
- Need monomial → string (e.g. X₁², X₂², X₁·X₂).
- Format: `{X₁²+X₂²-2X₃², -X₁·X₂+X₄²}` (SPEC Test 5).
- Helper: `formatRelation(const vector<Frac>& coeffs, const vector<vector<int>>& monomialExponents, const vector<string>& varNames) -> string`
- varNames default: X₁, X₂, ... (Unicode subscripts or X1, X2).

### findhomcombo / findnonhomcombo

- Solution vector c = coefficients for monomials.
- Format as polynomial: `f = c1*mon1 + c2*mon2 + ...`
- Watson (Test 8): `T² = (49ξ³+35ξ²+7ξ)T + 343ξ⁷+...`

---

## Common Pitfalls

1. **q vs T arg order**: C++ etaq(q,k,T), theta3(q,T). User types `etaq(1,50)` or `theta3(100)`. Must inject q from env when arg count is 1 less. Don't confuse (k,T) with (q,k,T).

2. **Sum index shadowing**: The sum variable (e.g. n) must shadow any outer `n`. Use explicit sumIndices overlay; don't mutate env.

3. **set_trunc scope**: set_trunc(N) is global. It changes T and env["q"] for all subsequent commands. No block scope.

4. **Integer vs Series in eval**: Sum bounds, aqprod n, sift n,k, etc. must be integers. Use evalToInt for those. Don't try to convert Series to int implicitly.

5. **List eval for findhom**: `findhom([theta3(q,100), theta4(q,100), ...], 2, 0)` — first arg is a list. Parser produces Call("findhom", [List([...]), Int(2), Int(0)]). Eval the list elements to vector<Series>.

6. **jac2prod on variable**: User may do `jp := jacprodmake(x, 40)` then `jac2prod(jp)`. Requires storing non-Series values. Use `EnvValue = variant<Series, vector<JacFactor>, ...>` for env.

7. **T(8,8) for qfactor**: SPEC Test 7 uses T as a recursive procedure. We have `T_rn(r, n, T_trunc)` in convert.h. REPL needs a way to call it. Options: (a) built-in `T(r, n)` that calls T_rn(r, n, current_T), (b) user defines it (we don't have user-defined procs in V1). So add built-in `T(r, n)` → T_rn(r, n, env_T).

8. **Empty lines and comments**: Skip empty lines; parser strips # comments. Don't push empty to history.

9. **Error recovery**: Catch parse_error and eval_error; print message; continue to next prompt. Never exit on user input error.

---

## 9 Tests Mapping

| Test | REPL commands | Built-ins exercised |
|------|---------------|---------------------|
| 1 Rogers-Ramanujan | `rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)` then `prodmake(rr, 40)` | sum, aqprod, prodmake |
| 2 Partition | `p := 1/etaq(1, 50)` then `series(p, 20)` | etaq, series |
| 3 Theta as eta | `etamake(theta3(q,100), 100)` and `etamake(theta4(q,100), 100)` | theta3, theta4, etamake |
| 4 Jacobi RR | `rr := sum(...)` then `jacprodmake(rr, 40)` | sum, aqprod, jacprodmake |
| 5 Gauss AGM | `findhom([theta3(q,100), theta4(q,100), theta3(q^2,100), theta4(q^2,100)], 2, 0)` | theta3, theta4, subs_q, findhom |
| 6 Sifting + Rødseth | `PD := etaq(2,200)/etaq(1,200)`, `PD1 := sift(PD,5,1,199)`, `etamake(PD1,38)` | etaq, sift, etamake |
| 7 Qfactor | `t8 := T(8,8)`, `qfactor(t8, 20)` | T (T_rn), qfactor |
| 8 Watson | `xi := q^2*etaq(49,100)/etaq(1,100)`, `T := q*(etaq(7,100)/etaq(1,100))^4`, `findnonhomcombo(T^2, [T, xi], [1, 7], 0)` | etaq, findnonhomcombo |
| 9 Euler pentagonal | `EULER := etaq(1,500)`, `E0 := sift(EULER,5,0,499)`, `jacprodmake(E0,50)` | etaq, sift, jacprodmake |

**Note**: Test 8 uses variable `T` for a series; this shadows the truncation name in the document. REPL truncation is a separate concept (set_trunc). The variable T in Test 8 is user-chosen. No conflict.

---

## RESEARCH COMPLETE
