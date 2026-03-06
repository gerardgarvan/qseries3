# Phase 41: Robustness & Edge Cases - Research

**Researched:** 2026-02-27
**Domain:** C++20 defensive programming, input validation, error handling
**Confidence:** HIGH

## Summary

This phase hardens 9 specific bugs/gaps across 5 source files. All issues are well-understood from direct codebase inspection — no external libraries or novel patterns needed. The work is purely internal: adding guards, improving error messages, and preventing undefined behavior in existing code paths.

The 6 critical issues are: (1) `etaq(0,T)` infinite loop, (2) `sift(f,0,k,T)` infinite loop, (3) integer division by zero in `evalToInt`, (4) parser stack overflow on deep nesting, (5) integer literal overflow in `parsePrimary`, (6) `pow(INT_MIN)` undefined behavior. The 3 medium issues are: (7) bare `std::get<Series>` leaking `bad_variant_access`, (8) unknown built-in gives no suggestions, (9) integer-valued builtins can't be assigned to variables.

**Primary recommendation:** Fix each issue at the exact location identified below, using `std::runtime_error` consistently, with the error message format `"funcname: message"` including the offending value.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- Error message style: `"funcname: message"` pattern, include invalid value, use `std::runtime_error`, terse but actionable
- Validation placement: validate at function entry point, not in REPL dispatch layer; exception: integer division by zero in `evalToInt`
- Parser recursion depth: limit 256, simple integer counter, message `"parser: expression too deeply nested (limit 256)"`
- Integer literal overflow: detect in digit-accumulation loop in `parsePrimary`, check `v > (INT64_MAX - digit) / 10`, message `"parser: integer literal too large for int64"`
- Series::pow(INT_MIN): guard with `if (n < -10000) throw` with limit 10000
- Variant access errors: helper `getSeriesOrThrow(const EvalResult& r, const std::string& context)` that catches `bad_variant_access` and throws descriptive `runtime_error`
- Unknown built-in suggestions: Levenshtein distance, up to 2 suggestions with distance <= 3, specific message format
- Integer assignment: wrap `int64_t` in `Series::constant(Frac(value))` and store; `DisplayOnly` still cannot be assigned

### Claude's Discretion
- Exact Levenshtein implementation details
- Whether to add a `getTypeName(EvalResult)` helper or inline the type checks
- Order of validation checks within each function

### Deferred Ideas (OUT OF SCOPE)
- Resource limits (timeout, memory cap) for runaway computations — future phase
- `subs_q` truncation explosion guard (T * k > some limit) — future phase
- `sigma(n, k)` int64_t overflow for large k — low priority, defer
</user_constraints>

## Standard Stack

No new libraries. All fixes use existing C++20 standard library facilities:

| Facility | Purpose | Why |
|----------|---------|-----|
| `std::runtime_error` | All error throwing | Already used throughout; caught by REPL try/catch |
| `<climits>` / `INT64_MAX` | Overflow detection | Standard C++ constant for int64 bounds |
| `<cstdint>` | `int64_t` types | Already included |
| `<algorithm>` / `std::min` | Levenshtein distance | Already included |

## Architecture Patterns

### Pattern 1: Guard-at-Entry Validation

**What:** Add validation checks at the top of math functions, before any computation begins.
**When to use:** For parameter validation in `etaq()`, `sift()`, `pow()`.

```cpp
inline Series etaq(const Series& q, int k, int T) {
    if (k <= 0)
        throw std::runtime_error("etaq: k must be positive, got " + std::to_string(k));
    // ... existing code unchanged ...
}
```

**Key locations identified:**
- `src/qfuncs.h` line 177: `etaq()` — add `k <= 0` guard before cache check
- `src/convert.h` line 13: `sift()` — add `n <= 0` guard before the while loop
- `src/series.h` line 209: `pow()` — add `abs(n) > 10000` guard before `n < 0` branch

### Pattern 2: Safe Variant Access Helper

**What:** Replace bare `std::get<Series>(eval(...))` with a helper that produces descriptive errors.
**When to use:** Every place in `evalExpr()` that expects `eval()` to return a `Series`.

