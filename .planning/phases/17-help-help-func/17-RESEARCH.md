# Phase 17: help + help(func) — Research

**Domain:** Built-in documentation for qseries REPL.

## Summary

Implement `help` and `help(func)` as a new built-in in `dispatchBuiltin`. Use a static table mapping function names to (signature, description). No parser changes needed — `help` and `help(expr)` parse as Call.

---

## Current Architecture

- **repl.h** `dispatchBuiltin(name, args, env, sumIndices)` — long if-chain; throws "unknown built-in: " + name if no match
- **parser** — `help` → Call("help", []), `help(etaq)` → Call("help", [Var("etaq")])
- **evalStmt** — evaluates expr, passes Call to eval → dispatchBuiltin

## Built-in List (from dispatchBuiltin)

aqprod, etaq, theta2, theta3, theta4, theta, qbin, tripleprod, quinprod, winquist, sift, T, prodmake, etamake, jacprodmake, jac2prod, qfactor, series, coeffs, set_trunc, findhom, findnonhom, findhomcombo, findnonhomcombo, findpoly, legendre, sigma, subs_q

Plus: **sum**, **add** — handled in eval (Expr::Tag::Sum), not dispatchBuiltin. Include in help list.

## Implementation Approach

1. **Add help table** — `static const std::map<std::string, std::pair<std::string,std::string>>` or inline vectors. Each entry: (name, signature, description).
2. **Add help branch in dispatchBuiltin** — Before the final throw:
   - `help()`: Print usage line + flat list of all names (or name + one-liner per context)
   - `help(name)` where name is identifier: Lookup; if found print signature + description; else print "unknown function: " + name
3. **No new types** — Return `DisplayOnly{}` so display() is a no-op.

## Content Source

Signatures from existing error messages in dispatchBuiltin. Descriptions from qseriesdoc.md and REPL semantics.

---
*Phase: 17-help-help-func*
