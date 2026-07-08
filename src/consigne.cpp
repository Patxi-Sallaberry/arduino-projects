#include <Arduino.h>
#include "consigne.h"

float lireConsigne() {
  // Lecture ADC 10 bits : 0..1023 selon la position du potentiomètre.
  int adc = analogRead(PIN_CONSIGNE);

  // Conversion en consigne EN FLOTTANT.
  // PIÈGE : on n'utilise PAS la fonction map() d'Arduino. map() travaille en
  // arithmétique ENTIÈRE et tronque : sur une plage de 25 °C étalée sur 1024
  // pas, on n'obtiendrait jamais de décimales et on perdrait de la résolution.
  // On fait donc l'interpolation linéaire en float :
  //   position 0      -> CONSIGNE_MIN
  //   position 1023   -> CONSIGNE_MAX
  float fraction = adc / 1023.0;
  return CONSIGNE_MIN + fraction * (CONSIGNE_MAX - CONSIGNE_MIN);
}
