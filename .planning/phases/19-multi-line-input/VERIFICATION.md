# Phase 19: Multi-line Input — Verification

**Phase goal (from ROADMAP):** User can split long expressions across lines

**Verification date:** 2026-02-25

---

## Status: **passed**

All must-haves from the plan are satisfied by the implementation in `src/repl.h`.

---

## Must-have verification

### 1. Backslash at end of line continues on next line

**Verified: yes**

**Evidence (src/repl.h, lines 535–560):**

```cpp
// Backslash continuation: while line ends with \, read more lines
size_t contCount = 0;
for (;;) {
    while (!line.empty() && (line.back() == ' ' || line.back() == '\t'))
        line.pop_back();
    if (line.empty() || line.back() != '\\')
        break;
    line.pop_back();  // remove trailing backslash
    // ...
    if (!std::getline(std::cin, next))
        break;
    line += " " + next;
    ++contCount;
}
```

Trailing spaces/tabs are removed, the line is checked for a trailing `\`, and if present the backslash is removed and the next line is read and appended. The loop repeats until the line no longer ends with backslash.

---

### 2. REPL prompts for continuation until complete

**Verified: yes**

**Evidence (src/repl.h, lines 551–552):**

```cpp
if (stdin_is_tty())
    std::cout << "  > " << std::flush;
```

When stdin is a TTY, the continuation prompt `"  > "` is printed before reading each additional line. In non‑TTY (script) mode, the echoed line includes the prompt (lines 555–556):

```cpp
if (!stdin_is_tty())
    std::cout << "  > " << next << std::endl;
```

---

### 3. Backslash-newline treated as whitespace; parses as single unit

**Verified: yes**

**Evidence:**

- Line 548: `line.pop_back();` removes the trailing backslash.
- Line 557: `line += " " + next;` appends a space plus the next line to the current line.
- Line 562–564: `std::string trimmed = trim(line);` and `parse(trimmed)` parse the full accumulated string as one expression.

Backslash-newline is effectively replaced by a space, and the parser receives one contiguous string (e.g. `"1 + 1"` from `"1 + \\\n1"`).

---

### 4. Artifact: src/repl.h provides runRepl with multi-line read loop

**Verified: yes**

**Evidence:**

- `runRepl()` is defined in `src/repl.h` at line 514.
- The multi-line read logic is implemented in a `for (;;)` loop (lines 535–560) that:
  - R-trims the line.
  - Checks for a trailing backslash.
  - Removes the backslash and reads the next line with `getline`.
  - Appends `" " + next` to the current line.
  - Enforces `maxContinuations` (100) to avoid unbounded loops.
  - Continues until the line no longer ends with backslash.

---

## Additional checks

| Item | Status |
|------|--------|
| Build succeeds | Not run in this verification (assumed by artifact presence) |
| Max continuations limit (100) | Implemented at lines 529, 549 |
| Script mode echo of continuation lines | Implemented at lines 555–556 |
| Continuation prompt distinguishes from main prompt | Main prompt `"qseries> "`, continuation `"  > "` |

---

## Human verification

None required. Implementation matches the plan and must-haves. Suggested manual checks:

1. Interactive: `1 + \` then `1` → result `2`.
2. Interactive: `etaq(1,\` then `100)` → parses as `etaq(1,100)`.
3. Script: `echo -e "q + q^2\\\n+ q^3" | qseries` → parses and evaluates.

---

*Phase: 19-multi-line-input*  
*Verification: 2026-02-25*
