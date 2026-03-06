# Phase 87: Integration Testing — Context

**Phase:** 87  
**Goal:** End-to-end verification of all v6.0 packages, full regression  
**Depends on:** All v6.0 phases (76–86)

## Requirements

| ID | Description |
|----|-------------|
| INT-01 | Full regression: all existing tests pass |
| INT-02 | Cross-package integration: ETA + thetaids + modforms identity proofs work end-to-end |

## Success Criteria (ROADMAP)

1. All existing tests pass (maple-checklist, acceptance-*, integration-tcore)
2. Cross-package: ETA identity prover + modular forms basis used together
3. Mock theta + Bailey chain examples verified
4. Partition statistics consistent with rank/crank GFs from Phase 69

## Test Inventory

- **maple-checklist.sh** — qseriesdoc.md validation (blocks 1–27)
- **acceptance.sh** + acceptance-*.sh — per-phase acceptance
- **integration-tcore.sh** — t-core integration
- **run-all.sh** — aggregates acceptance + integration
