# Feature Research: Documentation Website with Wasm Playground

**Domain:** Mathematical software documentation site with live interactive playground  
**Researched:** 2026-02-27  
**Confidence:** HIGH (based on Rust/Go/Julia playgrounds, SageMathCell, Compiler Explorer, Codapi, Astro Starlight docs, xterm-pty)

## Feature Landscape

### Table Stakes (Users Expect These)

Features users assume any modern documentation site has. Missing these makes the site feel amateur or broken.

#### Site-Level Table Stakes

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| **Responsive layout** | Mathematicians use laptops, desktops, and occasionally tablets; broken mobile = unprofessional | LOW | Starlight provides this out of the box |
| **Dark mode toggle** | Standard on all dev-tool sites since ~2020; math notation reads well in both modes | LOW | Starlight built-in; verify KaTeX renders in both |
| **Full-text search** | Visitors need to find specific functions (e.g., "prodmake") quickly | LOW | Starlight includes Pagefind; zero config |
| **Sidebar navigation** | Manual has 10+ sections; users need persistent nav to jump between topics | LOW | Starlight default; auto-generated from file structure |
| **Code blocks with syntax highlighting** | Code examples are central to a math tool site; unhighlighted code blocks = unfinished | LOW | Starlight + Shiki; no custom highlighting needed (plain text blocks for REPL I/O are fine) |
| **Copy button on code blocks** | Users will paste commands into their local REPL constantly | LOW | Starlight built-in on all code fences |
| **Download / install instructions** | First thing visitors look for; must be prominent | LOW | Static content; link to release binary |
| **Math rendering (KaTeX)** | q-series documentation is meaningless without rendered formulas: Σ, Π, (a;q)_n, θ₃(q) | MEDIUM | Use remark-math + rehype-katex; test all formula types. Known Starlight compatibility issues — may need MathJax fallback |
| **Semantic URLs** | `/docs/functions/prodmake` not `/docs/page-37`; bookmarkable, shareable | LOW | Starlight file-based routing handles this |
| **Fast page loads** | Static site; no excuse for slow loads | LOW | Astro outputs static HTML; Starlight optimizes by default |

#### Playground Table Stakes

| Feature | Why Expected | Complexity | Notes |
|---------|--------------|------------|-------|
| **Code editor with run button** | Core purpose of a playground; Rust/Go/Julia all have this | MEDIUM | Textarea or CodeMirror with a Run button; Wasm executes client-side |
| **Output panel** | Users must see series output, product forms, error messages | LOW | Div below editor; render stdout text |
| **Preloaded examples** | Go Playground, Rust Playground, SageMathCell all ship curated starter examples | LOW | Dropdown or button bar; source from garvan-demo.sh commands |
| **Error display** | Parse errors and runtime errors must surface clearly, not silently fail | LOW | Route stderr to output panel; style errors distinctively (red/monospace) |
| **Client-side execution (Wasm)** | No server costs, no cold starts, instant feedback, privacy (code never leaves browser) | HIGH | Compile qseries with Emscripten to Wasm; bridge stdin/stdout. This is the core technical challenge |
| **Loading indicator** | Wasm binary is ~1-5 MB; users need to know it's loading, not broken | LOW | Spinner or progress bar while Wasm downloads and initializes |
| **Clear / reset** | Users experiment and need to start fresh | LOW | Button that clears editor and output |

---

### Differentiators (Competitive Advantage)

Features that make the qseries site stand out vs. a typical docs-only page.

| Feature | Value Proposition | Complexity | Notes |
|---------|-------------------|------------|-------|
| **Inline "Try it" buttons in docs** | Each example in the manual/tutorial becomes runnable in-place — no copy-paste needed. SageMathCell does this; Codapi proves the pattern. Transforms passive docs into interactive learning | MEDIUM | Each code block gets a "Run" button that sends code to the playground component. Requires a shared Wasm instance or lightweight per-block execution |
| **Full terminal emulation (xterm.js + xterm-pty)** | True REPL experience: multi-line input, up-arrow history, tab completion — all working in browser exactly like the CLI. Julia-wasm and Vim-in-browser prove feasibility | HIGH | xterm-pty bridges xterm.js and Emscripten stdin/stdout/ioctl. Enables interactive REPL, not just batch execution. Significant integration work |
| **Shareable permalinks** | Encode code in URL (base64 or compressed) so users can share computations via link. Go Playground, Rust Playground, SageMathCell all have this. Eliminates screenshots-of-terminals in emails/papers | MEDIUM | Encode editor contents into URL hash or query param. No server needed for client-side approach (LZ-string compression). Limits: URL length ~2KB |
| **Guided tutorial walkthrough** | The Garvan tutorial becomes a step-by-step interactive course: explanatory prose → math formula → runnable example → expected output. No competitor in the q-series space does this | MEDIUM | Sequence garvan-demo.sh sections as tutorial pages. Each section: math context (KaTeX), code (runnable), commentary. Content already exists in qseriesdoc.md |
| **Pre-filled example gallery** | Categorized collection of runnable examples beyond the tutorial: Rogers-Ramanujan, partition function, theta identities, Gauss AGM, Watson modular equation, Rødseth sifting, pentagonal theorem | LOW | JSON/markdown list of {title, code, description}. Source from garvan-demo.sh + MANUAL.md examples. Dropdown in playground |
| **Persistent variables across commands** | The Wasm REPL maintains state: `rr := ...` persists for `prodmake(rr, 40)`. Users build up multi-step computations naturally, just like the CLI REPL | LOW | This comes free if the Wasm binary runs a persistent REPL process via xterm-pty. Batch mode loses this |
| **Zero-install claim** | Landing page can say "Try it now — no download, no install, runs in your browser." Strong differentiator for a niche math tool; Maple/Sage require installation or cloud accounts | LOW | Enabled by Wasm playground; just marketing/messaging |

