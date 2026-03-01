# Phase 2: Frac - Research

**Researched:** 2025-02-24  
**Domain:** Exact rational arithmetic (C++20, BigInt-based, zero dependencies)  
**Confidence:** HIGH

## Summary

Phase 2 delivers exact rational numbers (Frac) built on BigInt. The project mandates zero external libraries — no GMP, no Boost. Frac uses `BigInt num, den` with `den > 0` after normalization, and must call `reduce()` (via GCD) in **every** constructor and after **every** arithmetic operation. Skipping reduction causes exponential BigInt growth (PITFALLS #3). Arithmetic is standard: `a/b + c/d = (ad+bc)/(bd)` then reduce. Comparison uses cross-multiplication: `a/b < c/d` iff `a*d < c*b` when `b,d > 0`.

**Primary recommendation:** Implement Frac with `reduce()` in every constructor and operator; use `bigGcd` from Phase 1; throw on zero denominator. Validate with 6/4→3/2, 0/5→0/1, and long-chain growth tests before Phase 3.

---

<user_constraints>

## User Constraints (from CONTEXT.md)

### Locked Decisions
- **Error Handling:** Throw on Frac(n,0) and on operator/(a,b) when b is zero. Same rule for both: both throw.
- **Validation Checkpoint:** Both standalone Frac tests and integration tests in Phase 3. Broader test suite: SPEC cases (6/4→3/2, 0/5→0/1, long-chain growth) plus add/sub/mul/div and sign cases. Critical: must pass before Phase 3.

### Claude's Discretion
- Overflow handling and exception type
- String output format (all details: integers, zero, negatives, large fractions, round-trip, reduced form, whitespace)
- Test file location (extend main.cpp vs separate file vs mode flag)
- Reduce policy (timing: strict per-op vs batched; compound expressions, intermediate values, deferral)

### Deferred Ideas (OUT OF SCOPE)
None — discussion stayed within phase scope.

</user_constraints>

---

## Standard Stack

### Core (Zero External Dependencies)
| Component | Version | Purpose | Why |
|-----------|---------|---------|-----|
| BigInt | Phase 1 | Numerator and denominator | Per SPEC; zero-dependency mandate |
| bigGcd | Phase 1 | GCD for reduce() | Euclidean via divmod; do not reimplement |
| g++ | 11+ (13.3 per SPEC) | Compiler | C++20 |
| C++20 | `-std=c++20` | Language | Required by project |

**Build:** Same as Phase 1: `g++ -std=c++20 -O2 -static -Wall -Wextra -o qseries main.cpp`

### No Libraries
| Instead of | Use | Reason |
|------------|-----|--------|
| GMP mpq_t | Frac over BigInt | Zero-dependency mandate; single static binary |
| Boost.Rational | Frac over BigInt | Same constraint |

---

## Architecture Patterns

### Recommended Structure
```
src/
  bigint.h   — Phase 1 (depends on this)
  frac.h     — Frac (includes bigint.h, uses bigGcd)
  main.cpp   — Entry point; extend with Frac tests for Phase 2
```

### Pattern 1: Struct with Methods (No Virtual)
**What:** Use `struct Frac` with `BigInt num, den` and member/const methods. No inheritance or virtual dispatch.  
**When:** Per .cursorrules — "No classes with virtual methods. Use structs with methods."  
**Why:** Consistency with BigInt; header-only, inlining.

### Pattern 2: reduce() in Every Constructor and Operator
**What:** Call `reduce()` at the end of every constructor and at the end of every `operator+`, `operator-`, `operator*`, `operator/`.  
**When:** Always. No exceptions.  
**Why:** PITFALLS #3 and SPEC §Common Pitfalls #4: "If you forget to reduce fractions, the BigInt numerators and denominators will grow exponentially."  
**Implementation:** `void reduce()`: (1) handle den < 0 by negating both; (2) if num is zero, set den = 1; (3) else compute `g = bigGcd(num.abs(), den)` and divide both by g.

### Pattern 3: Denormal Form Invariants
**What:** After reduce(): `den > 0`, `gcd(|num|, den) == 1`, and zero is represented as `0/1`.  
**When:** Enforced by reduce(); every Frac is assumed normalized.  
**Why:** Comparison and arithmetic rely on den > 0 and reduced form for correctness and minimal growth.

### Pattern 4: Zero Denominator Throws
**What:** In `Frac(n, d)` and `Frac(BigInt n, BigInt d)`, if d is zero, throw before reduce(). In `operator/(a, b)`, if b is zero, throw.  
**When:** At construction and division.  
**Why:** Per CONTEXT locked decision.

### Anti-Patterns to Avoid
- **Deferring reduce():** Do not batch or defer. Call reduce() immediately after every op.
- **Reimplementing GCD:** Use `bigGcd` from Phase 1. Do not hand-roll GCD for Frac.
- **Den ≤ 0 after reduce():** reduce() must ensure den > 0 (flip signs if den < 0).

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| GCD for Frac | Custom GCD | bigGcd from Phase 1 | Phase 1 provides correct Euclidean GCD; no reason to duplicate |
| Batching reduce() | Deferred reduction | Strict per-op reduce() | PITFALLS: "Several small GCDs better than one large GCD later"; exponential growth if deferred |
| Floating-point fallback | Approx comparison | Exact cross-multiply | Exact arithmetic mandate; no floats in math pipeline |

**Key insight:** Frac is built on BigInt. What we avoid is skipping reduce() or reimplementing GCD — both cause bugs and growth.

---

## Common Pitfalls

### Pitfall 1: Frac Reduction Omitted — Exponential BigInt Growth
**What goes wrong:** Numerators and denominators blow up exponentially. Operations like `a/b + c/d = (ad+bc)/(bd)` produce unreduced fractions; without GCD normalization, subsequent operations compound size.  
**Why it happens:** Skipping reduce() to avoid GCD cost. GMP Rational docs: "Casting out common factors at each stage is optimal… several small GCDs better than one large GCD later."  
**How to avoid:** Call `reduce()` in every Frac constructor and at the end of every operator. Enforce: den > 0, gcd(|num|, den) = 1, 0/1 for zero.  
**Warning signs:** etaq(1, 100) or prodmake becomes very slow or runs out of memory; coefficient strings grow unexpectedly.  
**Reference:** PITFALLS.md #3, SPEC §Common Pitfalls #4, .cursorrules "Always call Frac::reduce() after construction — forgetting this causes exponential BigInt growth."

### Pitfall 2: reduce() with den < 0
**What goes wrong:** After reduce(), den remains negative; comparison and cross-multiply assumptions (b,d > 0) break.  
**Why it happens:** Forgetting to flip signs when den < 0: `if (den < 0) { num = -num; den = -den; }` before GCD step.  
**How to avoid:** First step of reduce(): if den is zero, throw; if den < 0, negate both num and den.  
**Warning signs:** Comparison gives wrong ordering; negative fractions display incorrectly.

### Pitfall 3: Zero Numerator Not Normalized to 0/1
**What goes wrong:** 0/5 stored as-is; gcd(0,5)=5 but dividing 0/5 by 5 yields 0/1. If zero is left as 0/5, den grows over chains.  
**Why it happens:** Not handling num=0 explicitly in reduce().  
**How to avoid:** In reduce(), if num.isZero(), set den = 1 and return.  
**Reference:** SPEC: "0/5 → 0/1."

### Pitfall 4: Division by Zero Not Checked
**What goes wrong:** Frac(n, 0) or a/b when b is zero crashes or produces undefined behavior.  
**Why it happens:** Forgetting to validate before reduce() or in operator/.  
**How to avoid:** In Frac(BigInt n, BigInt d): if d.isZero(), throw. In operator/(a,b): if b.isZero(), throw.  
**Reference:** CONTEXT: "Throw on Frac(n,0) and on operator/(a,b) when b is zero."

### Pitfall 5: Comparison When den Not Positive
**What goes wrong:** a/b < c/d uses cross-multiply a*d < c*b; this holds only when b,d > 0.  
**Why it happens:** reduce() must ensure den > 0; otherwise cross-multiply gives wrong sign.  
**How to avoid:** Ensure reduce() always sets den > 0; then comparison is safe.

---

## Code Examples

### reduce() Implementation
```cpp
void reduce() {
    if (den.isZero())
        throw std::invalid_argument("Frac: denominator zero");
    if (den < BigInt(0)) {
        num = -num;
        den = -den;
    }
    if (num.isZero()) {
        den = BigInt(1);
        return;
    }
    BigInt g = bigGcd(num.abs(), den);
    num = num / g;
    den = den / g;
}
```

### Constructor Calling reduce()
```cpp
Frac(BigInt n, BigInt d) : num(std::move(n)), den(std::move(d)) {
    reduce();
}

Frac(int64_t n, int64_t d) : num(n), den(d) {
    reduce();
}
```

### Addition
```cpp
Frac operator+(const Frac& o) const {
    // a/b + c/d = (a*d + c*b) / (b*d)
    BigInt n = num * o.den + o.num * den;
    BigInt d = den * o.den;
    Frac r(std::move(n), std::move(d));  // reduce() in ctor
    return r;
}
```

### Multiplication
```cpp
Frac operator*(const Frac& o) const {
    BigInt n = num * o.num;
    BigInt d = den * o.den;
    Frac r(std::move(n), std::move(d));  // reduce() in ctor
    return r;
}
```

### Division (with Zero Check)
```cpp
Frac operator/(const Frac& o) const {
    if (o.isZero())
        throw std::invalid_argument("Frac: division by zero");
    BigInt n = num * o.den;
    BigInt d = den * o.num;
    // Handle negative denominator from o.den when o.num < 0
    Frac r(std::move(n), std::move(d));  // reduce() handles sign
    return r;
}
```

### Comparison (cross-multiply, when den > 0)
```cpp
bool operator<(const Frac& o) const {
    // a/b < c/d  iff  a*d < c*b  when b,d > 0
    return (num * o.den) < (o.num * den);
}
```

### str() Format (Claude's discretion — example)
```cpp
std::string str() const {
    if (den == BigInt(1))
        return num.str();
    return num.str() + "/" + den.str();
}
```
Zero: "0". Negatives: "-3/4". Integers: "7" not "7/1".

---

## State of the Art

| Approach | When | Impact |
|----------|------|--------|
| Reduce after every op | Standard (GMP, Python fractions) | Optimal; several small GCDs cheaper than one large GCD |
| Defer reduce | Avoided | Causes exponential growth per PITFALLS |
| bigGcd from Phase 1 | Project | Reuse; no custom GCD |

---

## Open Questions

1. **Reduce policy (batching vs strict per-op)**  
   - What we know: CONTEXT lists as Claude's discretion. PITFALLS and .cursorrules prescribe strict per-op.  
   - Recommendation: Strict per-op. Call reduce() in every ctor and op. No deferral.

2. **Exception type**  
   - What we know: BigInt uses `std::invalid_argument` for div-by-zero and bad string. CONTEXT says Claude's discretion.  
   - Recommendation: Use `std::invalid_argument` for Frac(n,0) and division by zero, for consistency with BigInt.

3. **Overflow handling**  
   - What we know: BigInt uses fixed-precision limbs; overflow in intermediate products could theoretically occur for huge numbers. At q-series scale (~100 digits), unlikely.  
   - Recommendation: Rely on BigInt; no extra overflow handling for Phase 2 unless tests reveal issues.

---

## Sources

### Primary (HIGH confidence)
- SPEC.md — Frac API, reduce() semantics, arithmetic formulas, Common Pitfalls #4
- .cursorrules — "Always call Frac::reduce() after construction — forgetting this causes exponential BigInt growth"
- .planning/research/PITFALLS.md — Pitfall 3: Frac Reduction Omitted
- 02-CONTEXT.md — Locked decisions, discretion areas

### Secondary (MEDIUM confidence)
- GMP Rational Internals — "Several small GCDs better than one large GCD" (concept; not fetched)
- Python fractions — Reduce on construction (concept)

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — SPEC and Phase 1 BigInt are authoritative
- Architecture: HIGH — struct-based, reduce-everywhere follows SPEC and PITFALLS
- Pitfalls: HIGH — PITFALLS #3, SPEC #4, .cursorrules align

**Research date:** 2025-02-24  
**Valid until:** 30 days (stable domain)

---

## RESEARCH COMPLETE
