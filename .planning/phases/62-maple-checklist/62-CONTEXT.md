# Phase 62 Context: Maple Checklist Satisfied

## Scope

**Source document:** `maple_checklist.md` (root of repo)
- Part 1: 41 Maple code blocks (Blocks 1–41)
- Part 2: 13 exercises (Exercises 1–13)

Both parts are in scope. Every block and exercise must be attempted in qseries3.

## Decision: Failure Handling

- **Fix what's reasonable.** If a block fails due to a missing feature or bug that can be fixed with moderate effort, fix it.
- **Document truly hard gaps.** If a block requires capabilities that are architecturally infeasible (e.g., `RootOf`/algebraic numbers, full Maple `proc` definitions), mark as a known limitation with a note explaining why.
- **Add small syntax features.** If a block needs for-loops, indexed variables (`X[1]`), or similar small syntax additions, add them in this phase rather than deferring.

## Decision: Output Matching

- **Mathematically equivalent is sufficient.** Coefficients must be correct but formatting differences (superscript style, eta product representation order, whitespace) are acceptable.
- Example: if Maple outputs `η(τ)^5*η(7τ)` and qseries3 outputs `η(τ)⁵ η(7τ)`, that's a pass.

## Decision: Exercises

- **Verify the math is sound** — run the computational steps, confirm the results make mathematical sense.
- **Provide qseries3 output** — show the actual commands and their output.
- **Proofs and generalizations** are out of scope — only the computational verification matters.
- Exercises go in a **separate file** (`exercises_solutions.md`), not inline in the checklist.

## Decision: Artifacts

1. **Automated test script** (`.sh`) that runs all 41 blocks through qseries3 and reports pass/fail. This is the primary artifact.
2. **Updated `maple_checklist.md`** — mark `[x]` for passing blocks, add notes for failures.
3. **`exercises_solutions.md`** — separate file with qseries3 commands and output for each exercise.

## Decision: Compile/Test Strategy

- Batch fixes vs. fix-and-retest-each: implementer's choice, whatever is efficient.

## Known Challenges (from prior work)

- Block 10 (`omega := RootOf(z^2+z+1=0)`) — qseries3 has no algebraic number support. Exercise 4's `b(q)` uses ω. Workaround: reformulate b(q) using the identity `b(q) = 3*S0 - a(q)` where `S0 = Σ Σ q^(3(n²+3nk+3k²))` (avoids complex numbers entirely).
- Blocks 3–6 use Maple `proc` definitions — translate to equivalent qseries3 commands or add minimal procedural support.
- Block 17 uses `numtheory[legendre]` — needs a `legendre(a,p)` function or workaround.
- Block 25 uses `radsimp` — qseries3 doesn't have symbolic simplification; translate to direct computation.
- Blocks referencing `X[1]`, `X[2]` indexed vars — may need indexed variable support for `findhom`/`findnonhom` output.

## Deferred Ideas

(None captured yet — will be added during execution if scope creep is detected.)
