#ifndef CAPTEUR_TEMP_H
#define CAPTEUR_TEMP_H

// ============================================================================
//  Module ACQUÉRIR — Lecture de la température via thermistance NTC (FP1)
//  Réalise la chaîne d'acquisition décrite dans docs/architecture.md §2 :
//    analogRead (ADC 10 bits) -> tension -> résistance NTC -> température (°C)
// ============================================================================

// Broche de mesure = point milieu du diviseur de tension NTC.
// Cf. table de correspondance des pins, architecture.md §4.
const int PIN_NTC = A0;

// Tension d'alimentation du diviseur (V). L'Uno alimente le capteur en 5 V.
const float TENSION_ALIM = 5.0;

// --- Paramètres physiques du capteur ---------------------------------------
// Le module NTC de Wokwi intègre un pont diviseur : une résistance fixe de
// 10 kΩ et une thermistance NTC de 10 kΩ à 25 °C, de coefficient Beta = 3950.
// En montage DISCRET réel, R_FIXE serait une résistance physique à câbler
// soi-même (cf. schéma du diviseur, architecture.md §2).
const float R_FIXE    = 10000.0;   // résistance fixe du diviseur (Ω)
const float R0        = 10000.0;   // résistance nominale de la NTC à T0 (Ω)
const float T0_KELVIN = 298.15;    // température nominale = 25 °C, en kelvin
const float BETA      = 3950.0;    // coefficient Beta de la NTC (K)

// Lit la broche NTC et renvoie la température mesurée, en degrés Celsius.
float lireTemperature();

#endif // CAPTEUR_TEMP_H
