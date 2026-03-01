# Phase 12: Rogers-Ramanujan Demo — Research

**Researched:** 2026-02-25  
**Domain:** Output format matching, demo script, REPL display  
**Confidence:** HIGH

## Summary

Phase 12 ensures the Rogers-Ramanujan example reproduces qseriesdoc §3.1 Output (1) and (2). The demo script already exists (garvan-demo.sh from Phase 11) and runs the correct commands. The main planning work is **output format alignment**: the current REPL produces correct math but differs in presentation from the Maple/qseriesdoc reference. Key gaps: (1) series display term count (30 vs 50), (2) prodmake factor style (1-q^n vs -q^n+1), and (3) optional line wrapping. The demo script itself likely needs only minor changes (e.g., add `series(rr, 50)` if we want 50-term display) once format helpers are adjusted.

**Primary recommendation:** Add optional parameters or overloads to Series::str and formatProdmake for qseriesdoc-style output; adjust demo script to emit both Output (1) and (2) in matching style. Verify with a side-by-side diff against qseriesdoc.

---

## qseriesdoc §3.1 Reference: Exact Output Format

### Maple commands (qseriesdoc lines 188–199)

```maple
> x:=add(q^(n^2)/aqprod(q,q,n),n=0..8):
> series(x,q,50);
```

**Output (1)** — series expansion to O(q⁵⁰):

```
1 + q + q² + q³ + 2q⁴ + 2q⁵ + 3q⁶ + 3q⁷ + 4q⁸ + 5q⁹ + 6q¹⁰ + 7q¹¹ + 9q¹² + 10q¹³
  + 12q¹⁴ + 14q¹⁵ + 17q¹⁶ + 19q¹⁷ + 23q¹⁸ + 26q¹⁹ + 31q²⁰ + 35q²¹ + 41q²²
  + 46q²³ + 54q²⁴ + 61q²⁵ + 70q²⁶ + 79q²⁷ + 91q²⁸ + 102q²⁹ + 117q³⁰ + 131q³¹
  + 149q³² + 167q³³ + 189q³⁴ + 211q³⁵ + 239q³⁶ + 266q³⁷ + 299q³⁸ + 333q³⁹
  + 374q⁴⁰ + 415q⁴¹ + 465q⁴² + 515q⁴³ + 575q⁴⁴ + 637q⁴⁵ + 709q⁴⁶ + 783q⁴⁷
  + 871q⁴⁸ + 961q⁴⁹ + O(q⁵⁰)
```

**Format rules for Output (1):**
| Rule | Example |
|------|---------|
| Unicode superscripts for exponents | q², q¹⁰, q⁴⁹ |
| No space between coeff and q | 2q⁴, not 2 q⁴ |
| Constant 1 shown; coeff 1 omitted for q^n | 1 + q + q² |
| Terms joined by " + " |
| Line wrap with continuation " + " on next line |
| Trailing O(q^T) with Unicode exponent |
| ~50 terms shown (T=50) |

```maple
> prodmake(x,q,40);
```

**Output (2)** — product form:

```
1 / ((1-q)(-q⁴+1)(-q⁶+1)(-q⁹+1)(-q¹¹+1)(-q¹⁴+1)(-q¹⁶+1)(-q¹⁹+1)
     (-q²¹+1)(-q²⁴+1)(-q²⁶+1)(-q²⁹+1)(-q³¹+1)(-q³⁴+1)(-q³⁶+1)(-q³⁹+1))
```

**Format rules for Output (2):**
| Rule | Example |
|------|---------|
| Leading "1 / ((" for denominator-only |
| Factor style: (1-q) for n=1, (-q^n+1) for n>1 | (1-q), (-q⁴+1) |
| Unicode superscripts in exponents |
| No spaces between factors in denominator |
| Line wrap with indent for continuation |
| Closing "))" at end |

---

## Current REPL Output Format

### Series display (Series::str, src/series.h)

```cpp
std::string str(int maxTerms = 30) const
```

**Current output example:**
```
1 + q + q² + q³ + 2q⁴ + 2q⁵ + 3q⁶ + ... + O(q^50)
```

