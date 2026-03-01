---
plan: 30-01
status: complete
started: 2026-02-25
completed: 2026-02-25
---

# Plan 30-01 Summary: Output on next line after input

## What was done
- Added `std::cout << std::endl;` after `readLineRaw(env)` returns in two places in `runRepl()`:
  1. Main input loop (TTY path)
  2. Backslash continuation loop (TTY path)
- Non-TTY (script/pipe) mode unchanged — it already prints a newline via `std::endl` on the echo line.

## Files modified
- `src/repl.h` — two 1-line insertions

## Verification
- Build: `g++ -std=c++20 -O2 -o dist/qseries.exe src/main.cpp` — success
- All 10 acceptance tests pass (`make acceptance`)
- Piped test `echo '1+1' | ./dist/qseries.exe` shows output on the line after the prompt
