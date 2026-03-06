# Phase 46: Playground - Research

**Researched:** 2026-02-27
**Domain:** xterm.js + Emscripten Wasm + Astro Starlight integration
**Confidence:** HIGH

## Summary

The playground requires three pieces working together: (1) xterm.js as a terminal UI, (2) the Emscripten-compiled `qseries.wasm` running in a Web Worker, and (3) an Astro Starlight custom page hosting the component. All three technologies are mature and well-documented.

The architecture is straightforward: a custom `.astro` page in `src/pages/playground.astro` uses `StarlightPage` for consistent site chrome, embeds an xterm.js terminal via `client:only`, and communicates with a Web Worker that loads the Wasm module. The main thread handles input collection (line buffering in xterm.js), sends complete expressions to the Worker via `postMessage`, and writes results back to the terminal. The Worker loads the modularized Emscripten module using `importScripts()` + `createQSeries()` and calls `Module.evaluate(expr)`.

**Primary recommendation:** Use xterm.js 5.5.0 (stable, widely deployed), `@xterm/addon-fit` for responsive sizing, a vanilla JS Web Worker (no framework needed), and the `StarlightPage` component for layout. Keep the line-editing logic simple — accumulate characters, handle backspace locally, submit on Enter.

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| `@xterm/xterm` | ^5.5.0 | Terminal emulator UI | Used by VS Code, Hyper, Tabby. v5 is stable; v6 released Dec 2024 with breaking changes — v5 is safer |
| `@xterm/addon-fit` | ^0.10.0 | Auto-resize terminal to container | Official addon, required for responsive layout |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `@xterm/addon-web-links` | ^0.11.0 | Clickable URLs in output | Optional — nice-to-have if output contains URLs |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| xterm.js v5 | xterm.js v6 | v6 has breaking changes (removed `windowsMode`, refactored overview ruler). No benefit for this use case. v5 is battle-tested |
| xterm.js | Custom `<textarea>` + `<pre>` | Would lose proper terminal rendering, ANSI color support, scrollback, selection. Not worth it |
| Web Worker | Main thread Wasm | UI would freeze during long computations (prodmake T=200 can take seconds). Worker is mandatory per requirements |
| Vanilla Worker | Comlink | Comlink adds proxy abstraction. Overkill for one function call (`evaluate`). postMessage is simpler |

**Installation:**
```bash
cd website
npm install @xterm/xterm@^5.5.0 @xterm/addon-fit@^0.10.0
```

## Architecture Patterns

### Recommended Project Structure
```
website/
├── src/
│   ├── pages/
│   │   └── playground.astro        # Custom page using StarlightPage
│   ├── components/
│   │   └── PlaygroundTerminal.astro # Client-side terminal component
│   ├── scripts/
│   │   └── qseries-worker.js       # Web Worker that loads Wasm
│   └── content/docs/
│       └── playground.md            # REMOVE or redirect to /playground/
├── public/
│   └── wasm/
│       ├── qseries.js              # Emscripten glue code
│       └── qseries.wasm            # Compiled Wasm binary
```

### Pattern 1: Starlight Custom Page with StarlightPage
**What:** Create playground as `src/pages/playground.astro` instead of a content doc, using `StarlightPage` for consistent navigation/styling.
**When to use:** When you need client-side interactivity beyond what MDX can offer.
**Example:**
```astro
---
// src/pages/playground.astro
import StarlightPage from '@astrojs/starlight/components/StarlightPage.astro';
import PlaygroundTerminal from '../components/PlaygroundTerminal.astro';
---
<StarlightPage frontmatter={{ title: 'Playground', description: 'Try qseries in your browser' }}>
  <PlaygroundTerminal client:only />
</StarlightPage>
```

**Key insight:** Using `src/pages/` bypasses the content collection system. The existing `src/content/docs/playground.md` should be removed to avoid route conflicts, and the sidebar config in `astro.config.mjs` should link to `/playground/` instead.

