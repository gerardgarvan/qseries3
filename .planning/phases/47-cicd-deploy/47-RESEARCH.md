# Phase 47: CI/CD & Deployment - Research

**Researched:** 2026-02-27
**Domain:** GitHub Actions CI/CD, Emscripten Wasm builds, Cloudflare Pages deployment
**Confidence:** HIGH

## Summary

The deployment pipeline requires three sequential build stages: (1) compile C++ to WebAssembly via Emscripten, (2) build the Astro Starlight website (which includes the Wasm artifacts), and (3) deploy the built site to Cloudflare Pages via direct upload. All three stages are well-supported by mature GitHub Actions and official tooling.

The key tools are `mymindstorm/setup-emsdk@v14` for Emscripten, `actions/setup-node` for Node.js/npm, and `cloudflare/wrangler-action@v3` for Cloudflare Pages deployment. The Wasm MIME type situation requires a `_headers` file as a safety net — Cloudflare Pages auto-detects MIME types from extensions but `.wasm` support is not explicitly documented. The Cloudflare Pages project must be created manually (one-time setup) before CI can deploy to it.

**Primary recommendation:** Single GitHub Actions workflow with three sequential jobs — wasm-build, site-build, deploy — using artifact passing between jobs and caching for emsdk + npm dependencies.

## Standard Stack

### Core
| Tool | Version | Purpose | Why Standard |
|------|---------|---------|--------------|
| `mymindstorm/setup-emsdk` | `@v14` | Install & cache Emscripten SDK in CI | Only maintained emsdk GitHub Action; 116+ stars, built-in caching |
| `actions/setup-node` | `@v4` | Install Node.js for Astro build | Official GitHub action for Node.js setup with npm caching |
| `cloudflare/wrangler-action` | `@v3` | Deploy to Cloudflare Pages | Official Cloudflare action; 1,700+ stars; v3.14.1+ current |
| `actions/checkout` | `@v4` | Check out repository | Standard GitHub Actions checkout |
| `actions/upload-artifact` / `actions/download-artifact` | `@v4` | Pass Wasm build artifacts between jobs | Official GitHub artifact actions |

### Supporting
| Tool | Version | Purpose | When to Use |
|------|---------|---------|-------------|
| `actions/cache` | `@v4` | Cache emsdk system libraries | Combined with setup-emsdk `actions-cache-folder` for faster builds |
| Emscripten SDK | `3.1.64` or `latest` | C++ → Wasm compiler | Pin a known-good version for reproducibility |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| `mymindstorm/setup-emsdk` | Manual emsdk git clone + install | More control but much more boilerplate; no built-in caching |
| `cloudflare/wrangler-action` | `cloudflare/pages-action` | pages-action is also viable but less actively maintained; wrangler-action is the officially recommended path |
| `wrangler-action` direct upload | Cloudflare Pages git integration | Git integration auto-builds but can't handle the Emscripten step; direct upload is required for custom build pipelines |
| Multi-job workflow | Single job | Single job is simpler but loses parallelism potential and clear failure isolation |

## Architecture Patterns

### Recommended Workflow Structure
```
.github/
└── workflows/
    └── deploy.yml          # Single workflow file
```

### Pattern 1: Sequential Build Pipeline
**What:** Three sequential stages in a single workflow — wasm → site → deploy
**When to use:** When build outputs depend on previous stages (Wasm files must exist before Astro build)
**Example:**
```yaml
# Source: Cloudflare docs + GitHub Actions docs
name: Deploy

on:
  push:
    branches: [main]
  workflow_dispatch:

jobs:
  build-and-deploy:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      deployments: write
    steps:
      - uses: actions/checkout@v4

      # Stage 1: Build Wasm
      - name: Setup Emscripten
        uses: mymindstorm/setup-emsdk@v14
        with:
          version: 3.1.64
          actions-cache-folder: emsdk-cache

      - name: Build Wasm
        run: make wasm

      # Stage 2: Build website
      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: 22
          cache: npm
          cache-dependency-path: website/package-lock.json

      - name: Copy Wasm to website
        run: make wasm-website

      - name: Install website dependencies
        run: npm ci
        working-directory: website

      - name: Build website
        run: npm run build
        working-directory: website

      # Stage 3: Deploy
      - name: Deploy to Cloudflare Pages
        uses: cloudflare/wrangler-action@v3
        with:
          apiToken: ${{ secrets.CLOUDFLARE_API_TOKEN }}
          accountId: ${{ secrets.CLOUDFLARE_ACCOUNT_ID }}
          command: pages deploy website/dist --project-name=qseries
```

