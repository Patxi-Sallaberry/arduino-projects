# CLAUDE.md — Contexte permanent du projet

> Ce fichier est relu automatiquement au début de chaque session. Il fixe QUI est
> l'utilisateur, COMMENT travailler avec lui, et la MÉTHODOLOGIE d'ingénierie attendue.
> À maintenir à jour au fil du projet.

## 1. Profil de l'utilisateur

- Étudiant 2e année **Génie Mécanique à l'INSA Toulouse**, spécialisation
  **Ingénierie Systèmes** en 4e/5e année.
- **Erasmus à Linköping (LiU, Suède)** à partir d'août 2026.
- Code déjà en **Next.js / TypeScript** pour une agence web (repo séparé `~/websites/`).
- Fait du **CFD / PIML** sur un autre projet (repo `cfd-projects` : NACA 0012, ANSYS
  Fluent, PyTorch).
- **Débute complètement en Arduino / électronique embarquée.**

## 2. Philosophie de travail — RÈGLE ABSOLUE

L'utilisateur veut **comprendre** ce qu'il fait, pas juste avoir du code qui marche.
Il doit pouvoir relire et défendre techniquement tout code (jury Formula Student,
entretiens d'embauche).

**Ne JAMAIS générer de code sans expliquer d'abord :**
- Pourquoi ce choix technique plutôt qu'un autre.
- Ce que fait chaque partie non-triviale, ligne par ligne si nécessaire.
- Les pièges classiques (ex : logique `INPUT_PULLUP` inversée, résolution ADC 10 bits
  = 0-1023, timing bloquant de `delay()`, etc.).

**Avant d'écrire du code** : exposer le plan et **attendre validation** si le
changement est non-trivial. Commentaires clairs dans le code plutôt que code « magique ».

## 3. Niveau Arduino actuel

Déjà fait manuellement sur wokwi.com (navigateur) :
- Blink LED (`digitalWrite`, `delay`, `setup`/`loop`).
- Bouton poussoir + `INPUT_PULLUP` + Serial Monitor.
- Potentiomètre + `analogRead` + notion de résolution ADC (0-1023, diviseur de tension).

**Pas encore fait** : capteur avec librairie externe, structuration multi-fichiers
(`.h`/`.cpp`), machine à états, tests automatisés.

## 4. Environnement technique (déjà installé)

- **WSL2 Ubuntu** sous Windows, **VS Code en Remote-WSL**.
- **arduino-cli** installé (core `arduino:avr`). Compilation :
  `arduino-cli compile --fqbn arduino:avr:uno --output-dir build src/`
- **wokwi-cli** installé + `WOKWI_CLI_TOKEN` configuré dans `~/.bashrc`. Simulation en
  terminal (sans GUI) : `wokwi-cli . --timeout Xms`
- Extensions VS Code : WSL, Wokwi Simulator, C/C++, Arduino Community Edition.
- **Carte cible par défaut : Arduino Uno** (`arduino:avr:uno`).

## 5. Méthodologie d'ingénierie — cycle en V

Produire les documents dans CET ordre, AVANT le code fonctionnel :

1. `docs/besoin.md` — pourquoi ce système existe, contexte d'usage.
2. `docs/cahier-des-charges.md` — exigences fonctionnelles avec critères **MESURABLES**
   (tableau : ID, exigence, critère chiffré).
3. `docs/analyse-fonctionnelle.md` — décomposition en fonctions indépendantes de la
   technologie (FP1, FP2…).
4. `docs/architecture.md` — choix des composants justifiés par fonction + tableau de
   correspondance des pins.
5. **Code** modulaire (pas tout dans un `.ino`), tests unitaires via wokwi-cli +
   scénarios YAML.
6. `docs/validation.md` — reprend le cahier des charges, coche chaque exigence validée
   avec preuve (log, résultat de test).

## 6. Workflow par session

1. Avancer **fonction par fonction**, jamais tout d'un coup.
2. Après chaque modif : **compiler** avec `arduino-cli`, montrer les erreurs.
3. **Tester** avec `wokwi-cli` avant de dire « c'est fait ».
4. Montrer `git diff` avant de valider un commit.
5. **Commit Git à chaque étape fonctionnelle validée**, message clair et descriptif.
6. **Ne JAMAIS push sans confirmation explicite.**

## 7. À proscrire

- Code en un seul bloc massif sans étapes intermédiaires.
- Librairies / abstractions compliquées avant compréhension du principe de base.
- Simplifications qui cachent la logique électronique/physique réelle.
- Push GitHub sans confirmation explicite.

## 8. État du projet

- **Projet choisi** : **Régulateur proportionnel de vitesse de ventilateur.**
  Un capteur de température mesure la T d'un système ; un potentiomètre fixe la
  consigne ; la vitesse d'un ventilateur (moteur DC en PWM) est modulée *continûment*
  (loi **proportionnelle**) pour ramener la mesure vers la consigne. Restitution :
  écran LCD (I²C) + Serial Monitor ; LED d'alarme sur seuil. Tout en **simulation
  Wokwi**, cible **Arduino Uno**.
  Ancrage réel : ventilateur de radiateur (Formula Student) / refroidissement d'enceinte.
- **Docs cycle en V** : besoin ✅, cahier des charges ✅, analyse fonctionnelle ✅,
  architecture ✅ (capteur = **NTC** analogique confirmé). `docs/validation.md` ouvert
  et tenu à jour au fil des modules.
- **Code** :
  - **Module 1 — FP1 acquisition NTC ✅** (`src/capteur_temp.h/.cpp`). Conversion validée
    0/25/50 °C, `tests/test_capteur.yaml` au vert.
  - **Module 2 — FP2 consigne potentiomètre ✅** (`src/consigne.h/.cpp`). Interpolation
    flottante 20–45 °C (pas de `map()`), `tests/test_consigne.yaml` au vert (potentiomètre
    pilotable par `set-control position`).
  - **Module 3 — FP3 régulation ✅** (`src/regulation.h/.cpp`). Loi P (BP=5, saturation),
    machine à états REPOS/REGULATION/ALARME. Testé à **2 niveaux** : test unitaire natif
    g++ (`tests/test_regulation_unit.cpp`, 9/9 déterministe) + intégration Wokwi
    (`tests/run_regulation_tests.sh`, incl. ALARME via `--diagram-file`).
  - **Module 4 — FP4 actionneur ✅** (`src/actionneur.h/.cpp`). PWM sur D9. En sim, Wokwi
    n'a pas de moteur CC → LED proxy de vitesse ; étage de puissance réel (MOSFET+diode)
    documenté dans `architecture.md §3`. EF4 vérifiée par **VCD** (analyseur logique +
    `tests/mesure_duty.py`, rapport cyclique 49,4 % à PWM=126). Ordonnanceur 100 ms/500 ms
    en place (EP1, EC3).
  - **Module 5 — FP6 alarme ✅** (`src/alarme.h/.cpp`). LED rouge sur D8, allumée SSI état
    ALARME. Vérifié par état réel de la broche D8 dans le VCD (`tests/run_alarme_test.sh`).
  - **Module 6 — FP5 affichage LCD I²C ✅** (`src/affichage.h/.cpp`). Librairie externe
    `LiquidCrystal_I2C` (0x27), bus I²C A4/A5. Preuve visuelle par screenshots
    (`docs/img/lcd_*.png`).
- **✅ PROJET COMPLET** : les 7 fonctions livrées et testées. **14/15 exigences validées**
  (seule réserve : EP2 erreur statique, non validable en sim — voir `validation.md §7`).
  README.md rédigé. Dépôt à jour sur GitHub.
- **Évolutions identifiées** (hors périmètre) : régulateur PID (annule l'erreur statique),
  banc de test thermique pour la boucle fermée dynamique, matériel réel.
- **Diagrammes** : `diagram.json` (procédé 25 °C) et `tests/diagram_alarme.json` (35 °C),
  tous deux avec LED ventilateur (D9), LED alarme (D8) et analyseur logique (D0=D9, D1=D8).
- **Astuce testabilité** : le module `regulation.cpp` n'inclut que `<math.h>` (aucune API
  Arduino) → compilable sur PC pour tests unitaires natifs. Le potentiomètre est pilotable
  dynamiquement (`set-control position` 0–1), le NTC non (température init-only → on relance
  avec `--diagram-file`).
- **Rendu visuel** : (1) extension VS Code Wokwi = simulateur graphique live interactif ;
  (2) `wokwi-cli --screenshot-part/-time/-file` = PNG headless ; (3) `--vcd-file` = chronogrammes.
- **Câblage retenu** : A0=NTC, A1=potentiomètre, A4/A5=I²C LCD, D9=PWM ventilateur,
  D8=LED alarme.
- **⚠️ Limite outil connue** : le capteur NTC Wokwi n'est pas pilotable dynamiquement
  (`set-control` sans effet) → stratégie de test en boucle fermée à trancher au module
  régulation (cf. `validation.md §1` et `architecture.md §7`).
- **Dépôt distant** : https://github.com/Patxi-Sallaberry/arduino-projects (public,
  `main` suit `origin/main`).

## 9. Structure du dépôt

```
arduino-projects/
├── CLAUDE.md                 # ce fichier
├── .gitignore
├── docs/                     # documents du cycle en V
├── src/                      # code source (.ino, .h, .cpp)
├── tests/                    # scénarios de test wokwi (.yaml) + diagram.json
└── .github/workflows/        # CI (compilation + tests automatisés)
```
