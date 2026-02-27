# Phase 11: Demo artifact — Research

**Researched:** 2026-02-25  
**Domain:** Shell scripting, REPL automation, demo artifact structure  
**Confidence:** HIGH

## Summary

Phase 11 creates a runnable demo script that pipes qseries commands to the REPL binary. The implementation approach is prescribed by locked decisions: a single `demo/garvan-demo.sh` that mirrors `tests/acceptance.sh` for BIN detection and `run()` helper, uses `printf '%s\n' "cmd1" "cmd2" | "$BIN"` for multi-command sequences, and structures content with comment section headers for Phases 12–15 to append Garvan examples.

The REPL prints a banner and `qseries> ` prompt before each line. For a human demo, keeping these is acceptable and provides context; filtering is optional and can be done with `grep -v` if desired. Echo/separator strategy: use `echo` or `echo "---"` between sections for readability. A Makefile target `demo` is recommended for consistency with `test` and `acceptance`.

**Primary recommendation:** Create `demo/garvan-demo.sh` with acceptance.sh-style BIN detection and run helper, minimal Phase 11 content (e.g., one prodmake block), and section comment headers. Add `demo` Makefile target. Keep REPL output unfiltered unless user explicitly requests filtering.

---

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions
- **Primary artifact:** Shell script (.sh) that pipes commands to qseries
- **Location:** demo/garvan-demo.sh (or demo/demo.sh)
- **Structure:** Single growing file; section headers via comments (e.g. `# === Rogers-Ramanujan (qseriesdoc §3.1) ===`)
- **Execution:** `printf '%s\n' "cmd1" "cmd2" | ./qseries` for multi-command sequences (variable state persists)
- **Execution model:** Fully automated; run `bash demo/garvan-demo.sh`; output streams to stdout; no interactive prompts
- **Output:** Demo shows output only; no assertions (that's acceptance.sh)
- **Extensibility:** Phases 12–15 append new sections to the same script
- **Binary location:** Script should locate qseries binary (./qseries or ./qseries.exe) like acceptance.sh

### Claude's Discretion
- Optionally a brief companion `demo/README.md` with intro and copy-paste commands
- Banner handling: keep vs filter (research options)
- Echo/separator strategy for readable output
- Makefile target for demo

### Deferred Ideas (OUT OF SCOPE)
- Demo-as-test (asserting output) — acceptance.sh covers that
- Interactive demo / walkthrough mode
- Separate .qs script format — script-only for v1.1
</user_constraints>

---

## Standard Stack

### Core
| Component | Purpose | Why Standard |
|-----------|---------|--------------|
| Bash script | Demo runner | Matches tests/acceptance.sh, portable on Cygwin/MinGW |
| printf '%s\n' | Multi-line command piping | Correct handling of special chars, one command per line |
| 2>&1 | Merge stderr into stdout | Ensures error output visible when piping |

### Binary Detection (from acceptance.sh)
| Pattern | Purpose |
|---------|---------|
| `BIN="./qseries.exe"` | Prefer .exe on Windows |
| `[ -f qseries.exe ] \|\| BIN="./qseries"` | Fallback to Unix binary |
| `[ -f "$BIN" ] \|\| { echo "error: ..."; exit 1; }` | Fail fast if missing |

**Installation:** None. Uses project Makefile to build `qseries.exe` or `qseries`; demo script uses existing binary.

---

## Architecture Patterns

### Recommended Project Structure
```
demo/
├── garvan-demo.sh   # Single growing script; sections via comments
└── README.md        # Optional: intro, copy-paste commands
```

### Pattern 1: BIN detection and run helper
**What:** Detect binary, define `run()` to pipe commands.
**When to use:** Any script that invokes qseries.
**Example:**
```bash
#!/usr/bin/env bash
# Garvan demo: runs qseries commands for human demonstration.
set -e
BIN="./qseries.exe"
[ -f qseries.exe ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found"; exit 1; }

run() {
    printf '%s\n' "$@" | "$BIN" 2>&1
}
```

### Pattern 2: Section blocks
**What:** Comment header + `run()` call with related commands.
**When to use:** Each Garvan example block (Phases 12–15).
**Example:**
```bash
# === Rogers-Ramanujan (qseriesdoc §3.1) ===
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "prodmake(rr, 40)"
```

### Anti-Patterns to Avoid
- **Using `echo` instead of `printf '%s\n'`:** `echo` can interpret `-n`, `-e`; `printf '%s\n'` is unambiguous.
- **Multiple pipes per block:** Use one `run "cmd1" "cmd2"` per block; variable state persists within that call.
- **Assertions in demo:** Demo is for display only; use acceptance.sh for verification.

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Binary path resolution | Custom logic | acceptance.sh pattern (qseries.exe / qseries) | Already tested, handles Cygwin/MinGW |
| Piping commands | `echo cmd \| qseries` | `printf '%s\n' "$@" \| "$BIN"` | Safe for special chars, multi-line |
| Script execution | Custom launcher | `bash demo/garvan-demo.sh` | Standard, no extra dependencies |

---

## REPL Banner and Prompt Handling

**Current REPL output (src/repl.h):**
- Banner (once at startup):
  ```
    /\  /\
   ( o  o )
   (  __  )
    \____/
   q-series REPL (Maple-like), version 1.0
  ```
- Prompt before each command: `qseries> `

**Options:**

| Option | Pros | Cons | Recommendation |
|--------|------|------|----------------|
| Keep | Shows REPL identity; human-friendly | Adds visual noise | **Default for Phase 11** |
| Filter with grep -v | Cleaner output | Requires maintaining grep pattern; may strip wanted lines | Use only if user requests |

**Filter example (if needed):**
```bash
run "cmd1" "cmd2" | grep -v "^qseries> " | grep -v "q-series REPL"
```
Risk: `q-series REPL` appears in banner; filtering multi-line banner is fragile. Prefer keeping.

---

## Echo / Separator Strategy

**Goal:** Make output readable when multiple sections run.

| Strategy | Implementation | When to Use |
|----------|----------------|-------------|
| Section echo | `echo ""` and `echo "=== Rogers-Ramanujan ==="` before `run` | Between major blocks |
| Blank line | `echo ""` between blocks | Minimal separation |
| No separator | Just run blocks | Minimal output |

**Recommended:** Use `echo ""` and `echo "--- Section Name ---"` before each `run` block. Keeps output structured without changing REPL output.

**Example:**
```bash
echo ""
echo "--- Rogers-Ramanujan (prodmake) ---"
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "prodmake(rr, 40)"
```

---

## Makefile Target

**Current targets (Makefile):** `all`, `test`, `acceptance`, `clean`, `debug`.

**Add:**
```makefile
# Run Garvan demo (human demonstration, no assertions)
demo: qseries.exe
	./demo/garvan-demo.sh
```

**Placement:** After `acceptance` target for consistency.

---

## Common Pitfalls

### Pitfall 1: Wrong working directory
**What goes wrong:** Demo fails with "binary not found" when run from another directory.
**Why it happens:** BIN uses `./qseries`; assumes current dir is project root.
**How to avoid:** Document in README: "Run from project root: `bash demo/garvan-demo.sh`" or `make demo`.
**Warning signs:** `error: qseries binary not found`

### Pitfall 2: Special characters in commands
**What goes wrong:** Quotes or backslashes break when passed via echo.
**Why it happens:** `echo "x := q^2"` can misbehave with certain shells.
**How to avoid:** Use `printf '%s\n' "$@"` and pass each command as a separate argument; avoid shell interpretation.

### Pitfall 3: Demo content in Phase 11
**What goes wrong:** Over-scoping Phase 11 with full Garvan examples.
**Why it happens:** Phases 12–15 add content; Phase 11 establishes structure only.
**How to avoid:** Phase 11 creates skeleton + one minimal block (e.g., Rogers-Ramanujan) to validate structure; defer richer examples to Phases 12–15.

### Pitfall 4: Cygwin vs native Windows
**What goes wrong:** `./qseries` fails on Windows; `./qseries.exe` needed.
**Why it happens:** Windows typically uses .exe extension.
**How to avoid:** BIN detection already handles both; ensure demo is run in Cygwin/bash.

---

## Code Examples

### Minimal Phase 11 skeleton
```bash
#!/usr/bin/env bash
# Garvan demo — run qseries commands (human demonstration).
# Phases 12–15 append sections below.
set -e
BIN="./qseries.exe"
[ -f qseries.exe ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found"; exit 1; }

run() { printf '%s\n' "$@" | "$BIN" 2>&1; }

# === Rogers-Ramanujan (qseriesdoc §3.1) ===
echo ""
echo "--- Rogers-Ramanujan ---"
run "rr := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)" "prodmake(rr, 40)"
```

### Section header convention for Phases 12–15
```bash
# === Product conversion: qfactor, etamake, jacprodmake (qseriesdoc §3.2–3.4) ===
# === Relations: findhom, findhomcombo, findnonhomcombo (qseriesdoc §4) ===
# === Sifting and product identities (qseriesdoc §5, §6) ===
```

---

## State of the Art

| Aspect | Approach | Notes |
|--------|----------|-------|
| Piping to REPL | printf + pipe | Standard; no expect/pty needed |
| Binary detection | Try .exe then no-ext | Matches acceptance.sh |
| Demo vs test | Separate scripts | Demo = display; acceptance = assert |

**No deprecated patterns identified for this domain.**

---

## Open Questions

1. **Companion README.md**
   - What we know: CONTEXT allows optional `demo/README.md`
   - What's unclear: Whether to include in Phase 11 or defer
   - Recommendation: Include in Phase 11 with one-line intro and `make demo` / `bash demo/garvan-demo.sh`

2. **Script name: garvan-demo.sh vs demo.sh**
   - What we know: CONTEXT lists both
   - Recommendation: Use `garvan-demo.sh` (matches Garvan examples theme)

---

## Sources

### Primary (HIGH confidence)
- `tests/acceptance.sh` — BIN detection, run() helper, printf pattern
- `src/repl.h` (lines 623–638) — Banner and prompt output
- `11-CONTEXT.md` — Locked decisions
- `Makefile` — Existing targets

### Secondary (MEDIUM confidence)
- `qseriesdoc.md` — Section structure (§3.1, §3.2–3.4, §4, §5, §6) for future phases
- `SPEC.md` — Project constraints

---

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — acceptance.sh provides exact pattern
- Architecture: HIGH — CONTEXT prescribes structure
- Pitfalls: HIGH — Based on acceptance.sh and REPL behavior

**Research date:** 2026-02-25  
**Valid until:** 30 days (stable domain)

---

## RESEARCH COMPLETE
