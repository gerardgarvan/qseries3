# Phase 1: BigInt - Research

**Researched:** 2025-02-24  
**Domain:** Arbitrary-precision integer arithmetic (C++20, zero dependencies)  
**Confidence:** HIGH

## Summary

Phase 1 delivers a signed arbitrary-precision integer (BigInt) as the foundation for exact rational arithmetic. The project mandates zero external libraries — no GMP, no Boost. BigInt is implemented from scratch using base 10⁹ digits in `std::vector<uint32_t>`, least-significant first.

The critical algorithms are: (1) schoolbook multiplication — O(n·m), sufficient for q-series workloads (~100 digits); (2) long division with binary-search quotient digit — for each digit, find largest `k` in [0, BASE-1] such that `k*divisor ≤ remainder`; (3) Euclidean GCD via `divmod`. Division is the hardest operation; edge cases (base boundaries, negatives, zero) must be tested explicitly.

**Primary recommendation:** Implement BigInt following cp-algorithms patterns, use binary-search quotient for divmod (per SPEC), and validate with the SPEC/.cursorrules division test cases before proceeding to Frac.

---

## User Constraints (from CONTEXT.md)

### Locked Decisions
- **Error Handling:** Throw on invalid situations (division by zero, invalid string input). Validate divisor before division; throw if zero. Exception type: Claude's discretion (std::invalid_argument, std::runtime_error, or custom).
- **String I/O Contract:** Claude's discretion — str() format (zero, negatives, leading zeros), constructor whitespace/leading-zeros handling.
- **Edge Case Scope:** Spec-driven — test only what SPEC and .cursorrules explicitly list. SPEC mentions: 0*anything=0, (-a)*(-b)=a*b, division (1000000000/1, 999999999/1000000000, 123456789012345/123, negatives).

### Claude's Discretion
- Exception type for thrown errors (std::invalid_argument vs std::runtime_error vs custom)
- String I/O format and constructor behavior (whitespace, leading zeros)
- Validation approach (standalone test binary vs integration vs both)

### Deferred Ideas (OUT OF SCOPE)
None — discussion stayed within phase scope.

---

## Standard Stack

### Core (Zero External Dependencies)
| Component | Version | Purpose | Why |
|-----------|---------|---------|-----|
| g++ | 11+ (13.3 per SPEC) | Compiler | C++20, full support |
| C++20 | `-std=c++20` | Language | Required by project |
| `std::vector<uint32_t>` | — | Digit storage | Base 10⁹ limbs, spec-defined |
| `uint64_t` | — | Intermediate products | Two base-10⁹ digits fit (9+9 decimal) |

**Build command:** `g++ -std=c++20 -O2 -static -Wall -Wextra -o qseries main.cpp`

### No Libraries
| Instead of | Use | Reason |
|------------|-----|--------|
| GMP, libgmp | Roll BigInt from scratch | Zero-dependency mandate; single static binary |
| Boost.Multiprecision | Roll BigInt | Same constraint |
| Any package manager | Header-only, single main.cpp | Project architecture |

---

## Architecture Patterns

### Recommended Structure
```
src/
  bigint.h   — All BigInt code (header-only, inline)
  main.cpp   — Entry point (include bigint.h, minimal test/driver for Phase 1)
```

### Pattern 1: Struct with Methods (No Virtual)
**What:** Use `struct BigInt` with `bool neg`, `std::vector<uint32_t> d`, and member/static methods. No inheritance or virtual dispatch.  
**When:** Per .cursorrules — "No classes with virtual methods. Use structs with methods."  
**Why:** Simplicity, header-only, inlining.

### Pattern 2: Abs-First Arithmetic Helpers
**What:** Implement unsigned versions (e.g., `addAbs`, `subAbs` requiring |a| ≥ |b|), then wrap with sign logic in public operators.  
**When:** For `+`, `-`, `*`, `divmod` — sign rules are applied at the boundary.  
**Example:** `a - b` → if signs differ, route to `addAbs`; if both negative, compute `|b| - |a|` with appropriate sign.

### Pattern 3: Zero Representation
**What:** Zero is always non-negative. `neg = false`, `d = {0}` or `d = {}` (normalize to single canonical form).  
**When:** Every constructor, every operation that can produce zero.  
**Why:** Avoids ambiguity in comparison and division.