**Alignment with Output (1):**
| Aspect | Current | qseriesdoc | Match? |
|--------|---------|------------|--------|
| Unicode superscripts | ✓ expToUnicode() | ✓ | YES |
| No space coeff+q | ✓ | ✓ | YES |
| O(q^T) format | O(q^50) plain caret | O(q⁵⁰) Unicode | MINOR |
| Term count | 30 (hardcoded default) | 50 | NO |
| Line wrapping | None | Yes | OPTIONAL |

### prodmake display (formatProdmake, src/repl.h)

```cpp
inline std::string formatProdmake(const std::map<int, Frac>& a)
```

**Current output style:** `1 / ((1-q)(1-q⁴)(1-q⁶)(1-q⁹)...))`

**Alignment with Output (2):**
| Aspect | Current | qseriesdoc | Match? |
|--------|---------|------------|--------|
| Factor form | (1-q^n) | (1-q) and (-q^n+1) | DIFFERENT |
| Unicode exponents | ✓ | ✓ | YES |
| Structure "1 / ((" | ✓ | ✓ | YES |
| Line wrapping | None | Yes | OPTIONAL |

**Note:** (1-q^n) and (-q^n+1) are algebraically identical. qseriesdoc uses Maple’s factored form (-q^n+1). DEMO-02 says “matches Output (1) and (2) **style**”—“style” can be interpreted as information-equivalent or character-level. Recommendation: support optional “Maple style” factor form if strict match is required.

---

## Gaps Between Current and Target

### Gap 1: Series term count
- **Current:** Assignments and series display use str(30) or str(min(30, T)).
- **Target:** Output (1) shows 50 terms.
- **Fix:** Either increase default maxTerms when displaying RR-type series, or add an explicit `series(rr, 50)` step in the demo and ensure `series(f, T)` uses str(T) (or a configurable limit) instead of str(min(30, T)).

### Gap 2: series(f, T) built-in caps at 30 terms
- **Current (repl.h:244):** `std::cout << f.str(std::min(30, Tr))` — even `series(rr, 50)` only shows 30 terms.
- **Target:** Show up to T terms when user requests series(f, T).
- **Fix:** Use `f.str(Tr)` or `f.str(std::min(Tr, MAX_SAFE_TERMS))` instead of min(30, Tr).

### Gap 3: prodmake factor form
- **Current:** (1-q)(1-q⁴)(1-q⁶)...
- **Target (Maple):** (1-q)(-q⁴+1)(-q⁶+1)...
- **Fix:** Add optional parameter or variant to formatProdmake to emit (-q^n+1) for n>1. Low priority if “style” means “same information.”

### Gap 4: O(q^T) vs O(q⁵⁰)
- **Current:** Plain ASCII exponent in O(q^50).
- **Target:** Unicode O(q⁵⁰).
- **Fix:** Use expToUnicode in Series::str for the O-term.

### Gap 5: Line wrapping
- **Current:** Single long line.
- **Target:** Multi-line with continuation.
- **Fix:** Add optional wrap logic to str() and formatProdmake(). Lower priority for demo.

---

## Demo Script: Current vs Required

### Current demo block (demo/garvan-demo.sh)

```bash
# === Rogers-Ramanujan (qseriesdoc §3.1) ===
echo ""
echo "--- Rogers-Ramanujan ---"
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "prodmake(rr, 40)"
```

**Behavior:**
1. `rr := sum(...)` — assigns Rogers-Ramanujan sum, displays result via display(Series) → str(30).
2. `prodmake(rr, 40)` — displays product via formatProdmake.

**To match qseriesdoc Output (1) and (2):**
- Output (1): Need 50-term series display. Options:
  - **A)** Add `series(rr, 50)` before prodmake, and fix series() to honor T.
  - **B)** Raise default display terms for this demo (e.g. set_trunc(50) and/or change display logic).
- Output (2): prodmake output is already correct; optional format tweaks for exact Maple style.

**Recommended demo change:** Add explicit series display for clarity:
```bash
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "series(rr, 50)" "prodmake(rr, 40)"
```
Then ensure `series(rr, 50)` shows 50 terms (requires repl.h change).

