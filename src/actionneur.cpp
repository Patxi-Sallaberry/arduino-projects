#include <Arduino.h>
#include "actionneur.h"

void initialiserActionneur() {
  pinMode(PIN_VENTILATEUR, OUTPUT);
}

void appliquerCommande(int pwm) {
  // PIÈGE : analogWrite() ne produit PAS une vraie tension analogique (l'Uno n'a
  // pas de DAC). C'est un signal PWM (carré, ~490 Hz sur D9) dont on fait varier
  // le RAPPORT CYCLIQUE. Le moteur (ou la LED) réagit à la tension MOYENNE :
  //   V_moyenne = Vcc * pwm / 255
  // pwm=0 -> 0 % (arrêt) ; pwm=128 -> ~50 % ; pwm=255 -> 100 %.
  analogWrite(PIN_VENTILATEUR, pwm);
}
