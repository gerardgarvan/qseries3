# Phase 109: Help Extensions — Research

**Researched:** 2026-03-06
**Domain:** Per-function REPL help with examples, SYNOPSIS, SEE ALSO
**Confidence:** HIGH

## Summary

Phase 109 extends the qseries REPL help system so `help(func)` is more useful: add 1–2 examples per key built-in (prodmake, etamake, jacprodmake, aqprod, etaq, theta, etc.), introduce SYNOPSIS (and optionally SEE ALSO) sections, and source examples from qseriesdoc.md / Garvan tutorial. The current `getHelpTable()` returns `map<string, pair<sig, desc>>`; help prints `sig — desc`. No examples, no structure.

**Primary recommendation:** Extend the help table value to a struct or tuple with optional fields (sig, desc, examples, seeAlso). Add a `formatHelpEntry()` that emits SYNOPSIS, EXAMPLES, SEE ALSO when present. Use qseriesdoc.md as the authoritative content source; translate Maple syntax to qseries REPL syntax (`sum(expr, var, lo, hi)` not `add(..., n=0..8)`).

<user_constraints>

## User Constraints (from CONTEXT.md)

No CONTEXT.md exists for Phase 109. Requirements from ROADMAP and requirements spec apply.

- **HELP-01:** Per-function help includes 1–2 examples for key built-ins (prodmake, etamake, …)
- **HELP-02:** Help structure includes SYNOPSIS and optionally SEE ALSO
- **HELP-03:** Examples sourced from qseriesdoc.md / Garvan tutorial where applicable

</user_constraints>

## Standard Stack

### Core (Existing)
| Component | Location | Purpose |
|-----------|----------|---------|
| getHelpTable | src/repl.h:465-580 | `map<string, pair<sig, desc>>` — single source of truth for help + tab completion |
| dispatchBuiltin("help") | src/repl.h:2174-2198 | Prints sig — desc for help(func); lists all names for help |
| qseriesdoc.md | repo root | Garvan q-series tutorial; prodmake, etamake, jacprodmake examples |
| ansi::gold/reset | src/repl.h:119-144 | Help output coloring (signature in gold) |

### External Reference
| Source | Purpose |
|--------|---------|
| Maple prodmake.html | qseries.org — CALLING SEQUENCE, SYNOPSIS, EXAMPLES, SEE ALSO |
| qseriesdoc.md | Garvan tutorial — Rogers-Ramanujan (prodmake), theta→eta (etamake), RR→JAC (jacprodmake) |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Extend value to (sig, desc, examples, seeAlso) | Separate examples map / file | Single source keeps help and completion consistent; no extra lookups |
| Inline examples as string in table | Load from qseriesdoc.md at runtime | Inline is simpler; no file dependency; content curated once |

## Architecture Patterns

### Maple prodmake Help Structure (Reference)
- **CALLING SEQUENCE:** `prodmake(f,q,T)`
- **PARAMETERS:** f (q-series), T (positive integer)
- **SYNOPSIS:** One short paragraph
- **EXAMPLES:** Input/output blocks
- **SEE ALSO:** Related functions

qseries3 targets CALLING SEQUENCE + SYNOPSIS + EXAMPLES parity; SEE ALSO is optional per HELP-02.

### Recommended Data Structure Change

**Option A (struct, preferred):**
```cpp
struct HelpEntry {
    std::string sig;
    std::string desc;
    std::vector<std::string> examples;  // empty = none
    std::vector<std::string> seeAlso;   // empty = none
};
std::map<std::string, HelpEntry> getHelpTable();
```

**Option B (minimal, backward compatible):**
```cpp
// Keep pair for most entries; overload or variant for extended
std::map<std::string, std::pair<std::string, std::string>> base;
// Or: third field optional — pair<sig, desc> vs tuple<sig, desc, examples, seeAlso>
```

Recommendation: Use a struct `HelpEntry` so all entries share the same shape; functions without examples have empty `examples` and `seeAlso`. Tab completion and `getCompletionCandidates` use `getHelpTable()` keys and optionally `sig` — no change needed if struct provides `.first`-like access for sig.

### Help Display Flow

1. `help(func)` → lookup in `getHelpTable()`
2. If found: call `formatHelpEntry(name, entry)`:
   - **SYNOPSIS:** `entry.sig` (gold) + `entry.desc`
   - **EXAMPLES:** for each `entry.examples[i]`, print `  qseries> ` + example
   - **SEE ALSO:** if non-empty, print `See also: entry.seeAlso[0], ...`
3. If not found: `unknown function: name` (existing)

### Syntax Translation (Maple → qseries REPL)

| Maple | qseries REPL |
|-------|--------------|
| `add(expr, n=0..8)` | `sum(expr, n, 0, 8)` |
| `aqprod(q,q,n)` | `aqprod(q,q,n,T)` — T from env or explicit |
| `series(x,q,50)` | `series(x, 50)` or implicit when assigning |
| `x:=...:` | `x := ...` or `x := ...;` |

