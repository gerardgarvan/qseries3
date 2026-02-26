---
phase: 21-error-messages
plan: 01
subsystem: parser
tags: [parser, error-messages, position-tracking, diagnostics]

# Dependency graph
requires: []
provides:
  - Token with offset for position tracking
  - offsetToLineCol helper for line/column from offset
  - Parse errors with "parser: line L, col C: message" format
  - kindToExpected for human-readable expected token strings
affects: [repl, script-mode]

# Tech tracking
tech-stack:
  added: []
  patterns: [offset-based position tracking, std::runtime_error with formatted position]

key-files:
  created: []
  modified: [src/parser.h]

key-decisions:
  - "Use offset in Token; compute line/col on demand via offsetToLineCol (O(n) scan)"
  - "Throw std::runtime_error with formatted string; no custom ParseError type"
  - "Parser stores inputStr copy for offsetToLineCol lookup when throwing"

patterns-established:
  - "Parse errors include line and column; use peek().offset or consume() result for position"

# Metrics
duration: 15min
completed: 2026-02-25
---

# Phase 21 Plan 01: Parser Position Tracking Summary

**Position-aware parse errors: Token offset, offsetToLineCol helper, and all throw sites report line/column plus expected token**

## Performance

- **Duration:** ~15 min
- **Started:** 2026-02-25
- **Completed:** 2026-02-25
- **Tasks:** 2
- **Files modified:** 1 (src/parser.h)

## Accomplishments

- Token struct carries `size_t offset`; Tokenizer assigns offset to every returned token
- `offsetToLineCol(s, off, line, col)` converts offset to 1-based line/column
- `kindToExpected(Kind)` maps token kinds to human-readable strings (e.g. "')'", "'identifier'")
- Tokenizer throw sites (expected `:=`, unexpected character) include line/col and printable char when useful
- Parser throw sites (expect(), parseSumCall, parsePrimary) include line/col using `peek().offset` or consumed token
- Parse errors now show format: `parser: line L, col C: expected '...'` or `parser: line L, col C: unexpected character 'x'`

## Task Commits

Each task was committed atomically:

1. **Task 1+2: Position infrastructure and throw site updates** - `3a18005` (feat)
   - Both tasks implemented together in src/parser.h (tightly coupled)

**Plan metadata:** `79e3a04` (docs: complete 21-01 plan)

## Files Created/Modified

- `src/parser.h` - Token.offset, offsetToLineCol, kindToExpected; Tokenizer assigns offset and throws with position; Parser stores inputStr, expect/parseSumCall/parsePrimary throw with line/col

## Decisions Made

- Use formatted `std::runtime_error` string instead of custom ParseError type (minimal API change, no REPL changes)
- Compute line/col on demand from offset; no line/col in Token (keeps Token small)

## Deviations from Plan

None - plan executed exactly as written.

## Verification

**Planned verification** (run from Cygwin/bash with g++ in PATH):
```bash
g++ -std=c++20 -O2 -o qseries src/main.cpp
echo "foo : bar" | ./qseries    # expect: parser: line 1, col 6: expected :=
echo "sum(1, 2, 3)" | ./qseries # expect: parser: line 1, col 8: sum/add expects identifier
./tests/acceptance.sh           # must pass
```

**Note:** Build verification was not run in executor environment (g++ not in PATH for PowerShell; Cygwin g++ had shared-library loading issue). Implementation follows plan exactly; user should verify from dev environment.

## Issues Encountered

- Build toolchain not available in executor shell (PowerShell, no g++ in PATH; Cygwin g++ cc1plus load error)

## Next Phase Readiness

- Parse errors show line/column and expected token
- Ready for Phase 21 Plan 02 (runtime error message improvements)

## Self-Check

- [x] src/parser.h exists and contains offset, offsetToLineCol, kindToExpected, position in throws
- [x] Commit 3a18005 exists
- [ ] Build and acceptance tests (deferred to user dev environment)

---
*Phase: 21-error-messages*
*Completed: 2026-02-25*
