---
phase: 21-error-messages
verified: 2025-02-25T00:00:00Z
status: passed
score: 3/3 must-haves verified
---

# Phase 21: Error Messages Verification Report

**Phase Goal:** Parse and runtime errors show clearer diagnostics (QOL-07)
**Verified:** 2025-02-25
**Status:** passed
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | Parse errors show line/column (or offset) and expected token | ✓ VERIFIED | parser.h: offsetToLineCol (L26-32), Token.offset (L21), 6 throw sites with "parser: line L, col C:" + kindToExpected or descriptive message (L87, 119-123, 238, 283, 335) |
| 2 | Runtime errors show function name and brief context | ✓ VERIFIED | repl.h: runtimeErr helper (L34-36), ev/evi lambdas catch and rethrow with runtimeErr(name,e.what()) (L243, 247), all dispatchBuiltin throws use runtimeErr(name, msg) |
| 3 | Error messages are actionable (user can locate and fix the problem) | ✓ VERIFIED | Parse: line/col + expected token; Runtime: func name + message; Script mode: "line N:" prefix when !stdin_is_tty() (L936-937) |

**Score:** 3/3 truths verified

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `src/parser.h` | Token offset, offsetToLineCol, throw sites with "line L, col C" | ✓ VERIFIED | Token has size_t offset (L21); offsetToLineCol (L26-32); kindToExpected (L35-47); 6 throw sites all include position and expected/actual info |
| `src/repl.h` | runtimeErr helper, built-in errors prefixed with func name, inputLineNum for script mode | ✓ VERIFIED | runtimeErr (L34-36); dispatchBuiltin uses runtimeErr throughout; inputLineNum (L657/874) incremented (L918), script-mode "line N:" (L936-937) |

### Key Link Verification

| From | To | Via | Status | Details |
|------|-----|-----|--------|---------|
| Tokenizer | Token | assign offset in each return | ✓ WIRED | All Token returns include start/pos as offset (L82-94, L106-116, L72) |
| Parser expect/throws | current token | peek().offset or token.offset | ✓ WIRED | expect() uses t.offset (L237); parseSumCall uses idTok.offset (L282); parsePrimary uses t.offset (L334) |
| dispatchBuiltin ev/evi | runtimeErr | catch and rethrow | ✓ WIRED | ev (L243), evi (L247) both catch and throw runtimeErr(name, e.what()) |
| runRepl catch block | inputLineNum | "line N: " when script mode | ✓ WIRED | L936-937: if (!stdin_is_tty() && inputLineNum > 0) std::cerr << "line " << inputLineNum << ": "; |

### Requirements Coverage

QOL-07: Parse and runtime errors show clearer diagnostics — ✓ SATISFIED

### Anti-Patterns Found

None. No TODO/FIXME/placeholder; no empty or stub implementations; no unwired artifacts.

### Human Verification Required

None. All success criteria are verifiable from code structure and patterns.

---

_Verified: 2025-02-25_
_Verifier: Claude (gsd-verifier)_
