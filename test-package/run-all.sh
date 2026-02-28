#!/usr/bin/env bash
# Run all tests: unit tests (qseries --test) then acceptance tests.
# Runs from test-package/ with qseries binary in same directory.

set -e
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"
BIN="./qseries.exe"
[ -f "$BIN" ] || BIN="./qseries"
[ -f "$BIN" ] || { echo "error: qseries binary not found in $(pwd)"; exit 1; }

echo "=== Unit tests (qseries --test) ==="
"$BIN" --test || { echo "Unit tests failed"; exit 1; }

echo ""
echo "=== Acceptance tests (9 SPEC tests) ==="
"$SCRIPT_DIR/acceptance.sh"
