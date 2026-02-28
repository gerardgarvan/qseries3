# Q-Series REPL — Test Package

Self-contained test package for third-party verification. Contains the qseries binary and all tests.

## Contents

- `qseries.exe` (or `qseries`) — q-series REPL binary
- `acceptance.sh` — 9 SPEC acceptance tests (Rogers-Ramanujan, partition, theta/eta, Jacobi, Gauss AGM, Rødseth, qfactor, Watson, Euler pentagonal)
- `run-all.sh` — runs unit tests (`qseries --test`) then acceptance tests

## Requirements

- bash (or compatible shell)
- No other dependencies

## Run All Tests

```bash
./run-all.sh
```

Or run individually:

```bash
./qseries.exe --test          # Unit tests (BigInt, Frac, Series, qfuncs, convert, etc.)
./acceptance.sh               # 9 SPEC acceptance tests via REPL
```

## Expected Output

- **Unit tests:** `PASS` lines for each check; exits 0 on success
- **Acceptance tests:** `PASS: Test 1 Rogers-Ramanujan` through `PASS: Test 9 Euler`; `Total: 9 passed, 0 failed`; exits 0 on success

## Creating This Package

From the project root:

```bash
make test-package
```

This builds the binary (if needed) and copies it into `test-package/`. Alternatively, build manually and copy:

```bash
g++ -std=c++20 -O2 -static -o dist/qseries.exe src/main.cpp
cp dist/qseries.exe test-package/
```
