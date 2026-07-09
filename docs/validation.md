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
| **EF2** | Régler la consigne (potentiomètre → 15–35 °C) | ✅ | §2 ci-dessous |
| **EF3** | Loi proportionnelle (BP = 5 °C) | ✅ | §3 ci-dessous |
| **EF4** | Piloter la vitesse du ventilateur (PWM 8 bits) | ✅ | §4 ci-dessous |
| **EF5** | Afficher sur LCD | ✅ | §6 ci-dessous |
| **EF6** | Tracer sur Serial (500 ms) | ✅ | Ligne parsable `T=.. \| Cons=.. \| e=.. \| PWM=.. \| ETAT=..` toutes les 500 ms |
| **EF7** | Alarme de seuil (e ≥ 8 °C) | ✅ | §5 ci-dessous |
| **EP1** | Boucle 100 ms non bloquante | ✅ | Ordonnanceur 2 cadences (régul. 100 ms / affichage 500 ms) par `millis()` dans `src.ino` |
| **EP2** | Erreur statique ≤ 2 °C | ⚠️ | Non validable en simulation — voir §7 (limite) |
| **EP3** | Réaction ≤ 1 cycle | ✅ | Ordonnanceur 100 ms : la commande est recalculée à chaque cycle (observé aux tests FP3/FP4) |
| **EC1** | Compile sans erreur (Uno) | ✅ | §1 (compilation `arduino-cli`, exit 0) |
| **EC2** | Code modulaire multi-fichiers | ✅ | 6 modules `.h/.cpp` (capteur_temp, consigne, regulation, actionneur, alarme, affichage) + `.ino` d'orchestration |
| **EC3** | Aucun `delay()` bloquant | ✅ | `grep delay( src/` → aucune occurrence (hors commentaire) |
| **EC4** | Testabilité (scénario wokwi-cli) | ✅ | Suite complète : scénarios YAML + test unitaire natif + mesures VCD |
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

**Principe** : potentiomètre sur A1, position → consigne 15–35 °C par interpolation
**flottante** (pas de `map()` entier, cf. code commenté). Plage **centrée sur les 25 °C
nominaux du procédé** : voir §8 pour la justification de ce dimensionnement.

**Test automatisé (EC4)** — `wokwi-cli . --scenario tests/test_consigne.yaml`.
Contrairement au NTC, le potentiomètre Wokwi **est pilotable** (contrôle `position`,
0.0–1.0), donc test **100 % automatique** :

| Position potentiomètre | Consigne lue | Attendu | Résultat |
|:---:|:---:|:---:|:---:|
| 0.0 (butée basse) | **15.0 °C** | 15,0 °C | ✅ |
| 0.5 (milieu)      | **25.0 °C** | 25,0 °C | ✅ |
| 1.0 (butée haute) | **35.0 °C** | 35,0 °C | ✅ |

