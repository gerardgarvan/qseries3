# Phase 44: Astro Starlight Scaffold - Research

**Researched:** 2026-02-27
**Domain:** Astro Starlight documentation site scaffolding
**Confidence:** HIGH

## Summary

Astro Starlight is a full-featured documentation framework built on Astro. It provides sidebar navigation, dark/light mode toggle, responsive layout, Pagefind full-text search, and Expressive Code syntax highlighting out of the box with zero configuration. The scaffold can be created in a single command (`npm create astro@latest -- --template starlight`) and customized via `astro.config.mjs` and CSS custom properties.

The latest version is `@astrojs/starlight` 0.37.x running on Astro 5.x. The framework uses file-based routing from `src/content/docs/` — each `.md` or `.mdx` file becomes a page. Sidebar navigation can be manually configured with labeled groups or auto-generated from directory structure. Pagefind search is built-in and requires no setup. Dark mode works by default with a toggle in the header.

**Primary recommendation:** Use `npm create astro@latest -- --template starlight` to scaffold, then configure `astro.config.mjs` with sidebar groups, social links, custom CSS for color scheme/fonts, and create placeholder `.md` files for each documentation section.

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- Scaffold an Astro Starlight documentation site in `website/`
- Must have sidebar navigation, dark mode, responsive layout, Pagefind search, and placeholder pages for all documentation sections
- No actual content — Phase 45 handles that
- No playground integration — Phase 46 handles that
- Dark mode as default (dev tool aesthetic)
- No custom logo required for MVP — text-based site title is fine
- Monospace font emphasis for code/math content areas
- Expected sections: Getting Started, Manual/Reference, Tutorial, Playground
- Hero should convey: what qseries does, that it's free/open, and link to playground
- Placeholder pages should have realistic titles and brief "content coming in Phase 45" notes
- Playground page placeholder should indicate "interactive REPL coming in Phase 46"
- Title "qseries", appropriate tagline about q-series computation
- Standard SEO meta tags
- GitHub repository link in header/footer

### Claude's Discretion
- Organize sidebar into logical groups based on the project's documentation needs
- Use Starlight's built-in theming with a color scheme appropriate for a math/computation tool
- Standard Starlight landing page with hero section
- Title and tagline wording
- SEO meta tag content

### Deferred Ideas (OUT OF SCOPE)
- None — discussion stayed within phase scope
</user_constraints>

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| `astro` | ^5.x | Core framework | Starlight 0.37.x requires Astro 5 |
| `@astrojs/starlight` | ^0.37.x | Documentation framework | The chosen doc framework; provides sidebar, search, dark mode, responsive layout |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| Pagefind | built-in | Full-text search | Included automatically by Starlight — zero config needed |
| Expressive Code | built-in | Code block syntax highlighting | Included automatically — relevant for code examples in docs |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Manual sidebar config | `autogenerate` from directories | Manual gives exact control over ordering and grouping; autogenerate is simpler but less control. Recommend **manual** for the structured sidebar this project needs |
| No Tailwind | Tailwind CSS integration | Tailwind adds complexity; plain custom CSS with Starlight's CSS custom properties is sufficient for this scope |

**Installation:**
```bash
npm create astro@latest -- --template starlight
```

This scaffolds a complete project. Run from the repo root, target directory `website/`.

## Architecture Patterns

### Recommended Project Structure
```
website/
├── astro.config.mjs          # Starlight integration config (sidebar, social, customCss)
├── package.json               # Dependencies (astro, @astrojs/starlight)
├── src/
│   ├── content/
│   │   └── docs/              # All documentation pages (.md files)
│   │       ├── index.mdx      # Landing page (template: splash, hero)
│   │       ├── getting-started/
│   │       │   ├── installation.md
│   │       │   └── quick-start.md
│   │       ├── manual/
│   │       │   ├── bigint-frac.md
│   │       │   ├── series-ops.md
│   │       │   ├── q-functions.md
│   │       │   ├── product-conversion.md
│   │       │   ├── relations.md
│   │       │   ├── sifting.md
│   │       │   └── repl-commands.md
│   │       ├── tutorial/
│   │       │   ├── index.md
│   │       │   ├── rogers-ramanujan.md
│   │       │   ├── theta-functions.md
│   │       │   ├── partition-identities.md
│   │       │   └── modular-equations.md
│   │       └── playground.md  # Placeholder for Phase 46
│   ├── content.config.ts      # Content collection schemas
│   └── styles/
│       └── custom.css         # Color scheme + font overrides
├── public/
│   └── favicon.svg            # Site favicon
└── tsconfig.json              # TypeScript config (Astro default)
```

