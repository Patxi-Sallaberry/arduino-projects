// ============================================================================
//  Régulateur proportionnel de vitesse de ventilateur
//  ÉTAPE 2 — Test des acquisitions : température (FP1) + consigne (FP2)
//
//  Ce croquis n'exerce QUE la chaîne d'acquisition (modules capteur_temp et
//  consigne) : il lit périodiquement la température et la consigne et les
//  affiche sur la liaison série. La régulation, le ventilateur, le LCD et
//  l'alarme viendront module par module.
// ============================================================================

#include "capteur_temp.h"
#include "consigne.h"

// Cadence d'affichage (ms). Principe NON BLOQUANT (EC3) : pas de delay(),
// on compare millis() (EP1). Affichage à 500 ms (EF6).
const unsigned long PERIODE_AFFICHAGE_MS = 500;
unsigned long dernierAffichage = 0;

void setup() {
  Serial.begin(9600);
  // Les entrées analogiques ne nécessitent pas de pinMode().
  Serial.println("== Test acquisitions : FP1 temperature + FP2 consigne ==");
}

void loop() {
  unsigned long maintenant = millis();

  // Ordonnancement non bloquant (la soustraction non signée gère le débordement
  // de millis()). C'est le patron de base du temps réel sur Arduino.
  if (maintenant - dernierAffichage >= PERIODE_AFFICHAGE_MS) {
    dernierAffichage = maintenant;

    float tempC     = lireTemperature();
    float consigneC = lireConsigne();

    Serial.print("T=");
    Serial.print(tempC, 1);
    Serial.print(" C | Cons=");
    Serial.print(consigneC, 1);
    Serial.println(" C");
  }
}
