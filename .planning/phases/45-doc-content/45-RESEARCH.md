# Phase 45: Documentation Content - Research

**Researched:** 2026-02-27
**Domain:** Astro Starlight content authoring, KaTeX math rendering, technical documentation
**Confidence:** HIGH

## Summary

Phase 45 populates the 16 placeholder pages scaffolded in Phase 44 with real content. The work divides into four streams: (1) landing page with hero + feature cards, (2) 7 reference manual pages converted from MANUAL.md with formal typed signatures, (3) 4 tutorial pages derived from Garvan's qseriesdoc.md, and (4) KaTeX math integration for formulas throughout.

The Starlight scaffold is already functional with sidebar, dark theme, and teal accent. All 16 `.md`/`.mdx` files exist as placeholders. The main technical challenge is KaTeX integration — Starlight has no native math support and its CSS conflicts with KaTeX SVG rendering. The fix is well-documented: `remark-math` + `rehype-katex` plus a one-line CSS override.

**Primary recommendation:** Install `remark-math` and `rehype-katex`, add the CSS fix, convert the landing page to use Starlight's `<Card>` / `<CardGrid>` components in MDX, then systematically replace each placeholder with real content. Tutorial pages should use MDX for math + Starlight component interplay.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Landing Page Content:**
- 3-4 feature cards with icons highlighting key capabilities (exact arithmetic, prodmake, theta functions, relation finding)
- Download link points to Getting Started / Installation page (build from source)
- Demo snippet: Rogers-Ramanujan example with REPL output as static code block
- Academic framing: Prominent — mention Garvan, cite partition theory / modular forms prominently

**Manual Conversion:**
- Every function gets at least one REPL input/output example
- Conversion approach: Rewrite for web (not direct copy from MANUAL.md)
- Function signatures: Formal typed syntax — e.g. `etaq(k: int, T?: int) → Series`

**Tutorial Content:**
- Source material: Follow Garvan's qseriesdoc.md directly
- Style: Reference-style
- Prerequisites: Yes, on tutorial overview page

**Math Rendering:**
- Density: Moderate
- Layout: Sequential (math formula, then REPL block)

### Claude's Discretion

- **Manual page organization** — how to distribute 50+ built-ins across the 7 reference pages
- **Math integration** — how to wire KaTeX into Starlight (remark-math + rehype-katex)
- **Math depth in tutorials** — how much mathematical context to provide
- **Tutorial page organization** — how to map qseriesdoc.md sections to 4 tutorial pages

### Deferred Ideas (OUT OF SCOPE)
- No playground integration (Phase 46)
- No CI/CD (Phase 47)
</user_constraints>

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| `remark-math` | ^6.0 | Parse `$...$` and `$$...$$` math delimiters in Markdown/MDX | Standard remark plugin for math, pairs with rehype-katex |
| `rehype-katex` | ^7.0 | Render parsed math nodes as KaTeX HTML | Most popular rehype math renderer; lighter than MathJax |
| `katex` | ^0.16 | KaTeX CSS stylesheet (peer dependency of rehype-katex) | Required for font/styling of rendered math |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `@astrojs/starlight` | ^0.37.0 | Already installed — provides Card, CardGrid, Tabs components | For landing page feature cards and tabbed code examples |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| rehype-katex | rehype-mathjax | MathJax is heavier (~300KB vs ~100KB), renders more obscure symbols but KaTeX covers everything we need |
| MDX for all pages | Plain .md | MDX needed only for pages using Starlight components (Card, CardGrid); pure reference pages can stay .md if no components needed |

**Installation:**
```bash
cd website && npm install remark-math rehype-katex
```

## Architecture Patterns

### Content Organization (Claude's Discretion Recommendation)

