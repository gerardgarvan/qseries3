---
plan: 31-01
status: complete
started: 2026-02-26
completed: 2026-02-26
---

# Plan 31-01 Summary: Up/down arrow history navigation

## What was done
- Changed `readLineRaw` signature to accept `const std::deque<std::string>& history`
- Added `histIdx` (starts at `history.size()`) and `savedLine` locals for navigation state
- Added ESC [ A (up arrow, code 65) handler: saves current line on first press, decrements index, replaces line with history entry, cursor at end
- Added ESC [ B (down arrow, code 66) handler: increments index, restores saved line when past newest entry
- Updated both call sites in `runRepl()` (main input + backslash continuation)
- Created `tests/acceptance-history.sh` with 3 test cases (recall, repeated up, down navigation)
- Added `acceptance-history` Makefile target

## Files modified
- `src/repl.h` — signature change + 2 locals + 2 escape branches + 2 call site updates
- `tests/acceptance-history.sh` — new acceptance test (3 tests)
- `Makefile` — new target

## Verification
- Build: `g++ -std=c++20 -O2 -o dist/qseries.exe src/main.cpp` — success
- All 10 core acceptance tests pass (`make acceptance`)
- All 3 history tests pass (`bash tests/acceptance-history.sh`)
