#!/usr/bin/env bash
# ============================================================================
#  Test du module actionneur (FP4, EF4).
#  Vérifie que le firmware pilote D9 au bon RAPPORT CYCLIQUE PWM.
#  Méthode : analyseur logique (diagram_vcd.json) -> export VCD -> mesure du
#  rapport cyclique en régime établi (mesure_duty.py).
#  Usage : bash tests/run_actionneur_test.sh
# ============================================================================
set -uo pipefail
cd "$(dirname "$0")/.."

arduino-cli compile --fqbn arduino:avr:uno --output-dir build src/ >/dev/null 2>&1 \
  || { echo "compilation firmware KO"; exit 1; }

wokwi-cli . \
             --scenario tests/test_actionneur_vcd.yaml \
             --vcd-file build/d9.vcd --timeout 2600 >/dev/null 2>&1 \
  || { echo "capture VCD KO"; exit 1; }

# À pot=0.1, e≈2.5 °C -> PWM≈126/255 -> rapport cyclique attendu ≈ 49,4 %.
python3 tests/mesure_duty.py build/d9.vcd D0 250 49.4 3.0