### Pattern 2: Single Job (Recommended)
**What:** All steps in one job to avoid artifact upload/download overhead
**When to use:** When total build time is under 15 minutes (likely for this project)
**Why preferred:** Avoids the ~30s overhead of uploading/downloading artifacts between jobs. The Wasm build is ~2-5 min, Astro build ~1-2 min, deploy ~30s. Total well under the 6-hour GitHub Actions limit.

### Anti-Patterns to Avoid
- **Using Cloudflare Pages git integration:** Pages git integration triggers its own build, but it cannot run Emscripten. Since we need a custom build pipeline (em++ first, then Astro), we must use direct upload via wrangler-action.
- **Running `npm install` instead of `npm ci`:** In CI, always use `npm ci` for reproducible builds from the lockfile.
- **Forgetting `make wasm-website` before `npm run build`:** The Astro build copies everything from `public/` into `dist/`. If the Wasm files aren't in `website/public/wasm/` before the build, they won't appear in the deployed site.
- **Not pinning emsdk version:** Using `latest` can cause surprise breakage if Emscripten releases a version that changes codegen behavior.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Emscripten setup | Manual git clone + install script | `mymindstorm/setup-emsdk@v14` | Handles caching, PATH setup, version management |
| Cloudflare deployment | Custom `curl` API calls or `npx wrangler` directly | `cloudflare/wrangler-action@v3` | Handles wrangler install, auth, version management |
| Node.js caching | Manual `actions/cache` for node_modules | `actions/setup-node@v4` with `cache: npm` | Built-in npm cache support, handles lockfile hashing |
| Wasm MIME headers | Server-side function to set Content-Type | `_headers` file in website/public | Zero-code solution, parsed by Cloudflare Pages at deploy time |

**Key insight:** Every component of this pipeline has a purpose-built GitHub Action or Cloudflare feature. Custom scripting should be minimal — mostly just `make wasm`, `make wasm-website`, `npm ci`, `npm run build`.

## Common Pitfalls

### Pitfall 1: Wasm files missing from deployed site
**What goes wrong:** The deployed site loads but the playground shows "Failed to load qseries engine" because `.wasm` and `.js` files are missing from `/wasm/`.
**Why it happens:** `make wasm-website` was not run before `npm run build`, so the Wasm artifacts weren't copied into `website/public/wasm/` before Astro built the site.
**How to avoid:** Ensure the workflow runs `make wasm` → `make wasm-website` → `npm run build` in strict sequence.
**Warning signs:** The Astro build succeeds but `website/dist/wasm/` doesn't contain `qseries.js` and `qseries.wasm`.

### Pitfall 2: Wrong MIME type for .wasm files
**What goes wrong:** Browser refuses to instantiate the Wasm module with an error like "Incorrect response MIME type. Expected 'application/wasm'."
**Why it happens:** Cloudflare Pages auto-detects MIME types from file extensions, but `.wasm` auto-detection is not explicitly guaranteed in documentation.
**How to avoid:** Add a `_headers` file in `website/public/` that explicitly sets the Content-Type for .wasm files. Also verify after first deployment.
**Warning signs:** Playground loads but the Wasm module fails to initialize.

### Pitfall 3: Cloudflare Pages project doesn't exist
**What goes wrong:** First CI deployment fails with "Project not found" or similar error.
**Why it happens:** `wrangler pages deploy` does NOT auto-create projects. The project must be created once before CI can deploy.
**How to avoid:** One-time manual setup: `npx wrangler pages project create qseries --production-branch=main` from a developer machine with Cloudflare credentials.
**Warning signs:** Deploy step fails immediately with an auth/project error on the very first run.