### Pattern 2: Web Worker + Emscripten MODULARIZE Message Protocol
**What:** Main thread sends expressions to Worker, Worker calls `Module.evaluate()`, Worker posts result back.
**When to use:** Always — the Wasm module must run off the main thread.
**Example — Worker side (`qseries-worker.js`):**
```javascript
// Worker script — loaded via new Worker()
let Module = null;

self.onmessage = async function(e) {
  const { type, id, expr } = e.data;

  if (type === 'init') {
    importScripts('/wasm/qseries.js');
    Module = await createQSeries({
      locateFile: (path) => '/wasm/' + path
    });
    const banner = Module.get_banner();
    self.postMessage({ type: 'ready', banner });
    return;
  }

  if (type === 'evaluate') {
    try {
      const result = Module.evaluate(expr);
      self.postMessage({ type: 'result', id, result });
    } catch (err) {
      self.postMessage({ type: 'error', id, error: err.message || String(err) });
    }
  }
};
```

**Example — Main thread side:**
```javascript
const worker = new Worker('/playground/qseries-worker.js');

worker.onmessage = (e) => {
  const { type, banner, result, error } = e.data;
  if (type === 'ready') {
    term.writeln(banner);
    term.write('qseries> ');
    hideLoadingSpinner();
  } else if (type === 'result') {
    if (result) term.writeln(result);
    term.write('qseries> ');
    hideComputingIndicator();
  } else if (type === 'error') {
    term.writeln('\x1b[31m' + error + '\x1b[0m');
    term.write('qseries> ');
    hideComputingIndicator();
  }
};

worker.postMessage({ type: 'init' });
```

### Pattern 3: Line-Buffered Input in xterm.js
**What:** Accumulate typed characters in a buffer, handle backspace/delete locally, submit complete line on Enter.
**When to use:** Always — xterm.js provides raw keystrokes, not line-buffered input.
**Example:**
```javascript
let inputBuffer = '';
let cursorPos = 0;

term.onData((data) => {
  if (data === '\r') {
    // Enter pressed — submit line
    term.write('\r\n');
    if (inputBuffer.trim()) {
      showComputingIndicator();
      worker.postMessage({ type: 'evaluate', expr: inputBuffer });
      history.push(inputBuffer);
      historyIndex = history.length;
    } else {
      term.write('qseries> ');
    }
    inputBuffer = '';
    cursorPos = 0;
  } else if (data === '\x7f') {
    // Backspace
    if (cursorPos > 0) {
      inputBuffer = inputBuffer.slice(0, cursorPos - 1) + inputBuffer.slice(cursorPos);
      cursorPos--;
      // Redraw current line
      redrawLine();
    }
  } else if (data === '\x1b[A') {
    // Up arrow — history navigation
    if (historyIndex > 0) {
      historyIndex--;
      inputBuffer = history[historyIndex];
      cursorPos = inputBuffer.length;
      redrawLine();
    }
  } else if (data === '\x1b[B') {
    // Down arrow — history navigation
    if (historyIndex < history.length - 1) {
      historyIndex++;
      inputBuffer = history[historyIndex];
      cursorPos = inputBuffer.length;
      redrawLine();
    } else {
      historyIndex = history.length;
      inputBuffer = '';
      cursorPos = 0;
      redrawLine();
    }
  } else if (data >= ' ') {
    // Printable character
    inputBuffer = inputBuffer.slice(0, cursorPos) + data + inputBuffer.slice(cursorPos);
    cursorPos++;
    redrawLine();
  }
});

function redrawLine() {
  const prompt = 'qseries> ';
  term.write('\r\x1b[K' + prompt + inputBuffer);
  // Position cursor
  const moveBack = inputBuffer.length - cursorPos;
  if (moveBack > 0) term.write('\x1b[' + moveBack + 'D');
}
```