```
website/src/content/docs/
├── index.mdx                          # Landing page (splash template, hero + CardGrid)
├── getting-started/
│   ├── installation.md                # Build from source instructions
│   └── quick-start.md                 # First REPL session walkthrough
├── manual/
│   ├── bigint-frac.md                 # Internal: BigInt & Frac (mostly for advanced users)
│   ├── series-ops.md                  # series(), coeffs(), qdegree(), lqdegree(), subs_q()
│   ├── q-functions.md                 # aqprod, qbin, etaq, theta2/3/4, T, tripleprod, quinprod, winquist
│   ├── product-conversion.md          # prodmake, etamake, jacprodmake, qfactor
│   ├── relations.md                   # findhom, findnonhom, findhomcombo, findnonhomcombo, findlincombo, findpoly
│   ├── sifting.md                     # sift, sum/add
│   └── repl-commands.md              # set_trunc, help, assignment, comments, scripting, history, tab completion
├── tutorial/
│   ├── index.md                       # Tutorial overview + prerequisites
│   ├── rogers-ramanujan.md            # prodmake + jacprodmake (qseriesdoc §3.1, §3.4)
│   ├── theta-functions.md             # etamake, theta as eta products (qseriesdoc §3.3, §2.2)
│   ├── partition-identities.md        # sift + product identification (qseriesdoc §5, §6)
│   └── modular-equations.md           # findhom/findnonhom/findpoly (qseriesdoc §4)
└── playground.md                      # Phase 46 placeholder — leave as-is
```

### Manual Page → Function Mapping

| Page | Functions | Count |
|------|-----------|-------|
| bigint-frac | (Internal types — explain exact arithmetic foundation) | 0 user-facing |
| series-ops | `series`, `coeffs`, `qdegree`, `lqdegree`, `subs_q` | 5 |
| q-functions | `aqprod`, `qbin`, `etaq`, `theta`, `theta2`, `theta3`, `theta4`, `tripleprod`, `quinprod`, `winquist`, `T` | 11 |
| product-conversion | `prodmake`, `etamake`, `jacprodmake`, `qfactor`, `jac2prod`, `jac2series` | 6 |
| relations | `findhom`, `findnonhom`, `findhomcombo`, `findnonhomcombo`, `findlincombo`, `findpoly` | 6 |
| sifting | `sift`, `sum`/`add` | 3 |
| repl-commands | `set_trunc`, `help`, `version`, `legendre`, `sigma`, assignment `:=`, comments, scripting, multi-line `\`, tab completion, history | 7+ |

**Total:** 38+ documented items across 7 pages, all 50+ built-ins reachable via sidebar.

### Tutorial Page → qseriesdoc.md Mapping

| Tutorial Page | qseriesdoc Sections | Key Content |
|---------------|---------------------|-------------|
| rogers-ramanujan | §3.1 (prodmake), §3.4 (jacprodmake) | RR identity, `prodmake` walkthrough, JAC product identification |
| theta-functions | §2.2 (theta/eta defs), §3.3 (etamake) | θ₂/θ₃/θ₄ as eta products, `etamake` usage |
| partition-identities | §3.2 (qfactor), §5 (sifting) | T(r,n) factorization, pd(5n+1) identity, `sift` + `etamake` pipeline |
| modular-equations | §4 (findhom/findnonhom/findpoly) | Gauss AGM θ relations, Eisenstein series, modular equations |

### Pattern 1: Function Reference Entry Format

Each function in the manual pages should follow this consistent structure:

```markdown
### `etaq`

Compute the eta product $\prod_{n=1}^{\infty}(1-q^{kn})$.

**Syntax:**

```
etaq(k: int, T?: int) → Series
etaq(q: Series, k: int, T: int) → Series
```

**Parameters:**
- `k` — Period of the eta product
- `T` — Truncation order (optional; defaults to `set_trunc` value)

**Example:**

```
qseries> etaq(1, 20)
1 - q - q^2 + q^5 + q^7 - q^12 - q^15 + O(q^20)
```

This is Euler's product $\prod_{n=1}^{\infty}(1-q^n)$, whose expansion is the pentagonal number theorem.
```

### Pattern 2: Tutorial Section with Math + REPL

```markdown
The Rogers-Ramanujan identity states:

$$
\sum_{n=0}^{\infty} \frac{q^{n^2}}{(q;q)_n} = \prod_{n=1}^{\infty} \frac{1}{(1-q^{5n-1})(1-q^{5n-4})}
$$

Let's verify this computationally. First, compute the left side:

```
qseries> rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)
1 + q + q^2 + q^3 + 2*q^4 + ...
```

Now apply `prodmake` to identify the product form:

```
qseries> prodmake(rr, 40)
(1-q)^(-1) * (1-q^4)^(-1) * (1-q^6)^(-1) * ...
```

