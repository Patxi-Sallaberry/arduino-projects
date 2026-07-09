#!/usr/bin/env bash
# ============================================================================
#  Test du module alarme (FP6, EF7).
#  Vérifie que la LED d'alarme (D8) s'allume SSI l'état est ALARME, via la
#  lecture de l'état réel de la broche D8 dans un VCD (analyseur logique).
#  Usage : bash tests/run_alarme_test.sh
# ============================================================================
set -uo pipefail
cd "$(dirname "$0")/.."
fail=0

arduino-cli compile --fqbn arduino:avr:uno --output-dir build src/ >/dev/null 2>&1 \
  || { echo "compilation firmware KO"; exit 1; }

echo "--- Cas ALARME (T=30 C, consigne 20 C -> e=10) : LED attendue ALLUMEE ---"
wokwi-cli . --diagram-file tests/diagram_alarme.json --scenario tests/test_alarme.yaml \
             --vcd-file build/alarme.vcd --timeout 2600 >/dev/null 2>&1 \
  || { echo "capture VCD (alarme) KO"; exit 1; }
python3 tests/verifie_pin.py build/alarme.vcd D1 1 || fail=1

echo "--- Cas hors alarme (T=25 C, consigne 20 C -> e=5 -> REGULATION) : LED attendue ETEINTE ---"
wokwi-cli . --scenario tests/test_alarme.yaml \
             --vcd-file build/regul.vcd --timeout 2600 >/dev/null 2>&1 \
  || { echo "capture VCD (regul) KO"; exit 1; }
python3 tests/verifie_pin.py build/regul.vcd D1 0 || fail=1

echo ""
[ "$fail" -eq 0 ] && echo "########## FP6 : LED alarme OK ##########" \
                  || echo "########## FP6 : ECHEC ##########"
exit "$fail"
