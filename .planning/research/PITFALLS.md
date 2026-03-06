# Pitfalls Research

**Domain:** REPL UX improvements (ergonomics, errors, help, input) for existing REPL — targeting Maple users
**Project:** qseries3
**Researched:** 2026-03-06
**Confidence:** HIGH (Maple docs verified; REPL pitfalls from community + Python/Julia post-mortems)

## Executive Summary

Adding REPL UX polish to an existing readline-style REPL (history, tab completion, multi-line, errors, help) introduces integration risks that break Maple users’ expectations. The main failure modes: (1) raw terminal mode leaving the shell unusable on Ctrl+C, (2) script vs interactive path divergence, (3) error/help formats that differ from Maple, (4) features not guarding on `stdin_is_tty()`. Prevention requires centralizing TTY checks, adopting Maple-style error messages, and registering signal handlers for raw mode. Phase placement: error/help polish before or alongside ergonomics; raw-mode safeguards in the same phase as line editing.

---

## Critical Pitfalls

### Pitfall 1: Raw Terminal Mode Leaves Shell Broken on Ctrl+C

**What goes wrong:**
When the REPL uses raw terminal mode (termios / SetConsoleMode with ECHO/ICANON disabled) for line editing, Tab completion, and arrow keys, Ctrl+C does not generate SIGINT in the usual way. If the process is killed or crashes before the destructor restores termios, the shell remains in raw mode: no echo, no line buffering, garbage output.

**Why it happens:**
Raw mode typically clears `ISIG` (or equivalent), so Ctrl+C is read as bytes (0x03) instead of raising SIGINT. A crash or `kill -9` bypasses the RAII destructor that restores termios.

**How to avoid:**
- Register `SIGINT` (and optionally `SIGTERM`) handlers that restore termios and re-raise. Use `atexit` or a scoped guard that restores on any exit path.
- Ensure `RawModeGuard` (or equivalent) restores in its destructor and is never bypassed. Python 3.13 fixed a similar bug via deep-copy of `c_cc` before modification.
- Document that users should run `reset` if the terminal is corrupted.

**Warning signs:**
- Terminal stops echoing input after Ctrl+C or crash.
- Output appears only after Enter.
- Tests that pipe input never hit raw mode; only manual testing exposes the issue.

**Phase to address:**
Phase 28 (Arrow-key line navigation) or the phase that introduces/enhances raw mode. Must be addressed before or in the same phase as any feature relying on raw terminal.

---

### Pitfall 2: Script vs Interactive Path Divergence

**What goes wrong:**
Banner, timing, history, tab completion, arrow keys, and raw-mode features are guarded by `stdin_is_tty()`. If a feature is added without this guard, it can: (a) break when `qseries < script.qs` (e.g. timing or banner to non-TTY stdout), or (b) block forever waiting for TTY input (e.g. Tab in raw mode when stdin is a pipe).

**Why it happens:**
Developers test interactively and forget that `stdin_is_tty()` is false in script/CI/pipe scenarios. New code assumes TTY.

**How to avoid:**
- Every REPL feature that depends on TTY must check `stdin_is_tty()` before use.
- Maintain a clear contract: script mode = `getline`, no banner, no timing, no raw-mode, no history load/save.
- Add CI/acceptance tests that run `qseries < script.qs` and assert exit 0 and expected output.

**Warning signs:**
- Tests pass interactively but hang or fail when piped.
- Demo script produces different output than interactive session (e.g. timing lines).
- Feature works in terminal but not in `qseries < demo.qs`.

**Phase to address:**
Phase 16 (Script mode) and any phase adding interactive-only behavior (20 Tab completion, 28 Arrow keys, 18 Timing). Each such phase must add or extend script-mode guards and tests.

---

### Pitfall 3: Error Message Format Breaks Maple Users’ Expectations

**What goes wrong:**
Maple users expect error messages in the form:  
`Error, (in funcname) invalid input: funcname expects its N-th argument, argname, to be of type X, but received Y`  
Generic messages like `runtime_error: etaq: k must be positive` or parser errors without function context feel unfamiliar and less actionable.

**Why it happens:**
The REPL wraps exceptions in `runtimeErr(func, msg)` but parser errors omit function context; many library throws use ad-hoc wording. Maple’s Error Message Guide explicitly structures errors for immediate troubleshooting.

**How to avoid:**
- Parse errors: include line/column and “expected X” (qseries already does this). Add “(in parser)” or similar for consistency.
- Runtime errors: always use `runtimeErr(func, msg)` with the top-level function name. Structure messages as: `expects its N-th argument, name, to be X, but received Y` when applicable.
- Reserve “invalid input” for type/argument errors; use distinct phrasing for “no solution” vs internal errors.

**Warning signs:**
- Error text doesn’t mention the function that failed.
- Users can’t tell which argument is wrong.
- Messages don’t match Maple’s Error Message Guide style.

**Phase to address:**
Phase 21 (Error messages). Ensure both parse and runtime paths produce Maple-compatible messages before or alongside other UX work.

---

### Pitfall 4: Help Format Mismatch with Maple

