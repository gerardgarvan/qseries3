# Technology Stack — Q-Series Website + Wasm Playground

**Project:** Q-Series REPL Documentation Website with WebAssembly Playground
**Researched:** 2026-02-27
**Focus:** NEW stack additions only (existing C++20 CLI stack is unchanged)

## Recommended Stack

### Documentation Site Framework

| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| Astro | ^5.7 | Static site generator | Zero-JS by default, islands architecture for selective hydration. Starlight is purpose-built for docs. |
| @astrojs/starlight | ^0.37 | Docs theme/framework | Built-in search (Pagefind), dark mode, sidebar nav, mobile-responsive, MDX support. No custom theme work needed. |
| @astrojs/react | ^4.4 | React integration | Needed for interactive playground island. `client:load` directive hydrates only the REPL component. |
| react / react-dom | ^19.0 | UI framework (playground only) | Drives the interactive Wasm REPL component. Astro renders everything else as static HTML. |

### WebAssembly Compilation

| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| Emscripten (emsdk) | ^4.0 | C++ → Wasm compiler | Only viable C++20 → Wasm toolchain. Supports `-std=c++20`, `std::map`, all stdlib features used by the codebase. |

**Critical compile command:**
```bash
em++ -std=c++20 -O2 -s MODULARIZE=1 -s EXPORT_ES6=1 \
     -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
     --bind -o qseries.js src/main.cpp
```

Key flags:
- `-std=c++20` — Match existing build standard
- `-O2` — Balance speed/size (expect ~1-3 MB .wasm for this codebase)
- `--bind` — Enable Embind for exposing `evaluate(string) → string` to JS
- `-s MODULARIZE=1 -s EXPORT_ES6=1` — ES module output for Astro import
- `-s ALLOW_MEMORY_GROWTH=1` — BigInt arithmetic may need dynamic memory

### Browser Terminal UI

| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| @xterm/xterm | ^5.5 | Terminal emulator in browser | Industry standard (powers VS Code terminal). Handles input, output, ANSI, scrollback. |
| @xterm/addon-fit | ^0.10 | Auto-resize terminal | Makes terminal fill its container responsively. |

**Why NOT xterm-pty or wasm-webterm:** These provide full PTY emulation (for programs that use raw termios). Our REPL already has a non-tty `std::getline` fallback path — we'll use a simpler architecture:
1. xterm.js captures user input line-by-line
2. JavaScript calls `Module.evaluate(line)` via Embind
3. Result string is written back to xterm.js

This avoids SharedArrayBuffer/COOP/COEP requirements, Asyncify overhead, and threading complexity. The existing `!stdin_is_tty()` code path in `repl.h` already handles piped/non-interactive input via `std::getline` — the Wasm build leverages this same path.

### Deployment

| Technology | Version | Purpose | Why |
|------------|---------|---------|-----|
| Cloudflare Pages | Free tier | Static site hosting | Free custom domains, unlimited static bandwidth, 25 MiB max file (plenty for .wasm), global CDN, `_headers` file for custom headers. |
| GitHub Actions | N/A | CI/CD | Builds site + Wasm on push, deploys to Cloudflare Pages via Wrangler. |
| wrangler | ^4.0 | Cloudflare CLI | `npx wrangler pages deploy dist` from CI. |

**Why Cloudflare Pages over GitHub Pages:**
- Cloudflare Pages supports `_headers` files natively for COOP/COEP headers (needed if we ever upgrade to threaded Wasm)
- Faster global CDN
- Same free tier, more generous limits (20,000 files, 500 builds/month)
- Better custom domain UX (DNS + SSL automatic)

### Supporting Tools

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| sharp | ^0.33 | Image optimization | Only if adding screenshots/diagrams to docs. Astro uses it for `<Image>`. Install only if needed. |
| Pagefind | built-in | Full-text search | Bundled with Starlight. No install needed. Indexes all docs at build time. |

## Architecture Decision: Embind vs. Full Terminal Emulation

**Decision: Use Embind `evaluate()` function, NOT full stdin/stdout emulation.**

| Approach | Complexity | Requirements | UX |
|----------|-----------|--------------|-----|
| Embind `evaluate(line) → string` | Low | No threading, no SharedArrayBuffer, no Asyncify | Line-by-line REPL (matches CLI behavior) |
| xterm-pty + Asyncify | High | SharedArrayBuffer, COOP/COEP headers, Asyncify bloat (+50-100% wasm size) | Char-by-char (unnecessary for math REPL) |
| PROXY_TO_PTHREAD | Very High | SharedArrayBuffer, Web Workers, complex I/O proxying | Full terminal (overkill) |

Rationale:
- Users type math expressions, hit Enter, see results. Line-by-line is the natural UX.
- The C++ code already has eval-a-string-return-result logic in `evalLine()` and the non-tty path.
- Asyncify adds 50-100% to .wasm size and introduces subtle bugs with complex C++ (templates, exceptions).
- SharedArrayBuffer requires COOP/COEP headers which break third-party embeds and complicate deployment.

### What This Means for C++ Changes

A thin Wasm entry point is needed — roughly:

```cpp
#ifdef __EMSCRIPTEN__
#include <emscripten/bind.h>
static Environment wasmEnv;
static bool wasmInitialized = false;

std::string wasmEvaluate(const std::string& input) {
    if (!wasmInitialized) {
        wasmEnv.set("T", Value(Series::one(100)));
        wasmInitialized = true;
    }
    // Reuse existing evalLine() logic
    // Return result as string
}

EMSCRIPTEN_BINDINGS(qseries) {
    emscripten::function("evaluate", &wasmEvaluate);
}
#endif
```

Conditional compilation (`#ifdef __EMSCRIPTEN__`) keeps the native CLI build unchanged.

