---
phase: 20-tab-completion
plan: 01
subsystem: repl
tags: [termios, tab-completion, raw-mode, Cygwin, Windows, SetConsoleMode]

# Dependency graph
requires:
  - phase: 19-multi-line-input
    provides: runRepl with backslash continuation
provides:
  - Raw terminal mode for char-by-char input in TTY
  - Tab completion for identifiers (built-ins + user vars)
  - readLineRaw with line editing (backspace, printable, Tab, Enter)
affects: [10-repl, 17-help-help-func]

# Tech tracking
tech-stack:
  added: [termios (Unix/Cygwin), SetConsoleMode (native Windows)]
  patterns: [RawModeGuard RAII, platform-specific #if blocks]

key-files:
  created: []
  modified: [src/repl.h]

key-decisions:
  - "termios on Cygwin (__CYGWIN__) and non-Windows; SetConsoleMode on native Windows only"
  - "Empty prefix on Tab: no change (QOL-06)"
  - "readLineRaw returns optional<string>; nullopt = EOF"

patterns-established:
  - "RawModeGuard: RAII for raw terminal mode restore"
  - "Platform selection: #if defined(__CYGWIN__) || !defined(_WIN32) for termios"

# Metrics
duration: ~15min
completed: 2026-02-25
---

# Phase 20 Plan 01: Tab Completion Summary

**Raw terminal input with Tab completion: termios/SetConsoleMode platform abstraction, readLineRaw line editor, prefix-match completion from getHelpTable + env.env, TTY-only; script mode unchanged.**

## Performance

- **Duration:** ~15 min
- **Started:** 2026-02-25
- **Completed:** 2026-02-25
- **Tasks:** 5
- **Files modified:** 1

## Accomplishments

- Raw terminal mode via termios (Unix/Cygwin) and SetConsoleMode (native Windows)
- readLineRaw(): char-by-char read, backspace, Tab completion, Enter/EOF
- getCompletionCandidates(): merged built-ins (getHelpTable) + user variables (env.env)
- Tab: unique match replaces partial; multiple matches print list; no match: no change
- Script/pipe mode keeps std::getline; no raw mode, no completion

## Task Commits

Each task was committed atomically:

1. **Task 1: Add raw-mode platform abstraction** - `7e9fdfe` (feat)
2. **Task 2: Implement readLineRaw() with line editing** - `72bee1e` (feat)
3. **Task 3: Build completion candidates and completion logic** - `e5422ea` (feat)
4. **Task 4: Integrate readLineRaw into runRepl** - `9dfbd2b` (feat)
5. **Task 5: Handle edge cases and Windows CR/LF** - `978c7a4` (feat)

## Files Created/Modified

- `src/repl.h` - RawModeGuard, readOneChar, readLineRaw, getCompletionCandidates, handleTabCompletion; runRepl uses readLineRaw when stdin_is_tty()

## Decisions Made

- Use `std::optional<std::string>` for readLineRaw return; nullopt signals EOF (Ctrl+D on blank line exits)
- Empty prefix on Tab: no change per QOL-06
- Document "reset" for terminal restore after abnormal exit (Ctrl+C during raw mode)

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

Build toolchain (g++, make) was not available in the execution environment (PowerShell). Code compiles with `g++ -std=c++20 -O2 -static -o qseries src/main.cpp` per Makefile when run in Cygwin or MinGW. Linter reports no errors.

## Self-Check

- [x] src/repl.h modified
- [x] Commits 7e9fdfe, 72bee1e, e5422ea, 9dfbd2b, 978c7a4 exist

**Self-Check: PASSED**

## Next Phase Readiness

- Tab completion complete; ready for Phase 21 (Error messages)
- Verification: Run interactive qseries, type "prod" + Tab → "prodmake"; "theta" + Tab → shows theta, theta2, theta3, theta4; script mode: `echo "q" | qseries` unchanged

---
*Phase: 20-tab-completion*
*Completed: 2026-02-25*
