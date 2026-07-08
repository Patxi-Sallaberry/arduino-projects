// ============================================================================
//  Régulateur proportionnel de vitesse de ventilateur
//  ÉTAPE 1 — Test isolé de l'acquisition de température (FP1)
//
//  Ce croquis n'exerce QUE le module capteur_temp : il lit la température
//  périodiquement et l'affiche sur la liaison série. Les autres fonctions
//  (consigne, régulation, ventilateur, LCD, alarme) viendront module par module.
// ============================================================================

#include "capteur_temp.h"

// Cadence d'affichage (ms). On respecte déjà le principe NON BLOQUANT (EC3) :
// pas de delay(), on compare millis() (EP1). L'affichage à 500 ms suit EF6.
const unsigned long PERIODE_AFFICHAGE_MS = 500;
unsigned long dernierAffichage = 0;

void setup() {
  Serial.begin(9600);
  // Une entrée analogique ne nécessite pas de pinMode().
  Serial.println("== Test acquisition NTC (FP1) ==");
}

void loop() {
  unsigned long maintenant = millis();

  // Ordonnancement non bloquant : la soustraction non signée gère correctement
  // le débordement de millis() (~toutes les 49 jours), contrairement à une
  // comparaison directe. C'est le patron de base du temps réel sur Arduino.
  if (maintenant - dernierAffichage >= PERIODE_AFFICHAGE_MS) {
    dernierAffichage = maintenant;

    float tempC = lireTemperature();
    Serial.print("T=");
    Serial.print(tempC, 1);   // 1 décimale
    Serial.println(" C");
  }
}
