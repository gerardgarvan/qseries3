# Architecture: Q-Series Documentation Website with Wasm Playground

**Domain:** Documentation website with live WebAssembly playground for a C++20 CLI REPL
**Researched:** 2026-02-27
**Confidence:** HIGH (Astro Starlight, Emscripten, xterm.js are mature; xterm-pty is proven for this exact use case)

## System Overview

The website adds two new build targets to the existing qseries repo: (1) an Astro Starlight static documentation site, and (2) an Emscripten-compiled Wasm binary of the REPL. The Wasm binary runs inside a browser terminal (xterm.js + xterm-pty) embedded as a React island in the Starlight site.

```
┌──────────────────────────────────────────────────────────────────────┐
│                        DEPLOYED STATIC SITE                          │
│  ┌────────────────────┐  ┌─────────────────────────────────────────┐ │
│  │  Starlight Pages   │  │         Playground Page                 │ │
│  │  (HTML/CSS/JS)     │  │  ┌──────────────────────────────────┐  │ │
│  │                    │  │  │  React Island (client:only)      │  │ │
│  │  - Manual          │  │  │  ┌────────────┐ ┌─────────────┐ │  │ │
│  │  - Function ref    │  │  │  │ xterm.js   │←│ xterm-pty   │ │  │ │
│  │  - Tutorial        │  │  │  │ (terminal  │ │ (PTY layer  │ │  │ │
│  │  - Examples        │  │  │  │  display)  │ │  + bridge)  │ │  │ │
│  │                    │  │  │  └────────────┘ └──────┬──────┘ │  │ │
│  │                    │  │  │                        │        │  │ │
│  │                    │  │  │              ┌─────────▼──────┐ │  │ │
│  │                    │  │  │              │ qseries.wasm   │ │  │ │
│  │                    │  │  │              │ qseries.js     │ │  │ │
│  │                    │  │  │              │ (Emscripten)   │ │  │ │
│  │                    │  │  │              └────────────────┘ │  │ │
│  │                    │  │  └──────────────────────────────────┘  │ │
│  └────────────────────┘  └─────────────────────────────────────────┘ │
└──────────────────────────────────────────────────────────────────────┘
```

## Decision: Monorepo (website/ folder)

**Recommendation:** Single repo with a `website/` subfolder. Do NOT use a separate repo.

**Rationale:**
- The Wasm build compiles the same `src/*.h` + `src/main.cpp` files — a separate repo would need to pull them in
- Documentation references code that changes with the REPL — keeping them in sync is automatic in a monorepo
- Single CI workflow can build native binary, Wasm binary, and website in one pipeline
- Astro Starlight projects are self-contained within their folder (own `node_modules/`, `package.json`)
- No npm workspaces needed — `website/` is an independent Astro project that happens to live in the repo

**Trade-off:** Repo gains ~200MB of `node_modules/` (gitignored) and ~50 files of website source. Acceptable for a project of this size.

## Repository Structure (New Files)

```
qseries3/                          # ← existing repo root
├── src/                            # ← existing C++ source (UNCHANGED)
│   ├── bigint.h
│   ├── frac.h
│   ├── series.h
│   ├── qfuncs.h
│   ├── convert.h
│   ├── linalg.h
│   ├── relations.h
│   ├── parser.h
│   ├── repl.h
│   └── main.cpp
├── Makefile                        # ← existing (add wasm target)
├── website/                        # ← NEW: Astro Starlight project
│   ├── package.json
│   ├── astro.config.mjs
│   ├── tsconfig.json
│   ├── src/
│   │   ├── content/
│   │   │   ├── docs/              # Markdown documentation pages
│   │   │   │   ├── index.mdx      # Landing/intro page
│   │   │   │   ├── getting-started.mdx
│   │   │   │   ├── manual/        # User manual (from MANUAL.md)
│   │   │   │   │   ├── basic-usage.mdx
│   │   │   │   │   ├── functions.mdx
│   │   │   │   │   └── examples.mdx
│   │   │   │   ├── reference/     # Function reference
│   │   │   │   │   ├── series-functions.mdx
│   │   │   │   │   ├── product-functions.mdx
│   │   │   │   │   └── relation-functions.mdx
│   │   │   │   └── tutorial/      # Worked examples from qseriesdoc.md
│   │   │   │       ├── rogers-ramanujan.mdx
│   │   │   │       └── theta-identities.mdx
│   │   │   └── config.ts
│   │   ├── components/
│   │   │   └── QSeriesPlayground.tsx  # React island: xterm.js + Wasm
│   │   ├── pages/
│   │   │   └── playground.astro       # Custom page hosting the React island
│   │   └── styles/
│   │       └── playground.css
│   ├── public/
│   │   ├── wasm/                  # Built Wasm artifacts (copied by build)
│   │   │   ├── qseries.wasm
│   │   │   ├── qseries.js
│   │   │   └── qseries.worker.js
│   │   ├── coi-serviceworker.js   # Cross-origin isolation for SharedArrayBuffer
│   │   └── CNAME                  # Custom domain file
│   └── node_modules/              # gitignored
└── .github/
    └── workflows/
        └── deploy-website.yml     # NEW: CI/CD pipeline
```

