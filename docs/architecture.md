# Architecture — Régulateur proportionnel de vitesse de ventilateur

> **Rôle de ce document.** Choisir les **composants** (dans l'univers Wokwi / Arduino Uno)
> qui réalisent chaque fonction de l'[analyse fonctionnelle](analyse-fonctionnelle.md),
> **justifier** chaque choix, exposer la **logique électronique réelle** (même si Wokwi
> la simplifie), établir la **table de correspondance des pins**, et formaliser la
> **machine à états**.

---

## 1. Correspondance Fonction → Composant

| Fonction | Composant retenu (Wokwi) | Alternative écartée | Justification du choix |
|---|---|---|---|
| **FP1** Acquérir la température | **Thermistance NTC** (analogique) | DHT22 (numérique), TMP36 (linéaire) | Réutilise et **approfondit** ta connaissance du diviseur de tension + ADC 10 bits ; oblige à comprendre la conversion physique (équation β) — excellent en jury. La « librairie externe » que tu veux découvrir sera apportée par le LCD (FP5), pas ici. |
| **FP2** Acquérir la consigne | **Potentiomètre** | Boutons +/− | Composant que tu maîtrises déjà (`analogRead` 0–1023) ; réglage continu naturel pour une consigne. |
| **FP3** Élaborer la commande | **Microcontrôleur (Uno)** — logiciel | — | C'est le calcul (écart + loi P + états), donc du code, pas un composant. |
| **FP4** Agir (refroidir) | **Moteur CC** + **MOSFET** + **diode de roue libre** | Servo, relais | Un ventilateur = moteur ; la vitesse se module en **PWM**. Le MOSFET et la diode sont la **réalité électronique** (cf. §3). |
| **FP5** Restituer l'état | **Écran LCD 16×2 I²C** + **liaison série** | OLED SSD1306 | LCD I²C = 2 fils seulement, et introduit une **librairie externe** + le **bus I²C** (objectif d'apprentissage). Le Serial sert au dev et aux tests. |
| **FP6** Alerter | **LED** (rouge) | Buzzer | Signal visuel simple, suffisant pour l'alarme de seuil. |

> **⚠️ Décision à valider** : je pars sur une **thermistance NTC** (analogique) plutôt
> qu'un DHT22. Si tu préfères découvrir une librairie de capteur dédiée dès FP1, on
> bascule sur DHT22 — mais tu perds l'exercice ADC/diviseur de tension, et tu auras de
> toute façon une librairie externe avec le LCD. **Mon conseil : NTC.**

## 2. Focus capteur : de la tension à la température (chaîne d'acquisition)

La thermistance NTC (Negative Temperature Coefficient) est une **résistance qui diminue
quand la température monte**. On l'insère dans un **diviseur de tension** avec une
résistance fixe `R_fixe` :

```
   +5V ──[ R_fixe ]──┬──[ NTC ]── GND
                     │
                     └──► broche analogique (A0)
```

Étapes de conversion (à coder explicitement, sans « magie ») :

