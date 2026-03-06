#!/usr/bin/env bash
cd "$(dirname "$0")/.."
total_pass=0
total_fail=0
for t in tests/acceptance.sh tests/acceptance-rank-crank.sh tests/acceptance-crank.sh tests/acceptance-bailey.sh tests/acceptance-eta-cusp.sh tests/acceptance-partition-type.sh tests/acceptance-tcore.sh tests/acceptance-gsk.sh tests/acceptance-vectors.sh tests/acceptance-tcrank-display.sh tests/integration-tcore.sh tests/acceptance-ptnstats.sh tests/acceptance-mock.sh tests/acceptance-misc.sh tests/integration-eta-theta-modforms.sh; do
    echo "--- Running $(basename $t) ---"
    result=$(bash "$t" 2>&1)
    line=$(echo "$result" | tail -1)
    echo "$line"
    p=$(echo "$line" | grep -oP '[0-9]+ passed' | grep -oP '[0-9]+')
    f=$(echo "$line" | grep -oP '[0-9]+ failed' | grep -oP '[0-9]+')
    total_pass=$((total_pass + ${p:-0}))
    total_fail=$((total_fail + ${f:-0}))
done
echo ""
echo "=== TOTAL: $total_pass passed, $total_fail failed ==="
exit $total_fail
