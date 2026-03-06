# Phase 41: Robustness & Edge Cases - Context

**Gathered:** 2026-02-27
**Status:** Ready for planning

<domain>
## Phase Boundary

Fix all critical crash/hang bugs (6 issues) and key medium-severity usability issues (3 issues) identified in the robustness audit. No new mathematical features — just hardening existing code.

</domain>

<decisions>
## Implementation Decisions

### Error message style
- Pattern: `"funcname: message"` (e.g., `"etaq: k must be positive, got 0"`)
- Include the invalid value in the message so the user can see what went wrong
- Use `std::runtime_error` consistently (caught by REPL try/catch, printed as `Error: ...`)
- Keep messages terse but actionable — no multi-line explanations

### Validation placement
- Validate at the function entry point (e.g., top of `etaq()`, top of `sift()`)
- Don't duplicate validation in the REPL dispatch layer — let the math functions guard themselves
- Exception: integer division by zero must be caught in `evalToInt` since it's a language-level operation

### Parser recursion depth
- Limit to 256 levels of nesting — generous enough for any real expression, catches abuse
- Track depth with a simple integer counter passed through parse calls
- Error message: `"parser: expression too deeply nested (limit 256)"`

### Integer literal overflow
- Detect overflow during the digit-accumulation loop in `parsePrimary`
- Check: if `v > (INT64_MAX - digit) / 10` before `v = v * 10 + digit`, it will overflow
- Error message: `"parser: integer literal too large for int64"`
- BigInt construction from string already handles arbitrary sizes, so this only affects the int64 path

### Series::pow(INT_MIN)
- Clamp: if `n == INT_MIN`, treat as `n = -2147483647` (off by one, mathematically negligible)
- Alternatively, just guard with `if (n < -10000) throw "pow: exponent too large"` — nobody needs pow(-2 billion)
- Go with the throw approach: `"pow: exponent magnitude too large (limit 10000)"`

### Variant access errors
- Replace bare `std::get<Series>(...)` calls in `evalExpr` with a helper that catches `bad_variant_access` and throws a descriptive `runtime_error`
- Pattern: `"expected series, got integer"` or `"expected series, got Jacobi product"`
- Helper function: `Series getSeriesOrThrow(const EvalResult& r, const std::string& context)`

### Unknown built-in suggestions
- On unknown built-in, compute edit distance to all known builtins
- Show up to 2 suggestions with edit distance <= 3
- Message: `"unknown built-in 'etamke'. Did you mean: etamake?"` or `"unknown built-in 'foo'. No close matches found."`
- Use simple Levenshtein distance — no external dependencies needed

### Integer assignment
- Expand assignment to accept `int64_t` results: wrap in `Series::constant(Frac(value))` and store
- This lets `x := mobius(6)` work — stores as a constant series
- `DisplayOnly` results still cannot be assigned (they've already printed)

### Claude's Discretion
- Exact Levenshtein implementation details
- Whether to add a `getTypeName(EvalResult)` helper or inline the type checks
- Order of validation checks within each function

</decisions>

<specifics>
## Specific Ideas

- Follow existing error style in the codebase (e.g., `runtimeErr(name, msg)` pattern in repl.h dispatch)
- The 6 critical fixes are the priority — medium issues are nice-to-haves but should be included

</specifics>

<deferred>
## Deferred Ideas

- Resource limits (timeout, memory cap) for runaway computations — future phase
- `subs_q` truncation explosion guard (T * k > some limit) — future phase
- `sigma(n, k)` int64_t overflow for large k — low priority, defer

</deferred>

---

*Phase: 41-robustness-edge-cases*
*Context gathered: 2026-02-27*