---

### Anti-Features (Commonly Requested, Often Problematic)

| Feature | Why Requested | Why Problematic | Alternative |
|---------|---------------|-----------------|-------------|
| **User accounts / saved sessions** | "I want to save my work" | Requires backend, database, auth — massive scope creep for an MVP. Privacy/security burden | Shareable permalinks (stateless). Users copy-paste to local files. LocalStorage for draft persistence |
| **Server-side execution** | "Wasm is limited / hard to build" | Ongoing hosting costs, security sandboxing, cold starts, scaling. Go/Rust Playgrounds need dedicated infrastructure teams | Client-side Wasm eliminates all server concerns. qseries has no network/file deps so Wasm is ideal |
| **Collaborative editing** | "Real-time pair programming on math" | Enormous complexity (CRDT/OT, WebSocket server, conflict resolution). Audience is solo researchers, not teams | Share permalinks; async collaboration via email/papers |
| **Full CAS in browser** | "Add Sage/Mathematica-like features" | Scope creep; the tool is deliberately focused on q-series. A general CAS Wasm build would be 100+ MB | Keep the focused 50-function scope. Link to SageMathCell for general CAS needs |
| **PDF export of output** | "Generate publication-ready output" | Complex layout engine; LaTeX output is a prerequisite; not core value | Users copy-paste to their LaTeX documents. Offer copy-as-text |
| **Jupyter notebook integration** | "I want notebooks" | Requires Python kernel, notebook server, Jupyter protocol. Completely different architecture | The REPL IS the interface. Script mode covers batch workflows |
| **Auto-complete in browser editor** | "VS Code-like editor experience" | Monaco/CodeMirror with custom language server is significant work for 50 keywords. xterm.js terminal already has tab completion via the REPL | If using xterm-pty, the CLI's built-in tab completion works in browser for free |
| **Blog / news section** | "Announce updates" | Maintenance burden; goes stale. Academic tools rarely maintain blogs | GitHub releases page for changelogs. Single "What's New" section on landing page |
| **Internationalization (i18n)** | "Translate to other languages" | Math is universal notation. Audience reads English. Translation maintenance cost is high | English only. Mathematical notation transcends language |

---

## Feature Dependencies

```
[Wasm Binary] (Emscripten compile of qseries)
    ├── [Playground: Batch Mode] (textarea + Run button + output div)
    │     ├── [Preloaded Examples] (dropdown populates editor)
    │     ├── [Shareable Permalinks] (encode editor text in URL)
    │     └── [Inline "Try It" Buttons] (docs code blocks → playground)
    │
    └── [Playground: Terminal Mode] (xterm.js + xterm-pty)
          ├── [Persistent Variables] (free with REPL process)
          ├── [Tab Completion in Browser] (free with REPL's built-in)
          └── [History Navigation] (free with REPL's built-in)

[Astro Starlight Site]
    ├── [Landing Page] (static content, feature highlights, download links)
    ├── [Manual Pages] (MANUAL.md → structured Starlight pages)
    │     └── [Math Rendering] (KaTeX via remark-math + rehype-katex)
    ├── [Tutorial Pages] (qseriesdoc sections → guided walkthrough)
    │     ├── requires [Math Rendering]
    │     └── requires [Inline "Try It" Buttons] or [Playground link]
    └── [Playground Page] (embeds Wasm playground component)

[Math Rendering] ──enhances──> [Tutorial Pages], [Manual Pages]

[Inline "Try It"] ──requires──> [Wasm Binary] + [Playground component]

[Terminal Mode] ──conflicts──> [Batch Mode] (choose one for MVP; terminal mode is strictly superior but harder)
```

