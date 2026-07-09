#!/usr/bin/env bash
# ============================================================================
#  Suite de tests du module de régulation (FP3).
#   1) Test UNITAIRE natif (g++)   : logique pure, assertions déterministes.
#   2) Intégration Wokwi à 25 °C   : REPOS + REGULATION (chaîne analogique réelle).
#   3) Intégration Wokwi à 30 °C   : ALARME (diagramme dédié, NTC init-only).
#  Usage : bash tests/run_regulation_tests.sh
# ============================================================================
set -uo pipefail
cd "$(dirname "$0")/.."
fail=0

echo "===== 1) Test unitaire natif (g++) ====="
g++ -Isrc tests/test_regulation_unit.cpp src/regulation.cpp -o build/test_regulation_unit \
  || { echo "compilation du test unitaire KO"; exit 1; }
./build/test_regulation_unit || fail=1

echo ""; echo "===== 2) Integration Wokwi — T procede = 25 C (REPOS + REGULATION) ====="
arduino-cli compile --fqbn arduino:avr:uno --output-dir build src/ >/dev/null 2>&1 \
  || { echo "compilation firmware KO"; exit 1; }
wokwi-cli . --scenario tests/test_regulation.yaml --timeout 12000 || fail=1

echo ""; echo "===== 3) Integration Wokwi — T procede = 30 C (ALARME) ====="
wokwi-cli . --diagram-file tests/diagram_alarme.json \
             --scenario tests/test_regulation_alarme.yaml --timeout 12000 || fail=1

echo ""
if [ "$fail" -eq 0 ]; then
  echo "########## FP3 : TOUS LES TESTS OK ##########"
else
  echo "########## FP3 : ECHEC ##########"
fi
exit "$fail"