### Pattern 4: Example Dropdown / Preloaded Examples
**What:** A `<select>` or button group above the terminal that inserts pre-built expressions.
**When to use:** Required by success criteria — helps new users explore without typing.
**Example:**
```javascript
const EXAMPLES = [
  {
    name: 'Rogers-Ramanujan (G)',
    commands: [
      'G := sum(q^(n^2)/aqprod(q,q,n), n, 0, 8)',
      'prodmake(G, 50)',
    ]
  },
  {
    name: 'Euler Partition Function',
    commands: [
      'p := 1/aqprod(q,q,50)',
      'prodmake(p, 50)',
    ]
  },
  {
    name: 'Jacobi Theta Functions',
    commands: [
      'theta2(q,100)',
      'theta3(q,100)',
      'theta4(q,100)',
    ]
  },
  {
    name: 'Finding Relations',
    commands: [
      'f1 := jacprodmake(theta2(q,100), q, 100)',
      'f2 := jacprodmake(theta3(q,100), q, 100)',
    ]
  }
];
```

### Anti-Patterns to Avoid
- **Loading Wasm on main thread:** Never call `createQSeries()` outside a Worker. Long computations (prodmake with high truncation) will freeze the UI for seconds.
- **Using xterm.js `onKey` for general input:** Use `onData` instead. `onKey` is lower-level and doesn't handle paste properly. Use `onKey` only if you need access to the DOM KeyboardEvent.
- **Importing xterm.js CSS via `import`:** In Astro, import the CSS file directly in the component or link it in the page head. Vite may not handle the xterm CSS import correctly if done inside a client-side script.
- **Using `client:load` instead of `client:only`:** The terminal component accesses browser APIs (`document`, `window`, `Worker`). `client:load` tries SSR first and will fail. Must use `client:only`.
- **Worker as ES module when using `importScripts`:** The Emscripten-generated `qseries.js` uses `var createQSeries = ...` pattern. Use a classic Worker (not `type: 'module'`) and `importScripts()` to load it, OR place the Wasm glue code in `public/` and use `importScripts('/wasm/qseries.js')`.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Terminal emulator | Custom `<div>` with keyboard listener | xterm.js | Scrollback, selection, ANSI escape codes, Unicode, accessibility — hundreds of edge cases |
| Terminal auto-sizing | Manual column/row calculation | `@xterm/addon-fit` | Must account for font metrics, container padding, device pixel ratio |
| Wasm loading/init | Custom fetch + WebAssembly.instantiate | Emscripten's `createQSeries()` | Handles memory growth, exception support, function binding automatically |

**Key insight:** The terminal UI and the Wasm runtime are both fully solved problems. The only custom code needed is the line-buffering logic and the Worker message protocol, both of which are ~100 lines each.

## Common Pitfalls

### Pitfall 1: Double Initialization of Wasm Module
**What goes wrong:** Calling `createQSeries()` more than once creates a second module instance, wasting memory and losing state from the first.
**Why it happens:** Hot module reloading in dev, or Worker being re-created on component re-mount.
**How to avoid:** Guard with a `let initialized = false` flag in the Worker. Only call `createQSeries()` on the first `init` message.
**Warning signs:** Memory usage climbing, variables disappearing between evaluations.

### Pitfall 2: Worker Script Path in Production
**What goes wrong:** `new Worker('./qseries-worker.js')` works in dev but breaks after `astro build` because Vite hashes filenames.
**Why it happens:** Vite's asset pipeline renames files with content hashes for caching.
**How to avoid:** Either (a) put the worker script in `public/` (not processed by Vite, served as-is) or (b) use Vite's `new Worker(new URL('./worker.js', import.meta.url))` pattern which handles bundling. Option (a) is simpler and recommended since the worker is small.
**Warning signs:** 404 error for worker script in production build.

### Pitfall 3: Wasm File Not Found in Worker
**What goes wrong:** The Emscripten glue code can't locate `qseries.wasm` when running inside a Worker.
**Why it happens:** `locateFile` defaults assume the script and wasm are in the same directory, but the Worker's base URL may differ from where the wasm is served.
**How to avoid:** Always pass `locateFile` to `createQSeries()`: `locateFile: (path) => '/wasm/' + path`. Place both `qseries.js` and `qseries.wasm` in `public/wasm/`.
**Warning signs:** "file not found" or "failed to fetch" errors in the console during Wasm init.