### Anti-Patterns to Avoid
- **Sign in digit representation:** Don't use two's complement in limbs. Use explicit `neg` flag.
- **Leading zeros in d:** Remove after every operation. `while (d.size() > 1 && d.back() == 0) d.pop_back();`
- **Division by zero:** Validate divisor before division; throw — do not return silently.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Fast multiplication (Karatsuba, FFT) | Custom FFT or Karatsuba | Schoolbook O(n·m) | SPEC: numbers rarely exceed ~100 digits; crossover ~100–200 digits; overhead not worth it |
| Division quotient guess (Newton, trial) | Complex digit-estimation | Binary search [0, BASE-1] per digit | SPEC mandates binary-search quotient; simpler, correct |
| BigInt library | — | Roll our own | Project mandates zero deps; this is the exception — we *must* hand-roll BigInt |

**Key insight:** For this project, BigInt itself is hand-rolled by design. What we avoid is over-engineering (Karatsuba, fancy quotient estimation) that adds complexity without benefit at q-series scale.

---

## Common Pitfalls

### Pitfall 1: Division — Wrong Quotient Digit
**What goes wrong:** Quotient or remainder is wrong for base-boundary cases: `1000000000/1`, `999999999/1000000000`, or `123456789012345/123`.  
**Why it happens:** Naive quotient guess (e.g., `remainder[top]/divisor[top]`) can overshoot when divisor is close to BASE.  
**How to avoid:** Binary search for largest `k` in [0, BASE-1] such that `k * divisor ≤ remainder`. Per SPEC and .cursorrules.  
**Warning signs:** Frac arithmetic or GCD fails on "nice" fractions; prodmake produces wrong rationals.  
**Reference:** PITFALLS.md Phase 1, SPEC §Common Pitfalls #3.

### Pitfall 2: Division — Wrong Iteration Width
**What goes wrong:** Remainder invariant `0 ≤ r < divisor` is violated; wrong number of quotient digits.  
**Why it happens:** When divisor has `n` digits, the "current remainder" for each quotient digit should span `n+1` digits (or normalized equivalent). Wrong width causes off-by-one.  
**How to avoid:** Follow Knuth Algorithm D / janmr long-division structure: use `n_d+1` digits of dividend for each quotient digit when divisor has `n_d` digits.  
**Reference:** janmr.com "Basic Multiple-Precision Long Division" (2014), Knuth TAOCP Vol 2.

### Pitfall 3: Sign Propagation in divmod
**What goes wrong:** `(-7) / 3` or `7 / (-3)` gives wrong quotient or remainder. Standard convention: `a = q*b + r` with `0 ≤ r < |b|`; sign of `q` = sign(a) XOR sign(b); sign of `r` = sign(a).  
**Why it happens:** Forgetting to normalize signs before/after unsigned divmod.  
**How to avoid:** Compute `divmod(|a|, |b|)` on positives, then apply sign rules: `q_neg = a.neg != b.neg`, `r_neg = a.neg`.  
**Warning signs:** `-7 % 3` or `7 % -3` produce unexpected results.

### Pitfall 4: Zero and Leading Zeros
**What goes wrong:** Zero represented as `neg=true, d={0}` or with leading zeros in `d`; comparisons and division break.  
**Why it happens:** Not normalizing after subtraction or division.  
**How to avoid:** Zero is always `neg=false`. Normalize: if `d.size()>1 && d.back()==0`, pop until one digit or empty; if result is 0, set `neg=false`.  
**Reference:** SPEC: "Zero is always non-negative."

### Pitfall 5: Multiplication Overflow in Digit Products
**What goes wrong:** `a[i] * b[j]` overflows if using `int` or `uint32_t`.  
**Why it happens:** Two base-10⁹ digits: 10⁹ × 10⁹ = 10¹⁸; fits in `uint64_t`, not in `uint32_t`.  
**How to avoid:** Use `uint64_t` for `cur = a[i] * 1ULL * b[j]` and for carry propagation.  
**Reference:** cp-algorithms uses `long long`; we use `uint64_t` for clarity.

---

## Code Examples

### Addition (cp-algorithms style, adapted to uint32_t)
```cpp
// Source: cp-algorithms.com/algebra/big-integer.html (adapted)
const uint32_t BASE = 1000000000;
uint64_t carry = 0;
for (size_t i = 0; i < max(a.d.size(), b.d.size()) || carry; ++i) {
    if (i == result.d.size()) result.d.push_back(0);
    uint64_t cur = carry + result.d[i] + (i < b.d.size() ? b.d[i] : 0);
    result.d[i] = cur % BASE;
    carry = cur / BASE;
}
// then normalize leading zeros
```
Note: For signed BigInt, wrap with sign logic (both same sign → add abs; different → subtract abs).

