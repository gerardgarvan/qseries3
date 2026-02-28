# Feature Research: v4.0 Core REPL Improvements

**Domain:** ANSI colored output, smart tab completion, session save/load, and clear screen for a mathematical C++20 REPL  
**Researched:** 2026-02-28  
**Confidence:** HIGH (based on IPython, Julia/OhMyREPL, PARI/GP, Mathematica, readline conventions, ANSI standards, and direct codebase analysis)

---

## Current State (What Already Exists)

Before detailing new features, here is what the REPL already has — each new feature must integrate with these:

| Existing Feature | Implementation Detail |
|---|---|
| **Tab completion** | Prefix-match against 53 built-in names + user variables. Single match: inline replace. Multiple matches: list all (no longest-common-prefix, no cycling). |
| **Help system** | `getHelpTable()`: 53 entries, each has `(signature, description)` pair. `help(func)` prints signature + description. `help` lists all names. Levenshtein-based typo suggestions. |
| **History** | In-memory `std::deque<std::string>`, 100 entries max. Up/down arrow navigation. No persistence. No search. No deduplication. |
| **Output formatting** | Unicode-rich: superscript digits, Greek η/τ, subscript digits, ∞ symbol, middle dot. Series, products, eta products, Jacobi products, relations, integers. Zero color — all plain text. |
| **Terminal I/O** | Custom raw mode: `termios` on Unix/Cygwin, Win32 `ReadFile`/`SetConsoleMode` on Windows. ANSI escapes used only for `\033[K` (erase line) and `\033[nD` (cursor left) in input redraw. |
| **Error output** | `std::cerr << "error: " << e.what()` — plain text, no color. |
| **Timing** | Printed after each result in interactive mode: `"0.003s"`. Plain text. |
| **Output suppression** | Trailing `:` suppresses display (Maple-style). |
| **Prompt** | Fixed string `"qseries> "` — no color, no numbering. |

---

## Feature 1: ANSI Colored Output

### Classification: **Table Stakes**

Every modern mathematical REPL uses color. IPython colors prompts, errors, and syntax. Julia/OhMyREPL colors keywords, operators, brackets, and numbers. PARI/GP has built-in `colors` configuration with `darkbg`/`lightbg` presets. A plain-text REPL in 2026 feels unfinished.

### What Mathematicians Expect

Based on IPython, Julia, PARI/GP, and SageMath conventions:

| Output Category | Expected Color | Why | REPL Precedent |
|---|---|---|---|
| **Error messages** | Red / bright red | Universal convention; immediate visual alarm | IPython, Julia, PARI/GP, every shell |
| **Prompt** | Green or cyan, bold | Distinguishes input from output; visually anchors the eye | IPython (green `In[n]:`), Julia (green `julia>`), PARI/GP (configurable) |
| **Timing** | Dim / gray | Metadata, not mathematical content — should recede visually | IPython shows execution time dimmed |
| **Result output** | Default terminal color (white/black) | Mathematical results are the primary content — must be maximally readable | All REPLs keep primary output in default color |
| **"No solution" / info messages** | Yellow or dim | Informational but not an error | IPython warnings in yellow |
| **Help text** | Cyan or default with bold function name | Reference material, should be readable but distinct from results | IPython `?` output, Julia `?` mode |
| **Banner / startup** | Cyan or bold | First impression; version info | IPython, Julia, PARI/GP all color their banners |

### What NOT to Color

**Do NOT syntax-highlight series output** (e.g., coloring coefficients differently from exponents). Mathematical REPLs that try this (OhMyREPL for Julia input) only color *input*, not *output*. Series like `1 + q + q² + 2q³ + 3q⁵ + O(q²⁰)` should remain uniform — mathematicians read these as mathematical expressions, not code. Coloring individual terms would be distracting and unprecedented.

**Do NOT color input syntax.** The REPL uses custom raw-mode character-by-character I/O. Syntax highlighting on input would require re-tokenizing the line on every keystroke and rewriting the `redrawLineRaw` function to emit colored ANSI fragments. This is a significant complexity increase for marginal benefit in a domain-specific (not general-purpose) REPL. IPython achieves this via Pygments + prompt_toolkit — full libraries. PARI/GP does not color input.

