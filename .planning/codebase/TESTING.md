# Testing Patterns

**Analysis Date:** 2025-03-04

## Test Framework

**Runner:**
- Bash scripts — No dedicated test runner
- Unit tests: C++ code in `src/main.cpp`, invoked via `./qseries --test`

**Assertion Library:**
- Custom `CHECK(cond)` macro in `main.cpp`; increments `fail_count`, prints PASS/FAIL
- Bash: `grep -q` to assert output contains expected strings

**Run Commands:**
```bash
make test                    # Quick Rogers-Ramanujan smoke test
make acceptance              # tests/acceptance.sh (9 SPEC tests)
make acceptance-maple        # tests/maple-checklist.sh (blocks 1–41)
make acceptance-all          # Full regression (maple + run-all + exercises + factor + modforms + theta-ids + integration)
./dist/qseries.exe --test    # Unit tests from main.cpp
make bench                   # Benchmark (bench_main.cpp)
bash tests/run-all.sh        # 14+ acceptance scripts
```

## Test File Organization

**Location:**
- Acceptance: `tests/acceptance-*.sh`, `tests/integration-*.sh`
- Maple checklist: `tests/maple-checklist.sh`
- Unit: Embedded in `src/main.cpp`

**Naming:**
- `acceptance-<feature>.sh` (e.g., `acceptance-eta-cusp.sh`, `acceptance-modforms.sh`)
- `integration-<feature>.sh` (e.g., `integration-eta-theta-modforms.sh`, `integration-tcore.sh`)
- `maple-checklist.sh` — Validates against `maple_checklist.md` blocks

**Structure:**
```
tests/
├── maple-checklist.sh       # Primary Maple/doc validation
├── acceptance.sh            # 9 SPEC acceptance tests
├── run-all.sh               # Runs 14 scripts, aggregates pass/fail
├── acceptance-*.sh          # ~25+ feature scripts
└── integration-*.sh         # Cross-package tests
```

## Test Structure

**Bash pattern:**
```bash
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
# ... fallbacks ...

run() {
    printf '%s\n' "$@" | "$BIN" 2>&1
}

PASS=0
FAIL=0
pass() { echo "PASS: $1"; PASS=$((PASS+1)); }
fail() { echo "FAIL: $1"; FAIL=$((FAIL+1)); }

# Test N: description
if run "cmd1" "cmd2" | grep -q "expected"; then
    pass "Test N description"
else
    fail "Test N description"
fi

echo "$PASS passed, $FAIL failed"
exit $FAIL
```

**Unit test pattern (main.cpp):**
```cpp
#define CHECK(cond) do { \
    if (!(cond)) { std::cerr << "FAIL: " << #cond << '\n'; ++fail_count; } \
    else { std::cout << "PASS: " << #cond << '\n'; } \
} while(0)

CHECK(BigInt(0) * BigInt(1) == BigInt(0));
CHECK(f.den == BigInt(1) && f.num == BigInt(26));
```

## Mocking

**Framework:** None — Tests run against real binary. No mocks.

**What to Mock:** N/A

**What NOT to Mock:** Nothing; integration/acceptance tests exercise full stack.

## Fixtures and Factories

**Test Data:**
- REPL commands as strings: `"x := sum(q^(n^2)/aqprod(q,q,n,50), n, 0, 8)"`, `"prodmake(x, 40)"`
- Expected output: grep patterns (e.g., `1-q`, `η`, `JAC`, `961`)

**Location:**
- Inline in shell scripts
- Reference: `qseriesdoc.md`, `maple_checklist.md`

## Coverage

**Requirements:** None enforced

**View Coverage:** Not implemented (no gcov/lcov)

## Test Types

**Unit Tests:**
- Scope: BigInt, Frac, Series, SeriesOmega, Omega3
- Location: `src/main.cpp` `runUnitTests()`
- Invoke: `./qseries --test`

**Acceptance Tests:**
- Scope: Full REPL; SPEC tests, Maple checklist blocks, feature-specific
- Approach: Pipe commands, grep output
- Scripts: `acceptance.sh`, `maple-checklist.sh`, `acceptance-*.sh`

**Integration Tests:**
- Scope: Cross-package (eta + theta IDs + modforms)
- Script: `tests/integration-eta-theta-modforms.sh`

**E2E Tests:**
- Not separate; acceptance tests serve as E2E

## Common Patterns

**Multi-command session (state persistence):**
```bash
run "x := sum(...)" "prodmake(x, 40)" | grep -q "1-q"
```

**Unicode in grep:**
```bash
grep -q $'\xCE\xB7'   # η (eta)
```

**Binary discovery:**
```bash
BIN="./dist/qseries.exe"
[ -f "$BIN" ] || BIN="./dist/qseries"
[ -f "$BIN" ] || BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: ..."; exit 1; }
```

## Makefile Test Targets

| Target | Script / Action |
|--------|-----------------|
| `test` | Quick smoke (2 piped commands) |
| `acceptance` | `acceptance.sh` |
| `acceptance-maple` | `maple-checklist.sh` |
| `acceptance-all` | maple + run-all + exercises + factor + modforms + theta-ids + integration |
| `acceptance-eta-cusp` | `acceptance-eta-cusp.sh` |
| `acceptance-modforms` | `acceptance-modforms.sh` |
| `acceptance-theta-ids` | `acceptance-theta-ids.sh` |
| `acceptance-omega` | `acceptance-omega.sh` |
| `acceptance-factor` | `acceptance-factor.sh` |
| `acceptance-exercises` | `acceptance-exercises.sh` |
| `integration-eta-theta-modforms` | `integration-eta-theta-modforms.sh` |
| `acceptance-wins`, `acceptance-suppress-output`, etc. | Various phase-specific scripts |

---

*Testing analysis: 2025-03-04*
