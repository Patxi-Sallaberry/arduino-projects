#include <Arduino.h>
#include "alarme.h"

void initialiserAlarme() {
  pinMode(PIN_LED_ALARME, OUTPUT);
  digitalWrite(PIN_LED_ALARME, LOW);   // éteinte au démarrage
}

void majAlarme(EtatRegulation etat) {
  // LED allumée UNIQUEMENT en état ALARME (EF7). Sortie tout-ou-rien simple.
  digitalWrite(PIN_LED_ALARME, (etat == ALARME) ? HIGH : LOW);
}