**What goes wrong:**
Maple supports `?topic` (preferred, no parens) and `help(topic)`. If qseries only supports `help(prodmake)` and Maple users try `?prodmake`, they get a parse error or unexpected behavior. Conversely, if qseries adds `?prodmake` but parses it as something else, it breaks expectations.

**Why it happens:**
Implementing only `help(name)` is simpler. The `?` form requires parser support (special token or command).

**How to avoid:**
- Support both `help` and `help(name)`. If adding `?topic`, ensure the parser treats `?` as a help shortcut and doesn’t conflate it with other operators.
- Keep help content concise: signature, short description, one example. Maple’s `??topic` (calling sequence only) and `???topic` (examples only) are optional but useful patterns.
- `help(unknown)` must return “unknown function” or similar, not a confusing parse error.

**Warning signs:**
- `?etaq` triggers parser error.
- `help(etaq` (unclosed paren) produces an unhelpful parse error instead of a hint.

**Phase to address:**
Phase 17 (help + help(func)). If adding `?` syntax, do it in the same phase or a dedicated “Maple-style help” phase.

---

### Pitfall 5: Tab Completion and History Fight Over the Same Buffer

**What goes wrong:**
Tab completion and history (up/down) both manipulate the current line. If Tab inserts into the wrong position, or history restoration overwrites a partial completion, the user sees jumps, duplication, or lost input.

**Why it happens:**
Tab completion must be cursor-position aware. History restores the full line and sets `pos = line.size()`. If completion logic assumes `pos` is at the end or uses a separate buffer, the two features interfere.

**How to avoid:**
- Use a single `(line, pos)` representation. Tab completion: find word at `pos`, replace with completion, set `pos` to end of completed word.
- History: replace entire line and set `pos = line.size()`. Do not mix in-place completion with history semantics.
- Ensure `redrawLineRaw` is called after every mutation so display matches state.

**Warning signs:**
- Tab in the middle of a word produces wrong completion.
- After Up then Down, the line differs from before.
- Multiple Tab presses behave inconsistently.

**Phase to address:**
Phase 20 (Tab completion) and Phase 31 (Up-down history). Integration tests should cover: Tab at start, middle, end of word; Up/Down then Tab; Tab then Up.

---

### Pitfall 6: Multi-Line Continuation Breaks Script Mode Semantics

**What goes wrong:**
Backslash continuation is designed for interactive use. In script mode, a trailing `\` at EOF or a malformed continuation can cause the REPL to block waiting for more input, or to treat the next “command” as part of the same expression.

**Why it happens:**
Continuation logic runs in both interactive and script modes. In script mode, EOF on stdin means “no more lines”; the REPL should not block.

**How to avoid:**
- On EOF in script mode, treat it as end of input. If the last line ends with `\`, either (a) treat as error “unexpected EOF in continued line” or (b) strip the backslash and use the line as-is, depending on spec.
- Cap continuations (e.g. max 100 lines) to avoid runaway loops on corrupt input.
- Test `echo 'x := etaq(1,50)\' | qseries` and ensure it exits (does not hang).

