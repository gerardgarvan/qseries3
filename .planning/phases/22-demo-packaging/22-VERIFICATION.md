---
phase: 22-demo-packaging
verified: 2026-02-25T00:00:00Z
status: passed
score: 3/3 must-haves verified
gaps: []
---

# Phase 22: Demo Packaging Verification Report

**Phase Goal:** Single distributable folder (binary + demo + README) for distribution (QOL-08)

**Verified:** 2026-02-25

**Status:** passed

**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| #   | Truth                                                          | Status     | Evidence                                                                 |
| --- | -------------------------------------------------------------- | ---------- | ------------------------------------------------------------------------ |
| 1   | User can obtain qseries-demo/ containing binary, garvan-demo.sh, README.md | ✓ VERIFIED | Makefile package-demo (lines 44–48) copies all three; qseries-demo/ exists with qseries.exe, garvan-demo.sh, README.md |
| 2   | cd qseries-demo && bash garvan-demo.sh runs the demo regardless of CWD | ✓ VERIFIED | garvan-demo.sh lines 6–11 use DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)", BIN="$DIR/qseries.exe" with fallbacks — script-location resolution |
| 3   | README explains contents, Bash requirement for Windows, and how to run | ✓ VERIFIED | demo/README-dist.md has ## Contents, ## Requirements (Bash — Git Bash, Cygwin, WSL), ## How to Run with cd + bash garvan-demo.sh |

**Score:** 3/3 truths verified

### Required Artifacts

| Artifact            | Expected                                                    | Status | Details                                                                 |
| ------------------- | ----------------------------------------------------------- | ------ | ----------------------------------------------------------------------- |
| `demo/README-dist.md` | Distribution README template with Contents, Requirements, How to run | ✓ VERIFIED | 47 lines; includes Bash requirement, run instructions, optional rebuild |
| `Makefile`          | package-demo target copying binary, garvan-demo.sh, README  | ✓ VERIFIED | Lines 44–48: mkdir, cp binary, cp garvan-demo.sh, cp README-dist.md as README.md |
| `demo/garvan-demo.sh` | DIR-based BIN resolution for flat and nested layouts        | ✓ VERIFIED | BASH_SOURCE[0], BIN=$DIR/qseries.exe, fallbacks for $DIR/../qseries.exe |

### Key Link Verification

| From                | To           | Via                         | Status | Details                                                |
| ------------------- | ------------ | --------------------------- | ------ | ------------------------------------------------------ |
| Makefile            | qseries-demo/| package-demo cp commands    | ✓ WIRED| cp dist/qseries.exe qseries-demo/; cp demo/garvan-demo.sh qseries-demo/; cp demo/README-dist.md qseries-demo/README.md |
| demo/garvan-demo.sh | binary       | DIR-based BIN resolution    | ✓ WIRED| BIN="$DIR/qseries.exe" with fallbacks; run() invokes "$BIN" |

### Requirements Coverage

| Requirement | Status   | Blocking Issue |
| ----------- | -------- | -------------- |
| QOL-08      | ✓ SATISFIED | None — user can obtain single folder (binary + demo script + README) for sharing |

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
| ---- | ---- | ------- | -------- | ------ |
| None | -    | -       | -        | -      |

### Human Verification Required

Optional: run `make package-demo` then `cd qseries-demo && bash garvan-demo.sh` to confirm the demo runs end-to-end. Automated checks show all artifacts and wiring are correct; qseries-demo/ contains demo-output.txt from a prior successful run.

### Gaps Summary

None. All must-haves verified. Phase goal achieved.

---

_Verified: 2026-02-25_
_Verifier: Claude (gsd-verifier)_
