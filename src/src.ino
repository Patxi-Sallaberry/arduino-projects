// ============================================================================
//  Régulateur proportionnel de vitesse de ventilateur
//  ÉTAPE 3 — Test de la régulation : mesure (FP1) + consigne (FP2) + loi P (FP3)
//
//  Ce croquis lit la température et la consigne, calcule la commande de
//  régulation et l'état, et les affiche sur la liaison série. Le ventilateur
//  (FP4), le LCD (FP5) et la LED (FP6) viendront ensuite.
//  NB : la cadence de régulation à 100 ms (EP1) sera figée lors de l'intégration
//  finale de l'actionneur ; ici on calcule au rythme d'affichage (500 ms).
// ============================================================================

#include "capteur_temp.h"
#include "consigne.h"
#include "regulation.h"

const unsigned long PERIODE_AFFICHAGE_MS = 500;
unsigned long dernierAffichage = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("== Test regulation : FP1 + FP2 + FP3 ==");
}

void loop() {
  unsigned long maintenant = millis();

  // Ordonnancement non bloquant (soustraction non signée = robuste au débordement).
  if (maintenant - dernierAffichage >= PERIODE_AFFICHAGE_MS) {
    dernierAffichage = maintenant;

    float tempC     = lireTemperature();
    float consigneC = lireConsigne();
    SortieRegulation reg = calculerRegulation(tempC, consigneC);

    // Trace parsable : T | Consigne | erreur | PWM | ETAT
    Serial.print("T=");
    Serial.print(tempC, 1);
    Serial.print(" C | Cons=");
    Serial.print(consigneC, 1);
    Serial.print(" C | e=");
    Serial.print(reg.erreur, 1);
    Serial.print(" | PWM=");
    Serial.print(reg.commande);
    Serial.print(" | ETAT=");
    Serial.println(nomEtat(reg.etat));
  }
}