### Dependency Notes

- **Wasm binary is the critical path:** Everything playground-related depends on a working Emscripten build of qseries. This is the first thing to validate.
- **Terminal mode vs. batch mode is a fork decision:** Batch mode (textarea → run → output) is simpler but loses the interactive REPL experience. Terminal mode (xterm.js + xterm-pty) preserves full REPL fidelity. Both require the Wasm binary. Terminal mode requires additional xterm-pty integration. Recommendation: start with batch mode, upgrade to terminal mode if time permits.
- **Math rendering is independent of playground:** KaTeX/MathJax setup is a separate workstream from Wasm compilation. Can be done in parallel.
- **Inline "Try It" depends on having a working playground:** The code blocks need somewhere to send their code. Build playground first, then wire up inline buttons.
- **Content conversion (MANUAL.md → pages) has no technical deps:** Can begin immediately. It's content work, not engineering.

---

## MVP Definition

### Launch With (v1)

Minimum viable documentation site — validates the concept, gets the tool in front of users.

- [ ] **Landing page** — hero section, feature highlights, download links, "Try it in your browser" CTA
- [ ] **Manual as HTML pages** — MANUAL.md split into Starlight pages: Getting Started, Functions Reference, Examples
- [ ] **Math rendering** — KaTeX for formulas in tutorial/manual pages (Σ, Π, θ, (a;q)_n notation)
- [ ] **Playground page (batch mode)** — code editor, Run button, output panel, 5-8 preloaded examples from garvan-demo.sh
- [ ] **Wasm binary** — qseries compiled via Emscripten, stdin/stdout bridged to editor/output
- [ ] **Dark mode, search, responsive** — all Starlight defaults; verify they work with KaTeX and playground
- [ ] **Shareable permalinks** — encode playground code in URL hash for sharing

### Add After Validation (v1.x)

Features to add once the core site is live and working.

- [ ] **Terminal mode playground (xterm.js + xterm-pty)** — upgrade from batch to full interactive REPL in browser. Trigger: users request multi-step computations, variable persistence
- [ ] **Inline "Try It" buttons** — each code example in manual/tutorial pages gets a Run button. Trigger: analytics show users copy-paste frequently
- [ ] **Guided tutorial walkthrough** — Garvan tutorial as multi-page interactive course. Trigger: core site is stable, content pipeline proven
- [ ] **Example gallery** — categorized collection of 20+ runnable examples with descriptions. Trigger: community contributes examples

### Future Consideration (v2+)

- [ ] **LocalStorage draft persistence** — save in-progress playground code across sessions. Why defer: no user accounts needed; URL permalinks cover sharing
- [ ] **Multiple output formats** — side-by-side series view and product view. Why defer: complex UI, niche value
- [ ] **Embedded playground in external sites** — embeddable iframe/Web Component for use in course materials or papers. Why defer: requires stable API, CORS considerations

---

## Feature Prioritization Matrix

| Feature | User Value | Implementation Cost | Priority |
|---------|------------|---------------------|----------|
| Landing page | HIGH | LOW | P1 |
| Manual → HTML pages | HIGH | LOW | P1 |
| Math rendering (KaTeX) | HIGH | MEDIUM | P1 |
| Wasm binary (Emscripten) | HIGH | HIGH | P1 |
| Playground (batch mode) | HIGH | MEDIUM | P1 |
| Preloaded examples | HIGH | LOW | P1 |
| Error display in playground | MEDIUM | LOW | P1 |
| Shareable permalinks | MEDIUM | LOW | P1 |
| Dark mode / search / responsive | MEDIUM | LOW | P1 (free from Starlight) |
| Terminal mode (xterm.js) | HIGH | HIGH | P2 |
| Inline "Try It" buttons | HIGH | MEDIUM | P2 |
| Guided tutorial walkthrough | MEDIUM | MEDIUM | P2 |
| Example gallery | MEDIUM | LOW | P2 |
| LocalStorage persistence | LOW | LOW | P3 |
| Embeddable widget | LOW | MEDIUM | P3 |

**Priority key:**
- P1: Must have for launch — site is incomplete without these
- P2: Should have, add after core site is stable
- P3: Nice to have, future consideration

---

## Competitor / Comparable Feature Analysis

