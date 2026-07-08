# Besoin — Régulateur proportionnel de vitesse de ventilateur

> **Rôle de ce document.** Il capture le *pourquoi* du système, de façon
> **qualitative**. Il ne contient **volontairement aucun chiffre ni critère
> mesurable** : ceux-ci sont la mission du cahier des charges (`cahier-des-charges.md`).
> Tant qu'on n'est pas d'accord sur le *pourquoi*, on ne quantifie rien.

---

## 1. Origine et contexte du besoin

De nombreux systèmes doivent être **refroidis** pour rester dans une plage de
température de fonctionnement : composant électronique dans un boîtier, radiateur de
moteur (contexte **Formula Student**), alimentation, baie informatique…

Deux façons *naïves* de refroidir posent problème :

- **Ventilateur toujours à l'arrêt** → le système surchauffe dès que la charge augmente.
- **Ventilateur toujours à pleine vitesse** → bruit permanent, usure prématurée,
  consommation électrique inutile, et aucune adaptation à la charge réelle.

Une **régulation tout-ou-rien** (le ventilateur démarre à fond au-dessus d'un seuil,
s'arrête en dessous) est déjà mieux, mais elle produit des **à-coups** (le ventilateur
claque entre 0 % et 100 %) et un **battement** autour du seuil.

Le besoin est donc celui d'une régulation **fine et continue** : faire varier la
*vitesse* du ventilateur **proportionnellement** à l'écart entre la température mesurée
et la température souhaitée, pour maintenir le système à la bonne température **tout en
ne dépensant que l'effort de refroidissement nécessaire**.

## 2. Énoncé du besoin (méthode APTE — « bête à cornes »)

- **À qui le système rend-il service ?**
  À l'**utilisateur/opérateur** qui doit maintenir un système à une température maîtrisée
  (ici : un étudiant instrumentant un banc ; par analogie, l'électronique embarquée d'un
  véhicule Formula Student).

- **Sur quoi le système agit-il ?**
  Sur la **température d'un système à refroidir**, via le **débit d'air** d'un ventilateur.

- **Dans quel but ? (pourquoi le besoin existe)**
  Pour **maintenir automatiquement la température proche d'une consigne réglable**, en
  **modulant la vitesse du ventilateur** plutôt qu'en le pilotant en tout-ou-rien —
  afin de limiter le bruit, l'usure et la consommation, et d'informer l'opérateur en
  cas d'anomalie.

## 3. Parties prenantes et acteurs

| Acteur | Rôle vis-à-vis du système |
|---|---|
| **Opérateur** | Règle la consigne (potentiomètre), lit l'état (LCD / Serial), est alerté (LED). |
| **Système à refroidir** | Source de la température mesurée ; c'est la grandeur régulée. |
| **Ventilateur** | Actionneur : son débit d'air évacue la chaleur. |
| **Environnement** | Apporte des perturbations (température ambiante, charge thermique variable). |

## 4. Contexte d'usage

- **Cadre** : projet d'apprentissage en **ingénierie systèmes** (cycle en V complet),
  premier projet Arduino structuré de l'utilisateur.
- **Réalisation** : entièrement en **simulation Wokwi** (aucun achat de matériel dans un
  premier temps), pilotée en ligne de commande (`wokwi-cli`), cible **Arduino Uno**.
- **Restitution attendue** : la mesure, la consigne et l'état du système doivent être
  **visibles en continu** (écran LCD pour l'autonomie, Serial Monitor pour le
  développement et les tests), et une **anomalie** doit être **signalée** (LED d'alarme).
- **Interactivité attendue** : l'opérateur doit pouvoir **changer la consigne à tout
  moment** et voir le ventilateur réagir — c'est un système de **régulation en boucle
  fermée**, pas un simple enregistreur de données.

## 5. Finalité et bénéfices attendus (qualitatifs)

- **Régulation en boucle fermée** : le système mesure, décide, agit, puis re-mesure.
- **Réponse graduée** : la vitesse du ventilateur suit l'écart à la consigne, sans
  à-coups tout-ou-rien.
- **Consigne réglable en temps réel** par l'opérateur.
- **Observabilité** : à tout instant, on sait ce que le système mesure et ce qu'il fait.
- **Sûreté** : une température anormalement haute (hors de la plage de régulation) est
  signalée.

## 6. Périmètre

**Dans le périmètre :**
- Mesure de température, réglage de consigne, calcul d'une commande **proportionnelle**,
  pilotage de la vitesse du ventilateur, affichage, alarme de seuil.
- Architecture logicielle modulaire et testable (machine à états, code multi-fichiers).

**Hors périmètre (pour ce premier projet, à discuter plus tard) :**
- Régulation **PID complète** (termes intégral et dérivé) — on commence par le terme
  **proportionnel** seul, pour comprendre la boucle avant de la complexifier.
- Matériel physique réel, alimentation de puissance, étage transistor du moteur.
- Enregistrement de données (datalogging SD), communication réseau.

---

*Prochaine étape : traduire ces intentions en **exigences mesurables** dans
`cahier-des-charges.md` (chaque ligne du §5 deviendra une exigence chiffrée et testable).*