### Pattern 1: Landing Page with Hero (template: splash)
**What:** Starlight's `splash` template provides a full-width landing page without sidebar.
**When to use:** For the site's index page / homepage.
**Example:**
```yaml
# src/content/docs/index.mdx
---
title: qseries
description: Exact q-series computation in the browser
template: splash
hero:
  title: qseries
  tagline: Exact q-series arithmetic — partition functions, infinite products, theta functions, and modular relations
  actions:
    - text: Get Started
      link: /getting-started/installation/
      icon: right-arrow
    - text: Try Playground
      link: /playground/
      variant: minimal
      icon: external
---
```
Source: https://starlight.astro.build/reference/frontmatter/

### Pattern 2: Sidebar Configuration with Groups
**What:** Manual sidebar with labeled, collapsible groups.
**When to use:** For organizing docs into logical sections.
**Example:**
```javascript
// astro.config.mjs
starlight({
  title: 'qseries',
  sidebar: [
    {
      label: 'Getting Started',
      items: [
        { slug: 'getting-started/installation' },
        { slug: 'getting-started/quick-start' },
      ],
    },
    {
      label: 'Manual',
      items: [
        { slug: 'manual/bigint-frac' },
        { slug: 'manual/series-ops' },
        // ...
      ],
    },
    {
      label: 'Tutorial',
      items: [
        { slug: 'tutorial' },
        { slug: 'tutorial/rogers-ramanujan' },
        // ...
      ],
    },
    {
      label: 'Playground',
      items: [
        { slug: 'playground' },
      ],
    },
  ],
})
```
Source: https://starlight.astro.build/guides/sidebar/

### Pattern 3: Social Links in Header
**What:** GitHub icon link in the site header.
**When to use:** Standard for open-source project documentation.
**Example:**
```javascript
// astro.config.mjs
starlight({
  social: [
    { icon: 'github', label: 'GitHub', href: 'https://github.com/user/qseries' },
  ],
})
```
Source: https://starlight.astro.build/reference/configuration/

### Pattern 4: Custom CSS for Theming
**What:** Override Starlight CSS custom properties for colors and fonts.
**When to use:** To set a math/dev-tool aesthetic with appropriate accent colors and monospace emphasis.
**Example:**
```css
/* src/styles/custom.css */
:root {
  /* Override the monospace font stack */
  --sl-font-mono: 'JetBrains Mono', ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace;

  /* Accent color — a teal/cyan for math-tool feel */
  --sl-color-accent-low: hsl(190, 54%, 20%);
  --sl-color-accent: hsl(190, 80%, 50%);
  --sl-color-accent-high: hsl(190, 80%, 80%);
}
```
Then register in config:
```javascript
starlight({
  customCss: ['./src/styles/custom.css'],
})
```
Source: https://github.com/withastro/starlight/blob/main/packages/starlight/style/props.css

### Anti-Patterns to Avoid
- **Don't use `autogenerate` for sidebar:** The project needs a specific sidebar structure with curated groups. Autogenerate sorts alphabetically by slug and doesn't give the control needed here.
- **Don't add Tailwind:** This is a pure documentation scaffold. Starlight's built-in CSS custom properties provide everything needed. Tailwind adds unnecessary complexity.
- **Don't create custom Astro components:** Phase 44 is scaffold only. Custom components (e.g., for playground) belong in Phase 46.
- **Don't write actual documentation content:** Phase 45 handles all content. Placeholder text only.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Search | Custom search implementation | Starlight's built-in Pagefind | Pagefind indexes all pages at build time automatically; zero config needed |
| Dark mode toggle | Custom theme switcher | Starlight's built-in toggle | Built into header component, handles localStorage persistence and system preference detection |
| Responsive layout | Custom CSS breakpoints | Starlight's built-in responsive design | Sidebar collapses to hamburger menu on mobile automatically |
| Sidebar navigation | Custom nav component | Starlight's `sidebar` config | Supports groups, collapsing, badges, and auto-generation from directory |
| Code highlighting | Custom syntax highlighter | Expressive Code (built-in) | Starlight bundles Expressive Code with Shiki; supports line highlighting, diffs, frames |

