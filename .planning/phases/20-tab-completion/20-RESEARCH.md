# Phase 20: Tab completion — Research

**Phase goal:** User can autocomplete identifiers and function names.

**Requirements (QOL-06):**
- Press Tab to complete partial identifier (variable or function)
- Unique match: completes; multiple matches: shows options; no match: no change
- Completion works for built-ins and user-defined variables

**Constraint:** Zero external dependencies. No readline, no libedit, no ncurses. C++20, single binary, `g++ -std=c++20 -O2 -static`.

---

## 1. Raw keystroke reading without readline

### Why raw input is required

The REPL currently uses `std::getline(std::cin, line)`, which runs in **canonical (line-buffered) mode**:
- Input is buffered until Enter
- Tab is consumed by the terminal driver or echoed as whitespace; the program never sees it as a special key
- To react to Tab before Enter, the program must read character-by-character in **raw mode**

### Unix / POSIX: termios

**Headers:** `<termios.h>`, `<unistd.h>`

**Steps:**
1. Save current attributes: `tcgetattr(STDIN_FILENO, &orig)`
2. Modify: clear `ICANON` (disable line buffering) and `ECHO` (disable automatic echo)
3. Apply: `tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)`
4. Read: `read(STDIN_FILENO, &c, 1)` — returns one character per call
5. Restore on exit: `atexit(disableRawMode)` or explicit `tcsetattr(..., &orig)`

**Minimal raw mode flags:**
```c
raw.c_lflag &= ~(ICANON | ECHO);
raw.c_cc[VMIN] = 1;   // minimum chars to read
raw.c_cc[VTIME] = 0;  // no timeout
```

`cfmakeraw()` is a convenience function but is a 4.4BSD extension; manually clearing `ICANON | ECHO` is more portable and sufficient for our needs.

**Important:** When echo is disabled, the program must echo printable characters itself (write to stdout).

---

### Windows (native): SetConsoleMode + ReadFile

**Headers:** `<windows.h>` (or `<conio.h>` for `_getch` — see caveats)

**Approach A — SetConsoleMode (recommended for raw line editing):**
```c
HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
DWORD mode;
GetConsoleMode(hStdin, &mode);
mode &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
SetConsoleMode(hStdin, mode);
// Then ReadFile(hStdin, buf, 1, &read, NULL) for each char
```

- `ENABLE_LINE_INPUT` (0x0002): when enabled, input waits for Enter. Disable for char-by-char.
- `ENABLE_ECHO_INPUT` (0x0004): when enabled, input is echoed. Disable so we can echo manually.

**Approach B — _kbhit() / _getch() from `<conio.h>`:**
- `_getch()`: reads one key without echo, blocks until keypress
- `_kbhit()`: non-blocking check for keypress
- Limitation: `_getch()` returns virtual key codes for special keys; Tab is `'\t'` (0x09)
- Available on MSVC and MinGW, but **not on Cygwin** (Cygwin has a stub or no conio)

---

### Cygwin considerations

**Detection:** `#ifdef __CYGWIN__` — Cygwin defines this; native Windows compilers do not.

**Behavior:**
- Cygwin terminals (Mintty, rxvt) use PTYs, not the native Windows console
- `SetConsoleMode` / `GetStdHandle` act on the Windows console handle; in Mintty, stdin is a PTY
- `SetConsoleMode` does **not** affect PTY input — raw mode will not work with Windows API under Mintty
- **Use termios on Cygwin.** Cygwin provides full POSIX termios; `tcgetattr` / `tcsetattr` work correctly with Mintty

**Platform selection logic:**
```cpp
#if defined(__CYGWIN__) || !defined(_WIN32)
  // Unix, Linux, macOS, Cygwin: use termios
  #include <termios.h>
  #include <unistd.h>
#else
  // Native Windows (MSVC, MinGW, cmd.exe): use SetConsoleMode + ReadFile
  #include <windows.h>
#endif
```

**Note:** Cygwin’s g++ does **not** define `_WIN32` by default. Native Windows compilers (MSVC, MinGW) define `_WIN32`. So `_WIN32` alone is not enough to choose termios vs Windows API; Cygwin must be treated as Unix for terminal I/O.

---

## 2. Candidate sources for completion

| Source            | API                          | Contents                          |
|-------------------|------------------------------|-----------------------------------|
| Built-in functions| `getHelpTable()`             | `std::map<std::string, ...>` keys |
| User variables    | `env.env`                    | `std::map<std::string, EnvValue>` keys |

**Identifier rules** (must match parser): start with letter or `_`, then letters, digits, or `_`:
- `std::isalpha(c) || c == '_'` for first char
- `std::isalnum(c) || c == '_'` for rest