```cpp
inline std::string getTypeName(const EvalResult& r) {
    if (std::holds_alternative<Series>(r)) return "series";
    if (std::holds_alternative<int64_t>(r)) return "integer";
    if (std::holds_alternative<std::vector<JacFactor>>(r)) return "Jacobi product";
    if (std::holds_alternative<DisplayOnly>(r)) return "display-only result";
    if (std::holds_alternative<std::map<int, Frac>>(r)) return "product";
    // ... etc
    return "unknown";
}

inline Series getSeriesOrThrow(const EvalResult& r, const std::string& context) {
    if (std::holds_alternative<Series>(r))
        return std::get<Series>(r);
    throw std::runtime_error(context + ": expected series, got " + getTypeName(r));
}
```

**Recommendation:** Add a `getTypeName` helper — it's cleaner than inlining type checks at each site and reusable for future error messages.

**Exact call sites needing replacement in `evalExpr()` (repl.h):**
1. Line 738: `Series l = std::get<Series>(eval(e->left.get(), ...))` — BinOp left operand
2. Line 739: `Series r = std::get<Series>(eval(e->right.get(), ...))` — BinOp right operand
3. Line 753: `Series s = std::get<Series>(eval(e->operand.get(), ...))` — UnaryOp operand
4. Line 767: `Series term = std::get<Series>(eval(e->body.get(), ...))` — Sum body

### Pattern 3: Depth-Limited Recursive Descent

**What:** Thread a depth counter through parser recursive calls, throw at limit.
**When to use:** Parser's `parsePrimary` → `parseExpr` → `parsePrimary` recursion.

The depth counter must be incremented:
- On entry to `parsePrimary()` when consuming `LPAREN` (parenthesized sub-expression)
- On entry to `parseExpr()` (optional, but LPAREN is the key recursion point)

**Implementation approach:** Add `int depth = 0` as a member of `Parser`, increment when entering `parsePrimary` (or when recursing through LPAREN), check against limit 256.

```cpp
class Parser {
    // ... existing members ...
    int depth = 0;
    static constexpr int MAX_DEPTH = 256;

    ExprPtr parsePrimary() {
        if (++depth > MAX_DEPTH)
            throw std::runtime_error("parser: expression too deeply nested (limit 256)");
        // ... existing code ...
        // decrement depth on function exit (or use RAII guard)
    }
};
```

