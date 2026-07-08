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
- **Étape en cours** : rédaction du besoin (`docs/besoin.md`), puis cahier des charges.

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