### Schoolbook Multiplication (cp-algorithms)
```cpp
// c = a * b; a, b non-negative
std::vector<uint32_t> c(a.d.size() + b.d.size(), 0);
for (size_t i = 0; i < a.d.size(); ++i)
    for (size_t j = 0, carry = 0; j < b.d.size() || carry; ++j) {
        uint64_t cur = c[i+j] + a.d[i] * 1ULL * (j < b.d.size() ? b.d[j] : 0) + carry;
        c[i+j] = cur % BASE;
        carry = cur / BASE;
    }
while (c.size() > 1 && c.back() == 0) c.pop_back();
```

### Binary-Search Quotient Digit (SPEC / .cursorrules)
```cpp
// Find largest k in [0, BASE-1] such that k * divisor <= remainder
uint32_t lo = 0, hi = BASE - 1;
while (lo < hi) {
    uint32_t mid = lo + (hi - lo + 1) / 2;
    BigInt prod = divisor * BigInt(static_cast<int64_t>(mid));
    if (cmpAbs(prod, remainder) <= 0) lo = mid;
    else hi = mid - 1;
}
// lo is the quotient digit; remainder -= lo * divisor
```
Ensure `remainder` and `divisor` are normalized (no leading zeros) before each step.

### Euclidean GCD
```cpp
BigInt bigGcd(BigInt a, BigInt b) {
    a = a.abs(); b = b.abs();
    while (!b.isZero()) {
        auto [q, r] = BigInt::divmod(a, b);
        a = std::move(b);
        b = std::move(r);
    }
    return a;
}
```
Per SPEC: "Standard Euclidean using divmod."

### Decimal Output (cp-algorithms)
```cpp
// Most significant digit (d.back()) printed as-is; lower digits padded to 9
std::string str() const {
    if (d.empty()) return "0";
    std::string s = std::to_string(d.back());
    for (int i = (int)d.size()-2; i >= 0; --i) {
        std::string t = std::to_string(d[i]);
        s += std::string(9 - t.size(), '0') + t;  // pad to 9 digits
    }
    if (neg) s = "-" + s;
    return s;
}
```

---

## State of the Art

| Old Approach | Current Approach | When | Impact |
|--------------|------------------|------|--------|
| Base 10 digits | Base 10⁹ | Common since 2000s | 9× fewer limbs; `uint64_t` holds digit×digit |
| Trial quotient digit | Binary-search quotient | SPEC/.cursorrules mandate | Correctness over micro-optimization |
| GMP for bignum | Roll own (this project) | Project constraint | Zero deps, single binary |

**Not applicable:** Karatsuba, FFT — skip for q-series scale.

---

## Open Questions

1. **Exception type for division by zero / invalid input**
   - What we know: CONTEXT says throw; type at Claude's discretion.
   - Options: `std::invalid_argument`, `std::runtime_error`, custom `BigIntError`.
   - Recommendation: `std::invalid_argument` for bad string; `std::domain_error` or `std::runtime_error` for division by zero — or keep both as `std::invalid_argument` for simplicity. Planner can pick.

2. **Canonical zero representation**
   - What we know: Zero is non-negative. SPEC doesn't mandate `d = {0}` vs `d = {}`.
   - Recommendation: Use `d = {0}` (single zero digit) for consistency with cp-algorithms "single zero digit" valid representation. Avoid empty vector for zero to simplify `d.back()` access.

---

## Sources

### Primary (HIGH confidence)
- [cp-algorithms Big Integer](https://cp-algorithms.com/algebra/big-integer.html) — base 10⁹, addition, subtraction, multiplication (schoolbook), division by short integer. Fetched 2025-02-24.
- [janmr.com Basic Multiple-Precision Long Division](https://janmr.com/blog/2014/04/basic-multiple-precision-long-division/) — Algorithm G (Knuth-style), quotient digit selection, remainder invariant. Fetched 2025-02-24.
- SPEC.md, .cursorrules (workspace) — API, algorithms, edge cases, test requirements.

### Secondary (MEDIUM confidence)
- .planning/research/STACK.md, PITFALLS.md, ARCHITECTURE.md — prior research, pitfalls, architecture. Consistent with SPEC.

### Tertiary
- Knuth TAOCP Vol 2 — cited by janmr for Algorithm D; not directly fetched.

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — SPEC and .cursorrules are authoritative; cp-algorithms and janmr provide verified algorithms.
- Architecture: HIGH — header-only, struct-based design is project-mandated.
- Pitfalls: HIGH — drawn from SPEC, .cursorrules, PITFALLS.md, and algorithm references.

**Research date:** 2025-02-24  
**Valid until:** 30 days (stable domain; algorithms unchanged)

---

## RESEARCH COMPLETE