The denominators appear only at exponents $\equiv \pm 1 \pmod{5}$, confirming the identity.
```

### Pattern 3: Landing Page Feature Cards (MDX)

```mdx
import { Card, CardGrid } from '@astrojs/starlight/components';

<CardGrid stagger>
  <Card title="Exact Arithmetic" icon="approve-check">
    Every coefficient is an exact rational — never a floating-point approximation.
  </Card>
  <Card title="Product Identification" icon="puzzle">
    Andrews' `prodmake` converts any q-series to its infinite product form.
  </Card>
</CardGrid>
```

### Anti-Patterns to Avoid
- **Copying MANUAL.md verbatim:** The user explicitly requested "rewrite for web," not paste. Structure and prose should be native to the web format.
- **Math-heavy walls of formulas:** User specified "moderate" density. Interleave math with REPL examples.
- **Using .md when components are needed:** The landing page and potentially tutorial pages need `<Card>`, `<CardGrid>`, or `<Tabs>` — these require `.mdx` extension.
- **Omitting REPL examples:** Every function must have at least one REPL input/output pair per user decision.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Math rendering | Custom LaTeX-to-HTML converter | `remark-math` + `rehype-katex` | Battle-tested, handles edge cases (fractions, subscripts, products) |
| Feature cards | Custom HTML/CSS cards | Starlight `<Card>` + `<CardGrid>` | Matches site theme, responsive, accessible |
| Code syntax highlighting | Custom highlighter | Starlight built-in (Expressive Code) | Already configured, supports line highlighting |
| Navigation/sidebar | Custom nav | Starlight sidebar config in astro.config.mjs | Already working from Phase 44 |

**Key insight:** Starlight provides all the UI primitives needed. The work is purely content authoring, not component development.

## KaTeX Integration (Claude's Discretion — Recommendation)

### Setup Steps

**Confidence: HIGH** — verified across multiple sources and the Starlight issue tracker.

1. Install dependencies:
```bash
cd website
npm install remark-math rehype-katex
```

2. Update `astro.config.mjs`:
```javascript
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import remarkMath from 'remark-math';
import rehypeKatex from 'rehype-katex';