### Pitfall 4: Missing GitHub Secrets
**What goes wrong:** Workflow fails at deploy step with auth errors.
**Why it happens:** `CLOUDFLARE_API_TOKEN` and `CLOUDFLARE_ACCOUNT_ID` not configured in repo settings.
**How to avoid:** Document the one-time setup steps: create API token (Account > API Tokens > "Cloudflare Pages: Edit" permission), find account ID, add both as repository secrets.
**Warning signs:** Deploy step errors with "authentication" or "account" in the error message.

### Pitfall 5: emsdk cache invalidation
**What goes wrong:** Emscripten build takes 5+ minutes every run instead of ~1 minute.
**Why it happens:** Not using `actions-cache-folder` with setup-emsdk, or cache key not matching.
**How to avoid:** Set `actions-cache-folder: emsdk-cache` in the setup-emsdk step. Pin a specific emsdk version so the cache key stays stable.
**Warning signs:** "Installing SDK" appears in logs on every run instead of "Using cached SDK."

### Pitfall 6: package-lock.json not committed
**What goes wrong:** `npm ci` fails with "This command requires an existing lockfile."
**Why it happens:** The website/ directory has `package.json` but `package-lock.json` may not be committed to git.
**How to avoid:** Ensure `website/package-lock.json` is committed to the repository. Run `npm install` locally first to generate it.
**Warning signs:** CI fails at the npm ci step.

## Code Examples

### Complete Workflow File
```yaml
# Source: Cloudflare Pages docs + GitHub Actions docs (verified)
name: Deploy to Cloudflare Pages

on:
  push:
    branches: [main]
  workflow_dispatch:

jobs:
  build-and-deploy:
    runs-on: ubuntu-latest
    permissions:
      contents: read
      deployments: write

    steps:
      - name: Checkout
        uses: actions/checkout@v4

      # --- Wasm Build ---
      - name: Setup Emscripten
        uses: mymindstorm/setup-emsdk@v14
        with:
          version: 3.1.64
          actions-cache-folder: emsdk-cache

      - name: Verify Emscripten
        run: em++ --version

      - name: Build Wasm
        run: make wasm EMXX=em++

      - name: Copy Wasm to website
        run: make wasm-website

      # --- Website Build ---
      - name: Setup Node.js
        uses: actions/setup-node@v4
        with:
          node-version: 22
          cache: npm
          cache-dependency-path: website/package-lock.json

      - name: Install dependencies
        working-directory: website
        run: npm ci

      - name: Build Astro site
        working-directory: website
        run: npm run build

      - name: Verify Wasm in output
        run: |
          ls -la website/dist/wasm/
          test -f website/dist/wasm/qseries.wasm
          test -f website/dist/wasm/qseries.js

      # --- Deploy ---
      - name: Deploy to Cloudflare Pages
        uses: cloudflare/wrangler-action@v3
        with:
          apiToken: ${{ secrets.CLOUDFLARE_API_TOKEN }}
          accountId: ${{ secrets.CLOUDFLARE_ACCOUNT_ID }}
          command: pages deploy website/dist --project-name=qseries
```

### _headers File for Wasm MIME Type
```
# Place in website/public/_headers
/wasm/*.wasm
  Content-Type: application/wasm
  Cache-Control: public, max-age=31536000, immutable

/wasm/*.js
  Cache-Control: public, max-age=31536000, immutable
```

