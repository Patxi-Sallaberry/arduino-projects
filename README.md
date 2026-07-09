# Régulateur proportionnel de vitesse de ventilateur

[![CI](https://github.com/Patxi-Sallaberry/arduino-projects/actions/workflows/ci.yml/badge.svg)](https://github.com/Patxi-Sallaberry/arduino-projects/actions/workflows/ci.yml)

Système embarqué **Arduino Uno** de régulation en **boucle fermée** : un capteur de
température mesure un procédé, un potentiomètre fixe la consigne, et la **vitesse d'un
ventilateur** est modulée **continûment** (loi **proportionnelle**) pour ramener la
température vers la consigne. Restitution sur **écran LCD** + liaison série, **alarme**
LED sur seuil.

Développé entièrement en **simulation Wokwi** (ligne de commande), avec une démarche
d'**ingénierie systèmes en cycle en V**.

> Ancrage réel : ventilateur de radiateur (Formula Student) / refroidissement d'enceinte
> électronique — on ne fait pas tourner le ventilateur à fond en permanence, on module
> sa vitesse selon la température.

![Écran LCD en régulation](docs/img/lcd_regulation.png)

---

## Principe (boucle fermée)

```
consigne ─► [ écart e = T_mesurée − T_consigne ] ─► [ loi proportionnelle ] ─► ventilateur (PWM)
                     ▲                                                                │
                     └──────────────── capteur de température ◄─────── procédé ◄──────┘
```

**Loi de commande** (bande proportionnelle de 5 °C) :
- `e ≤ 0` → ventilateur à l'arrêt (état **REPOS**)
- `0 < e < 8 °C` → vitesse ∝ écart, `PWM = round(255·e/5)` (état **REGULATION**)
- `e ≥ 8 °C` → vitesse maxi + **LED d'alarme** (état **ALARME**)

## Matériel (câblage Arduino Uno)

| Broche | Élément | Fonction |
|---|---|---|
| A0 | Thermistance NTC (diviseur) | Mesure de température |
| A1 | Potentiomètre | Consigne (20–45 °C) |
| A4 / A5 | LCD 16×2 I²C (SDA / SCL) | Affichage |
| D9 (PWM) | Ventilateur (MOSFET + moteur + diode) | Commande de vitesse |
| D8 | LED rouge | Alarme de seuil |

> En simulation, Wokwi n'ayant pas de moteur CC, la vitesse est visualisée par la
> luminosité d'une LED sur D9 ; l'étage de puissance réel (MOSFET + diode de roue libre)
> est spécifié dans [`docs/architecture.md`](docs/architecture.md).

## Structure du code (modulaire)

| Module | Rôle | Fonction |
|---|---|---|
| `src/capteur_temp.*` | ADC → °C (équation β) | FP1 |
| `src/consigne.*` | Potentiomètre → consigne | FP2 |
| `src/regulation.*` | Écart + loi P + machine à états | FP3 |
| `src/actionneur.*` | Commande PWM du ventilateur | FP4 |
| `src/affichage.*` | Écran LCD I²C | FP5 |
| `src/alarme.*` | LED d'alarme | FP6 |
| `src/src.ino` | Ordonnanceur non bloquant (100 ms / 500 ms) | FP7 |

## Compiler et tester

```bash
# Compilation (firmware pour Arduino Uno)
arduino-cli compile --fqbn arduino:avr:uno --output-dir build src/

# Test unitaire natif de la logique de régulation (déterministe, sur PC)
g++ -Isrc tests/test_regulation_unit.cpp src/regulation.cpp -o build/test_regulation_unit
./build/test_regulation_unit

# Tests d'intégration en simulation (headless)
wokwi-cli . --scenario tests/test_capteur.yaml    --timeout 6000   # FP1 mesure
wokwi-cli . --scenario tests/test_consigne.yaml   --timeout 12000  # FP2 consigne
bash tests/run_regulation_tests.sh                                  # FP3 loi + états
bash tests/run_actionneur_test.sh                                   # FP4 PWM (via VCD)
bash tests/run_alarme_test.sh                                       # FP6 LED (via VCD)
```

**Rendu visuel interactif** : ouvrir le projet dans VS Code et lancer l'extension
*Wokwi Simulator* (palette → « Wokwi: Start Simulator ») pour manipuler le potentiomètre
et le curseur de température en temps réel.

## Intégration continue

À chaque `push`, [GitHub Actions](.github/workflows/ci.yml) compile le firmware et exécute
le test unitaire natif de la régulation. Les tests d'intégration Wokwi s'exécutent en plus
si le secret `WOKWI_CLI_TOKEN` est défini dans le dépôt (*Settings → Secrets and variables
→ Actions*) ; sinon ils sont sautés et le job reste vert.

## Démarche d'ingénierie (cycle en V)

Documents dans [`docs/`](docs/), rédigés **avant** le code :
1. [`besoin.md`](docs/besoin.md) — le *pourquoi* (qualitatif)
2. [`cahier-des-charges.md`](docs/cahier-des-charges.md) — exigences **mesurables**
3. [`analyse-fonctionnelle.md`](docs/analyse-fonctionnelle.md) — fonctions FP1…FP7
4. [`architecture.md`](docs/architecture.md) — composants + pins + machine à états
5. [`validation.md`](docs/validation.md) — preuve de chaque exigence

## Résultats de validation

**14 exigences sur 15 validées** (voir [`docs/validation.md`](docs/validation.md)) :
- Fonctionnelles EF1–EF7 : **7/7** ✅
- Performance : EP1 (100 ms), EP3 (réaction) ✅ ; **EP2** (erreur statique) ⚠️ non
  validable en simulation (capteur NTC non pilotable dynamiquement + erreur statique
  intrinsèque du régulateur P).
- Conception EC1–EC5 : **5/5** ✅

## Limites et évolutions

- **Régulateur PID** : ajouter les termes intégral (annule l'erreur statique, EP2) et
  dérivé.
- **Banc de test thermique** : modèle de procédé pour valider la boucle fermée dynamique.
- **Matériel réel** : étage de puissance MOSFET, vrai ventilateur, alimentation dédiée.

---

*Projet d'apprentissage — ingénierie systèmes embarqués. Arduino Uno / Wokwi / arduino-cli.*
