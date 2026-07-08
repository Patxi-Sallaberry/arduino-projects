#ifndef REGULATION_H
#define REGULATION_H

// ============================================================================
//  Module TRAITER — Régulation proportionnelle + machine à états (FP3, FP6)
//  Calcule l'écart e = T_mesurée − T_consigne, en déduit la commande PWM du
//  ventilateur (loi proportionnelle, cahier-des-charges.md §3) et l'état
//  courant du système (machine à états, EC5 ; cf. architecture.md §5).
// ============================================================================

// États nommés du système.
enum EtatRegulation { REPOS, REGULATION, ALARME };

// Résultat d'un cycle de régulation (regroupé pour rester observable/testable).
struct SortieRegulation {
  float erreur;          // e = T_mesurée − T_consigne (°C)
  int   commande;        // consigne de vitesse ventilateur, PWM 0..255
  EtatRegulation etat;   // état courant du système
};

// --- Paramètres de la loi (cahier-des-charges.md §3) -----------------------
const float BANDE_PROPORTIONNELLE = 5.0;   // BP (°C) : e >= BP  -> commande maxi
const int   PWM_MIN = 0;                    // ventilateur à l'arrêt
const int   PWM_MAX = 255;                  // ventilateur à 100 % (PWM 8 bits)
const float SEUIL_ALARME = 8.0;             // e >= seuil (°C)   -> état ALARME (EF7)

// Calcule la sortie de régulation à partir des mesures courantes (logique pure,
// sans effet de bord matériel -> testable en boucle ouverte).
SortieRegulation calculerRegulation(float tMesuree, float tConsigne);

// Nom lisible de l'état (pour l'affichage et les traces de test).
const char* nomEtat(EtatRegulation etat);

#endif // REGULATION_H