## Alternatives Considered

| Category | Recommended | Alternative | Why Not |
|----------|-------------|-------------|---------|
| Docs framework | Astro Starlight | Docusaurus, MkDocs, VitePress | Starlight is lighter, faster builds, better Wasm island support via Astro islands |
| Terminal UI | @xterm/xterm | jquery.terminal, custom `<textarea>` | xterm.js is the standard; jquery.terminal is smaller but less maintained |
| Wasm bridge | Embind (`evaluate()`) | ccall/cwrap, full stdio emulation | Embind handles std::string natively; ccall requires manual memory management |
| Hosting | Cloudflare Pages | GitHub Pages, Vercel, Netlify | CF Pages: best free tier for static + custom domain + header control |
| JS framework for playground | React | Preact, Svelte, Vanilla JS | React has best xterm.js ecosystem; Preact viable but smaller community |

## Wasm Build Size Estimates

| Component | Estimated Size |
|-----------|---------------|
| qseries.wasm (O2, no Asyncify) | 1–3 MB |
| qseries.js (glue code) | 50–150 KB |
| xterm.js + addon-fit | ~300 KB |
| Total playground payload | ~2–4 MB |

Optimization levers if too large:
- `-Oz` instead of `-O2` (smaller but slower)
- `-s MINIMAL_RUNTIME=1` (strips unused runtime)
- gzip/brotli compression (Cloudflare does this automatically, ~60-70% reduction)
- Lazy-load playground (only fetch .wasm when user navigates to playground page)

## Project Structure (New Files Only)

```
website/                          # Astro project root
├── astro.config.mjs              # Starlight + React config
├── package.json
├── public/
│   └── wasm/
│       ├── qseries.js            # Emscripten glue (build artifact)
│       └── qseries.wasm          # Compiled Wasm (build artifact)
├── src/
│   ├── content/
│   │   └── docs/
│   │       ├── index.mdx         # Landing page
│   │       ├── manual/           # MANUAL.md converted to pages
│   │       ├── tutorial/         # Tutorial examples from qseriesdoc.md
│   │       └── playground.mdx    # Wasm REPL page
│   └── components/
│       └── Playground.tsx        # React: xterm.js + Wasm evaluate()
├── _headers                      # Cloudflare response headers (COOP/COEP if needed later)
└── wasm-build.sh                 # em++ build script
```

## Installation

```bash
# 1. Create Astro project with Starlight
npm create astro@latest -- --template starlight website
cd website

# 2. Add React integration
npx astro add react

# 3. Install terminal UI
npm install @xterm/xterm @xterm/addon-fit

# 4. Install Emscripten (one-time, outside project)
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh

# 5. Install deployment CLI
npm install -D wrangler
```

## Key Integration Points

### 1. Astro Config (astro.config.mjs)

```javascript
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import react from '@astrojs/react';

export default defineConfig({
  site: 'https://qseries.example.com',
  integrations: [
    starlight({
      title: 'Q-Series REPL',
      sidebar: [
        { label: 'Getting Started', link: '/' },
        { label: 'Manual', autogenerate: { directory: 'manual' } },
        { label: 'Tutorials', autogenerate: { directory: 'tutorial' } },
        { label: 'Playground', link: '/playground' },
      ],
    }),
    react(),
  ],
});
```

### 2. Playground Page (playground.mdx)

```mdx
---
title: Interactive Playground
description: Try q-series computations in your browser
---
import Playground from '../../components/Playground.tsx';

<Playground client:load />
```

### 3. Wasm Build Script (wasm-build.sh)

```bash
#!/bin/bash
em++ -std=c++20 -O2 \
  -s MODULARIZE=1 \
  -s EXPORT_ES6=1 \
  -s ALLOW_MEMORY_GROWTH=1 \
  -s EXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
  --bind \
  -I../src \
  -o website/public/wasm/qseries.js \
  wasm_entry.cpp
```

### 4. Cloudflare Pages _headers

```
/*
  X-Content-Type-Options: nosniff

/wasm/*
  Cache-Control: public, max-age=31536000, immutable
  Content-Type: application/wasm
```

## What NOT to Add

| Don't Add | Why |
|-----------|-----|
| GMP / Boost | Violates zero-dependency constraint; BigInt.h handles all arithmetic |
| node-pty | Server-side only; we need browser terminal |
| Asyncify | Unnecessary complexity — Embind evaluate() is simpler and smaller |
| SSR / server functions | Pure static site; all computation runs client-side in Wasm |
| Database | No user accounts, no persistence beyond browser session |
| Tailwind CSS | Starlight's built-in styles are sufficient for docs |
| Monaco Editor | Overkill for single-line math expressions; xterm.js is better fit |
| Service workers | Premature optimization; add only if offline support requested |

## Sources

- Astro Starlight docs: https://starlight.astro.build/ (HIGH confidence)
- Emscripten Embind docs: https://emscripten.org/docs/porting/connecting_cpp_and_javascript/embind.html (HIGH confidence)
- Emscripten Asyncify docs: https://emscripten.org/docs/porting/asyncify.html (HIGH confidence)
- xterm.js: https://xtermjs.org/ (HIGH confidence)
- xterm-pty: https://xterm-pty.netlify.app/ (HIGH confidence — evaluated and rejected)
- Cloudflare Pages limits: https://developers.cloudflare.com/pages/platform/limits/ (HIGH confidence)
- Cloudflare Pages headers: https://developers.cloudflare.com/pages/configuration/headers (HIGH confidence)
- Emscripten npm: npmjs.com/package/@xterm/xterm (HIGH confidence)
- Astro React integration: https://docs.astro.build/en/guides/integrations-guide/react (HIGH confidence)
