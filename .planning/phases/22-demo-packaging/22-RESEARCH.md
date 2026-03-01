# Phase 22: Demo packaging — Research

**Researched:** 2026-02-25  
**Domain:** Build/packaging, distributable folder layout, cross-platform demo  
**Confidence:** HIGH

## Summary

Phase 22 delivers a single distributable folder (binary + demo script + README) to satisfy QOL-08. The project already has most pieces: `demo/garvan-demo.sh` (full Phase 15 content), `Makefile` with `package-demo` and `dist-demo`, and a `qseries-demo/` layout that was assembled manually. The Makefile `package-demo` target is incomplete — it copies only the binary, not the demo script or README. The gap is small: extend `package-demo` to copy all three artifacts and ensure the demo script uses directory-relative resolution so it runs regardless of CWD.

**Primary recommendation:** Extend `package-demo` to produce a flat `qseries-demo/` folder containing `qseries.exe` (or `qseries`), `garvan-demo.sh`, and `README.md`. Use the DIR-based BIN resolution (as in the existing `qseries-demo/garvan-demo.sh`) for robustness. Add a distribution-specific README that explains contents, bash requirement, and how to run. Document that Windows users need Git Bash, Cygwin, or WSL to run the demo script.

---

<user_constraints>
## User Constraints (from CONTEXT.md)

No CONTEXT.md exists for Phase 22. Research options and recommend approaches.
</user_constraints>

---

## 1. Current State vs QOL-08

### What exists

| Component | Location | Status |
|-----------|----------|--------|
| Binary build | `Makefile`: dist/qseries.exe | ✓ Uses x86_64-w64-mingw32-g++ or g++ |
| dist-demo | `Makefile`: copies demo/ → dist/demo/ | ✓ Binary in dist/, script in dist/demo/ |
| Demo script | `demo/garvan-demo.sh` | ✓ Full Phase 15 content (RR, prod conversion, relations, sifting) |
| demo/README.md | `demo/README.md` | ✓ Brief: make demo, cd dist && ./demo/garvan-demo.sh |
| dist/README.md | `dist/README.md` | ✓ Describes dist/ layout, how to run |
| package-demo | `Makefile` | ⚠ Incomplete: copies only binary |
| qseries-demo/ | Manual/prior assembly | ✓ Flat layout, distribution-ready script, comprehensive README |

### What’s missing for QOL-08

1. **`package-demo` does not copy garvan-demo.sh or README** — only the binary is copied (Makefile lines 43–47).
2. **Two script variants** — `demo/garvan-demo.sh` uses CWD-relative `./qseries.exe` (works when run from dist/). `qseries-demo/garvan-demo.sh` uses `DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"` and `BIN="$DIR/qseries.exe"` — works regardless of CWD. For distribution, the DIR-based approach is preferable.
3. **No single source of truth** — `qseries-demo/` was populated manually; it should be produced entirely by `package-demo`.

---

## 2. Folder Layout

### Recommended: flat layout

```
qseries-demo/
├── qseries.exe     # or qseries on Unix
├── garvan-demo.sh
└── README.md
```

- Single top-level folder name: `qseries-demo`
- Flat layout: simpler for users, no nested paths
- Matches existing successful `qseries-demo/` structure

### Alternative: nested (dist-like)

```
qseries-demo/
├── qseries.exe
├── demo/
│   ├── garvan-demo.sh
│   └── README.md
└── README.md
```

- Mirrors `dist/` but adds extra nesting
- User would run `./demo/garvan-demo.sh` and must ensure they run from the folder containing `qseries.exe`
- Not needed for QOL-08; flat layout is sufficient

**Recommendation:** Use flat layout.

---

## 3. Build/Packaging

### Current Makefile

```makefile
package-demo: dist/qseries.exe
	mkdir -p qseries-demo
	cp dist/qseries.exe qseries-demo/ 2>/dev/null || cp dist/qseries qseries-demo/
	@echo "qseries-demo/ ready. Zip and share. Run: cd qseries-demo && bash garvan-demo.sh"
```

### Required changes

1. Copy `garvan-demo.sh` — use a distribution variant that resolves binary by script directory, or transform `demo/garvan-demo.sh` at package time.
2. Copy `README.md` — distribution-specific README (contents below).
3. Ensure demo script uses DIR-based BIN resolution for robustness.