**Design choice:** Increment/decrement via RAII struct to ensure depth is always decremented even on exceptions. Alternatively, increment at the LPAREN consumption point only (simpler, since that's the only place recursion increases depth).

**Recommended:** Increment at the LPAREN consumption in `parsePrimary` only. This is the only point where recursion depth increases meaningfully (nested parentheses). Other recursion points (unary minus, binary operators) don't increase depth dangerously because they're bounded by input length.

```cpp
if (peek().kind == Token::Kind::LPAREN) {
    if (++depth > MAX_DEPTH)
        throw std::runtime_error("parser: expression too deeply nested (limit 256)");
    consume();
    ExprPtr e = parseExpr(0);
    expect(Token::Kind::RPAREN);
    --depth;
    return e;
}
```

### Pattern 4: Levenshtein Edit Distance for Suggestions

**What:** When an unknown built-in is encountered, compute edit distance to all known builtins and suggest close matches.
**When to use:** At the `throw` site at the end of `dispatchBuiltin()` (repl.h line 709).

```cpp
inline int levenshteinDistance(const std::string& a, const std::string& b) {
    size_t m = a.size(), n = b.size();
    std::vector<size_t> prev(n + 1), curr(n + 1);
    for (size_t j = 0; j <= n; ++j) prev[j] = j;
    for (size_t i = 1; i <= m; ++i) {
        curr[0] = i;
        for (size_t j = 1; j <= n; ++j) {
            size_t cost = (a[i-1] == b[j-1]) ? 0 : 1;
            curr[j] = std::min({prev[j] + 1, curr[j-1] + 1, prev[j-1] + cost});
        }
        std::swap(prev, curr);
    }
    return static_cast<int>(prev[n]);
}
```

**Space optimization:** Use two-row DP (O(n) space) instead of full matrix — there are ~45 builtins, so efficiency isn't critical, but two-row is standard best practice.

### Anti-Patterns to Avoid

- **Duplicating validation in REPL dispatch AND math function:** Validate once, at the math function entry. The REPL dispatch just calls through.
- **Catching `bad_variant_access` with try/catch:** Don't wrap `std::get` in try/catch; instead check with `std::holds_alternative` first. The helper function pattern is cleaner.
- **Using `assert()` for input validation:** Asserts are compiled out in release builds. Use `throw` for user-facing validation.

## Exact Fix Locations

### Fix 1: etaq(0, T) infinite loop — `src/qfuncs.h:177`

**Bug:** `for (int n = 1; k * n < T; ++n)` — when k=0, condition is `0 < T`, always true.
**Fix:** Add at line 177 (after function signature, before cache check):
```cpp
if (k <= 0)
    throw std::runtime_error("etaq: k must be positive, got " + std::to_string(k));
```

### Fix 2: sift(f, 0, k, T) infinite loop — `src/convert.h:13`

**Bug:** `while (n * i + k < T)` — when n=0, condition is `k < T`, always true for valid k.
**Fix:** Add at line 14 (after function signature):
```cpp
if (n <= 0)
    throw std::runtime_error("sift: n must be positive, got " + std::to_string(n));
```

### Fix 3: Integer division by zero — `src/repl.h:226`

**Bug:** `case BinOp::Div: return l / r;` — no zero check.
**Fix:** Add before the division:
```cpp
case BinOp::Div:
    if (r == 0) throw std::runtime_error("division by zero");
    return l / r;
```

### Fix 4: Parser recursion depth — `src/parser.h`

**Bug:** No limit on nesting depth. 1000+ nested `(` causes stack overflow.
**Fix:** Add `int depth = 0` member to `Parser`, check in `parsePrimary` at LPAREN.

### Fix 5: Integer literal overflow — `src/parser.h:301`

**Bug:** `for (char c : t.text) v = v * 10 + (c - '0');` — overflows silently for large numbers.
**Fix:** Replace with overflow-checked accumulation:
```cpp
int64_t v = 0;
for (char ch : t.text) {
    int digit = ch - '0';
    if (v > (INT64_MAX - digit) / 10)
        throw std::runtime_error("parser: integer literal too large for int64");
    v = v * 10 + digit;
}
```

### Fix 6: pow(INT_MIN) undefined behavior — `src/series.h:209`

**Bug:** `if (n < 0) return inverse().pow(-n);` — when n=INT_MIN, `-n` overflows (UB for signed int).
**Fix:** Add guard before the `n < 0` branch:
```cpp
Series pow(int n) const {
    if (n == 0) return one(trunc);
    if (n > 10000 || n < -10000)
        throw std::runtime_error("pow: exponent magnitude too large (limit 10000)");
    if (n < 0) return inverse().pow(-n);
    // ... rest unchanged
}
```

### Fix 7: Variant access errors — `src/repl.h` evalExpr

**Bug:** `std::get<Series>(eval(...))` at lines 738, 739, 753, 767 throws `bad_variant_access` if eval returns non-Series.
**Fix:** Add `getSeriesOrThrow` helper and replace all bare `std::get<Series>` calls.

### Fix 8: Unknown built-in suggestions — `src/repl.h:709`

**Bug:** Generic "unknown built-in" error with no help.
**Fix:** Compute Levenshtein distance to all `getHelpTable()` keys, suggest up to 2 with distance ≤ 3.

### Fix 9: Integer assignment — `src/repl.h` evalStmt (~line 1107)

**Bug:** `evalStmt` only handles `Series` and `std::vector<JacFactor>` in assignment; `int64_t` from builtins like `mobius(6)` throws.
**Fix:** Add `int64_t` handler:
```cpp
if (std::holds_alternative<int64_t>(res)) {
    int64_t v = std::get<int64_t>(res);
    env.env[s->assignName] = Series::constant(Frac(v), env.T);
    return res;
}
```

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Levenshtein distance | Complex DP with fancy optimizations | Simple two-row DP | ~45 builtins, performance is irrelevant; simple code is correct code |
| Depth guard RAII | Custom RAII struct for depth++ / depth-- | Manual increment/decrement at LPAREN only | Only one recursion point matters; RAII is overkill |

## Common Pitfalls

### Pitfall 1: pow(-n) when n = INT_MIN

**What goes wrong:** `-INT_MIN` is undefined behavior for `int` (and `int32_t`). On two's complement, INT_MIN = -2147483648 and -INT_MIN overflows to... still -2147483648 (or UB).
**Why it happens:** The `pow(int n)` function does `inverse().pow(-n)` when n < 0.
**How to avoid:** Guard BEFORE the `n < 0` branch with `abs(n) > 10000` check. Since the guard throws for any n < -10000, the `-n` path is safe because -n will be at most 10000.
**Warning signs:** Sanitizer reports, infinite loops in pow.

### Pitfall 2: Integer overflow detection order

**What goes wrong:** The check `v > (INT64_MAX - digit) / 10` must happen BEFORE `v = v * 10 + digit`, not after.
**Why it happens:** If you check after, the overflow already happened (UB for signed).
**How to avoid:** Check first, then accumulate. The formula `(INT64_MAX - digit) / 10` works because both sides are positive and the division is integer division.
**Warning signs:** Large number inputs producing wrong results silently.

### Pitfall 3: Depth counter not decremented on exception

**What goes wrong:** If `parseExpr(0)` throws (e.g., syntax error inside parens), `--depth` is never executed.
**Why it happens:** Exception unwinds past the decrement.
**How to avoid:** Either use RAII, or don't worry about it — once an exception is thrown, parsing is over and the Parser object is destroyed. The depth counter doesn't need to survive exceptions.
**Warning signs:** None — this is actually a non-issue for a parser that terminates on first error.

### Pitfall 4: getSeriesOrThrow vs evalAsSeries overlap

**What goes wrong:** `evalAsSeries` already exists (repl.h line 253) and does the same thing — checks `holds_alternative<Series>` and throws.
**Why it happens:** The new helper `getSeriesOrThrow` might seem redundant.
**How to avoid:** Use `evalAsSeries` where calling `eval()` + extracting Series in one step. Use `getSeriesOrThrow` at the bare `std::get<Series>(eval(...))` sites in `evalExpr` where `eval()` is already called and you need to extract the Series from the result. Or better: refactor those sites to call `evalAsSeries` directly (which already does `eval` + extract).
**Recommendation:** The sites at lines 738-739, 753, 767 can be refactored to use `evalAsSeries` with improved error messages. The `getSeriesOrThrow` helper is still needed for the `EvalResult` → `Series` extraction pattern with context.

### Pitfall 5: Levenshtein with empty strings

**What goes wrong:** Edge case where the input name is empty or a builtin name is empty.
**Why it happens:** The DP table handles this correctly (distance = length of the other string), but it could suggest everything if input is very short.
**How to avoid:** The distance ≤ 3 threshold naturally handles this — a 1-char input will only match builtins of length ≤ 4. Not a real problem.

### Pitfall 6: __builtin_unreachable after switch

**What goes wrong:** Lines 234 and 750 use `__builtin_unreachable()` after switch statements. This is GCC/Clang specific and fine for this project (compiled with g++), but worth noting it's not portable.
**Why it happens:** Used to suppress "control reaches end of non-void function" warnings.
**How to avoid:** Not an issue for this phase; just don't add more of them.

## Code Examples

### Levenshtein Distance (Two-Row DP)

```cpp
inline int levenshteinDistance(const std::string& a, const std::string& b) {
    size_t m = a.size(), n = b.size();
    std::vector<size_t> prev(n + 1), curr(n + 1);
    for (size_t j = 0; j <= n; ++j) prev[j] = j;
    for (size_t i = 1; i <= m; ++i) {
        curr[0] = i;
        for (size_t j = 1; j <= n; ++j) {
            size_t cost = (a[i-1] == b[j-1]) ? 0 : 1;
            curr[j] = std::min({prev[j] + 1, curr[j-1] + 1, prev[j-1] + cost});
        }
        std::swap(prev, curr);
    }
    return static_cast<int>(prev[n]);
}
```

### Unknown Built-in Suggestion Pattern

```cpp
// At end of dispatchBuiltin, replacing the current throw:
{
    const auto& table = getHelpTable();
    std::vector<std::pair<int, std::string>> suggestions;
    for (const auto& [key, _] : table) {
        int d = levenshteinDistance(name, key);
        if (d <= 3)
            suggestions.push_back({d, key});
    }
    std::sort(suggestions.begin(), suggestions.end());
    std::string msg = "unknown built-in '" + name + "'.";
    if (!suggestions.empty()) {
        msg += " Did you mean:";
        for (size_t i = 0; i < std::min(suggestions.size(), size_t(2)); ++i)
            msg += " " + suggestions[i].second;
        msg += "?";
    } else {
        msg += " No close matches found.";
    }
    throw std::runtime_error(msg);
}
```

### Integer Literal Overflow Check

```cpp
// In parsePrimary(), replacing the bare accumulation loop:
int64_t v = 0;
for (char ch : t.text) {
    int digit = ch - '0';
    if (v > (INT64_MAX - digit) / 10)
        throw std::runtime_error("parser: integer literal too large for int64");
    v = v * 10 + digit;
}
```

### Type Name Helper

```cpp
inline std::string getTypeName(const EvalResult& r) {
    if (std::holds_alternative<Series>(r)) return "series";
    if (std::holds_alternative<int64_t>(r)) return "integer";
    if (std::holds_alternative<std::vector<JacFactor>>(r)) return "Jacobi product";
    if (std::holds_alternative<std::map<int, Frac>>(r)) return "product map";
    if (std::holds_alternative<DisplayOnly>(r)) return "display-only result";
    if (std::holds_alternative<std::monostate>(r)) return "void";
    return "result";
}
```

## Open Questions

1. **Should `sift` also validate `k < 0`?**
   - What we know: `sift(f, n, -1, T)` with n > 0 produces `n*i + (-1) < T` — this works fine, it just reads negative-exponent coefficients. The original Maple function allows negative k.
   - What's unclear: Whether negative k is ever useful in this REPL.
   - Recommendation: Don't validate k; only validate n > 0. Negative k is harmless and potentially useful.

2. **Should the pow limit be 10000 or something else?**
   - What we know: User decided 10000. In practice, `pow(100)` is already slow for non-trivial series.
   - Recommendation: Use 10000 as decided. It's generous enough for any real use case.

3. **evalExpr BinOp::Pow path has duplicate code**
   - What we know: Line 746-747 calls `evalToInt` for the exponent, same as the q^n special case at line 735. Both paths need the result to be integer.
   - What's unclear: Whether the non-q pow path should also go through `getSeriesOrThrow` for the left operand (it does `std::get<Series>` at line 738).
   - Recommendation: Yes, replace the `std::get<Series>` at 738-739 with `getSeriesOrThrow`.

## Sources

### Primary (HIGH confidence)
- Direct codebase inspection of `src/qfuncs.h`, `src/convert.h`, `src/parser.h`, `src/repl.h`, `src/series.h`
- C++20 standard: `INT64_MAX` from `<climits>`, `std::holds_alternative` / `std::get` from `<variant>`

## Metadata

**Confidence breakdown:**
- Fix locations: HIGH — verified by reading every line of the relevant functions
- Error message format: HIGH — matches existing `runtimeErr` pattern already in codebase
- Levenshtein implementation: HIGH — well-known algorithm, trivial to implement
- Parser depth tracking: HIGH — standard recursive descent pattern

**Research date:** 2026-02-27
**Valid until:** Indefinite — this is defensive programming, not library-dependent