| Feature | Rust Playground | Go Playground | SageMathCell | Compiler Explorer | PARI/GP Online | Our Approach |
|---------|-----------------|---------------|--------------|-------------------|----------------|--------------|
| **Execution model** | Server-side | Server-side | Server-side | Server-side | Server-side | **Client-side Wasm** (no server) |
| **Code editor** | Ace editor | Simple textarea | CodeMirror | Monaco (VS Code) | Basic textarea | CodeMirror or textarea (MVP) |
| **Preloaded examples** | ✗ (blank start) | ✓ Hello World | ✓ via interact | ✗ (user loads) | ✓ | ✓ Garvan demo examples |
| **Shareable links** | ✓ GitHub Gist | ✓ URL hash | ✓ Permalinks | ✓ Short links | ✗ | ✓ URL hash (client-side) |
| **Math rendering** | N/A | N/A | ✓ (Sage output) | N/A | ✓ (PDF docs) | ✓ KaTeX in docs |
| **Inline doc examples** | ✗ | ✗ | ✓ (embeddable cells) | ✗ | ✗ | ✓ (P2: "Try It" buttons) |
| **Variable persistence** | ✗ (batch) | ✗ (batch) | ✗ (per-cell) | ✗ | ✗ | ✓ (with terminal mode) |
| **Offline capable** | ✗ | ✗ | ✗ | ✗ | ✗ | ✓ (Wasm is fully offline) |
| **Install required** | ✗ | ✗ | ✗ | ✗ | ✗ | ✗ (browser-only) |
| **Focused docs site** | Separate (doc.rust-lang.org) | Separate (go.dev) | Separate (doc.sagemath.org) | N/A | Separate (pari.math.u-bordeaux.fr) | **Integrated** (docs + playground together) |

### Key Competitive Insights

1. **Client-side Wasm is rare.** Nearly all playgrounds use server-side execution. qseries is uniquely suited to client-side Wasm because it has zero dependencies, no file I/O, no network calls — pure computation. This eliminates hosting costs and enables offline use.

2. **Integrated docs + playground is uncommon.** Most projects separate documentation from playground. SageMathCell's embeddable cells are the closest precedent. Having "Try It" buttons inline with documentation would be a genuine differentiator.

3. **No math software project has a modern Starlight-quality docs site.** PARI/GP and SageMath have functional but dated documentation. A polished Starlight site with dark mode, fast search, and rendered math would stand out visually.

4. **Variable persistence is a unique Wasm advantage.** Server-side playgrounds run each submission independently. A persistent Wasm REPL maintains state across commands, matching the real CLI experience.

---

## Existing Content Dependencies

Content that already exists and can be directly transformed:

| Existing Asset | Target Feature | Transformation Needed |
|----------------|----------------|----------------------|
| `MANUAL.md` | Manual HTML pages | Split into sections; add KaTeX for math notation; structure as Starlight pages |
| `demo/garvan-demo.sh` | Preloaded playground examples | Extract individual command groups; add titles and descriptions |
| `qseriesdoc.md` | Tutorial walkthrough pages | Extract sections; add explanatory prose, KaTeX formulas, runnable code blocks |
| `help` / `help(func)` output | Function reference pages | Already structured; cross-reference with manual pages |
| REPL banner / `--version` | Landing page content | Version string, feature list for hero section |
| `tests/acceptance-wins.sh` | Example gallery candidates | Working commands that demonstrate correctness |

---

## Sources

- Rust Playground (play.rust-lang.org) — GitHub Gist sharing, multi-mode compilation, server-side execution
- Go Playground (go.dev/play) — simple textarea UX, deterministic output, sandboxed server execution
- SageMathCell (sagecell.sagemath.org) — embeddable interactive cells, permalink sharing, math output rendering
- Julia-wasm (keno.github.io/julia-wasm) — browser REPL via WebAssembly, xterm.js integration
- Compiler Explorer (godbolt.org) — Monaco editor, 92M annual compilations, server-side architecture
- PARI/GP (pari.math.u-bordeaux.fr) — online calculator, PDF documentation, mathematical focus
- xterm-pty (github.com/mame/xterm-pty) — bridge between xterm.js and Emscripten stdin/stdout; Vim-in-browser demo
- xterm.js (xtermjs.org) — terminal emulation, themes, resize, copy/paste, addon system
- Codapi (codapi.org) — inline "Try It" pattern for documentation; 30+ language support
- LiveCodes (livecodes.io) — embedding best practices, responsive playground design, versioned permalinks
- Astro Starlight (starlight.astro.build) — Pagefind search, dark mode, code components, component overriding
- KaTeX (katex.org) — fast math rendering; known Starlight compatibility issues with rehype-katex
- Emscripten (emscripten.org) — C++ to Wasm compilation; generates JS glue + .wasm binary
- marimo (docs.marimo.io) — Wasm playground with shareable permalinks, LocalStorage persistence

---

*Feature research for: Q-Series documentation website with Wasm playground*  
*Researched: 2026-02-27*
