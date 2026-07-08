#ifndef AFFICHAGE_H
#define AFFICHAGE_H

#include "regulation.h"   // pour EtatRegulation

// ============================================================================
//  Module COMMUNIQUER — Affichage sur écran LCD 16x2 I²C (FP5)
//  Restitue en autonomie : température mesurée, consigne, vitesse ventilateur (%)
//  et état courant. Introduit une LIBRAIRIE EXTERNE (LiquidCrystal_I2C) et le
//  BUS I²C (broches imposées de l'Uno : A4 = SDA, A5 = SCL ; cf. architecture.md §4).
// ============================================================================

void initialiserAffichage();

// Met à jour les deux lignes de l'écran avec l'état courant du système.
void majAffichage(float tMesuree, float tConsigne, int pwm, EtatRegulation etat);

#endif // AFFICHAGE_H