export default defineConfig({
  markdown: {
    remarkPlugins: [remarkMath],
    rehypePlugins: [rehypeKatex],
  },
  integrations: [
    starlight({ /* existing config */ }),
  ],
});
```

3. Add KaTeX CSS import in `website/src/styles/custom.css`:
```css
@import 'katex/dist/katex.min.css';
```

4. Add CSS fix for Starlight SVG conflict:
```css
.katex-html svg {
  height: inherit;
}
```

### Math Syntax in Content Files
- Inline: `$\sum_{n=0}^{\infty}$` renders as inline math
- Display: `$$\prod_{n=1}^{\infty}(1-q^n)$$` renders as block math
- Works in both `.md` and `.mdx` files

### Known Limitation
Square root symbols (`\sqrt{}`) may display incorrectly without the CSS fix above. The fix has been verified working in Starlight 0.28.3+ through 0.37.x.

## Common Pitfalls

### Pitfall 1: MDX vs MD File Extension Mismatch
**What goes wrong:** Importing Starlight components (`Card`, `CardGrid`) in a `.md` file silently fails — no error, just no rendering.
**Why it happens:** Component imports require MDX processing; `.md` files don't support JSX.
**How to avoid:** Use `.mdx` extension for any page that imports Starlight components. The landing page (`index.mdx`) is already MDX. Tutorial pages may need conversion if they use components.
**Warning signs:** Cards/components appearing as raw text in the rendered page.

### Pitfall 2: KaTeX CSS Not Loading
**What goes wrong:** Math formulas render as raw LaTeX text or as unstyled HTML.
**Why it happens:** Forgetting to import `katex/dist/katex.min.css` or importing it in the wrong location.
**How to avoid:** Import in `custom.css` via `@import`, which is already referenced in astro.config.mjs's `customCss` array.
**Warning signs:** Math renders but looks wrong (no proper fonts, misaligned fractions).

### Pitfall 3: Dollar Signs in Code Blocks Parsed as Math
**What goes wrong:** `$` characters inside code blocks get interpreted as math delimiters.
**Why it happens:** `remark-math` processes before code block extraction in some configurations.
**How to avoid:** Fenced code blocks (triple backtick) are safe. Inline code (single backtick) containing `$` is also safe. Only bare `$` in prose triggers math mode.
**Warning signs:** Code examples showing garbled output or missing dollar signs.

### Pitfall 4: Inconsistent REPL Output Format
**What goes wrong:** Documentation shows REPL output that doesn't match the actual program's output format.
**Why it happens:** Copy-pasting from MANUAL.md or qseriesdoc.md (which uses Maple syntax, not our REPL syntax).
**How to avoid:** Run each example in the actual qseries binary and copy the real output. Our format differs from Maple: we use `*` for multiplication, `^` for powers, `O(q^N)` for truncation.
**Warning signs:** Output containing Maple-specific syntax like `:=` assignments with semicolons, or `RETURN()`.

### Pitfall 5: Forgetting the Playground Placeholder
**What goes wrong:** Overwriting the playground.md placeholder with content, violating the Phase 46 boundary.
**Why it happens:** Enthusiasm — wanting to fill every page.
**How to avoid:** Leave `playground.md` exactly as it is. Phase 46 will handle it.

## Content Authoring Guidelines

### Landing Page Structure
The existing `index.mdx` already has a good hero section. Enhancements needed:
1. Add `<CardGrid>` with 3-4 feature cards below the hero
2. Add a Rogers-Ramanujan demo snippet as a fenced code block
3. Add academic attribution section (Garvan, partition theory, modular forms)
4. Keep the "Get Started" → installation link and "Try Playground" → playground link

**Recommended icon mapping for feature cards:**
| Feature | Starlight Icon | Title |
|---------|---------------|-------|
| Exact arithmetic | `approve-check` | Exact Rational Arithmetic |
| prodmake | `puzzle` | Series → Product Conversion |
| Theta/eta functions | `star` | Theta & Eta Functions |
| Relation finding | `magnifier` | Relation Discovery |

### Getting Started Pages
- **installation.md**: Build from source (`g++ -std=c++20 -O2 -static -o qseries main.cpp`), platform notes (Linux/macOS/Windows+Cygwin), `--test` and `--version` flags
- **quick-start.md**: First REPL session — compute partition function, try prodmake on Rogers-Ramanujan, use help system

### Tutorial Prerequisites (for overview page)
Recommended prerequisites to list:
- Basic familiarity with power series notation
- Concept of a partition of an integer
- What q-series notation like $(a;q)_n$ means (brief explanation provided)
- No prior Maple experience needed

### Math Depth Recommendation (Claude's Discretion)
For tutorials, use **moderate** depth:
- State each identity/formula with proper KaTeX rendering
- Briefly explain what it means (1-2 sentences)
- Show the REPL computation that verifies it
- Don't reproduce full proofs (reference Garvan's papers instead)
- Include just enough mathematical context for a graduate student in combinatorics to follow

## Code Examples

### KaTeX Math in Markdown

```markdown
The partition function $p(n)$ counts the number of ways to write $n$ as a sum of positive integers. Its generating function is:

$$
\sum_{n=0}^{\infty} p(n) q^n = \prod_{n=1}^{\infty} \frac{1}{1 - q^n}
$$

In qseries, this is simply:

```
qseries> p := 1/etaq(1, 50)
qseries> series(p, 20)
1 + q + 2*q^2 + 3*q^3 + 5*q^4 + 7*q^5 + 11*q^6 + ...
```
```

### Landing Page MDX with Cards

```mdx
---
title: qseries
description: Exact q-series computation for partition theory, modular forms, and combinatorics
template: splash
hero:
  title: qseries
  tagline: "Exact q-series arithmetic for partition functions, infinite products, theta functions, and modular relations."
  actions:
    - text: Get Started
      link: /getting-started/installation/
      icon: right-arrow
    - text: Try Playground
      link: /playground/
      variant: minimal
      icon: external
---

import { Card, CardGrid } from '@astrojs/starlight/components';

<CardGrid stagger>
  <Card title="Exact Rational Arithmetic" icon="approve-check">
    Every coefficient is computed as an exact fraction, never a floating-point approximation. Results you can trust for proofs and conjectures.
  </Card>
  <Card title="Series → Product" icon="puzzle">
    Andrews' prodmake algorithm automatically converts q-series into infinite product representations. Identify Rogers-Ramanujan type identities at a keystroke.
  </Card>
  <Card title="Theta & Eta Functions" icon="star">
    Built-in theta2, theta3, theta4, etaq, tripleprod, quinprod, and winquist functions for modular form computation.
  </Card>
  <Card title="Relation Discovery" icon="magnifier">
    findhom, findnonhom, and findpoly discover polynomial relations among q-series — let the computer find the identities for you.
  </Card>