**Key insight:** Starlight's entire value proposition is that these common doc-site features are built-in. The scaffold phase should configure, not build.

## Common Pitfalls

### Pitfall 1: Content Collection Config Missing
**What goes wrong:** Site fails to build with cryptic content collection errors.
**Why it happens:** `src/content.config.ts` must exist with the Starlight schema imports. The `create astro` template includes this, but manual setup can miss it.
**How to avoid:** Use the template-based creation (`npm create astro@latest -- --template starlight`) which scaffolds this correctly.
**Warning signs:** Build errors mentioning "content collection" or "schema validation".

### Pitfall 2: Sidebar Slugs Don't Match File Paths
**What goes wrong:** Sidebar links return 404s or are grayed out.
**Why it happens:** The `slug` in sidebar config must match the file path relative to `src/content/docs/` without the extension. E.g., file at `src/content/docs/manual/series-ops.md` → slug is `manual/series-ops`.
**How to avoid:** Keep a consistent naming convention; verify each sidebar entry has a matching `.md` file.
**Warning signs:** Build warnings about missing slugs; dev server shows broken links.

### Pitfall 3: Trying to Force Dark Mode Only
**What goes wrong:** Starlight doesn't have a built-in config option to disable the theme toggle or force dark mode only.
**Why it happens:** Dark/light mode toggle is a core Starlight UI element. There is no `defaultTheme` or `disableLightMode` config option.
**How to avoid:** Accept that both modes exist. The context says "dark mode as default" which Starlight handles by respecting `prefers-color-scheme: dark` (most developer systems). If truly needed, a small CSS override can hide the toggle and force dark, but this is generally not recommended.
**Warning signs:** Spending time trying to find a non-existent config option.

### Pitfall 4: Wrong File Extension for Landing Page
**What goes wrong:** Hero component's `actions` or `image` imports don't work in plain `.md`.
**Why it happens:** If you need to import local images or use Astro components, you need `.mdx`. For a simple hero with actions (no imports), `.md` works fine. But `.mdx` is safer if you might add components later (Phase 45/46).
**How to avoid:** Use `.mdx` for the landing page index file. Use `.md` for simple placeholder pages.
**Warning signs:** Build errors about unsupported syntax in `.md` files.

### Pitfall 5: Pagefind Search Not Working in Dev Mode
**What goes wrong:** Search returns no results during `npm run dev`.
**Why it happens:** Pagefind indexes the built site, not the dev server. Search only works after `npm run build`.
**How to avoid:** Test search by building (`npm run build`) and previewing (`npm run preview`), not via dev server.
**Warning signs:** Search bar appears but no results ever show up in dev mode. This is expected behavior.

## Code Examples

### Complete astro.config.mjs
```javascript
// Source: Starlight configuration reference
// https://starlight.astro.build/reference/configuration/
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';

export default defineConfig({
  integrations: [
    starlight({
      title: 'qseries',
      description: 'Exact q-series computation — partition functions, products, theta functions, and modular relations',
      social: [
        { icon: 'github', label: 'GitHub', href: 'https://github.com/GarvanResearchGroup/qseries' },
      ],
      customCss: ['./src/styles/custom.css'],
      sidebar: [
        {
          label: 'Getting Started',
          items: [
            { slug: 'getting-started/installation' },
            { slug: 'getting-started/quick-start' },
          ],
        },
        {
          label: 'Reference Manual',
          items: [
            { slug: 'manual/bigint-frac' },
            { slug: 'manual/series-ops' },
            { slug: 'manual/q-functions' },
            { slug: 'manual/product-conversion' },
            { slug: 'manual/relations' },
            { slug: 'manual/sifting' },
            { slug: 'manual/repl-commands' },
          ],
        },
        {
          label: 'Tutorial',
          items: [
            { slug: 'tutorial' },
            { slug: 'tutorial/rogers-ramanujan' },
            { slug: 'tutorial/theta-functions' },
            { slug: 'tutorial/partition-identities' },
            { slug: 'tutorial/modular-equations' },
          ],
        },
        {
          label: 'Playground',
          items: [
            { slug: 'playground' },
          ],
        },
      ],
    }),
  ],
});
```

### Placeholder Page Template
```markdown
---
title: Series Operations
description: Truncated power series arithmetic — addition, multiplication, inverse, composition
---

# Series Operations

*This page will contain the full reference for Series arithmetic operations.*

Content for this documentation section is coming in a future update.
```