**Warning signs:**
- Script with trailing `\` hangs.
- Pipe from file that ends mid-expression blocks forever.

**Phase to address:**
Phase 19 (Multi-line input). Verify behavior in script mode and add tests.

---

### Pitfall 7: Semicolon Suppression Inconsistency

**What goes wrong:**
Maple uses trailing semicolon to suppress output. If qseries implements this but (a) still prints timing when output is suppressed, (b) prints blank lines, or (c) behaves differently for assignments vs expressions, users get inconsistent feedback.

**Why it happens:**
Semicolon handling is often added late and only for assignments. Expressions and timing logic may not respect the suppress flag.

**How to avoid:**
- Define a single “suppress output” flag: when set, do not print result, timing, or extra prompts. Apply to both `x := expr;` and bare `expr;`.
- Ensure the parser accepts optional semicolon as a terminator without changing expression semantics.
- Document that `;` suppresses printing (like Maple).

**Warning signs:**
- `x := etaq(1,50);` still shows timing.
- `1+1;` prints nothing but a newline appears in a different way than for `1+1`.

**Phase to address:**
Phase 27 (Suppress output semicolon). Audit all display/timing paths.

---

## Technical Debt Patterns

| Shortcut | Immediate Benefit | Long-term Cost | When Acceptable |
|----------|-------------------|----------------|-----------------|
| Skip TTY check for “simple” feature | Faster implementation | Script mode breaks; hard to debug | Never — always guard |
| Ad-hoc error strings | Quick to add | Inconsistent style; Maple users confused | Never for user-facing errors |
| Parser throws without line/column | Fewer parser changes | Users can’t locate error | Never — offsetToLineCol exists |
| Raw mode without signal handler | One less dependency | Broken terminal on Ctrl+C | Never if raw mode is used |
| Help only for subset of built-ins | Faster ship | Users hit “unknown” for documented functions | Only in MVP; add incrementally |

---

## Integration Gotchas

| Integration | Common Mistake | Correct Approach |
|-------------|----------------|------------------|
| readLineRaw vs getline | Using readLineRaw when !stdin_is_tty() | Always branch: TTY → readLineRaw, else getline |
| Raw mode + Ctrl+C | Assuming destructor always runs | Add SIGINT handler that restores termios, then re-raises |
| Tab + History | Separate buffers or pos-ignorant Tab | Single (line, pos) state; Tab uses pos for word boundary |
| Error catch + display | Catching but printing raw exception text | Format as Maple-style: Error, (in func) message |
| Help table | Forgetting new built-ins | Audit after each phase that adds built-ins; add to help table |

---

## UX Pitfalls (Maple User Expectations)

| Pitfall | User Impact | Better Approach |
|---------|-------------|-----------------|
| Error without function name | “Where did this fail?” | Every runtime error: `(in funcname) message` |
| Error without “expected X, received Y” | Can’t fix argument mistakes | Use Maple’s “expects its N-th argument, name, to be type X, but received Y” |
| `?name` not supported | Muscle memory from Maple fails | Support `?topic` or document that only `help(topic)` works |
| Timing in script output | Demos and CI output are noisy | Only show timing when stdin_is_tty() |
| Banner in piped/script mode | Pollutes programmatic output | Suppress banner when !stdin_is_tty() |
| No line/column on parse error | Hard to fix syntax errors | Always include line and column (or offset) |
| Help(typo) gives parse error | Unhelpful | Return “unknown function: typo” from help built-in |

---

## "Looks Done But Isn't" Checklist

- [ ] **Raw mode:** Often missing SIGINT handler — verify Ctrl+C restores terminal
- [ ] **Script mode:** New features often forget stdin_is_tty() — run `qseries < script.qs` after each change
- [ ] **Error messages:** Often missing “(in func)” — grep for throw and ensure runtimeErr used
- [ ] **Help:** New built-ins often missing from help table — audit after every built-in addition
- [ ] **Tab completion:** Often breaks at cursor in middle of word — test with cursor before last char
- [ ] **Semicolon:** Timing often still prints — verify display and timing both check suppress flag
- [ ] **Multi-line:** EOF in script with trailing `\` can hang — test with piped input

---

## Recovery Strategies

| Pitfall | Recovery Cost | Recovery Steps |
|---------|---------------|----------------|
| Broken terminal (raw mode) | LOW | User runs `reset`; add SIGINT handler in next release |
| Script mode broken | MEDIUM | Add stdin_is_tty() guards, regression tests |
| Error format inconsistency | LOW | Refactor to runtimeErr + structured messages; phase 21 |
| Help gaps | LOW | Add entries to help table; no API change |
| Tab/history conflict | MEDIUM | Unify (line, pos) handling; add integration tests |

---

## Pitfall-to-Phase Mapping

| Pitfall | Prevention Phase | Verification |
|---------|------------------|--------------|
| Raw mode leaves shell broken | Phase 28 (Arrow keys) or raw-mode phase | Ctrl+C during input, then type; terminal should echo |
| Script vs interactive divergence | Phases 16, 18, 20, 28 | `qseries < demo.qs` exits 0, no banner/timing in output |
| Error format | Phase 21 | All errors include “(in X)” and “expects/received” where applicable |
| Help format | Phase 17 | help(name) works; help(typo) returns “unknown function” |
| Tab + history conflict | Phases 20, 31 | Tab at various positions; Up/Down then Tab; no corruption |
| Multi-line EOF hang | Phase 19 | `printf 'x := 1\\' | qseries` exits (does not hang) |
| Semicolon inconsistency | Phase 27 | `x:=1;` shows no output and no timing line |

---

## Phase Ordering Recommendations

1. **Phase 21 (Error messages)** — Do early. Improves every subsequent phase’s failure reporting.
2. **Phase 17 (Help)** — Can run in parallel with 21. Foundation for discoverability.
3. **Phase 28 (Arrow keys) / Raw mode** — Must include SIGINT/termios recovery in the same phase.
4. **Phase 27 (Semicolon)** — Simple but touches display logic; do before heavy ergonomics.
5. **Phase 19 (Multi-line)** — Add script-mode EOF tests in same phase.
6. **Phases 20, 31 (Tab, History)** — Integrate together; test Tab + history interactions.

---

## Sources

- Maple Error Message Guide Overview: https://www.maplesoft.com/support/help/maple/view.aspx?path=ErrorMessageGuideOverview
- Maple help syntax (?topic vs help(topic)): Maple Help
- Python termios Ctrl+C / raw mode: cpython#128330, Python tty.setraw docs, Stack Overflow “Restoring terminal settings on exit”
- REPL line editing and readline pitfalls: Python readline, Julia Evans blog, runebook readline
- Beginner REPL struggles (editing past entries, state): Joe Politz, https://jpolitz.github.io/notes/2020/07/10/repl-problems.html
- OhMyREPL error display patterns: color, reverse stack, numbering
- qseries3 codebase: repl.h (readLineRaw, RawModeGuard, stdin_is_tty), parser.h (offsetToLineCol, throws)