```
Expected text matched: "Cons=15.0 C"
Expected text matched: "Cons=25.0 C"
Expected text matched: "Cons=35.0 C"
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

- à **T = 25 °C** (`test_regulation.yaml`) : consigne 35 °C → `PWM=0 | ETAT=REPOS` ✅ ;
  consigne 22,5 °C → `e=2.4 | PWM=125 | ETAT=REGULATION` ✅
  *(PWM 125 ≠ 128 : quantification analogique — d'où l'intérêt du test unitaire pour la
  valeur exacte).*
- à **T = 30 °C** (`test_regulation_alarme.yaml`, via `--diagram-file`) : consigne 15 °C →
  `e=15.0 | PWM=255 | ETAT=ALARME` ✅ ; consigne 35 °C → `e=-5.0 | PWM=0 | ETAT=REPOS` ✅
  *(démontre que la régulation vise la consigne, pas une température absolue).*

**Choix des points de test** — chaque assertion est prise **à distance des frontières
d'état** (`e = 0` et `e = 8`). Le cas REGULATION vise `e = 2,5` (milieu de bande) et non la
butée basse, qui donnerait `e = 10` donc ALARME ; le cas REPOS à chaud vise `e = −5` et non
`e = 0`, où un dixième de degré de bruit ADC ferait basculer l'état. Un test posé sur une
égalité exacte ne teste rien.

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

| Consigne (pot 0.375) | Écart e | PWM attendu | Rapport cyclique mesuré sur D9 |
|:---:|:---:|:---:|:---:|
| 22,5 °C | 2,5 °C | 126/255 | **49,1 %** (attendu 49,4 % ±3) → ✅ |

```
Rapport cyclique D9 (regime etabli 250 ms) : 49.1 % (attendu 49.4 +/- 3.0)  [OK]
```

*Finesse mesurée* : sur la fenêtre complète on lisait 39 % — car elle incluait le
**transitoire de démarrage** (~100 ms à PWM=0 avant le 1ᵉʳ cycle de régulation). En
excluant le transitoire (mesure sur les derniers 250 ms), on retrouve 49,4 % exact.
→ **EF4 validée.**

**Ordonnancement (EP1, EC3)** : `src.ino` cadence la régulation à **100 ms** et
l'affichage à **500 ms** par comparaison de `millis()`, **sans aucun `delay()`**
(vérifié par `grep`). → **EP1 et EC3 validées.**

---

## §5 — EF7 : LED d'alarme (module `alarme`)

**Principe** : `majAlarme(etat)` = `digitalWrite(D8, etat == ALARME ? HIGH : LOW)`.

**Vérification de l'état réel de la broche D8** — `tests/run_alarme_test.sh` : l'état
final de D8 est lu dans le VCD (`tests/verifie_pin.py`) selon la température du procédé :

| Cas | T procédé | Consigne | État | Broche D8 (LED) | Résultat |
|---|:---:|:---:|:---:|:---:|:---:|
| Alarme | 30 °C | 20 °C | ALARME (e=10) | **1 (allumée)** | ✅ |
| Hors alarme | 25 °C | 20 °C | REGULATION (e=5) | **0 (éteinte)** | ✅ |

*Le scénario est **partagé** entre les deux diagrammes : une seule consigne (20 °C, pot à
0.25) doit discriminer les deux cas. Le seuil d'alarme (8 °C) tombe entre `e=5` et `e=10`,
avec 3 °C de marge d'un côté et 2 °C de l'autre.*

```
Etat final D1 (= broche D8, LED alarme) : 1 (attendu 1)  [OK]
Etat final D1 (= broche D8, LED alarme) : 0 (attendu 0)  [OK]
########## FP6 : LED alarme OK ##########
```

→ **EF7 validée.**

---

## §6 — EF5 : Affichage LCD 16x2 I²C (module `affichage`)

**Principe** : librairie externe `LiquidCrystal_I2C` (adresse 0x27) sur le bus I²C
(A4 = SDA, A5 = SCL). Deux lignes, largeur fixe 16 caractères (padding `%-16s` pour
effacer les résidus). Rafraîchi toutes les 500 ms.

**Preuve visuelle** (captures headless `wokwi-cli --screenshot-part lcd1`) :

État régulation (procédé 25 °C, consigne 22,5 °C) :

![LCD en régulation](img/lcd_regulation.png)

```
T:25.0 C:22.5
Vt:49% REGUL
```

État alarme (procédé 30 °C, consigne 20 °C) :

![LCD en alarme](img/lcd_alarme.png)

```
T:30.0 C:20.0
Vt:100% ALARM
```

→ **EF5 validée.**

*Effet de bord constaté* : l'`init()` de la librairie LCD contient des `delay()`
internes (init du contrôleur I²C) qui rallongent `setup()`. Cela n'affecte pas la boucle
(aucun `delay()` dans `loop()`), mais a imposé d'allonger les fenêtres de capture VCD des
tests FP4/FP6 (le régime établi arrive plus tard).

---

## §7 — EP2 : erreur statique — limite de validation assumée

**EP2 (`|e| ≤ 2 °C` en régime établi) n'est PAS validée en simulation**, pour deux
raisons cumulées, honnêtement documentées :

1. **Pas de boucle fermée dynamique possible** : le capteur NTC de Wokwi n'est pilotable
   qu'à l'initialisation (cf. §1) et Wokwi n'a pas de moteur/procédé thermique. On ne peut
   donc pas faire *réagir* la température au ventilateur pour observer la convergence.
2. **Le régulateur proportionnel seul laisse, par principe, une erreur statique non nulle**
   (cf. `cahier-des-charges.md §3`). Garantir `|e| ≤ 2 °C` dépend du gain et de la
   dynamique du procédé réel.

**Comment la valider** : sur matériel réel (procédé thermique + ventilateur), ou en
ajoutant un **modèle thermique** au banc de test, puis en mesurant l'écart en régime
établi. C'est aussi le point d'entrée naturel vers un **terme intégral (PI/PID)** pour
annuler cette erreur — évolution déjà identifiée hors périmètre.

---

## §8 — Révision d'exigence : dimensionnement de la plage de consigne (EF2)

Seule exigence **modifiée après validation initiale**. Le cycle en V impose d'en tracer le
constat, la cause, la décision et la revalidation.

### 8.1 — Constat

Relevé en **simulation interactive** (panneau graphique Wokwi, procédé à 25 °C), et non par
un test automatisé : en balayant le potentiomètre sur toute sa course, la commande
ventilateur ne réagissait que sur le **premier cinquième** de celle-ci. Au-delà, système
figé en `REPOS`, `PWM=0`, LED éteinte. L'opérateur a l'impression que le réglage est mort.

### 8.2 — Cause

La loi ne produit une commande que si `e = T − Cons > 0` (`regulation.cpp`, 1ʳᵉ branche).
À procédé fixé à 25 °C, cela impose `Cons < 25 °C`. Or la plage EF2 initiale était
**20–45 °C**, c'est-à-dire **entièrement au-dessus** du point de fonctionnement :

| Portion de la course | Consigne | Écart `e` | Comportement |
|---|:---:|:---:|---|
| 0 % → 20 % | 20 → 25 °C | +5 → 0 °C | zone active : PWM 255 → 0 |
| 20 % → 100 % | 25 → 45 °C | 0 → −20 °C | **zone morte** : PWM 0, `REPOS` |

**80 % de la course inopérante.** Corollaire : l'état `ALARME` (`e ≥ 8 °C`, soit
`Cons ≤ 17 °C`) était **inatteignable au potentiomètre**.

Ce n'était **pas un défaut d'implémentation** : EF2 et EF3 étaient l'une et l'autre
respectées, et leurs tests au vert. Le défaut était **dans l'exigence elle-même** — la
plage avait été fixée sans la confronter au point de fonctionnement du procédé. C'est le
type d'écart qu'un cycle en V ne révèle qu'à la validation, alors que le choix a été fait
à la spécification.

### 8.3 — Décision

Plage de consigne **centrée sur la température nominale du procédé** : **15–35 °C**
(`consigne.h`), au lieu de 20–45 °C. Le point de fonctionnement (25 °C) tombe désormais au
**milieu** de la course. Répartition obtenue :

| Portion de la course | Consigne | Écart `e` | État |
|---|:---:|:---:|---|
| 0 % → 10 % | 15 → 17 °C | +10 → +8 °C | **ALARME** (désormais atteignable) |
| 10 % → 50 % | 17 → 25 °C | +8 → 0 °C | **REGULATION** (bande proportionnelle) |
| 50 % → 100 % | 25 → 35 °C | 0 → −10 °C | REPOS (marge de refroidissement) |

Les **trois états** sont atteignables au potentiomètre seul. Le REPOS résiduel n'est pas
une zone morte : c'est la réserve de consigne au-dessus du point de fonctionnement, qui a
un sens physique (tolérer un procédé plus chaud sans ventiler).

Ancrage réel : sur un radiateur de Formula Student, la consigne se règle **autour** de la
température d'eau cible (~85 °C), pas de 0 à 120 °C.

### 8.4 — Impact et revalidation

Le code ne change que par **deux constantes** (`CONSIGNE_MIN`, `CONSIGNE_MAX`) :
`consigne.cpp` interpole entre elles, `regulation.cpp` ignore la plage. La cascade porte
sur les **points de test**, qui étaient exprimés en positions de potentiomètre :

| Fichier | Avant | Après | Raison |
|---|---|---|---|
| `test_consigne.yaml` | 20 / 32,5 / 45 °C | 15 / 25 / 35 °C | nouvelles butées (EF2) |
| `test_regulation.yaml` | pot 0 → REGULATION | pot 0,375 → REGULATION | pot 0 donne `e=10` → **ALARME** |
| `test_actionneur_vcd.yaml` | pot 0,1 | pot 0,375 | même point physique (`e=2,5`) |
| `test_alarme.yaml` | pot 0 | pot 0,25 | consigne 20 °C, discrimine les 2 diagrammes |
| `diagram_alarme.json` | 35 °C | 30 °C | voir ci-dessous |

**Pourquoi abaisser le diagramme d'alarme à 30 °C** : à 35 °C, la nouvelle consigne
maximale vaut exactement 35 °C, donc `e = 0,0` — l'assertion `REPOS` du test se poserait
**pile sur la frontière d'état**, et 0,1 °C de bruit ADC la ferait basculer en
`REGULATION`. À 30 °C, les deux cas retrouvent des marges franches (`e = +15` et `e = −5`).

**Revalidation complète, tous tests au vert** (`test_regulation_unit` 9/9,
`run_regulation_tests.sh`, `run_actionneur_test.sh`, `run_alarme_test.sh`) ; captures LCD
régénérées (§6). Le test unitaire natif n'a **pas été touché** : il éprouve la logique pure
`calculerRegulation(T, Cons)`, indépendante de la plage de consigne — bénéfice direct de
la séparation acquisition / traitement.

**Traçabilité** : aucun statut du tableau de suivi ne change. EF2 reste ✅, sur un critère
révisé.

---

## Synthèse

| Catégorie | Validées | Réserve |
|---|---|---|
| Fonctionnelles (EF1–EF7) | **7 / 7** ✅ | — |
| Performance (EP1, EP3) | **2 / 3** ✅ | EP2 ⚠️ (non validable en sim, cf. §7) |
| Conception (EC1–EC5) | **5 / 5** ✅ | — |

**14 exigences sur 15 validées**, la seule réserve (EP2) étant une **limite intrinsèque
de la simulation + du régulateur P**, clairement tracée et assortie de la voie de
résolution (PID + banc thermique). Le système remplit son cahier des charges dans le
périmètre défini.