Merge both sources into one candidate set. Overlaps (e.g. `"q"` as variable and possibly in help) are fine; use a `std::set<std::string>` to deduplicate.

---

## 3. Completion algorithm

### Step 1: Extract partial identifier

Given current line and cursor position (cursor at end for a minimal implementation):

- Scan backward from cursor
- Include characters while `std::isalnum((unsigned char)c) || c == '_'`
- Stop at the first character that is not part of an identifier
- Result: the partial identifier before the cursor

Example: `"prod"` → `"prod"`, `"f := eta"` → `"eta"`.

### Step 2: Prefix match

Filter candidates: keep only those where `candidate.substr(0, prefix.size()) == prefix` (or `candidate.compare(0, prefix.size(), prefix) == 0`).

### Step 3: Apply completion

| Matches | Action |
|---------|--------|
| 0       | No change (QOL-06: "no match: no change") |
| 1       | Replace partial with full identifier |
| 2+      | Show options; do not alter the line |

### Step 4: Display when multiple matches

Simple options:
- Space-separated on one line: `prodmake  prodmake_coeffs  prodt`
- Newline-separated list (e.g. zsh-style)

Recommendation: space-separated on a new line, then redraw prompt and current line so the user sees the list and can continue typing.

---

## 4. Line editor integration

### Current flow

`runRepl()` uses `std::getline(std::cin, line)` — no per-key handling.

### Required change

Introduce a **custom readline** used only when `stdin_is_tty()` is true:

1. Put terminal in raw mode (termios or SetConsoleMode).
2. Read characters in a loop until Enter:
   - Printable: append to line, echo to stdout.
   - Backspace: remove last char, echo `\b \b` (or redraw line).
   - Tab: run completion; either replace partial or print matches.
   - Enter: exit loop and return line.
3. Restore original terminal mode.
4. Feed the resulting line into the existing parse/eval/display path.

### Minimal behavior for Phase 20

- Handle: printable ASCII, Backspace, Tab, Enter.
- Optional: arrow keys and Ctrl-C can be deferred; arrows as no-op or left unhandled.
- Cursor at end of line only (no internal cursor position) keeps partial extraction trivial.

---

## 5. Implementation considerations

### Terminal mode restoration

- Save original attributes with `tcgetattr` (Unix) or `GetConsoleMode` (Windows).
- Restore via `atexit()` and on normal/error exit paths.
- If the process is killed (e.g. SIGKILL), restoration will not run; document that the user may need to run `reset` or restart the terminal.

### Echo and control characters

- With echo disabled, echo printable characters explicitly.
- Handle Backspace: typical codes are `'\b'` (0x08) or `127` (DEL); support both.
- Tab: ASCII 0x09; treat as completion trigger, not literal tab insert (or support both per spec).

### Script / pipe mode

- Keep using `std::getline` when `!stdin_is_tty()` so scripts and pipes behave unchanged.
- No raw mode, no completion in non-interactive use.

### Static linking

- termios and Windows console APIs are part of the system C runtime / kernel32; no extra libraries.
- `-static` is acceptable for both Unix and native Windows builds.

### Order of candidates

- Use sorted order for consistent display (e.g. iterate over `std::set` or sort before printing).

---

## 6. Recommended approach (zero deps)

| Platform         | Method                             |
|------------------|------------------------------------|
| Unix, Linux, macOS, Cygwin | termios: `ICANON`/`ECHO` off, `read(STDIN_FILENO, &c, 1)` |
| Native Windows   | `SetConsoleMode` + `ReadFile` on stdin handle |

**Completion:**
- Candidates: keys from `getHelpTable()` and `env.env`, merged and deduplicated.
- Matching: prefix match on identifiers.
- Unique match: replace partial with full.
- Multiple matches: print space-separated list on new line, then redraw prompt and line.
- No match: no change.

**Line editor:**
- Replace `std::getline` with a small custom loop in TTY mode.
- Handle: printable, Backspace, Tab, Enter.
- Restore terminal mode on exit.

---

## 7. Gotchas summary

| Issue                  | Mitigation |
|------------------------|------------|
| Cygwin + SetConsoleMode | Use termios on Cygwin; SetConsoleMode does not work with Mintty PTY |
| Echo off leaves terminal broken on crash | Document `reset` / restart; use `atexit` when possible |
| Backspace encoding     | Handle both `\b` (0x08) and DEL (127) |
| Windows CR/LF          | `ReadFile` may return `\r` (0x0D) before `\n`; treat `\r` as Enter or ignore |
| Tab in script mode     | Do not enable raw mode when `!stdin_is_tty()` |
| Common prefix for 2+   | Optional enhancement: complete to common prefix on first Tab, show list on second Tab |

---

*Phase: 20-tab-completion*
*Research completed: 2026-02-25*
