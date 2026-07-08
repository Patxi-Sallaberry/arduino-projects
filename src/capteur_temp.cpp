#include <Arduino.h>
#include "capteur_temp.h"

// Conversion ADC -> température, détaillée en 4 étapes explicites
// (plutôt qu'une formule condensée) pour rester lisible et défendable.
float lireTemperature() {
  // Étape 1 — Lecture ADC : entier 0..1023.
  // PIÈGE : la résolution est 10 bits (0..1023), PAS 0..255 ni 0..4095.
  int adc = analogRead(PIN_NTC);

  // Garde-fou numérique : aux extrémités (0 ou 1023), le calcul du diviseur
  // à l'étape 3 diviserait par zéro (-> inf / NaN). On borne donc l'ADC.
  if (adc < 1)    adc = 1;
  if (adc > 1022) adc = 1022;

  // Étape 2 — Tension au point milieu du diviseur (V).
  float tension = adc * TENSION_ALIM / 1023.0;

  // Étape 3 — Résistance de la NTC par la loi du diviseur de tension.
  // Montage : VCC --[R_FIXE]-- (point A0) --[NTC]-- GND
  //   => la NTC est la branche basse, d'où :  R_ntc = R_FIXE * V / (Vcc - V)
  float r_ntc = R_FIXE * tension / (TENSION_ALIM - tension);

  // Étape 4 — Température par l'équation Beta (Steinhart-Hart simplifiée) :
  //   1/T = 1/T0 + (1/Beta) * ln(R_ntc / R0)      avec T en KELVIN
  float invT = 1.0 / T0_KELVIN + (1.0 / BETA) * log(r_ntc / R0);
  float tempKelvin = 1.0 / invT;

  // Retour en degrés Celsius (0 K = -273,15 °C).
  return tempKelvin - 273.15;
}
