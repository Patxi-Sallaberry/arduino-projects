// ============================================================================
//  Régulateur proportionnel de vitesse de ventilateur — croquis principal
//  Chaîne complète : mesure (FP1) + consigne (FP2) + régulation (FP3) +
//  ventilateur (FP4). LCD (FP5) et LED d'alarme (FP6) ajoutés ensuite.
//
//  Ordonnancement NON BLOQUANT à deux cadences (EP1, EC3) :
//    - régulation : toutes les 100 ms (mesurer -> décider -> agir)
//    - affichage  : toutes les 500 ms (restituer)
//  Aucun delay() : tout est cadencé par comparaison de millis().
// ============================================================================

#include "capteur_temp.h"
#include "consigne.h"
#include "regulation.h"
#include "actionneur.h"

const unsigned long PERIODE_REGULATION_MS = 100;  // EP1 : boucle à 10 Hz
const unsigned long PERIODE_AFFICHAGE_MS  = 500;  // EF6 : trace toutes les 500 ms

unsigned long derniereRegulation = 0;
unsigned long dernierAffichage   = 0;

// État courant, partagé entre la boucle de régulation et l'affichage.
SortieRegulation regCourante = { 0.0f, 0, REPOS };
float tempCourante     = 0.0f;
float consigneCourante = 0.0f;

void setup() {
  Serial.begin(9600);
  initialiserActionneur();
  Serial.println("== Regulateur ventilateur : FP1..FP4 ==");
}

void loop() {
  unsigned long maintenant = millis();

  // --- Boucle de RÉGULATION (100 ms) : mesurer -> décider -> agir ---
  if (maintenant - derniereRegulation >= PERIODE_REGULATION_MS) {
    derniereRegulation = maintenant;

    tempCourante     = lireTemperature();
    consigneCourante = lireConsigne();
    regCourante      = calculerRegulation(tempCourante, consigneCourante);
    appliquerCommande(regCourante.commande);      // FP4 : PWM vers le ventilateur
  }

  // --- AFFICHAGE (500 ms) : restituer l'état ---
  if (maintenant - dernierAffichage >= PERIODE_AFFICHAGE_MS) {
    dernierAffichage = maintenant;

    Serial.print("T=");        Serial.print(tempCourante, 1);
    Serial.print(" C | Cons="); Serial.print(consigneCourante, 1);
    Serial.print(" C | e=");    Serial.print(regCourante.erreur, 1);
    Serial.print(" | PWM=");    Serial.print(regCourante.commande);
    Serial.print(" | ETAT=");   Serial.println(nomEtat(regCourante.etat));
  }
}