### Landing Page with Hero
```yaml
---
title: qseries
description: Exact q-series computation in the browser
template: splash
hero:
  title: qseries
  tagline: Exact q-series arithmetic for partition theory, modular forms, and combinatorics. Free, open-source, runs in your browser.
  actions:
    - text: Get Started
      link: /getting-started/installation/
      icon: right-arrow
    - text: Try Playground
      link: /playground/
      variant: minimal
      icon: external
---
```

### Content Collection Config
```typescript
// src/content.config.ts
// Source: https://starlight.astro.build/manual-setup/
import { defineCollection } from 'astro:content';
import { docsLoader } from '@astrojs/starlight/loaders';
import { docsSchema } from '@astrojs/starlight/schema';

export const collections = {
  docs: defineCollection({ loader: docsLoader(), schema: docsSchema() }),
};
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `sidebar` items as `{ label, link }` | `{ slug: 'path' }` shorthand | Starlight 0.28+ | Simpler sidebar config — slug auto-resolves title from frontmatter |
| `src/content/config.ts` | `src/content.config.ts` | Astro 5.x | Content collection config file renamed; old name still works but new name is standard |
| `social` as object `{ github: 'url' }` | `social` as array `[{ icon, label, href }]` | Starlight 0.30+ | More flexible — supports custom ordering and multiple links of same type |

**Deprecated/outdated:**
- Old `social` object format (`{ github: 'url' }`) — use array format instead
- Old content config location (`src/content/config.ts`) — use `src/content.config.ts`

## Open Questions

1. **GitHub repository URL**
   - What we know: Social links need a GitHub URL
   - What's unclear: The exact GitHub repo URL for this project
   - Recommendation: Use a placeholder URL (e.g., `https://github.com/user/qseries3`) that can be updated when known

2. **Exact sidebar page breakdown for Manual**
   - What we know: The REPL has 50+ built-in functions across several categories
   - What's unclear: The optimal grouping of manual pages (one page per category vs. one page per function)
   - Recommendation: Group by category (series ops, q-functions, product conversion, relations, sifting, REPL commands) — roughly 7 pages. This matches Phase 45's scope and the SPEC.md architecture layers.

3. **Accent color choice**
   - What we know: Should convey math/computation tool, dev-tool aesthetic
   - What's unclear: Exact hue preference
   - Recommendation: Use a teal/cyan accent (hue ~190-200) which is commonly associated with developer tools and scientific computing. Starlight's default blue-indigo (hue 224) also works well.

## Sources

### Primary (HIGH confidence)
- Starlight Configuration Reference — https://starlight.astro.build/reference/configuration/
- Starlight CSS & Styling Guide — https://starlight.astro.build/guides/css-and-tailwind/
- Starlight Sidebar Guide — https://starlight.astro.build/guides/sidebar/
- Starlight Project Structure — https://starlight.astro.build/guides/project-structure/
- Starlight Frontmatter Reference — https://starlight.astro.build/reference/frontmatter/
- Starlight props.css (CSS custom properties) — https://github.com/withastro/starlight/blob/main/packages/starlight/style/props.css
- NPM @astrojs/starlight — https://www.npmjs.com/package/@astrojs/starlight (version 0.37.1 confirmed)

### Secondary (MEDIUM confidence)
- Starlight Getting Started — https://starlight.astro.build/getting-started/
- Starlight Pagefind search — https://starlight.astro.build/guides/site-search/
- Astro docs on Node.js requirements — Node.js v18.20.8+ or v20.3.0+ or v22.0.0+

### Tertiary (LOW confidence)
- Dark-mode-only forcing: GitHub discussions (#949, #1258, #398) suggest no built-in option exists — needs validation if user insists on removing toggle

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — Astro Starlight is well-documented, version confirmed on npm
- Architecture: HIGH — Project structure is well-defined by Starlight's official documentation and template
- Pitfalls: HIGH — Verified against official docs and known GitHub issues
- CSS custom properties: HIGH — Read directly from Starlight source (props.css)
- Dark mode default: MEDIUM — Starlight respects `prefers-color-scheme` but has no "force dark" config

**Research date:** 2026-02-27
**Valid until:** 2026-04-27 (Starlight is actively developed but core patterns are stable)