### What's New vs Modified

| Path | Status | Purpose |
|------|--------|---------|
| `website/` (entire tree) | **NEW** | Astro Starlight project |
| `Makefile` | **MODIFIED** | Add `wasm` and `website` targets |
| `.github/workflows/deploy-website.yml` | **NEW** | CI/CD: build wasm → build site → deploy |
| `.gitignore` | **MODIFIED** | Add `website/node_modules/`, `website/dist/` |

The `src/` directory is **not modified**. The Emscripten build reads the same source files as the native build.

## Emscripten Build Pipeline

### How It Works Alongside g++

The existing build uses `g++ -std=c++20 -O2 -static`. The Emscripten build uses `em++ -std=c++20` with Wasm-specific flags. Both compile the same `src/main.cpp` which includes all headers.

```
                    src/main.cpp + src/*.h
                           │
              ┌────────────┴────────────┐
              ▼                          ▼
    g++ -std=c++20 -O2 -static    em++ -std=c++20 -O3 -sASYNCIFY
              │                          │
              ▼                          ▼
    dist/qseries.exe              website/public/wasm/qseries.wasm
    (native CLI binary)           website/public/wasm/qseries.js
                                  (browser Wasm + JS glue)
```

### Emscripten Compilation Command

```bash
em++ -std=c++20 -O3 \
  -sASYNCIFY \
  -sFORCE_FILESYSTEM \
  -sALLOW_MEMORY_GROWTH=1 \
  -sEXPORTED_RUNTIME_METHODS='["callMain"]' \
  -sMODULARIZE=1 \
  -sEXPORT_NAME='createQSeries' \
  --js-library=website/emscripten-pty.js \
  -o website/public/wasm/qseries.mjs \
  src/main.cpp
```

**Flag rationale:**

| Flag | Why |
|------|-----|
| `-std=c++20` | Match native build; Emscripten supports C++20 via Clang/LLVM |
| `-O3` | Critical with ASYNCIFY — unoptimized builds are 2-3x larger |
| `-sASYNCIFY` | Allows blocking `getline()`/`fgets()` to yield to browser event loop |
| `-sFORCE_FILESYSTEM` | REPL's `isatty()` checks need filesystem emulation |
| `-sALLOW_MEMORY_GROWTH=1` | BigInt computations can use variable memory |
| `-sMODULARIZE=1` | Exports a factory function, not global side effects |
| `-sEXPORT_NAME='createQSeries'` | Clean import: `const mod = await createQSeries(...)` |
| `--js-library=emscripten-pty.js` | xterm-pty's Emscripten integration library |

### ASYNCIFY vs PROXY_TO_PTHREAD

Two approaches exist for handling blocking stdin reads in the browser:

| Approach | Pros | Cons |
|----------|------|------|
| **ASYNCIFY** (recommended) | No SharedArrayBuffer needed, works everywhere, simpler deployment | ~70% Wasm size overhead, output not flushed during long computations |
| **PROXY_TO_PTHREAD** | No size overhead, output streams in real-time | Requires SharedArrayBuffer → needs COOP/COEP headers → needs coi-serviceworker on GitHub Pages |

**Recommendation: Start with ASYNCIFY.** It works on all browsers without special headers. The qseries REPL is interactive (short computations, then output) so the output-flushing limitation rarely matters. If binary size becomes a problem (>5MB), revisit PROXY_TO_PTHREAD.

If switching to PROXY_TO_PTHREAD later, add `coi-serviceworker.js` to `website/public/` and a `<script>` tag in the layout. The architecture supports both without structural changes.

### Source Compatibility Considerations

The existing `src/repl.h` uses `isatty()` to distinguish interactive vs script mode:

```cpp
#ifdef _WIN32
#define stdin_is_tty() _isatty(_fileno(stdin))
#else
#define stdin_is_tty() isatty(STDIN_FILENO)
#endif
```

