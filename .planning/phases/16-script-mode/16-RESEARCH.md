# Phase 16: Script mode — Research

**Domain:** Non-interactive execution via stdin redirect; TTY detection.

## Summary

Implementation is minimal (~2 lines): wrap the banner output in `if (stdin_is_tty())`. The REPL already has `stdin_is_tty()`, script-mode prompt/echo behavior, and stdin-driven loop. No architectural changes.

---

## Current State

- **repl.h** (lines 19–24): `stdin_is_tty()` already defined — Windows: `_isatty(_fileno(stdin))`, Unix: `isatty(STDIN_FILENO)`
- **runRepl()** (lines 484–514): Banner printed unconditionally before the loop; prompt and command echo already TTY-aware:
  - TTY: print `qseries> ` before reading
  - Non-TTY: echo `qseries> <line>` after reading each line
- **main.cpp**: Calls `runRepl()` directly; no file-argument handling

---

## TTY Detection Behavior

| stdin source            | `stdin_is_tty()` | Script mode? |
|-------------------------|------------------|--------------|
| Terminal (interactive)  | true             | No           |
| `qseries < script.qs`   | false            | Yes          |
| `echo "x:=1" \| qseries`| false            | Yes          |
| `printf '%s\n' ... \| qseries` | false  | Yes          |
| `/dev/null`             | false            | Yes          |

`isatty()` / `_isatty()` return true only for character devices (terminal, console). Pipes and file redirects are not TTYs.

---

## Edge Cases

1. **Cygwin vs native Windows** — Cygwin uses `isatty(STDIN_FILENO)`; native MSVC/MinGW uses `_isatty(_fileno(stdin))`. The existing `#ifdef _WIN32` covers both.
2. **Process substitution** — `qseries < <(cat file.qs)` uses a pipe; stdin is not a TTY.
3. **stdin closed immediately** — `echo "" | qseries` exits on first empty `getline`; no special handling needed.
4. **stdout redirected** — `qseries < script.qs > out.txt` still treats stdin as non-TTY; banner suppression applies.

---

## Banner Suppression

**Decision (locked):** Suppress banner when stdin is not a TTY.

**Implementation:**
```cpp
if (stdin_is_tty())
    std::cout << R"banner(...)banner" << std::endl;
```

---

## Command Echo

**Recommendation:** Keep current behavior — echo `qseries> <line>` when not TTY.

Rationale:
- Scripts like `garvan-demo.sh` can print commands before piping; REPL echo ensures command+result pairs remain visible when piping directly.
- Output format is: `qseries> <command>` followed by result, which supports logging and debugging.
- CONTEXT specifies: keep unless it conflicts with banner suppression; it does not.

---

## Invocation (Claude's Discretion)

- **Primary:** `qseries < script.qs` — stdin redirect (already supported).
- **Optional:** `qseries script.qs` — would require main.cpp to open the file and use it instead of stdin; out of scope for minimal Phase 16.

---

## Error Handling (Claude's Discretion)

- **Recommendation:** Keep continue-on-error (catch per statement, print to stderr, continue loop).
- Scripts can grep stderr for "error:" if needed.
- Exit-on-first-error would break demos that intentionally trigger some failures.

---

## What to PLAN

1. **Change:** Add `if (stdin_is_tty())` around the banner block in `runRepl()`.
2. **Verification:** Run `echo "1+1" | qseries` — no banner; run `qseries` interactively — banner appears.
3. **No changes:** main.cpp, parser, Environment, or command echo logic.
