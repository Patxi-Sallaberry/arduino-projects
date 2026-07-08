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
| **EF3** | Loi proportionnelle (BP = 5 °C) | ✅ | §3 ci-dessous |
| **EF4** | Piloter la vitesse du ventilateur (PWM 8 bits) | ✅ | §4 ci-dessous |
| **EF5** | Afficher sur LCD | ⬜ | — |
| **EF6** | Tracer sur Serial (500 ms) | ✅ | Ligne parsable `T=.. \| Cons=.. \| e=.. \| PWM=.. \| ETAT=..` toutes les 500 ms |
| **EF7** | Alarme de seuil (e ≥ 8 °C) | ⬜ | — |
| **EP1** | Boucle 100 ms non bloquante | ✅ | Ordonnanceur 2 cadences (régul. 100 ms / affichage 500 ms) par `millis()` dans `src.ino` |
| **EP2** | Erreur statique ≤ 2 °C | ⬜ | — |
| **EP3** | Réaction ≤ 1 cycle | ⬜ | — |
| **EC1** | Compile sans erreur (Uno) | ✅ | §1 (compilation `arduino-cli`, exit 0) |
| **EC2** | Code modulaire multi-fichiers | 🟡 | Module `capteur_temp.h/.cpp` séparé du `.ino` (à compléter) |
| **EC3** | Aucun `delay()` bloquant | ✅ | `grep delay( src/` → aucune occurrence (hors commentaire) |
| **EC4** | Testabilité (scénario wokwi-cli) | 🟡 | `tests/test_capteur.yaml` opérationnel |
| **EC5** | Machine à états explicite | ✅ | §3 : états REPOS/REGULATION/ALARME nommés, transitions testées |

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

---

## §3 — EF3 / EC5 : Régulation proportionnelle et machine à états (module `regulation`)

Double niveau de test (voir aussi le point méthodo ci-dessous) :

**(a) Test UNITAIRE natif** — `tests/test_regulation_unit.cpp`, compilé et exécuté sur PC
(g++). `calculerRegulation()` étant de la logique pure, on lui passe des couples
`(T, consigne)` **exacts** → vérification **déterministe** (impossible via la chaîne
analogique quantifiée). 9/9 cas au vert :

| Écart e (°C) | PWM attendu | État attendu | Résultat |
|:---:|:---:|:---:|:---:|
| −5 | 0 | REPOS | ✅ |
| 0 | 0 | REPOS | ✅ |
| 1 | 51 | REGULATION | ✅ |
| 2,5 | 128 | REGULATION | ✅ |
| 5 | 255 (saturé) | REGULATION | ✅ |
| 6 | 255 | REGULATION | ✅ |
| 7,9 | 255 | REGULATION | ✅ |
| 8 | 255 | **ALARME** | ✅ |
| 15 | 255 | **ALARME** | ✅ |

**(b) Tests d'INTÉGRATION Wokwi** — `tests/run_regulation_tests.sh` (chaîne réelle
capteur→consigne→loi→état) :

- à **T = 25 °C** (`test_regulation.yaml`) : consigne 45 °C → `PWM=0 | ETAT=REPOS` ✅ ;
  consigne 20 °C → `e=5.0 | PWM=253 | ETAT=REGULATION` ✅
  *(PWM 253 ≠ 255 : quantification analogique — d'où l'intérêt du test unitaire pour la
  valeur exacte).*
- à **T = 35 °C** (`test_regulation_alarme.yaml`, via `--diagram-file`) : consigne 20 °C →
  `e=15.0 | PWM=255 | ETAT=ALARME` ✅ ; consigne 45 °C → `PWM=0 | ETAT=REPOS` ✅
  *(démontre que la régulation vise la consigne, pas une température absolue).*

→ **EF3 et EC5 validées.**

**Note méthodo — deux niveaux de test complémentaires :**
le test **unitaire** prouve la *logique exacte* (déterministe, rapide, hors matériel) ;
le test d'**intégration** prouve que la *chaîne physique complète* se comporte bien.
Les deux sont nécessaires et ne se remplacent pas.

**Outil — capture visuelle** : `wokwi-cli --screenshot-part <id> --screenshot-time <ms>
--screenshot-file <png>` produit un PNG de la carte (vérifié fonctionnel). Sera utilisé
pour illustrer l'état de la LED (FP6) et du LCD (FP5) une fois câblés.

---

## §4 — EF4 / EP1 : Commande du ventilateur en PWM (module `actionneur`)

**Principe** : `appliquerCommande(pwm)` = `analogWrite(D9, pwm)`. Sur matériel réel D9
attaque un MOSFET (+ diode de roue libre) ; en simulation, faute de moteur CC dans Wokwi,
une LED sur D9 visualise le rapport cyclique (proxy de vitesse). Cf. `architecture.md §3`.

**Vérification du rapport cyclique réel (EF4)** — `tests/run_actionneur_test.sh` :
un **analyseur logique** (`tests/diagram_vcd.json`) capture D9, exporté en **VCD**, dont
on mesure le rapport cyclique en régime établi (`tests/mesure_duty.py`).

| Consigne (pot 0.1) | Écart e | PWM attendu | Rapport cyclique mesuré sur D9 |
|:---:|:---:|:---:|:---:|
| 22,5 °C | 2,5 °C | 126/255 | **49,4 %** (attendu 49,4 % ±3) → ✅ |

```
Rapport cyclique D9 (regime etabli 250 ms) : 49.4 % (attendu 49.4 +/- 3.0)  [OK]
```

*Finesse mesurée* : sur la fenêtre complète on lisait 39 % — car elle incluait le
**transitoire de démarrage** (~100 ms à PWM=0 avant le 1ᵉʳ cycle de régulation). En
excluant le transitoire (mesure sur les derniers 250 ms), on retrouve 49,4 % exact.
→ **EF4 validée.**

**Ordonnancement (EP1, EC3)** : `src.ino` cadence la régulation à **100 ms** et
l'affichage à **500 ms** par comparaison de `millis()`, **sans aucun `delay()`**
(vérifié par `grep`). → **EP1 et EC3 validées.**