Under Emscripten with xterm-pty, `isatty()` returns `true` for the PTY-connected stdin — this is the correct behavior. The REPL will show the banner, prompt, and timing as if running in a real terminal. **No source modifications needed** for the basic Wasm build.

The raw terminal input functions (`readLineRaw` with arrow keys, history) use platform-specific APIs that won't work under Emscripten. xterm-pty provides its own line discipline (input echo, line editing) so the REPL's non-TTY fallback path (`std::getline`) won't be hit. If `readLineRaw` uses `read(STDIN_FILENO, ...)` or Windows console APIs, a compile-time `#ifdef __EMSCRIPTEN__` guard may be needed to fall back to `std::getline`. This is a minor, localized change.

## Astro Starlight Project Structure

### Core Configuration

`website/astro.config.mjs`:
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
        { label: 'Getting Started', link: '/getting-started/' },
        {
          label: 'Manual',
          autogenerate: { directory: 'manual' },
        },
        {
          label: 'Function Reference',
          autogenerate: { directory: 'reference' },
        },
        {
          label: 'Tutorial',
          autogenerate: { directory: 'tutorial' },
        },
        { label: 'Playground', link: '/playground/' },
      ],
    }),
    react(),
  ],
});
```

### Content Sources

| Content | Source | Format |
|---------|--------|--------|
| User manual | Existing `MANUAL.md` (split into sections) | MDX |
| Function reference | Generated from `help()` strings in `repl.h` | MDX |
| Tutorial/examples | Existing `qseriesdoc.md` (Garvan tutorial) | MDX |
| Playground | Custom Astro page with React island | `.astro` + `.tsx` |

MDX is used instead of plain Markdown because the playground page and some reference pages need interactive components.

## Wasm ↔ Browser Terminal Wiring

### Component Stack

```
┌─────────────────────────────────────────────┐
│  QSeriesPlayground.tsx (React component)     │
│                                              │
│  ┌──────────────────────────────────────┐   │
│  │  xterm.js Terminal                    │   │
│  │  - Renders terminal UI in a <div>     │   │
│  │  - Handles keyboard input             │   │
│  │  - Renders ANSI output                │   │
│  └──────────┬───────────────────────────┘   │
│             │ Terminal.onData / Terminal.write│
│  ┌──────────▼───────────────────────────┐   │
│  │  xterm-pty (PTY layer)                │   │
│  │  - master ↔ slave pair                │   │
│  │  - Line discipline (echo, editing)    │   │
│  │  - Terminal signal handling (Ctrl+C)  │   │
│  └──────────┬───────────────────────────┘   │
│             │ emscripten-pty.js integration  │
│  ┌──────────▼───────────────────────────┐   │
│  │  Emscripten Runtime                   │   │
│  │  - qseries.wasm (compiled REPL)       │   │
│  │  - qseries.js (JS glue code)         │   │
│  │  - Asyncify pause/resume on stdin     │   │
│  └──────────────────────────────────────┘   │
└─────────────────────────────────────────────┘
```

### Data Flow

**User types `etaq(q,1,20)` + Enter:**
1. xterm.js captures keystrokes → sends to PTY master
2. PTY line discipline echoes characters, buffers until Enter
3. PTY slave makes buffered line available to `read()` syscall
4. Emscripten runtime (paused via Asyncify) resumes, `getline()` returns the line
5. C++ REPL parses and evaluates → writes result to stdout
6. Emscripten runtime's patched `write()` → PTY slave → line discipline → PTY master → xterm.js renders output

### React Component Architecture

```
website/src/components/QSeriesPlayground.tsx
```

```tsx
// Pseudocode — structural outline, not final implementation
import { useEffect, useRef } from 'react';
import { Terminal } from '@xterm/xterm';
import { FitAddon } from '@xterm/addon-fit';
import { openpty } from 'xterm-pty';

export default function QSeriesPlayground() {
  const termRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    const term = new Terminal({ cursorBlink: true, fontSize: 14 });
    const fitAddon = new FitAddon();
    term.loadAddon(fitAddon);
    term.open(termRef.current!);
    fitAddon.fit();

    const { master, slave } = openpty();
    term.loadAddon(master);

    // Load and run the Wasm REPL
    import('/wasm/qseries.mjs').then(({ default: createQSeries }) => {
      createQSeries({
        pty: slave,           // xterm-pty slave for I/O
        // Emscripten module overrides handled by emscripten-pty.js
      }).then(module => {
        module.callMain([]);  // Start the REPL
      });
    });

    return () => { term.dispose(); };
  }, []);

  return <div ref={termRef} style={{ height: '500px' }} />;
}
```

### Playground Page

```
website/src/pages/playground.astro
```

```astro
---
import StarlightPage from '@astrojs/starlight/components/StarlightPage.astro';
import QSeriesPlayground from '../components/QSeriesPlayground';
---

