# Analyse fonctionnelle — Régulateur proportionnel de vitesse de ventilateur

> **Rôle de ce document.** Décomposer le système en **fonctions**, *indépendamment de
> toute technologie*. On parle ici de « capteur de température », « actionneur de
> refroidissement », « afficheur » — **jamais** de NTC, PWM ou LCD (ça, c'est le rôle de
> [`architecture.md`](architecture.md)). But : choisir plus tard les composants **pour
> remplir des fonctions**, et non l'inverse.
>
> Chaque fonction est **tracée** vers les exigences du [cahier des charges](cahier-des-charges.md).

---

## 1. Le système est une boucle fermée (schéma-bloc)

Le principe fondamental : le système **mesure**, **compare à la consigne**, **agit**, puis
**re-mesure** l'effet de son action. C'est une régulation en **boucle fermée** (feedback).

```
        T_consigne
            │
            ▼
   ┌──────────────────┐   e = T_cons − T_mes   ┌──────────────────┐   commande   ┌──────────────┐
   │  Comparateur     ├───────────────────────►│  Loi de commande ├─────────────►│  Actionneur  │
   │ (calcul écart e) │                        │  (proportionnelle)│              │ refroidisst. │
   └──────────────────┘                        └──────────────────┘              └──────┬───────┘
            ▲                                                                             │ débit d'air
            │                                                                             ▼
            │                                                                    ┌──────────────┐
            │              T_mesuree                                             │   Procédé    │
            └────────────────────────────────────────────────────────────────── │ (système à   │
                              (retour capteur)          ┌──────────────┐         │  refroidir)  │
                                                        │   Capteur T  │◄────────┴──────────────┘
                                                        └──────────────┘
```

> **Point clé à défendre** : sans le **retour** (la flèche `T_mesuree` qui revient au
> comparateur), il n'y a pas de régulation — juste une commande « à l'aveugle ». C'est le
> bouclage qui permet au système de corriger ses propres écarts.

## 2. Diagramme des interacteurs (contexte)

Éléments de l'environnement avec lesquels le système échange :

| Interacteur | Nature de l'échange avec le système |
|---|---|
| **Opérateur** | Fournit la consigne ; reçoit l'information d'état et l'alerte. |
| **Système à refroidir** | Fournit la grandeur mesurée (température) ; reçoit le refroidissement. |
| **Air ambiant** | Milieu dans lequel le ventilateur puise/pousse l'air. |
| **Source d'énergie** | Alimente le système et l'actionneur. |

## 3. Fonctions principales (FP)

| ID | Fonction (formulation indépendante de la techno) | Exigences couvertes |
|----|--------------------------------------------------|---------------------|
| **FP1** | **Acquérir** la température du système à refroidir. | EF1 |
| **FP2** | **Acquérir** la consigne fixée par l'opérateur. | EF2 |
| **FP3** | **Élaborer la commande** de refroidissement à partir de l'écart consigne–mesure (comparaison + loi de régulation). | EF3, EP2, EP3 |
| **FP4** | **Agir** sur la température en modulant l'effort de refroidissement (débit d'air). | EF4 |
| **FP5** | **Restituer** l'état du système à l'opérateur (mesure, consigne, effort, état courant). | EF5, EF6 |
| **FP6** | **Alerter** l'opérateur en cas d'anomalie thermique. | EF7 |
| **FP7** | **Cadencer** l'ensemble à un rythme régulier sans se bloquer. | EP1, EC3 |

## 4. Fonctions contraintes (FC)

| ID | Contrainte | Exigences couvertes |
|----|-----------|---------------------|
| **FC1** | S'exécuter sur la cible matérielle imposée (calculateur embarqué unique). | EC1 |
| **FC2** | Être organisé de façon **modulaire** et **testable**. | EC2, EC4 |
| **FC3** | Réagir en **temps réel** sans blocage. | EP1, EC3 |
| **FC4** | Rendre le comportement **explicite** (états nommés). | EC5 |

## 5. Décomposition en chaînes fonctionnelles (SI)

On range les fonctions selon les deux chaînes classiques des systèmes asservis. C'est
cette décomposition qui **justifiera directement le découpage du code en modules**
(cf. EC2).

### Chaîne d'information — « percevoir et décider »

| Sous-fonction | Rôle | Fonctions concernées |
|---|---|---|
| **ACQUÉRIR** | Convertir les grandeurs physiques (température, position consigne) en valeurs exploitables. | FP1, FP2 |
| **TRAITER** | Calculer l'écart, appliquer la loi, déterminer l'état (machine à états), décider de l'alarme. | FP3, FP6, FP7 |
| **COMMUNIQUER** | Présenter l'information à l'opérateur (affichage, trace, signal d'alerte). | FP5, FP6 |

### Chaîne d'énergie — « agir sur le procédé »

| Sous-fonction | Rôle | Fonctions concernées |
|---|---|---|
| **ALIMENTER / DISTRIBUER** | Fournir et moduler l'énergie vers l'actionneur (dosage de la commande). | FP4 |
| **CONVERTIR** | Transformer l'énergie électrique en énergie mécanique (rotation). | FP4 |
| **TRANSMETTRE / AGIR** | Produire le débit d'air qui refroidit le procédé. | FP4 |

> **Conséquence pour l'architecture logicielle** : ces blocs (ACQUÉRIR / TRAITER /
> COMMUNIQUER / AGIR) donnent une **cartographie naturelle des modules** `.h`/`.cpp` :
> un module de mesure, un module de régulation (loi + états), un module d'affichage, un
> module de commande d'actionneur. On le concrétisera dans `architecture.md`.

## 6. Synthèse de traçabilité besoin → fonctions → exigences

Toutes les fonctions principales sont rattachées à au moins une exigence, et **toutes les
exigences fonctionnelles EF1–EF7 sont couvertes** par une fonction (FP1↔EF1, FP2↔EF2,
FP3↔EF3, FP4↔EF4, FP5↔EF5/EF6, FP6↔EF7). Aucune fonction « orpheline », aucune exigence
non prise en charge.

---

*Prochaine étape : `architecture.md` — choisir les **composants Wokwi** qui réalisent
chaque fonction, justifier ces choix, et établir la **table de correspondance des pins**
de l'Arduino Uno.*