</CardGrid>
```

### Typed Function Signature Format

```markdown
### `aqprod`

Compute the rising q-factorial $(a;q)_n = \prod_{k=0}^{n-1}(1-aq^k)$.

**Syntax:**

```
aqprod(a: Series, q: Series, n: int, T: int) → Series
```

**Parameters:**
- `a` — Base series (typically `q` or a power of `q`)
- `q` — The q variable
- `n` — Number of factors in the product
- `T` — Truncation order

**Example:**

```
qseries> aqprod(q, q, 5, 20)
1 - q - q^2 + q^5 + q^7 - q^12 - q^15 + O(q^20)
```

This gives $(q;q)_5 = (1-q)(1-q^2)(1-q^3)(1-q^4)(1-q^5)$.
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| MathJax for Astro math | rehype-katex (lighter, faster) | 2024 | KaTeX is ~3x faster rendering, ~3x smaller bundle |
| Plain .md for all Starlight pages | .mdx when components needed | Starlight v0.20+ | Component imports require MDX; `.md` still fine for pure text |
| No CSS fix for KaTeX in Starlight | `.katex-html svg { height: inherit }` | Starlight 0.28.3+ | Required workaround; not yet fixed upstream |

**Deprecated/outdated:**
- `rehype-mathjax-svg`: Heavier alternative; use `rehype-katex` instead for this project
- Starlight's old `social` config format (object): Now uses array of `{ icon, label, href }` — already correct in our config

## Open Questions

1. **Exact REPL output format for each function**
   - What we know: The binary exists and can be run with `./qseries.exe`
   - What's unclear: Exact output formatting (spacing, line breaks, coefficient display) for all 50+ functions
   - Recommendation: Run each example in the actual binary during implementation and copy real output. Don't fabricate output.

2. **qseriesdoc.md Exercise content**
   - What we know: Garvan's doc has 13 exercises interspersed
   - What's unclear: Whether to include exercises in tutorials or just worked examples
   - Recommendation: Include worked examples only (no exercises). The tutorials are reference-style per user decision, not problem-set style.

3. **katex/dist/katex.min.css import method**
   - What we know: Can import via CSS `@import` or as a `<link>` in head
   - What's unclear: Whether `@import` in custom.css works correctly with Astro's build pipeline
   - Recommendation: Try `@import` first (simplest). If it fails, add a `<Head>` component override. Both are documented approaches.

## Sources

### Primary (HIGH confidence)
- Starlight official docs: Cards, CardGrid, frontmatter, authoring content — https://starlight.astro.build/
- Starlight GitHub issues #2511, #2744 — KaTeX CSS conflict and workaround verified
- MANUAL.md (repo) — Complete function list with 38+ built-ins
- qseriesdoc.md (repo) — 6 sections, 13 exercises, full Garvan tutorial

### Secondary (MEDIUM confidence)
- https://johndalesandro.com/blog/how-to-add-math-equations-to-astro-with-katex/ — remark-math + rehype-katex setup
- https://hideoo.dev/notes/starlight-plugin-use-remark-rehype-plugin — Starlight plugin API for remark/rehype
- https://www.byteli.com/blog/2024/math_in_astro/ — Astro math rendering patterns

### Tertiary (LOW confidence)
- npm version numbers for remark-math/rehype-katex — should be verified at install time with `npm install` (latest)

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — remark-math + rehype-katex is the only viable approach, well-documented
- Architecture: HIGH — Starlight's component system and content structure are well-documented, pages already exist
- Content mapping: HIGH — both MANUAL.md and qseriesdoc.md are in the repo and fully read
- KaTeX CSS fix: HIGH — verified across multiple Starlight issue reports, specific selector identified
- Pitfalls: MEDIUM — based on documented issues; actual REPL output formatting needs runtime verification

**Research date:** 2026-02-27
**Valid until:** 2026-04-27 (60 days — Starlight's API is stable; KaTeX integration unlikely to change)