### One-Time Setup Commands
```bash
# 1. Create Cloudflare API token
# Go to: https://dash.cloudflare.com/?to=/:account/api-tokens
# Create token with "Cloudflare Pages: Edit" permission

# 2. Find account ID
# Go to: https://dash.cloudflare.com/ → right sidebar → Account ID

# 3. Create Pages project (one time only)
npx wrangler pages project create qseries --production-branch=main

# 4. Add GitHub secrets
# Go to: GitHub repo → Settings → Secrets and variables → Actions
# Add: CLOUDFLARE_API_TOKEN
# Add: CLOUDFLARE_ACCOUNT_ID

# 5. Ensure package-lock.json exists
cd website && npm install && git add package-lock.json
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| `cloudflare/wrangler-action@2.x` | `cloudflare/wrangler-action@v3` (note `v` prefix) | v3 release (2023+) | v3 dropped wrangler v1 support, global API key auth; requires `v` prefix in action ref |
| `mymindstorm/setup-emsdk@v11` | `mymindstorm/setup-emsdk@v14` | 2024+ | Improved caching, newer emsdk support |
| `actions/setup-node@v3` | `actions/setup-node@v4` | 2024 | Updated runtime, better caching |
| `wrangler pages publish` | `wrangler pages deploy` | Wrangler v3 | `publish` is deprecated, use `deploy` |

**Deprecated/outdated:**
- `wrangler pages publish` — renamed to `wrangler pages deploy`
- `cloudflare/wrangler-action@3.x.x` (without `v` prefix) — no longer valid syntax since v3
- Global API key authentication — removed in wrangler-action v3; use API tokens only

## Open Questions

1. **Exact Wasm MIME type behavior on Cloudflare Pages**
   - What we know: Cloudflare Pages auto-detects MIME types from file extensions; it serves `Content-Type: $CONTENT_TYPE` and `X-Content-Type-Options: nosniff` by default
   - What's unclear: Whether `.wasm` → `application/wasm` is in their auto-detection table (not explicitly documented)
   - Recommendation: Add `_headers` file as defense-in-depth. Verify after first deployment by checking response headers with `curl -I`. LOW risk since most CDNs handle .wasm correctly now.

2. **Emscripten version to pin**
   - What we know: Latest stable is 4.0.6 (as of Feb 2026). The project's Makefile uses `-fwasm-exceptions` which requires a relatively modern emsdk
   - What's unclear: Whether the project was tested with a specific emsdk version during Phase 44-45
   - Recommendation: Check what version was used during Wasm development phases; pin that. If unknown, use `3.1.64` (a well-tested LTS-like version) or the version already tested locally.

3. **Cloudflare Pages project name availability**
   - What we know: The project name (`qseries`) must be unique across all Cloudflare Pages projects globally
   - What's unclear: Whether `qseries` is already taken
   - Recommendation: Try creating the project. If taken, use `qseries-repl` or similar. The project name becomes the subdomain: `qseries.pages.dev`.

## Sources

### Primary (HIGH confidence)
- [Cloudflare Pages Direct Upload with CI docs](https://developers.cloudflare.com/pages/how-to/use-direct-upload-with-continuous-integration) — verified workflow pattern, secrets setup
- [Cloudflare Pages Headers docs](https://developers.cloudflare.com/pages/configuration/headers) — `_headers` file format, URL pattern syntax
- [Cloudflare Pages Serving Pages docs](https://developers.cloudflare.com/pages/configuration/serving-pages/) — default headers including Content-Type auto-detection
- [cloudflare/wrangler-action README](https://github.com/cloudflare/wrangler-action) — v3 syntax, Pages deploy examples, permissions
- [mymindstorm/setup-emsdk README](https://github.com/mymindstorm/setup-emsdk) — v14 usage, caching options

### Secondary (MEDIUM confidence)
- [Emscripten downloads docs](https://emscripten.org/docs/getting_started/downloads.html) — version info (4.0.6 latest stable)
- [GitHub Actions setup-node docs](https://github.com/actions/setup-node) — v4 with npm caching
- Cloudflare community forums — MIME type behavior discussion

### Tertiary (LOW confidence)
- Auto-detection of `.wasm` MIME type — inferred from general Cloudflare behavior but not explicitly confirmed for Pages static assets

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — all tools are official/well-maintained with verified documentation
- Architecture: HIGH — workflow pattern matches official Cloudflare docs exactly; build sequencing is straightforward
- Pitfalls: HIGH — all pitfalls are based on verified documentation (project must pre-exist, secrets required, wasm-website ordering)
- Wasm MIME type: MEDIUM — likely works automatically but adding `_headers` is zero-cost insurance

**Research date:** 2026-02-27
**Valid until:** 2026-04-27 (stable domain; GitHub Actions and Cloudflare Pages APIs change slowly)
