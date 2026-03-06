---
phase: 108-error-diagnostics
verified: 2026-03-06T00:00:00Z
status: human_needed
score: 6/6 must-haves verified (code); script execution not run
human_verification:
  - test: "Run bash tests/acceptance-error-diagnostics.sh"
    expected: "All 8 checks pass (parse, runtime, script mode)"
    why_human: "Bash not available in verification environment; script exists and targets correct behaviors"
---

# Phase 108: Error Diagnostics Verification Report

**Phase Goal:** Users get clear, actionable error messages that point to the problem location
**Verified:** 2026-03-06
**Status:** human_needed
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| #   | Truth | Status | Evidence |
| --- | ----- | ------ | -------- |
| 1   | Runtime errors follow Maple format: Error, (in funcname) message | ✓ VERIFIED | `repl.h` line 50: `return ("Error, (in " + func + ") " + msg)`. `main_wasm.cpp` lines 45–48 use same format for parser errors. |
| 2   | Argument errors use expectArg with ordinals | ✓ VERIFIED | `repl.h` lines 52–63: `ordinal(n)` and `expectArg(n, name, expected, received)` produce "invalid input: expects its 1st argument...". |
| 3   | Parse errors display source line with caret (GCC/Clang style) | ✓ VERIFIED | `repl.h` lines 102–114: `formatParseErrorWithSnippet` builds `"  " + srcLine + "\n  " + caretLine(col) + "\nError, (in parser)..."` via `getLineAt`, `caretLine`. |
| 4   | Parse errors show line:col in message | ✓ VERIFIED | `formatParseErrorWithSnippet` outputs `"Error, (in parser) line " + line + ", col " + col + ": " + rest`. Parser throws `"parser: line N, col M: message"`. |
| 5   | Script mode prepends line N: or line ?: | ✓ VERIFIED | `repl.h` lines 3005–3008: `if (!stdin_is_tty()) { prefix = (inputLineNum > 0) ? ("line " + std::to_string(inputLineNum) + ": ") : "line ?: "; }`; prefix applied in `std::cerr << prefix << msg`. |
| 6   | acceptance-error-diagnostics.sh exists and passes | ⚠️ HUMAN | Script exists, has shebang and BIN resolution; tests parse (caret, Error in parser, line 1 col N, expected, source), runtime (etaq, invalid input/expects/1st), script (line 2:). Script not executable in verification environment. |

**Score:** 6/6 truths verified in code; 1 item requires human execution check

### Required Artifacts

| Artifact | Expected | Status | Details |
| -------- | -------- | ------ | ------- |
| `src/repl.h` | Maple format, ordinal, expectArg, parse helpers, catch block | ✓ VERIFIED | Lines 49–114: runtimeErr, ordinal, expectArg, parseParserMessage, getLineAt, caretLine, formatParseErrorWithSnippet. Lines 3005–3012: catch uses formatParseErrorWithSnippet for parser, script prefix. |
| `tests/acceptance-error-diagnostics.sh` | Phase 108 acceptance tests | ✓ VERIFIED | 92 lines; tests parse (Error in parser, line 1 col N, expected, source, caret), runtime (etaq, invalid input/expects), script (line 2:). |
| `src/main_wasm.cpp` | Maple format for parser errors | ✓ VERIFIED | Lines 44–48: if `parser: ` prefix, output `Error, (in parser) ` + rest; else output `e.what()`. |
| `src/parser.h` | parser: line N, col M messages | ✓ VERIFIED | Lines 88, 121, 247, 292, 356 use `"parser: line " + line + ", col " + col + ": ..."`. |

### Key Link Verification

| From | To | Via | Status | Details |
| ---- | --- | --- | ------ | ------- |
| etaq arity error | runtimeErr + expectArg | throw std::runtime_error(runtimeErr(name, expectArg(...))) | ✓ WIRED | Line 784 |
| prodmake arity error | runtimeErr + expectArg | Same pattern | ✓ WIRED | Line 869 |
| etamake arity error | runtimeErr + expectArg | Same pattern | ✓ WIRED | Line 1000 |
| Catch block | formatParseErrorWithSnippet | text.compare(0, 8, "parser: ") == 0 | ✓ WIRED | Lines 3010–3011 |
| Parser throws | parser: line N, col M | std::runtime_error("parser: line "...) | ✓ WIRED | parser.h |

### Requirements Coverage

Phase 108 maps to ERR-01, ERR-02, ERR-03, ERR-04, ERR-05 (per plans). Parse errors (snippet + caret + line:col), runtime Maple format, expectArg/ordinals, and script line prefix are implemented.

### Anti-Patterns Found

None. No TODO/FIXME/placeholder; no stubs; implementations are complete.

### Human Verification Required

**Test:** Run `bash tests/acceptance-error-diagnostics.sh` from project root in a Cygwin/bash environment.

**Expected:** All 8 checks pass (parse: Error in parser, line 1 col N, expected, source, caret; runtime: etaq Error, invalid input/expects; script: line 2: prefix).

**Why human:** Bash was not available in the verification environment, so the script could not be executed. Code-level checks confirm the behaviors the script tests are implemented.

### Gaps Summary

None. All 6 truths are implemented in the codebase. Only remaining step is human execution of `acceptance-error-diagnostics.sh` to confirm end-to-end behavior.

---

_Verified: 2026-03-06_
_Verifier: Claude (gsd-verifier)_