qseriesdoc §3.1 prodmake example:
- Maple: `x:=add(q^(n^2)/aqprod(q,q,n),n=0..8):` then `prodmake(x,q,40)`
- REPL: `rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)` then `prodmake(rr, 40)`

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Help formatting | Custom rich markup | Simple line-by-line text with ansi::gold for sig | Terminal-only; no HTML/Man; consistency with existing |
| Example execution | Execute examples at runtime | Static strings in help table | Deterministic, fast, no eval side effects |
| Loading examples from file | Parse qseriesdoc.md at runtime | Curate examples into getHelpTable at build time | Zero file dependency; single binary |

## Common Pitfalls

### Pitfall 1: Maple Syntax in Examples
**What goes wrong:** Copy-paste from qseriesdoc produces `add(..., n=0..8)` which fails in REPL.
**How to avoid:** Always translate to `sum(expr, var, lo, hi)`; include T in aqprod/etaq when needed.
**Warning signs:** User reports "help example doesn't work".

### Pitfall 2: Breaking Tab Completion
**What goes wrong:** Changing getHelpTable return type breaks `getCompletionCandidates()` which iterates keys and may use `.second.first` for sig.
**How to avoid:** Ensure getHelpTable keys unchanged; if value type changes, update all consumers (help branch, getCompletionCandidates, levenshtein suggestions) to use new accessors.
**Warning signs:** Tab completion stops listing built-ins; compile errors in repl.h.

### Pitfall 3: Examples That Don't Run
**What goes wrong:** Example uses `series(x, 50)` but `x` undefined in fresh REPL; or T too small.
**How to avoid:** Each example must be self-contained or clearly show the setup. Rogers-Ramanujan: two lines — assign then prodmake — is the intended pattern.
**Warning signs:** Acceptance test fails when piping example to qseries.

## Code Examples

### Current help display (repl.h:2189-2191)
```cpp
if (it != table.end()) {
    std::cout << ansi::gold() << it->second.first << ansi::reset() << " — " << it->second.second << std::endl;
}
```

### Target format (conceptual)
```
  prodmake(f,T) — Andrews' algorithm: series → infinite product

  SYNOPSIS
  prodmake(f,T) converts the q-series f into an infinite product that agrees
  with f to O(q^T).

  EXAMPLES
  qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)
  qseries> prodmake(rr, 40)

  SEE ALSO
  etamake, jacprodmake
```

### Key built-ins for examples (from qseriesdoc)
| Function | qseriesdoc Section | Example |
|----------|--------------------|---------|
| prodmake | §3.1 | Rogers-Ramanujan: sum + prodmake |
| etamake | §3.3 | theta2/theta3/theta4 → eta products |
| jacprodmake | §3.4 | RR sum → JAC(0,5)/JAC(1,5) |
| aqprod | §2.1.1 | (a;q)_n definition |
| etaq | §2.2.1 | eta product Π(1-q^{kn}) |
| theta, theta2, theta3, theta4 | §2.2.2 | Truncated theta series |

## State of the Art

| Old Approach | Current Approach | Impact |
|--------------|------------------|--------|
| Maple: CALLING SEQUENCE, SYNOPSIS, EXAMPLES, SEE ALSO in HTML | qseries: sig + one-line desc only | Users expect examples; learning curve steeper |
| Phase 17: getHelpTable added | Phase 109: extend with examples | Incremental; no API break if struct backward-compatible |

**Deprecated/outdated:** None. Maple prodmake.html remains the reference format.

## Open Questions

1. **Which functions get examples in phase 109?**
   - Required: prodmake, etamake (HELP-01)
   - Recommended: jacprodmake, aqprod, etaq, theta (or theta2/3/4) — all have qseriesdoc examples
   - Others: defer to later phases; ~100 built-ins total

2. **Format of SEE ALSO**
   - Simple comma-separated: `etamake, jacprodmake`
   - Or "See also: ..." with ansi::dim for names
   - Recommendation: Plain text list; optional ansi::dim

3. **Multi-line SYNOPSIS**
   - Single paragraph vs multiple lines
   - Recommendation: One paragraph; wrap at 72 chars if desired (optional)

## Sources

### Primary (HIGH confidence)
- src/repl.h — getHelpTable, help branch, ~100 entries
- qseriesdoc.md — Garvan tutorial; prodmake §3.1, etamake §3.3, jacprodmake §3.4
- Maple prodmake.html (qseries.org) — CALLING SEQUENCE, SYNOPSIS, EXAMPLES, SEE ALSO

### Secondary (MEDIUM confidence)
- .planning/research/FEATURES.md — Maple help expectations; per-function examples as differentiator
- .planning/research/SUMMARY.md — getHelpTable extension; content from qseriesdoc

### Tertiary
- ROADMAP Phase 109 — HELP-01, HELP-02, HELP-03; success criteria

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — getHelpTable, qseriesdoc, Maple format are well understood
- Architecture: HIGH — struct extension + formatHelpEntry is straightforward
- Pitfalls: HIGH — Maple→REPL syntax translation is documented in phase 45 and 62

**Research date:** 2026-03-06
**Valid until:** 30 days (stable domain)

---

## RESEARCH COMPLETE
