# Deferred Items — Phase 117

Items discovered during execution but out of scope (not caused by 117-01 changes).

## Pre-existing Build Failures (Cygwin g++ 13)

Build fails with errors unrelated to expectArgCount, migration, or readLineFn:

1. **H macro comma splitting:** `H(..., {"etaq", "prodmake"})` — preprocessor splits on comma → 5 args. Fix: wrap in parens `({"etaq", "prodmake"})` or change macro.

2. **getBuiltinRegistry undeclared:** getHelpTable (line ~555) calls getBuiltinRegistry before its definition. Fix: forward declaration.

3. **evalToInt undeclared:** Used in RootOf, Gamma1ModFunc, etc. Declaration order.

4. **formatEtamake, formatProdmake, formatUndefinedVariableMsg undeclared:** Used in handlers before definition.

5. **tripleprod/quinprod return type:** Inconsistent variant deduction.

6. **add/sum regBuiltin:** Lambda not convertible to BuiltinHandler (throws, returns void).

7. **L.push_back({...})** vector initializer list conversion.

These affect build in Cygwin g++ 13. Docker (gcc:14) or project CI may succeed. Not fixed per scope boundary.
