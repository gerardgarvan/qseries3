# Phase 102: Cross-Package Integration — Context

**Phase:** 102  
**Goal:** ETA identity prover + theta IDs + modforms work together; integration test passes  
**Requirements:** INT-02

---

## Decisions

### 1. Success criterion 2: "single proof chain"
**Interpretation:** Strict — at least one end-to-end flow that uses eta identity + theta ID + modular form basis **together** in a single proof chain (not three separate flows).  
**Rationale:** The wording "in a single proof chain" is explicit. Current script has three separate flows.

### 2. Modular form basis
**Requirement:** Use `makebasisM` or `makeALTbasisM` in at least one test.  
**Rationale:** DELTA12 alone is a single form, not a basis. INT-02 calls for "modular form basis."

### 3. Scope (Option B from RESEARCH)
- Ensure `integration-eta-theta-modforms.sh` passes robustly (PATH / environment handling if needed).
- Add at least one new test that chains eta identity + theta ID + modular form basis in one flow.
- Include integration script in `run-all.sh` if not already there (research says it's not).
- No regressions: maple-checklist, run-all, acceptance-all must pass.

### 4. Environment
If integration script fails with "grep: command not found" in minimal PATH, fix by ensuring test runners use a shell with standard Unix tools (e.g. Cygwin /usr/bin in PATH).
