---
title: Installation
description: Build qseries from source
---

## Prerequisites

A C++20-capable compiler:

- **GCC** 10 or later
- **Clang** 13 or later
- **MSVC** 2022 (Visual Studio 17.0+)

No external libraries, package managers, or build systems are required. The entire
program compiles from a single `main.cpp` file with all dependencies included as
headers.

## Build from source

```bash
g++ -std=c++20 -O2 -static -o qseries main.cpp
```

This produces a single static binary. No Makefile, CMake, or other build tooling is
needed for the default build. A `Makefile` is included in the repository for
convenience, but the one-liner above is all that is strictly necessary.

## Platform notes

### Linux / macOS

Use `g++` or `clang++` with the `-std=c++20` flag:

```bash
g++ -std=c++20 -O2 -static -o qseries main.cpp
# or
clang++ -std=c++20 -O2 -o qseries main.cpp
```

On macOS, the `-static` flag may cause a linker warning because static `libc++` is
not always available. Omit `-static` if linking fails — the resulting binary will
still work correctly.

### Windows (Cygwin)

Install `gcc-g++` via the Cygwin setup tool, then build as usual:

```bash
g++ -std=c++20 -O2 -static -o qseries main.cpp
```

The binary runs in the Cygwin terminal. Arrow keys, tab completion, and history
work out of the box.

### Windows (MSYS2 / MinGW)

```bash
g++ -std=c++20 -O2 -static -o qseries.exe main.cpp
```

## Verify installation

```bash
./qseries --version
# Output: qseries 2.0

./qseries --test
# Runs the internal test suite
```

Both flags exit immediately after printing their output.

## Running

**Interactive mode** — launch the REPL:

```bash
./qseries
```

**Script mode** — pipe a script file:

```bash
./qseries < script.qs
```

In script mode, the startup banner is suppressed and each input line is echoed
before its output. See [Quick Start](/getting-started/quick-start/) for your first
session.
