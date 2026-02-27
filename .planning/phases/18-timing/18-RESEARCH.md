# Phase 18: Timing — Research

**Domain:** Elapsed time per command in REPL loop.

## Summary

Wrap parse + evalStmt + display in std::chrono timing. Print elapsed on next line when stdin_is_tty(). Format: seconds with 3 decimal places (e.g. 0.042s). No new dependencies — C++20 std::chrono.

---

## Implementation

- **repl.h** runRepl(): before parse(), record steady_clock::now(); after display(), compute duration; if stdin_is_tty(), print duration as seconds (e.g. 0.042s) on new line.
- **Scope:** Time only commands that produce output (skip continue on monostate). Time includes parse + eval + display.
- **Format:** `std::chrono::duration<double>(end-start).count()` → `std::fixed << std::setprecision(3) << secs << "s"`

---
*Phase: 18-timing*
