# Cahier des charges — Régulateur proportionnel de vitesse de ventilateur

> **Rôle de ce document.** Il traduit le [besoin](besoin.md) en **exigences
> mesurables et vérifiables**. Chaque exigence porte : un **identifiant** stable, un
> **critère chiffré**, et un **moyen de vérification**. C'est la référence contractuelle
> du projet : le document `validation.md` reprendra chaque ID pour prouver qu'il est
> satisfait.
>
> Convention d'identifiants :
> - **EF** — exigence **fonctionnelle** (ce que le système doit faire),
> - **EP** — exigence de **performance** (avec quelle qualité / dans quels délais),
> - **EC** — exigence de **conception / contrainte** (comment c'est réalisé).

---

## 1. Rappel du besoin (traçabilité)

Maintenir automatiquement la température d'un système proche d'une **consigne réglable**,
en **modulant continûment** la vitesse d'un ventilateur (loi **proportionnelle**), avec
restitution visible (LCD + Serial) et signalement d'anomalie (LED). Cible **Arduino Uno**,
en **simulation Wokwi**. Voir [`besoin.md`](besoin.md) pour le *pourquoi*.

## 2. Grandeurs et notations

| Symbole | Signification | Unité |
|---|---|---|
| `T_mesuree` | Température lue par le capteur | °C |
| `T_consigne` | Température cible réglée par l'opérateur (potentiomètre) | °C |
| `e` | **Erreur** de régulation, `e = T_mesuree − T_consigne` | °C |
| `cmd` | Commande envoyée au ventilateur (rapport cyclique PWM) | 0–255 (8 bits) |

> Convention de signe : on **refroidit**, donc `e > 0` signifie « trop chaud » → il faut
> **augmenter** la vitesse du ventilateur. `e ≤ 0` signifie « assez froid » → ventilateur
> au repos.

## 3. Spécification de la loi de commande (le cœur du système)

Loi **proportionnelle pure (P)**, à bande proportionnelle `BP = 5 °C` :

```
si  e <= 0        :  cmd = 0            (ventilateur au repos)
si  e >= BP (5°C) :  cmd = 255          (ventilateur à 100 %)
sinon             :  cmd = round(255 * e / BP)   (variation linéaire)
```

- Gain proportionnel équivalent : `Kp = 255 / 5 = 51` pas de PWM par °C d'erreur.
- **Limite assumée du P seul** : en régime établi, la commande n'est non nulle que si
  l'erreur est non nulle ⇒ il subsiste une **erreur statique** (voir **EP2**). C'est
  volontaire : on veut comprendre la boucle proportionnelle avant d'ajouter les termes
  intégral/dérivé (PID) — inscrit comme **évolution future**, hors périmètre ici.

## 4. Exigences fonctionnelles (EF)

| ID | Exigence | Critère mesurable | Moyen de vérification |
|----|----------|-------------------|-----------------------|
| **EF1** | Le système mesure la température ambiante. | Plage exploitable **0 à 60 °C**. | Scénario Wokwi : imposer plusieurs températures, vérifier la valeur lue (tolérance de conversion documentée). |
| **EF2** | L'opérateur règle la consigne via le potentiomètre. | Position du potentiomètre → consigne **linéaire de 15 à 35 °C** (butée basse = 15, butée haute = 35). Plage **centrée sur la température nominale du procédé** (25 °C, au milieu de la course). | Scénario Wokwi : imposer 3 positions (0 %, 50 %, 100 %), vérifier consigne ≈ 15 / 25 / 35 °C. |
| **EF3** | Le système applique une loi **proportionnelle** (cf. §3). | Bande proportionnelle **5 °C** ; `e≤0 → cmd=0` ; `e≥5 → cmd=255` ; linéaire entre les deux. | Scénario Wokwi : imposer `e = −2, 0, +2,5, +5, +8 °C`, vérifier `cmd = 0, 0, 128, 255, 255`. |
| **EF4** | Le système pilote la vitesse du ventilateur. | Sortie **PWM 8 bits (0–255)** via `analogWrite`, recalculée à chaque cycle. | Scénario Wokwi : lire le rapport cyclique du moteur, cohérent avec `cmd`. |
| **EF5** | Le système affiche l'état sur écran LCD. | Affiche `T_mesuree`, `T_consigne`, `% ventilo`, `état`, rafraîchi toutes les **500 ms**. | Inspection visuelle + trace ; vérifier période de rafraîchissement. |
| **EF6** | Le système trace l'état sur la liaison série. | Une ligne **formatée et parsable** toutes les **500 ms** (ex. `T=..;C=..;PWM=..;ETAT=..`). | Capture du flux série par `wokwi-cli`, parsing automatique. |
| **EF7** | Le système signale une anomalie thermique. | LED d'alarme **allumée** dès que `e ≥ 8 °C` (ventilateur saturé mais température toujours trop haute), **éteinte** sinon. | Scénario Wokwi : imposer `e = 7 → 9 → 7 °C`, vérifier LED off→on→off. |

## 5. Exigences de performance (EP)

| ID | Exigence | Critère mesurable | Moyen de vérification |
|----|----------|-------------------|-----------------------|
| **EP1** | La boucle de régulation tourne à cadence fixe et **non bloquante**. | Cycle complet mesure→décision→commande toutes les **100 ms (10 Hz)**, cadencé par `millis()` (jamais `delay()`). | Analyse de code (EC3) + mesure de la période effective sur trace série. |
| **EP2** | La régulation maintient la température près de la consigne. | En régime établi sous charge nominale, **`|e| ≤ 2 °C`**. *(Erreur statique résiduelle assumée du P seul ; l'annulation complète relèverait d'un terme intégral, hors périmètre.)* | Scénario Wokwi avec modèle thermique : laisser converger, mesurer l'écart final. |
| **EP3** | La commande réagit sans retard perceptible à une variation d'entrée. | Après un changement de `T_mesuree` ou de `T_consigne`, `cmd` est recalculée en **≤ 1 cycle (≤ 100 ms)**. | Scénario Wokwi : échelon d'entrée, vérifier mise à jour de `cmd` au cycle suivant. |

## 6. Exigences de conception / contraintes (EC)

| ID | Exigence | Critère mesurable | Moyen de vérification |
|----|----------|-------------------|-----------------------|
| **EC1** | Cible matérielle et compilation. | Compile **sans erreur ni warning bloquant** pour `arduino:avr:uno` via `arduino-cli`. | `arduino-cli compile --fqbn arduino:avr:uno`. |
| **EC2** | Architecture logicielle modulaire. | Code **multi-fichiers** : responsabilités séparées (acquisition mesure, calcul régulation, affichage, machine à états) en `.h`/`.cpp` distincts, pas tout dans le `.ino`. | Revue de l'arborescence `src/`. |
| **EC3** | Boucle non bloquante. | **Aucun `delay()`** dans la boucle principale de régulation ; ordonnancement par comparaison de `millis()`. | Revue de code (recherche de `delay(` ). |
| **EC4** | Testabilité automatisée. | Chaque exigence EF/EP dispose d'**au moins un scénario `wokwi-cli` (YAML)** rejouable en ligne de commande. | Exécution de la suite de tests dans `tests/`. |
| **EC5** | Machine à états explicite. | Le comportement se décrit par des **états nommés** (ex. `REPOS`, `REGULATION`, `ALARME`) et des transitions documentées. | Revue de code + diagramme dans `architecture.md`. |

## 7. Récapitulatif des états attendus (préfiguration)

| État | Condition d'entrée | Comportement ventilateur | LED |
|---|---|---|---|
| `REPOS` | `e ≤ 0` | Arrêt (`cmd = 0`) | Éteinte |
| `REGULATION` | `0 < e < 8 °C` | Vitesse proportionnelle à `e` | Éteinte |
| `ALARME` | `e ≥ 8 °C` | Vitesse maximale (`cmd = 255`) | **Allumée** |

*(Cette table sera formalisée en diagramme d'états dans `architecture.md` ; elle est
donnée ici pour vérifier la cohérence entre EF3, EF7 et EC5.)*

---

## 8. Ce qui n'est PAS exigé (rappel de périmètre)

- Régulation **PID complète** (termes I et D) — évolution future.
- Matériel physique, étage de puissance, datalogging SD, communication réseau.

*Prochaine étape du cycle en V : `analyse-fonctionnelle.md` (décomposition en fonctions
FP1, FP2… indépendantes de la technologie), puis `architecture.md` (choix des composants
Wokwi et table de correspondance des pins).*