---

## Architecture Patterns

### Pattern: Garvan-style display helpers
- **Location:** src/repl.h (formatProdmake, formatEtamake, etc.), src/series.h (str, expToUnicode).
- **Convention:** Use Series::expToUnicode(n) for superscripts; avoid floating point in math pipeline.
- **Extend:** Add optional style flags or separate functions rather than breaking existing behavior.

### Pattern: Demo script section block
- **From Phase 11:** One `run "cmd1" "cmd2" ...` per block; variable state persists.
- **Phase 12:** Same pattern; possibly one extra command (series) for explicit Output (1).

---

## Don’t Hand-Roll

| Problem | Don’t Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Unicode superscripts | Custom mapping | Series::expToUnicode | Already exists, used everywhere |
| Series formatting | New display path | Extend Series::str(maxTerms) | Single source of truth |
| Product formatting | Rewrite formatProdmake | Add optional style param | Keeps existing behavior |
| Demo command piping | Custom runner | run "cmd1" "cmd2" | Phase 11 pattern |

---

## Common Pitfalls

### Pitfall 1: Breaking existing display
- **Risk:** Changing str() or formatProdmake() alters output for other demos/tests.
- **Mitigation:** Add parameters (e.g. `str(maxTerms, useUnicodeO = true)`) or new overloads; keep defaults backward compatible.

### Pitfall 2: series(f, T) ignoring T
- **Risk:** series(rr, 50) still caps at 30 terms.
- **Mitigation:** Change repl.h series dispatch to use T as maxTerms, with a reasonable upper bound (e.g. 200).

### Pitfall 3: Demo script fragility
- **Risk:** Extra commands can change behavior (e.g. truncation) if not careful.
- **Mitigation:** set_trunc(50) is already env default; rr has trunc 50 from aqprod; series(rr, 50) truncates to 50. No conflicting truncation if env.T ≥ 50.

---

## Code Examples

### Series::str with configurable maxTerms (already exists)
```cpp
// series.h
std::string str(int maxTerms = 30) const;
```

### Fix for series() built-in to honor T
```cpp
// repl.h — current
std::cout << f.str(std::min(30, Tr)) << std::endl;

// Proposed: show up to Tr terms, cap for safety
std::cout << f.str(std::min(Tr, 100)) << std::endl;
```

### formatProdmake optional Maple-style factors
```cpp
// Option: add bool mapleStyle = false
// When true: emit (-q^n+1) for n>1 instead of (1-q^n)
```

---

## Open Questions

1. **Exact vs style match for prodmake**
   - Known: (1-q^n) and (-q^n+1) are equivalent.
   - Unknown: Whether DEMO-02 requires character-level match.
   - **Recommendation:** Implement Maple-style as optional; treat as LOW priority unless verification fails.

2. **Line wrapping**
   - Known: qseriesdoc wraps long lines.
   - Unknown: Whether wrapping is required for DEMO-02.
   - **Recommendation:** Defer; single-line output is acceptable if content matches.

3. **O(q^50) vs O(q⁵⁰)**
   - Known: qseriesdoc uses Unicode in O-term.
   - **Recommendation:** Use expToUnicode in str() for the O-term for full style match.

---

## Sources

### Primary (HIGH confidence)
- qseriesdoc.md §3.1 (lines 183–210) — Output (1) and (2) verbatim
- src/series.h — Series::str, expToUnicode
- src/repl.h — formatProdmake, display, series built-in
- demo/garvan-demo.sh — current Rogers-Ramanujan block
- .planning/REQUIREMENTS.md — DEMO-02
- .planning/ROADMAP.md — Phase 12 success criteria

### Secondary (MEDIUM confidence)
- Phase 11 RESEARCH.md — demo script patterns, BIN detection

---

## Metadata

**Confidence breakdown:**
- Output format spec: HIGH — taken from qseriesdoc
- Current implementation: HIGH — inspected in series.h, repl.h
- Gap analysis: HIGH — direct comparison
- Demo script changes: MEDIUM — depends on format fix scope

**Research date:** 2026-02-25  
**Valid until:** 30 days (stable codebase)