### Implementation Approach

**Minimal, high-impact coloring** — color the *frame* (prompt, errors, timing, info), not the *content* (series, products, relations):

```cpp
namespace ansi {
    inline const char* red     = "\033[31m";
    inline const char* green   = "\033[32m";
    inline const char* yellow  = "\033[33m";
    inline const char* cyan    = "\033[36m";
    inline const char* bold    = "\033[1m";
    inline const char* dim     = "\033[2m";
    inline const char* reset   = "\033[0m";
}
```

Gate all color output behind `stdin_is_tty()` — never emit ANSI codes when piped or in script mode. This is critical for `./qseries < script.q > output.txt` workflows.

### Platform Considerations

| Platform | ANSI Support | Action Needed |
|---|---|---|
| **Unix / macOS / Cygwin** | Native in all modern terminals | None — `\033[...m` works directly |
| **Windows 10+ (native cmd/PowerShell)** | Requires `ENABLE_VIRTUAL_TERMINAL_PROCESSING` flag on stdout handle | One-time `SetConsoleMode` call at startup, ~5 lines |
| **Windows Terminal / VS Code terminal** | Native | None |
| **Legacy Windows (pre-10)** | No support | Graceful fallback: detect failure of `SetConsoleMode` → disable colors |
| **Emscripten/WASM** | N/A (xterm.js handles ANSI) | WASM build already uses separate output path; no conflict |

### Complexity: **LOW**

- ~30 lines for an ANSI color namespace
- ~10 lines for Windows VT enable
- ~20 lines to wrap existing `std::cout`/`std::cerr` calls in the REPL loop
- No changes to Series/Frac/BigInt layers
- No changes to parser
- Touch points: `runRepl()` (prompt, error catch, timing), `display()` (optional), banner

### Dependencies on Existing Code

- **`stdin_is_tty()`** — already exists, use as color gate
- **`display()` function** — wrap output calls, or leave untouched (recommended: leave mathematical output uncolored)
- **`runRepl()` main loop** — modify prompt print, error print, timing print
- **Windows `RawModeGuard`** — extend constructor to also enable VT processing on stdout handle

### User Workflow Example

```
 qseries>  f := etaq(1,20)           ← green bold prompt
 1 - q - q² + q⁵ + q⁷ + O(q²⁰)     ← default color (mathematical output)
 0.001s                               ← dim gray (timing)

 qseries>  prodmake(f,20)
 (1-q)(1-q²)(1-q³)...(1-q¹⁹)
 0.002s

 qseries>  foo(3)
 error: unknown built-in 'foo'. Did you mean: etaq?     ← red (error)
```

---

## Feature 2: Smart Tab Completion (Auto-Parens, Arg Hints)

### Classification: **Differentiator** (auto-parens) / **Strong Differentiator** (arg hints)

Auto-parentheses on completion are standard in Julia (OhMyREPL bracket_complete), IPython (Jedi integration), and most IDE environments. Showing argument hints in a terminal REPL is rarer — Julia 1.11 added opt-in hints, IPython shows docstrings on `?`, PARI/GP shows signatures on `Tab`. For a domain-specific math REPL with 53 specialized functions, argument hints are exceptionally valuable because mathematicians often forget argument order (is it `aqprod(a,q,n,T)` or `aqprod(q,a,n,T)`?).

### Sub-Feature 2a: Longest Common Prefix Completion