### Pitfall 4: Terminal Not Rendering Until Container Has Size
**What goes wrong:** `term.open(el)` is called before the container element has non-zero dimensions, producing an invisible terminal.
**Why it happens:** With `client:only`, the component renders after page hydration. The container might not be laid out yet.
**How to avoid:** Call `fitAddon.fit()` after `term.open()`, and also on `window resize`. Use `requestAnimationFrame` or a small delay to ensure layout is complete.
**Warning signs:** Terminal container shows as 0x0, or has a single tiny line.

### Pitfall 5: Pasting Multi-line Input
**What goes wrong:** User pastes a multi-line expression. xterm.js fires `onData` with the entire pasted text including `\r` for newlines, which triggers multiple "Enter" submissions.
**Why it happens:** xterm.js normalizes pasted newlines to `\r`.
**How to avoid:** Detect paste by checking if `onData` receives a string longer than 1 character. If so, split on `\r`, take only the first line, or join lines with spaces for single-line evaluation.
**Warning signs:** Garbled output, partial expressions being evaluated.

### Pitfall 6: No Loading State During Wasm Download
**What goes wrong:** User sees a blank terminal for 2-5 seconds while qseries.wasm downloads (the file is ~2-4 MB).
**Why it happens:** No visual feedback between page load and Worker ready message.
**How to avoid:** Show a "Loading qseries engine..." spinner/overlay on the terminal container. Hide it when the Worker posts `{ type: 'ready' }`. Show a separate "Computing..." indicator when evaluating.
**Warning signs:** Users think the page is broken, leave before Wasm loads.

## Code Examples

### Complete Terminal Initialization
```javascript
import { Terminal } from '@xterm/xterm';
import { FitAddon } from '@xterm/addon-fit';

const term = new Terminal({
  cursorBlink: true,
  fontSize: 14,
  fontFamily: 'ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, monospace',
  theme: {
    background: '#1a1a2e',
    foreground: '#e0e0e0',
    cursor: '#00bcd4',
    cursorAccent: '#1a1a2e',
    selectionBackground: 'rgba(0, 188, 212, 0.3)',
    black: '#1a1a2e',
    brightBlack: '#4a4a6a',
    cyan: '#00bcd4',
    brightCyan: '#4dd0e1',
    green: '#66bb6a',
    brightGreen: '#81c784',
    red: '#ef5350',
    brightRed: '#e57373',
    yellow: '#ffa726',
    brightYellow: '#ffb74d',
  },
  scrollback: 5000,
  convertEol: true,
});

const fitAddon = new FitAddon();
term.loadAddon(fitAddon);
term.open(document.getElementById('terminal-container'));
fitAddon.fit();

window.addEventListener('resize', () => fitAddon.fit());
```

### Worker Lifecycle Management
```javascript
function initWorker() {
  const worker = new Worker('/wasm/qseries-worker.js');
  let pending = null;

  worker.onmessage = (e) => {
    switch (e.data.type) {
      case 'ready':
        onReady(e.data.banner);
        break;
      case 'result':
        onResult(e.data.result);
        break;
      case 'error':
        onError(e.data.error);
        break;
    }
  };

  worker.onerror = (err) => {
    term.writeln('\x1b[31mWorker error: ' + err.message + '\x1b[0m');
  };

  worker.postMessage({ type: 'init' });

  return {
    evaluate(expr) {
      worker.postMessage({ type: 'evaluate', expr });
    },
    terminate() {
      worker.terminate();
    }
  };
}
```

