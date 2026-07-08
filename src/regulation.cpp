// Ce module ne fait que du calcul : aucune API Arduino (pas d'analogRead, de
// Serial...). On inclut donc <math.h> (pour round()) plutôt que <Arduino.h>.
// Conséquence : le fichier compile AUSSI sur PC -> tests unitaires natifs (g++).
#include <math.h>
#include "regulation.h"

SortieRegulation calculerRegulation(float tMesuree, float tConsigne) {
  SortieRegulation s;

  // Écart de régulation. Convention : e > 0 => trop chaud => il faut refroidir.
  s.erreur = tMesuree - tConsigne;

  // --- Loi proportionnelle : 3 branches, saturation EXPLICITE -------------
  if (s.erreur <= 0.0) {
    // Assez froid : ventilateur au repos.
    s.commande = PWM_MIN;
  } else if (s.erreur >= BANDE_PROPORTIONNELLE) {
    // Au-delà de la bande proportionnelle : commande saturée à 100 %.
    s.commande = PWM_MAX;
  } else {
    // Zone proportionnelle : commande linéaire en fonction de l'écart.
    // Calcul EN FLOTTANT puis arrondi : une division entière tronquerait.
    s.commande = (int) round((float) PWM_MAX * s.erreur / BANDE_PROPORTIONNELLE);
  }

  // --- Machine à états : combinatoire, déterminée par l'écart courant ------
  // (Pas d'hystérésis : conforme à EF7. L'hystérésis anti-scintillement est
  //  notée comme raffinement futur, au même titre que le passage en PID.)
  if (s.erreur >= SEUIL_ALARME) {
    s.etat = ALARME;        // refroidissement dépassé (déjà saturé) -> on alerte
  } else if (s.erreur > 0.0) {
    s.etat = REGULATION;    // on module activement la vitesse
  } else {
    s.etat = REPOS;         // rien à faire
  }

  return s;
}

const char* nomEtat(EtatRegulation etat) {
  switch (etat) {
    case REPOS:      return "REPOS";
    case REGULATION: return "REGULATION";
    case ALARME:     return "ALARME";
    default:         return "?";
  }
}