**Classification: Table Stakes.** Every readline-based REPL does this. Current behavior (list all matches, don't fill common prefix) is noticeably worse than bash/zsh/IPython/Julia.

**Current behavior:**
```
qseries> the[TAB]
theta theta2 theta3 theta4        ← lists all, line unchanged
```

**Expected behavior (readline standard):**
```
qseries> the[TAB]
qseries> theta                    ← fills longest common prefix "theta"
[TAB again]
theta  theta2  theta3  theta4     ← lists all on second Tab
```

**Complexity: LOW** — ~15 lines. Compute `std::string commonPrefix` from matches, fill it on first Tab, list on second Tab. Add a `bool lastWasTab` state flag.

### Sub-Feature 2b: Auto-Parentheses on Function Completion

When a single match completes to a known function name (exists in `getHelpTable()`), automatically append `(` and position cursor inside. If the function takes no arguments (e.g., `version`, `clear_cache()`), append `()`.

**Expected behavior:**
```
qseries> prod[TAB]
qseries> prodmake(                ← appended "(" automatically
```

**Complexity: LOW** — ~10 lines. After single-match completion, check if the completed name is in `getHelpTable()`. If yes, append `(`. For zero-arg functions (identifiable from signature), append `()`.

**Dependency:** `getHelpTable()` — already exists and contains all 53 function signatures.

### Sub-Feature 2c: Argument Signature Hint on Tab Inside Parens

When the user presses Tab inside a function call (cursor after `funcname(` with no prefix), display the function signature as a hint line. This is the PARI/GP approach — simple, no GUI, no popup, just print the signature below.

**Expected behavior:**
```
qseries> etaq([TAB]
  etaq(k) or etaq(k,T) or etaq(q,k,T)     ← hint line printed
qseries> etaq(                              ← line restored, cursor unchanged
```

**Detection logic:** On Tab press, if the prefix being completed is empty AND the character before the scan position is `(`, scan backwards to find the function name. Look it up in `getHelpTable()`. If found, print its signature string as a hint.

**Complexity: MEDIUM** — ~30 lines. Requires:
1. Detecting "inside function call" context (scan back for `(` and extract function name)
2. Looking up the function name in `getHelpTable()`
3. Printing the signature and redrawing the prompt

**Dependency:** `getHelpTable()` signatures — already contain the exact format needed (e.g., `"etaq(k) or etaq(k,T) or etaq(q,k,T)"`). No new metadata needed.

### What NOT to Build

**Do NOT build inline ghost-text suggestions** (gray text ahead of cursor, Fig/Kiro style). This requires per-keystroke prediction, significant rendering complexity in raw mode, and is overkill for 53 functions.

**Do NOT build a dropdown/popup menu.** Terminal REPLs are text-only. Cycling and listing are the standard interaction patterns.

### User Workflow Example (All Sub-Features Combined)

```
qseries> eta[TAB]
qseries> etaq                     ← longest common prefix (etaq vs etamake)
[TAB again]
etamake  etaq                     ← list matches
qseries> etaq[TAB]
qseries> etaq(                    ← single match, auto-paren
[TAB inside empty parens]
  etaq(k) or etaq(k,T) or etaq(q,k,T) — eta product Π(1-q^{kn})
qseries> etaq(                    ← cursor position unchanged
```

---

## Feature 3: Session Save/Load (History Persistence)

### Classification: **Table Stakes** (history persistence) / **Differentiator** (full session save/load)

### Sub-Feature 3a: Persistent History Across Sessions

**Classification: Table Stakes.** IPython persists to SQLite. Julia persists to `~/.julia/logs/repl_history.jl`. PARI/GP persists via `histfile` default. Bash/zsh persist to `~/.bash_history` / `~/.zsh_history`. Losing history on exit is the single most common complaint about custom REPLs.

**Expected behavior:**
- On exit: write history to `~/.qseries_history` (plain text, one command per line)
- On startup: load history from file into `std::deque<std::string>`
- History file location: `$HOME/.qseries_history` (Unix) or `%USERPROFILE%\.qseries_history` (Windows)
- File format: Plain text, one line per entry. No SQLite (zero-dependency constraint). No binary format.
- Cap: 1000 lines (current in-memory cap is 100 — increase to match readline default of 500-1000)
- Deduplication: Optional, but consecutive duplicates should be suppressed (readline default)

**Complexity: LOW** — ~40 lines total:
- ~15 lines: read file on startup, push each line into deque
- ~10 lines: write deque to file on clean exit (in `runRepl()` after loop ends or on SIGINT/EOF)
- ~5 lines: determine file path (`getenv("HOME")` / `getenv("USERPROFILE")`)
- ~10 lines: error handling (file not found on first run, write failure)

**Dependencies:**
- **`history` deque** — already exists, just persist it
- **`maxHistory`** — increase from 100 to 1000
- **`stdin_is_tty()`** — only persist in interactive mode

### Sub-Feature 3b: Session Save Command (`save`)

**Classification: Differentiator.** IPython has `%save`. Mathematica has `Save`/`DumpSave`. PARI/GP has `\w` (write session to file). This is especially valuable for mathematicians who build up long computations and want to reproduce them later.

**Two levels of session save:**

**Level 1: Save input history to file (script-replayable)**

```
qseries> save("mysession.q")
Session saved to mysession.q (23 lines)
```

Writes all history entries to a `.q` file, one per line. The file is directly executable via `./qseries < mysession.q` (script mode already exists). This is the IPython `%save` / PARI/GP `\w` pattern.

**Level 2: Save variable state (workspace)**

```
qseries> save("mywork.q", vars)
Workspace saved to mywork.q (5 variables, 23 commands)
```

Writes variable assignments as executable statements: `f := etaq(1,20)` etc. This is the Mathematica `Save` pattern. More complex because it requires serializing Series/JacFactor values.

**Recommendation: Implement Level 1 only.** Level 2 requires serializing Series (map<int,Frac>) back to constructor expressions, which is fragile. Level 1 (save input history as a replayable script) covers 90% of the use case with 10% of the effort.

**Complexity: LOW (Level 1)** — ~25 lines:
- Parse `save("filename")` as a built-in command
- Write `history` deque to file
- Print confirmation

**Complexity: HIGH (Level 2)** — Would require Series→expression serialization, which doesn't exist and is non-trivial (prodmake output is *display* format, not parseable input).

**Dependencies:**
- **Parser** — add `save` as a recognized built-in (or handle as special command before parsing, like `help`)
- **`history` deque** — iterate and write
- **Script mode** — already exists, so saved files are immediately replayable

### Sub-Feature 3c: Load/Replay Command

```
qseries> load("mysession.q")
```

Read file line by line, execute each through `evalStmt`. This is essentially script mode (`./qseries < file.q`) but invoked from within the REPL. PARI/GP has `\r` for this. IPython has `%run`.

**Complexity: LOW** — ~20 lines. Read file, tokenize/parse/eval each line, print results. Reuse the existing `runRepl` loop body.

**Dependencies:**
- **Parser + eval pipeline** — already exists, just feed lines through it
- **Error handling** — should continue on error (print error, proceed to next line) rather than abort

### User Workflow Example

```
qseries> f := etaq(1,20)
1 - q - q² + q⁵ + q⁷ + O(q²⁰)
qseries> g := prodmake(f,20)
(1-q)(1-q²)...(1-q¹⁹)
qseries> save("euler.q")
Session saved to euler.q (2 lines)

[exit and restart later]

qseries> load("euler.q")
f := etaq(1,20)
1 - q - q² + q⁵ + q⁷ + O(q²⁰)
g := prodmake(f,20)
(1-q)(1-q²)...(1-q¹⁹)
Loaded euler.q (2 lines)
```

---

## Feature 4: Clear Screen

### Classification: **Table Stakes**

Every REPL supports clearing the screen. `Ctrl+L` is the universal keybinding (bash, zsh, IPython, Julia, PARI/GP, Node.js). `clear` or `cls` as a typed command is also common. Not having this feels broken.

### Expected Behavior

| Trigger | Action | Precedent |
|---|---|---|
| `Ctrl+L` keypress | Clear screen, redraw prompt at top | bash, zsh, IPython, Julia, PARI/GP, Node.js REPL |
| `clear` typed command | Clear screen, redraw prompt at top | PARI/GP (`\c`), some shells |

### Implementation

**ANSI approach (preferred, cross-platform):**
```cpp
std::cout << "\033[2J\033[H" << std::flush;
```

- `\033[2J` — erase entire screen
- `\033[H` — move cursor to row 1, column 1

Then redraw the prompt: `redrawLineRaw(line, pos)`.

**Ctrl+L detection:** In `readOneChar()` loop, `Ctrl+L` produces character code 12 (`0x0C`, form feed). Add a case:
```cpp
if (c == 12) {  // Ctrl+L
    std::cout << "\033[2J\033[H" << std::flush;
    redrawLineRaw(line, pos);
    continue;
}
```

**`clear` command:** Handle as a special command in the REPL loop (before parsing), similar to `help`:
```cpp
if (trimmed == "clear") {
    std::cout << "\033[2J\033[H" << std::flush;
    continue;
}
```

### Platform Considerations

| Platform | Behavior |
|---|---|
| **Unix/Cygwin** | ANSI `\033[2J\033[H` works natively |
| **Windows 10+ with VT enabled** | Works once `ENABLE_VIRTUAL_TERMINAL_PROCESSING` is set (same prerequisite as Feature 1) |
| **Legacy Windows** | Fallback: `system("cls")` — functional but spawns a subprocess |

### Complexity: **VERY LOW**

- ~5 lines for `Ctrl+L` in the input loop
- ~3 lines for `clear` command check
- Shares the Windows VT prerequisite with Feature 1 (ANSI colored output) — implement together

### Dependencies on Existing Code

- **`readOneChar()` loop** — add case for char 12
- **REPL loop** — add `"clear"` command check before `parse()`
- **Windows VT processing** — shared with Feature 1; must be enabled for ANSI clear to work

### User Workflow Example

```
qseries> f := etaq(1,20)
1 - q - q² + q⁵ + q⁷ + O(q²⁰)
0.001s
qseries> [Ctrl+L]

[screen clears, prompt reappears at top]
qseries> █
```

---

## Feature Priority Matrix

| Feature | Classification | Complexity | Effort (lines) | Impact | Recommended Phase |
|---|---|---|---|---|---|
| **ANSI colored output** | Table stakes | LOW | ~60 | HIGH — transforms visual impression | Phase 1 (foundation for all visual improvements) |
| **Clear screen (Ctrl+L + `clear`)** | Table stakes | VERY LOW | ~10 | MEDIUM — expected but rarely used | Phase 1 (shares VT prereq with colors) |
| **Longest common prefix completion** | Table stakes | LOW | ~15 | MEDIUM — removes a daily annoyance | Phase 2 |
| **History persistence** | Table stakes | LOW | ~40 | HIGH — losing history is the #1 REPL complaint | Phase 2 |
| **Auto-parens on completion** | Differentiator | LOW | ~10 | MEDIUM — small quality-of-life improvement | Phase 2 |
| **Arg signature hints** | Differentiator | MEDIUM | ~30 | HIGH — exceptional for 53 specialized functions | Phase 3 |
| **`save` command (history export)** | Differentiator | LOW | ~25 | MEDIUM — valuable for reproducibility | Phase 3 |
| **`load` command** | Differentiator | LOW | ~20 | MEDIUM — natural companion to save | Phase 3 |

---

## Anti-Features (Explicitly Do Not Build)

| Anti-Feature | Why Avoid | What to Do Instead |
|---|---|---|
| **Input syntax highlighting** | Requires per-keystroke tokenization + colored `redrawLineRaw`; massive complexity for a 53-function DSL | Keep input as plain text; color the *frame* (prompt, errors) not input |
| **Output syntax highlighting** (coloring coefficients vs exponents) | Mathematical output is read as math, not code; coloring breaks reading flow; no REPL does this | Keep mathematical output in default terminal color |
| **Dropdown/popup completion menu** | Requires cursor positioning, multi-line overwrites, z-ordering in raw terminal; enormous complexity | Use readline-style list-below + common-prefix |
| **SQLite history storage** | Zero-dependency constraint; plain text is simpler, greppable, and editable | Plain text file, one line per entry |
| **Full workspace serialization** (save all variables) | Requires Series → parseable-expression serialization; fragile, hard to maintain | Save input history as replayable script instead |
| **Configurable color themes** | Overkill for a math REPL with 5-6 color uses; adds UI complexity | Hardcode sensible defaults; respect `NO_COLOR` env var |
| **Ghost-text / inline prediction** | Per-keystroke rendering complexity; overkill for 53 functions | Tab-triggered hints only |

---

## Feature Dependencies

```
Feature 1: ANSI Colored Output
  └── Prerequisite: Windows VT processing (ENABLE_VIRTUAL_TERMINAL_PROCESSING)
  └── Used by: Feature 4 (clear screen)
  └── Used by: Feature 2c (colored hint text)

Feature 2: Smart Tab Completion
  ├── 2a (common prefix) ── no dependencies
  ├── 2b (auto-parens) ── depends on: getHelpTable() [exists]
  └── 2c (arg hints) ── depends on: getHelpTable() [exists], 
                          nice-to-have: Feature 1 (dim color for hint text)

Feature 3: Session Save/Load
  ├── 3a (history persistence) ── depends on: history deque [exists]
  ├── 3b (save command) ── depends on: history deque [exists], parser [for built-in dispatch]
  └── 3c (load command) ── depends on: parser + eval pipeline [exists]

Feature 4: Clear Screen
  └── depends on: Feature 1's Windows VT prerequisite (shared)
```

---

## Standards and Conventions to Follow

### NO_COLOR Convention

The `NO_COLOR` environment variable (https://no-color.org/) is the standard way to disable color output. Check `getenv("NO_COLOR")` at startup — if set (to any value), disable all ANSI color output. IPython, Julia, and many CLI tools respect this. Implementation: one boolean flag checked before emitting any escape code.

### History File Convention

| REPL | History File |
|---|---|
| IPython | `~/.ipython/profile_default/history.sqlite` |
| Julia | `~/.julia/logs/repl_history.jl` |
| PARI/GP | `~/.gp_history` |
| Python | `~/.python_history` |
| Node.js | `~/.node_repl_history` |

**Recommendation:** `~/.qseries_history` — follows the `~/.tool_history` convention used by PARI/GP and Python. Simple, discoverable.

### Ctrl+L Convention

Character code 12 (form feed / `^L`). Universal across bash, zsh, IPython, Julia, PARI/GP, Node.js. No REPL omits this.

---

## Complexity Summary by Codebase Layer

| Layer | Files Touched | Nature of Changes |
|---|---|---|
| **repl.h** | Primary target | Prompt coloring, error coloring, timing coloring, Ctrl+L, tab completion improvements, history persistence, save/load commands, clear command |
| **main.cpp** | Minor | Windows VT initialization (before `runRepl()`) |
| **parser.h** | None | No changes needed (save/load handled as special commands, not parsed expressions) |
| **series.h, frac.h, bigint.h** | None | Mathematical core untouched |
| **convert.h, relations.h, qfuncs.h** | None | Algorithm layers untouched |
| **linalg.h** | None | Untouched |

All changes are confined to the REPL presentation layer. The mathematical computation core is not affected by any of these features.

---

## Sources

- IPython coloransi module: https://ipython.readthedocs.io/en/stable/api/generated/IPython.utils.coloransi.html (HIGH confidence)
- IPython history persistence: https://ipython.readthedocs.io/en/8.19.0/api/generated/IPython.core.history.html (HIGH confidence)
- IPython %save/%history: https://ipython.readthedocs.io/en/stable/interactive/reference.html (HIGH confidence)
- Julia REPL docs: https://docs.julialang.org/en/v1/stdlib/REPL (HIGH confidence)
- OhMyREPL bracket completion: https://kristofferc.github.io/OhMyREPL.jl/latest/features/bracket_complete/ (HIGH confidence)
- OhMyREPL syntax highlighting: https://kristofferc.github.io/OhMyREPL.jl/latest/features/syntax_highlighting/ (HIGH confidence)
- PARI/GP defaults (colors, histfile): https://pari.math.u-bordeaux.fr/dochtml/ref/GP_defaults.html (HIGH confidence)
- PARI/GP keyboard shortcuts: https://pari.math.u-bordeaux.fr/dochtml/html-stable/readline.html (HIGH confidence)
- Mathematica Save: https://reference.wolfram.com/language/ref/Save.html (HIGH confidence)
- NO_COLOR standard: https://no-color.org/ (HIGH confidence)
- ANSI escape codes for C++: https://dev.to/ahmedmuneeb/enhancing-your-c-terminal-output-with-ansi-escape-codes-27fg (MEDIUM confidence)
- GNU readline tab completion: https://thoughtbot.com/blog/tab-completion-in-gnu-readline (HIGH confidence)
- Windows VT processing: direct codebase analysis of existing `SetConsoleMode` in `RawModeGuard` (HIGH confidence)
