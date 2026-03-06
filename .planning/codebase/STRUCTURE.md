# Codebase Structure

**Analysis Date:** 2025-03-04

## Directory Layout

```
qseries3/
├── src/                   # All C++ headers and main
│   ├── main.cpp           # Entry point, unit tests, runRepl
│   ├── main_wasm.cpp      # Emscripten bindings
│   ├── bench_main.cpp     # Benchmark driver
│   ├── bigint.h           # BigInt
│   ├── frac.h             # Frac
│   ├── series.h           # Series, BivariateSeries
│   ├── omega3.h           # Omega3 (Q(ω))
│   ├── series_omega.h     # SeriesOmega
│   ├── qfuncs.h           # aqprod, etaq, theta, eisenstein, etc.
│   ├── convert.h          # prodmake, etamake, jacprodmake, sift, qfactor
│   ├── linalg.h           # Kernel over Q
│   ├── relations.h        # findhom, findnonhom, findpoly
│   ├── parser.h           # Tokenizer, Parser
│   ├── repl.h             # REPL, Environment, built-ins
│   ├── eta_cusp.h         # Cusp theory, provemodfunc
│   ├── theta_ids.h        # provemodfuncid
│   ├── modforms.h         # Modular form helpers
│   ├── bailey.h           # Bailey identities
│   ├── mock.h             # Mock theta
│   ├── crank_tables.h     # Crank
│   ├── tcore.h            # t-core
│   └── rr_ids.h           # Rogers-Ramanujan identity check
├── tests/                 # Bash acceptance/integration tests
│   ├── maple-checklist.sh # Blocks 1–41 from maple_checklist.md
│   ├── acceptance.sh      # 9 SPEC tests
│   ├── run-all.sh         # Runs 14+ acceptance scripts
│   ├── acceptance-*.sh    # Feature-specific (eta-cusp, modforms, etc.)
│   └── integration-*.sh   # Cross-package integration
├── demo/                  # Demo scripts
│   ├── garvan-demo.sh
│   └── README.md
├── dist/                  # Build output (qseries.exe, demo/)
├── build/                 # WASM build output
├── .planning/             # Planning docs, phases
├── gaps/                  # Reference material (Maple, eta, etc.)
├── Makefile
├── Dockerfile
├── install.sh             # Fetch-and-install from GitHub releases
├── index.html             # Web demo UI
├── qseriesdoc.md          # Garvan tutorial (acceptance reference)
├── maple_checklist.md     # Block-by-block checklist
├── CLAUDE.md              # AI assistant instructions
├── SPEC.md                # Full specification
└── .cursorrules           # Coding constraints
```

## Directory Purposes

**src/:**
- Purpose: All C++ implementation
- Contains: Header-only logic plus main.cpp, bench_main.cpp, main_wasm.cpp
- Key files: `main.cpp`, `repl.h`, `parser.h`, `series.h`, `qfuncs.h`, `convert.h`, `relations.h`

**tests/:**
- Purpose: REPL acceptance and integration tests
- Contains: Bash scripts that pipe commands to qseries binary, grep output
- Key files: `maple-checklist.sh`, `acceptance.sh`, `run-all.sh`

**demo/:**
- Purpose: Human demonstration scripts
- Contains: `garvan-demo.sh`, README

**dist/:**
- Purpose: Build output (binary, demo copies)
- Generated: Yes
- Committed: No (in .gitignore)

**build/:**
- Purpose: WASM build output
- Generated: Yes
- Committed: No

**gaps/:**
- Purpose: Reference material (Maple install scripts, eta/theta data)
- Generated: No
- Committed: Mixed (some untracked per git status)

**.planning/:**
- Purpose: GSD phases, research, verification
- Contains: phases/*, research/*, config.json
- Committed: Yes

## Key File Locations

**Entry Points:**
- `src/main.cpp`: Native REPL, unit tests
- `src/main_wasm.cpp`: WASM module
- `src/bench_main.cpp`: Benchmark

**Configuration:**
- `Makefile`: Build, test targets
- `Dockerfile`: Container build
- `.github/workflows/release.yml`: CI/release

**Core Logic:**
- `src/bigint.h`, `src/frac.h`, `src/series.h`: Foundation
- `src/qfuncs.h`, `src/convert.h`: Q-series math
- `src/linalg.h`, `src/relations.h`: Linear algebra, relations
- `src/parser.h`, `src/repl.h`: REPL

**Testing:**
- `tests/maple-checklist.sh`: Maple checklist blocks
- `tests/acceptance.sh`: 9 SPEC tests
- `tests/run-all.sh`: Full regression
- `src/main.cpp` (runUnitTests): Embedded unit tests

## Naming Conventions

**Files:**
- Headers: `snake_case.h` (e.g., `series.h`, `theta_ids.h`)
- Tests: `acceptance-<feature>.sh`, `integration-<feature>.sh`

**Directories:**
- Lowercase: `src/`, `tests/`, `demo/`, `dist/`, `build/`, `gaps/`, `.planning/`

## Where to Add New Code

**New q-series function:**
- Primary: `src/qfuncs.h` (or new domain header like `eta_cusp.h`)
- REPL binding: `src/repl.h` (built-in dispatch)
- Tests: `tests/acceptance-<name>.sh` or extend `maple-checklist.sh`

**New conversion/relation:**
- Primary: `src/convert.h` or `src/relations.h`
- Tests: `tests/acceptance-*.sh`

**New REPL built-in:**
- Parser: `src/parser.h` (if new syntax)
- REPL: `src/repl.h` (evalBuiltin, help text)

**Utilities:**
- Shared helpers: Inline in relevant header (`qfuncs.h` for number theory, etc.)

## Special Directories

**dist/:**
- Purpose: Binary and demo files
- Generated: Yes (by Makefile)
- Committed: No

**build/wasm/:**
- Purpose: Emscripten output
- Generated: Yes
- Committed: No

**gaps/:**
- Purpose: Maple/Mathematica reference, install scripts
- Generated: No
- Committed: Partial (many untracked)

---

*Structure analysis: 2025-03-04*
