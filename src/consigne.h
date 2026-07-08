#ifndef CONSIGNE_H
#define CONSIGNE_H

// ============================================================================
//  Module ACQUÉRIR — Lecture de la consigne de température (FP2)
//  Le potentiomètre est un diviseur de tension réglable : sa position fixe une
//  tension sur A1, lue par l'ADC (0..1023) puis convertie en consigne (°C).
// ============================================================================

// Broche de lecture du curseur (SIG) du potentiomètre.
// Cf. table de correspondance des pins, architecture.md §4.
const int PIN_CONSIGNE = A1;

// Bornes de la consigne réglable (cf. EF2, cahier-des-charges.md).
const float CONSIGNE_MIN = 20.0;   // °C — potentiomètre en butée basse
const float CONSIGNE_MAX = 45.0;   // °C — potentiomètre en butée haute

// Lit le potentiomètre et renvoie la consigne demandée, en degrés Celsius.
float lireConsigne();

#endif // CONSIGNE_H