### Option A: Single source script with conditional logic

Add DIR-based resolution to `demo/garvan-demo.sh` when CWD-based resolution fails, so one script works for both `dist/` and `qseries-demo/`.

### Option B: Two files (preferred)

- Keep `demo/garvan-demo.sh` for `dist/` and `make demo`.
- Add `demo/garvan-demo-dist.sh` (or generate at package time) that uses DIR-based resolution.
- `package-demo` copies `garvan-demo-dist.sh` as `garvan-demo.sh` into `qseries-demo/`.

### Option C: One script, DIR-first

Change `demo/garvan-demo.sh` to use DIR-based resolution only. Then it works in both layouts: in `dist/` the script lives in `demo/`, and `BIN="$DIR/qseries.exe"` would point to `$DIR` = `dist/demo/`, so the binary would need to be `../qseries.exe`. For flat layout, `$DIR` = folder with binary. So a single DIR-based script would need to check `$DIR` and `$DIR/..` for the binary when in nested layout.

**Recommendation:** Use Option C with a small search path: try `$DIR/qseries.exe`, then `$DIR/../qseries.exe`, then `./qseries.exe`. Or keep two variants (A) — less risk of breaking `make demo` for dist.

**Minimal approach:** Unify on DIR-based script. In `dist/`, structure is `dist/{qseries.exe, demo/garvan-demo.sh}`. If the script uses `DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"`, then `DIR=dist/demo`, and we need `BIN=../qseries.exe` (parent dir). So:
- Flat: `DIR` = qseries-demo, `BIN="$DIR/qseries.exe"` ✓
- Nested: `DIR` = dist/demo, `BIN="$DIR/../qseries.exe"` ✓

A single script can do:
```bash
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN="$DIR/qseries.exe"
[ -f "$BIN" ] || BIN="$DIR/qseries"
[ -f "$BIN" ] || BIN="$DIR/../qseries.exe"
[ -f "$BIN" ] || BIN="$DIR/../qseries"
[ -f "$BIN" ] || { echo "error: qseries not found"; exit 1; }
```
This covers flat (binary in DIR) and nested (binary in DIR/..). One script for both.

---

## 4. README Content

Distribution README should include:

| Section | Content |
|---------|---------|
| **Title** | Q-Series Demo |
| **Contents** | List: qseries.exe (or qseries), garvan-demo.sh, README |
| **Requirements** | Bash (Git Bash, Cygwin, or WSL on Windows; built-in on Linux/macOS) |
| **How to run** | `cd qseries-demo` then `bash garvan-demo.sh` |
| **Interactive REPL** | `./qseries.exe` or `./qseries` |
| **Optional: rebuild** | Point to source build instructions if user has source |

Reference: `qseries-demo/README.md` already has this structure and can be used as the template (possibly moved to `demo/README-dist.md` or similar and copied by `package-demo`).

---

## 5. Cross-Platform

### Binary naming

| Environment | Output | Makefile CXX |
|-------------|--------|--------------|
| x86_64-w64-mingw32-g++ | qseries.exe | Default in Makefile |
| g++ (Cygwin) | qseries.exe | build.sh fallback |
| g++ (Linux/macOS) | qseries (no extension) | build.sh fallback |

`package-demo` uses `cp dist/qseries.exe ... 2>/dev/null || cp dist/qseries ...` — correct for both cases.

### Running the demo on Windows

- **Command Prompt:** Cannot run `.sh` — user needs a bash environment.
- **Git Bash, Cygwin, WSL:** Provide bash; `bash garvan-demo.sh` works.
- **Cygwin/Mintty:** Same as Unix; script runs normally.

### Binary portability

- **Static linking (`-static`):** No dependency on libstdc++, libgcc, libwinpthread (MinGW may still use system DLLs; SPEC notes acceptable Windows system DLLs).
- **One build per platform:** Default Makefile produces Windows `.exe`. For Linux/macOS, user rebuilds with native `g++` — document in README.

**Recommendation:** Document that the packaged binary is for Windows by default; Linux/macOS users can rebuild from source. This is sufficient for QOL-08.

---

