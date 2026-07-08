#ifndef ACTIONNEUR_H
#define ACTIONNEUR_H

// ============================================================================
//  Module AGIR — Commande de la vitesse du ventilateur (FP4)
//  La vitesse est pilotée en PWM sur une broche dédiée.
//
//  SUR MATÉRIEL RÉEL : cette broche attaque la GRILLE d'un MOSFET qui commute le
//  moteur, avec une DIODE DE ROUE LIBRE (cf. architecture.md §3). On ne pilote
//  JAMAIS un moteur directement depuis une broche (courant trop faible, surtension
//  inductive à la coupure).
//
//  EN SIMULATION Wokwi : Wokwi ne fournit pas de moteur CC. On visualise donc le
//  rapport cyclique par la LUMINOSITÉ d'une LED sur D9 (proxy honnête de la vitesse).
// ============================================================================

const int PIN_VENTILATEUR = 9;   // D9 : broche PWM (Timer1). Cf. architecture.md §4.

// Configure la broche du ventilateur en sortie.
void initialiserActionneur();

// Applique la commande de vitesse (rapport cyclique PWM, 0..255).
void appliquerCommande(int pwm);

#endif // ACTIONNEUR_H