<StarlightPage frontmatter={{ title: 'Interactive Playground', description: 'Try qseries in your browser' }}>
  <QSeriesPlayground client:only="react" />
</StarlightPage>
```

**`client:only="react"`** is critical: the playground has no server-side representation (xterm.js, Wasm loading are browser-only). This directive skips SSR entirely and only hydrates in the browser.

## Build and Deployment Pipeline

### Makefile Additions

```makefile
# Wasm build (requires emsdk activated)
wasm: website/public/wasm/qseries.mjs

website/public/wasm/qseries.mjs: src/main.cpp src/*.h
	mkdir -p website/public/wasm
	em++ -std=c++20 -O3 \
	  -sASYNCIFY -sFORCE_FILESYSTEM -sALLOW_MEMORY_GROWTH=1 \
	  -sMODULARIZE=1 -sEXPORT_NAME='createQSeries' \
	  --js-library=website/emscripten-pty.js \
	  -o website/public/wasm/qseries.mjs \
	  src/main.cpp

# Website dev server (for local development)
website-dev:
	cd website && npm run dev

# Website production build
website-build: wasm
	cd website && npm run build
```

### GitHub Actions Pipeline

```
.github/workflows/deploy-website.yml
```

```yaml
name: Deploy Website
on:
  push:
    branches: [main]
  workflow_dispatch:

permissions:
  contents: read
  pages: write
  id-token: write

concurrency:
  group: pages
  cancel-in-progress: true

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      # 1. Checkout
      - uses: actions/checkout@v4

      # 2. Setup Emscripten
      - uses: mymindstorm/setup-emsdk@v14

      # 3. Build Wasm binary
      - name: Build qseries.wasm
        run: make wasm

      # 4. Setup Node.js
      - uses: actions/setup-node@v4
        with:
          node-version: 20

      # 5. Install website dependencies
      - name: Install dependencies
        working-directory: website
        run: npm ci

      # 6. Build Astro site
      - name: Build website
        working-directory: website
        run: npm run build

      # 7. Upload artifact
      - uses: actions/upload-pages-artifact@v3
        with:
          path: website/dist

  deploy:
    needs: build
    runs-on: ubuntu-latest
    environment:
      name: github-pages
      url: ${{ steps.deployment.outputs.page_url }}
    steps:
      - id: deployment
        uses: actions/deploy-pages@v4
```

**Build order is strict:** Wasm binary must be built before `npm run build` because Astro copies `public/wasm/*` into the final `dist/` during its build step.

### Local Development Workflow

```bash
# One-time setup
cd website && npm install

# Option A: Full rebuild (after C++ changes)
make wasm              # Rebuild Wasm binary
cd website && npm run dev  # Start Astro dev server with hot reload

# Option B: Docs-only changes (no C++ changes)
cd website && npm run dev  # Wasm binary already in public/wasm/
```

Astro's dev server hot-reloads Markdown and component changes. Wasm binary changes require a manual `make wasm` because Emscripten is not part of the Astro dev pipeline.

## Dependency Summary

### Website npm Dependencies

| Package | Purpose | Required |
|---------|---------|----------|
| `astro` | Static site framework | Yes |
| `@astrojs/starlight` | Documentation theme | Yes |
| `@astrojs/react` | React integration for islands | Yes |
| `react` + `react-dom` | UI framework for playground | Yes |
| `@xterm/xterm` | Terminal emulator component | Yes |
| `@xterm/addon-fit` | Auto-resize terminal to container | Yes |
| `xterm-pty` | PTY bridge for Emscripten I/O | Yes |

### Build-time Dependencies

| Tool | Purpose | Where |
|------|---------|-------|
| `emsdk` / `em++` | Compile C++ to Wasm | CI + local dev |
| Node.js 20+ | Run Astro build | CI + local dev |
| `g++` (existing) | Native binary | Existing — unchanged |

### xterm-pty Integration File

The `emscripten-pty.js` file from the xterm-pty package is needed at Emscripten compile time (passed via `--js-library`). Place it at `website/emscripten-pty.js`. It can be copied from the xterm-pty npm package or downloaded from the xterm-pty GitHub repository.

## Custom Domain Deployment

For GitHub Pages with a custom domain:

1. Set `site: 'https://yourdomain.com'` in `astro.config.mjs`
2. Place a `CNAME` file in `website/public/` containing the domain name
3. Configure DNS: CNAME record pointing to `username.github.io`
4. Enable HTTPS in GitHub Pages settings (automatic with GitHub-managed certs)

No `base` path needed when using a custom domain (unlike `username.github.io/repo-name` which requires `base: '/repo-name'`).

## Integration Points Between Phases

### Dependency Graph

```
Phase: Build Wasm Binary
    │
    ├── Depends on: existing src/*.h + src/main.cpp (no changes)
    │   May need: small #ifdef __EMSCRIPTEN__ in repl.h for readLineRaw
    │
    ▼
Phase: Scaffold Astro Starlight Site
    │
    ├── Independent of Wasm (can be done in parallel)
    │   Produces: website/ skeleton with docs pages
    │
    ▼
Phase: Build Playground Component
    │
    ├── Depends on: Wasm binary exists + Astro site exists
    │   Wires: xterm.js + xterm-pty + qseries.wasm
    │
    ▼
Phase: Write Documentation Content
    │
    ├── Independent (can be done in parallel with playground)
    │   Sources: MANUAL.md, qseriesdoc.md
    │
    ▼
Phase: CI/CD Pipeline
    │
    ├── Depends on: all above working locally
    │   Produces: automated build + deploy
    │
    ▼
Phase: Custom Domain + Polish
    │
    └── Depends on: site deployed to GitHub Pages
```

### Parallelization Opportunities

| Can Run In Parallel | Why |
|---------------------|-----|
| Astro scaffold + Wasm build | No dependency between them |
| Documentation content + Playground component | Content is Markdown; playground is React+Wasm |
| CI pipeline setup + content writing | CI config doesn't block content |

| Must Be Sequential | Why |
|--------------------|-----|
| Wasm build → Playground component | Playground needs the .wasm file to load |
| Astro scaffold → Playground page | Playground page lives inside the Astro project |
| Everything → CI/CD | CI automates what must already work locally |

## Component Boundaries

| Component | Responsibility | Inputs | Outputs | Communicates With |
|-----------|---------------|--------|---------|-------------------|
| **Emscripten build** | Compile C++ to Wasm | `src/*.h`, `src/main.cpp` | `qseries.wasm`, `qseries.js` | Makefile target |
| **Astro Starlight** | Static site generation | MDX content, config | HTML/CSS/JS in `dist/` | Content authors |
| **QSeriesPlayground** | Browser terminal UI | Wasm binary, user input | Terminal display | xterm.js, xterm-pty, Emscripten |
| **xterm-pty** | PTY emulation layer | Keystrokes from xterm.js | Buffered lines to Wasm | xterm.js ↔ Emscripten runtime |
| **coi-serviceworker** | COOP/COEP header injection | (none) | HTTP headers on first load | Only needed if using PROXY_TO_PTHREAD |
| **GitHub Actions** | CI/CD automation | Push to main | Deployed site | emsdk, Node.js, GitHub Pages |

## Sources

- [Astro Starlight Project Structure](https://starlight.astro.build/guides/project-structure) — official docs, HIGH confidence
- [Astro Starlight Custom Pages](https://starlight.astro.build/guides/pages) — StarlightPage wrapper for custom pages
- [Astro Islands Architecture](https://docs.astro.build/ar/concepts/islands/) — client:only, client:load directives
- [Emscripten Asyncify](https://emscripten.org/docs/porting/asyncify.html) — official docs on async C++ in browser
- [Emscripten Pthreads](https://emscripten.org/docs/porting/pthreads.html) — PROXY_TO_PTHREAD alternative
- [xterm-pty GitHub](https://github.com/mame/xterm-pty) — PTY bridge for Emscripten CUI programs, HIGH confidence (VIM demo proves it works)
- [xterm-pty Emscripten integration](https://github.com/mame/xterm-pty#emscripten-integration) — exact compile flags and wiring
- [coi-serviceworker](https://github.com/gzuidhof/coi-serviceworker) — SharedArrayBuffer on static hosts
- [Astro GitHub Pages Deployment](https://docs.astro.build/en/guides/deploy/github) — official withastro/action
- [mymindstorm/setup-emsdk](https://github.com/mymindstorm/setup-emsdk) — GitHub Action for Emscripten in CI
- [Emscripten C++20 Support](https://stackoverflow.com/questions/74508184/emscripten-and-c-20) — confirmed working with -std=c++20

---
*Architecture research for: Q-Series documentation website with Wasm playground*
*Researched: 2026-02-27*