## 6. Minimal Approach to Satisfy Success Criteria

| Criterion | Approach |
|-----------|----------|
| 1. Folder with binary, demo script, README | Extend `package-demo` to copy all three into `qseries-demo/` |
| 2. Self-contained | Static binary; no external deps except bash to run the script |
| 3. README explains how to run | Use README template with: contents, bash requirement, `cd qseries-demo && bash garvan-demo.sh`, optional interactive REPL and rebuild |

### Implementation outline

1. **Update `demo/garvan-demo.sh`** to use DIR-based BIN resolution with fallback to `$DIR/../qseries.exe` so it works in both flat and nested layouts. (Or keep two script variants; minimal fix is to just have `package-demo` copy a working script.)
2. **Add `demo/README-dist.md`** (or equivalent) as the distribution README template.
3. **Extend `package-demo` in Makefile:**
   ```makefile
   package-demo: dist/qseries.exe
   	mkdir -p qseries-demo
   	cp dist/qseries.exe qseries-demo/ 2>/dev/null || cp dist/qseries qseries-demo/
   	cp demo/garvan-demo.sh qseries-demo/
   	cp demo/README-dist.md qseries-demo/README.md
   	@echo "qseries-demo/ ready. Zip and share. Run: cd qseries-demo && bash garvan-demo.sh"
   ```
4. **Ensure `demo/garvan-demo.sh`** works in flat layout when copied to `qseries-demo/` — use DIR-based resolution so it finds the binary in the same directory.

---

## Standard Stack

| Component | Purpose |
|-----------|---------|
| Makefile | Build and packaging |
| Bash | Demo script, portable |
| demo/garvan-demo.sh | Single source; DIR-based resolution for both dist/ and qseries-demo/ |
| demo/README-dist.md | Distribution README template (or reuse/adapt qseries-demo/README.md) |

---

## Common Pitfalls

### Pitfall 1: Script fails when run from wrong directory

**What goes wrong:** User runs `bash /path/to/qseries-demo/garvan-demo.sh` from another directory; `./qseries.exe` fails.  
**Prevention:** Use `DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"` and `BIN="$DIR/qseries.exe"`.

### Pitfall 2: package-demo overwrites user edits

**What goes wrong:** `package-demo` recreates `qseries-demo/` each run; any local edits are lost.  
**Mitigation:** Document that `qseries-demo/` is generated; do not edit it directly. Minor; acceptable for QOL-08.

### Pitfall 3: Windows users run from cmd.exe

**What goes wrong:** `garvan-demo.sh` is not runnable from Command Prompt.  
**Prevention:** README clearly states: "You need bash (Git Bash, Cygwin, or WSL on Windows)."

---

## Don't Hand-Roll

| Problem | Don't Build | Use Instead |
|---------|-------------|-------------|
| Zip/tarball creation | Custom archive logic in Makefile | Document `zip -r qseries-demo.zip qseries-demo` or leave to user |
| Cross-compilation for multiple OSes | CI matrix, multi-target Makefile | Single platform per build; document rebuild for others |
| Installer (MSI, .deb, etc.) | Installer build chain | Not in scope for QOL-08; folder is sufficient |

---

## Sources

### Primary (codebase)

- `Makefile` — package-demo, dist-demo, build targets
- `demo/garvan-demo.sh` — current script, BIN resolution
- `qseries-demo/garvan-demo.sh` — DIR-based resolution
- `qseries-demo/README.md` — distribution README template
- `build.sh` — build flow, compiler selection

### Secondary

- `.planning/ROADMAP.md` — Phase 22 success criteria
- `REQUIREMENTS.md` — QOL-08
- `.planning/phases/11-demo-artifact/11-RESEARCH.md` — demo script patterns
- `.planning/phases/15-sifting-product-identities-demo/15-RESEARCH.md` — demo structure

---

## Metadata

**Confidence breakdown:**
- Current vs missing: HIGH — from direct codebase inspection
- Folder layout: HIGH — matches existing qseries-demo
- Build/packaging: HIGH — small, clear Makefile changes
- Cross-platform: MEDIUM — based on project docs; MinGW static linking may have edge cases

**Research date:** 2026-02-25  
**Valid until:** 30 days (stable packaging domain)
