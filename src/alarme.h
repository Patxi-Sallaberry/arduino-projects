#ifndef ALARME_H
#define ALARME_H

#include "regulation.h"   // pour le type EtatRegulation

// ============================================================================
//  Module COMMUNIQUER — Signalement d'anomalie thermique (FP6)
//  Allume une LED dès que le système est en état ALARME (écart e >= seuil, EF7),
//  c.-à-d. quand le refroidissement est saturé mais la température reste trop haute.
// ============================================================================

const int PIN_LED_ALARME = 8;   // D8 : sortie numérique. Cf. architecture.md §4.

// Configure la broche de la LED d'alarme en sortie (éteinte au départ).
void initialiserAlarme();

// Met à jour la LED en fonction de l'état courant : allumée SSI état == ALARME.
void majAlarme(EtatRegulation etat);

#endif // ALARME_H
