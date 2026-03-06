# Phase 114: make lint — Research

**Researched:** 2026-03-06
**Domain:** cppcheck integration
**Confidence:** HIGH

## Summary

Add `make lint` target that runs cppcheck on source files. Per v11.3 research (STACK.md): cppcheck 2.x, standalone, no compile_commands.json required.

## Recommended Invocation

```
cppcheck --enable=warning,style,performance -I src src/main.cpp
```

Or for full src/ directory:
```
cppcheck --enable=warning,style,performance -I src src/
```

Single-TU project: main.cpp includes all headers. cppcheck on main.cpp analyzes the full codebase. Use `-I src` so includes resolve.

## Makefile Integration

Add to .PHONY: lint
Add target:
```
lint:
	cppcheck --enable=warning,style,performance -I src src/main.cpp
```

Optional: `--error-exitcode=1` to fail on issues; or exit 0 and report (phase says "reports findings or passes").

## Source Layout

- src/main.cpp (includes repl.h, parser.h, series.h, etc.)
- src/*.h (headers)
- No compile_commands.json needed
