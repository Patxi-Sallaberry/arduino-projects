# Validation — Régulateur proportionnel de vitesse de ventilateur

> **Rôle de ce document.** Boucler le cycle en V : pour **chaque exigence** du
> [cahier des charges](cahier-des-charges.md), consigner le **statut** et la **preuve**
> (log, résultat de test). Document **vivant**, complété au fil des modules.
>
> Statuts : ✅ validé · 🟡 partiel · ⬜ non commencé

---

## Tableau de suivi des exigences

| ID | Exigence | Statut | Preuve |
|----|----------|:------:|--------|
| **EF1** | Mesurer la température (plage 0–60 °C) | ✅ | §1 ci-dessous |
| **EF2** | Régler la consigne (potentiomètre → 20–45 °C) | ✅ | §2 ci-dessous |
| **EF3** | Loi proportionnelle (BP = 5 °C) | ⬜ | — |
| **EF4** | Piloter la vitesse du ventilateur (PWM 8 bits) | ⬜ | — |
| **EF5** | Afficher sur LCD | ⬜ | — |
| **EF6** | Tracer sur Serial (500 ms) | 🟡 | Trace série fonctionnelle dès le module FP1 (format à figer) |
| **EF7** | Alarme de seuil (e ≥ 8 °C) | ⬜ | — |
| **EP1** | Boucle 100 ms non bloquante | 🟡 | Patron `millis()` en place (module FP1), période à mesurer sur cycle complet |
| **EP2** | Erreur statique ≤ 2 °C | ⬜ | — |
| **EP3** | Réaction ≤ 1 cycle | ⬜ | — |
| **EC1** | Compile sans erreur (Uno) | ✅ | §1 (compilation `arduino-cli`, exit 0) |
| **EC2** | Code modulaire multi-fichiers | 🟡 | Module `capteur_temp.h/.cpp` séparé du `.ino` (à compléter) |
| **EC3** | Aucun `delay()` bloquant | 🟡 | Respecté dans le module FP1 (à re-vérifier au global) |
| **EC4** | Testabilité (scénario wokwi-cli) | 🟡 | `tests/test_capteur.yaml` opérationnel |
| **EC5** | Machine à états explicite | ⬜ | — |

---

## §1 — EF1 : Acquisition de la température (module `capteur_temp`)

**Compilation (EC1)** — `arduino-cli compile --fqbn arduino:avr:uno --output-dir build src/` :

```
Sketch uses 3826 bytes (11%) of program storage space. Maximum is 32256 bytes.
Global variables use 242 bytes (11%) of dynamic memory...
exit code: 0
```

**Vérification de la conversion ADC → °C sur la plage** — température imposée via
l'attribut `temperature` du capteur (valeur initiale ; cf. limite de l'outil ci-dessous),
lecture série relevée :

| T imposée (°C) | T lue par le firmware | Écart | Interprétation |
|:---:|:---:|:---:|---|
| 0  | **-0.1 °C** | 0,1 °C | Quantification ADC 10 bits (1 LSB ≈ 0,1 °C) — attendu, pas un bug |
| 25 | **25.0 °C** | 0,0 °C | Point de calibration trivial (`R_ntc = R0`) |
| 50 | **50.0 °C** | 0,0 °C | L'équation β suit correctement |

→ Conversion **exacte à ±0,1 °C** sur la plage testée : **EF1 validée**.

**Test automatisé (EC4)** — `wokwi-cli . --scenario tests/test_capteur.yaml` :

```
[FP1 — Acquisition NTC (lecture nominale à 25 °C)] Expected text matched: "T=25.0 C"
Scenario completed successfully
exit: 0
```

### Limite d'outil identifiée (à tracer pour la suite)

Le capteur **NTC de Wokwi n'expose pas de contrôle dynamique** (`set-control` sans
effet) : sa température est une **valeur d'initialisation** seulement (confirmé par la
doc « initial temperature value » et l'absence de test NTC dans `wokwi/wokwi-part-tests`).

**Impact** : la vérification multi-points se fait en **relançant** la simulation ; et
surtout, on ne pourra pas faire évoluer la température **en réaction** au ventilateur pour
tester la boucle fermée (EP2, EP3). Stratégie de test en boucle fermée **à trancher au
module régulation** (options envisagées : modèle thermique logiciel injecté en mode test ;
capteur pilotable type DS18B20 ; banc de test dédié). Voir `architecture.md §7`.

---

## §2 — EF2 : Acquisition de la consigne (module `consigne`)

**Principe** : potentiomètre sur A1, position → consigne 20–45 °C par interpolation
**flottante** (pas de `map()` entier, cf. code commenté).

**Test automatisé (EC4)** — `wokwi-cli . --scenario tests/test_consigne.yaml`.
Contrairement au NTC, le potentiomètre Wokwi **est pilotable** (contrôle `position`,
0.0–1.0), donc test **100 % automatique** :

| Position potentiomètre | Consigne lue | Attendu | Résultat |
|:---:|:---:|:---:|:---:|
| 0.0 (butée basse) | **20.0 °C** | 20,0 °C | ✅ |
| 0.5 (milieu)      | **32.5 °C** | 32,5 °C | ✅ |
| 1.0 (butée haute) | **45.0 °C** | 45,0 °C | ✅ |

```
Expected text matched: "Cons=20.0 C"
Expected text matched: "Cons=32.5 C"
Expected text matched: "Cons=45.0 C"
Scenario completed successfully  (exit 0)
```

→ Conversion linéaire correcte sur toute la plage : **EF2 validée**.
Non-régression FP1 vérifiée après changement du format d'affichage (test EF1 toujours
au vert).