1. `adc = analogRead(A0)` → entier **0–1023** (**ADC 10 bits**, piège classique : ce
   n'est pas 0–255 ni 0–4095).
2. Tension : `V = adc * 5.0 / 1023.0`.
3. Résistance de la NTC (loi du diviseur) : `R_ntc = R_fixe * V / (5.0 − V)`.
4. Température (équation **β / Steinhart-Hart simplifiée**) :
   `1/T = 1/T0 + (1/β)·ln(R_ntc/R0)`, avec `T` en **kelvin** (penser à `−273,15` pour °C).
   `R0`, `T0` (souvent 25 °C = 298,15 K) et `β` sont des **paramètres du composant**.

> On isolera ces 4 étapes dans un module dédié (chaîne **ACQUÉRIR**), avec les constantes
> `R_fixe`, `R0`, `T0`, `β` clairement nommées et commentées.

## 3. Focus actionneur : pourquoi on ne branche PAS le moteur sur une broche

**Piège électronique majeur** (que Wokwi masque, mais qu'il faut connaître pour le vrai) :

- Une broche de l'Uno délivre **~20–40 mA max**. Un moteur en tire **bien plus** → on
  grillerait le microcontrôleur.
- Un moteur est **inductif** : à la coupure, il génère une surtension qui détruit les
  semi-conducteurs.

Montage réel correct :

```
        +5V (ou alim moteur)
          │
        [Moteur]
          │
          ├──►|── (diode de roue libre, cathode vers +V)   ← évacue la surtension
          │
      Drain
      [MOSFET canal N, logic-level]   ← interrupteur commandé
      Source ── GND
          │
        Grille ◄── broche PWM (D9) de l'Uno   ← le PWM dose le temps d'ouverture
```

- Le **PWM** (`analogWrite`, 0–255) règle la **fraction de temps** où le MOSFET conduit
  → donc la vitesse moyenne du moteur.
- En simulation Wokwi on pilote plus directement, mais **on documente et on câble le vrai
  schéma** pour rester fidèle à la réalité (ta règle : ne pas cacher l'électronique).

## 4. Table de correspondance des pins (Arduino Uno)

| Broche Uno | Type | Élément connecté | Fonction | Remarque |
|---|---|---|---|---|
| **A0** | Entrée analogique | Diviseur NTC | FP1 mesure T | ADC 10 bits |
| **A1** | Entrée analogique | Potentiomètre | FP2 consigne | ADC 10 bits |
| **A4 (SDA)** | I²C data | LCD 16×2 I²C | FP5 affichage | Broche I²C **imposée** sur Uno |
| **A5 (SCL)** | I²C clock | LCD 16×2 I²C | FP5 affichage | Broche I²C **imposée** sur Uno |
| **D9** | Sortie **PWM** | Grille MOSFET → moteur | FP4 commande ventilo | D9 est PWM (Timer1) ; broches PWM Uno : 3,5,6,9,10,11 |
| **D8** | Sortie numérique | LED d'alarme | FP6 alerte | Simple TOR |

> Pas de conflit : A0/A1 pour l'analogique, A4/A5 réservées à l'I²C, D9 pour le PWM,
> D8 pour la LED. La liaison série (FP5) utilise l'USB (D0/D1), à ne pas réaffecter.

## 5. Machine à états (formalisation de FP3 + EC5)

```
                e ≤ 0
        ┌───────────────────┐
        ▼                   │
   ┌─────────┐   e > 0   ┌──────────────┐   e ≥ 8°C   ┌──────────┐
   │  REPOS  ├──────────►│  REGULATION  ├────────────►│  ALARME  │
   │ cmd=0   │◄──────────┤ cmd∝e (0-255)│◄────────────┤ cmd=255  │
   │ LED off │   e ≤ 0   │ LED off      │   e < 8°C   │ LED on   │
   └─────────┘           └──────────────┘             └──────────┘
```

| État | Condition | Ventilateur | LED |
|---|---|---|---|
| `REPOS` | `e ≤ 0` | `cmd = 0` | éteinte |
| `REGULATION` | `0 < e < 8 °C` | `cmd = round(255·e/5)`, saturé à 255 | éteinte |
| `ALARME` | `e ≥ 8 °C` | `cmd = 255` | **allumée** |

> Note de cohérence : dès `e ≥ 5 °C` la commande est déjà saturée à 255 (bande
> proportionnelle EF3) ; l'état `ALARME` (`e ≥ 8 °C`) ajoute uniquement le **signalement**
> — la physique de commande, elle, est déjà au maximum.

## 6. Cartographie des modules de code (préfiguration, cf. EC2)

Découpage issu des chaînes fonctionnelles (analyse fonctionnelle §5) :

| Module (`.h`/`.cpp`) | Responsabilité | Fonctions |
|---|---|---|
| `capteur_temp` | Lire A0 → °C (les 4 étapes du §2) | FP1 |
| `consigne` | Lire A1 → consigne 20–45 °C | FP2 |
| `regulation` | Écart, loi P, machine à états | FP3, FP6 |
| `actionneur` | Écrire le PWM sur D9 | FP4 |
| `affichage` | LCD + Serial | FP5 |
| `.ino` principal | Ordonnancement non bloquant (`millis()`) | FP7 |

## 7. Point ouvert : modèle thermique pour tester en boucle fermée

En simulation, la NTC ne « chauffe » pas toute seule : pour vérifier **EP2** (erreur
statique) et **EP3** (réaction) en *boucle fermée* réelle, il faudra **imposer/piloter la
température dans les scénarios de test** (ou modéliser un procédé thermique simple). À
décider au moment des tests (`tests/`) ; **noté, pas bloquant** pour l'architecture.

---

*Prochaine étape : développement du code, **module par module** (en commençant par la
chaîne d'acquisition FP1), avec compilation `arduino-cli` et test `wokwi-cli` à chaque
étape — conformément au workflow.*