### Astro Component with client:only
```astro
---
// PlaygroundTerminal.astro — this is a wrapper
// The actual logic must be in a <script> tag or imported JS
---
<div id="playground-wrapper">
  <div id="toolbar">
    <select id="examples">
      <option value="">Load an example...</option>
      <option value="rogers-ramanujan">Rogers-Ramanujan</option>
      <option value="prodmake">Product Conversion</option>
      <option value="theta">Theta Functions</option>
      <option value="relations">Finding Relations</option>
    </select>
    <span id="status">Loading...</span>
  </div>
  <div id="terminal-container"></div>
</div>

<style>
  #playground-wrapper {
    display: flex;
    flex-direction: column;
    gap: 0.5rem;
  }
  #toolbar {
    display: flex;
    align-items: center;
    gap: 1rem;
  }
  #terminal-container {
    height: 500px;
    border-radius: 8px;
    overflow: hidden;
  }
</style>

<script>
  import { Terminal } from '@xterm/xterm';
  import { FitAddon } from '@xterm/addon-fit';
  import '@xterm/xterm/css/xterm.css';
  // ... terminal setup and worker init ...
</script>
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| xterm.js v4 (`xterm` npm) | xterm.js v5+ (`@xterm/xterm` scoped) | 2023 | Package name changed; imports differ |
| `term.on('data', ...)` | `term.onData(...)` | xterm.js v4 | Method-based API instead of EventEmitter |
| Global `Module` object | `MODULARIZE` + factory function | Emscripten has supported for years | Factory pattern allows Worker loading |
| `import.meta.url` Worker | Classic Worker + importScripts | Ongoing | ES module Workers still have cross-browser issues; classic Workers are more reliable |

**Deprecated/outdated:**
- `xterm` (unscoped npm package): Renamed to `@xterm/xterm` in v5
- `term.on(event, handler)`: Replaced by `term.onData()`, `term.onKey()` etc.
- `Terminal.applyAddon()`: Replaced by `term.loadAddon(addonInstance)` in v4+

## Open Questions

1. **Worker script bundling vs. public directory**
   - What we know: Worker scripts in `public/` are served as-is, no Vite processing. Scripts using `new Worker(new URL(..., import.meta.url))` get bundled by Vite.
   - What's unclear: Whether the Vite worker bundling path handles `importScripts()` inside the worker correctly for the Emscripten glue code.
   - Recommendation: Use `public/wasm/` for both the worker script and the Wasm files. Simple, no build-tool surprises. The worker script is ~30 lines and doesn't need bundling.

2. **xterm.js v5 vs v6**
   - What we know: v6.0.0 released Dec 2024 with breaking changes. v5.5.0 is stable and battle-tested.
   - What's unclear: Whether v6 has any features we specifically need.
   - Recommendation: Use v5.5.0. No reason to adopt v6 breaking changes for a simple REPL.

3. **Starlight sidebar linking to custom page**
   - What we know: `StarlightPage` gives us the site chrome. The sidebar in `astro.config.mjs` uses `{ slug: 'playground' }` which points to `src/content/docs/playground.md`.
   - What's unclear: Whether sidebar can link to a `src/pages/` route directly.
   - Recommendation: Replace the `{ slug: 'playground' }` with `{ label: 'Playground', link: '/playground/' }` to link to the custom page. Remove `src/content/docs/playground.md`.

## Sources

### Primary (HIGH confidence)
- xterm.js official docs (https://xtermjs.org/docs/) — API, ITheme, fit addon, terminal methods
- xterm.js npm page (@xterm/xterm) — version info, changelog
- Emscripten official docs (https://emscripten.org/docs/compiling/Modularized-Output.html) — MODULARIZE pattern, factory function, locateFile
- Astro Starlight docs (https://starlight.astro.build/guides/pages/) — custom pages, StarlightPage component

### Secondary (MEDIUM confidence)
- Stack Overflow answers on xterm.js input handling — line buffering, backspace, onData vs onKey
- Vite docs on worker options — `new Worker(new URL())` bundling pattern
- xterm-pty and wasm-webterm GitHub repos — reference implementations of xterm.js + Wasm

### Tertiary (LOW confidence)
- Specific xterm.js v6 breaking changes — only from release notes, not tested

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — xterm.js and Emscripten are mature, well-documented technologies
- Architecture: HIGH — Worker + postMessage + terminal is a well-established pattern; Starlight custom pages are documented
- Pitfalls: HIGH — most pitfalls sourced from official docs and verified Stack Overflow answers

**Research date:** 2026-02-27
**Valid until:** 2026-04-27 (60 days — all technologies are stable)
