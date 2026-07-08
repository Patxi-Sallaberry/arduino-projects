#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "affichage.h"

// Écran à l'adresse I²C 0x27, 16 colonnes x 2 lignes.
// Le bus I²C utilise A4 (SDA) et A5 (SCL) sur l'Uno — broches imposées.
static LiquidCrystal_I2C lcd(0x27, 16, 2);

// Abréviation de l'état sur 5 caractères (l'écran ne fait que 16 colonnes).
static const char* etatCourt(EtatRegulation etat) {
  switch (etat) {
    case REPOS:      return "REPOS";
    case REGULATION: return "REGUL";
    case ALARME:     return "ALARM";
    default:         return "?";
  }
}

void initialiserAffichage() {
  lcd.init();        // initialise le contrôleur I²C (appelle Wire.begin())
  lcd.backlight();   // allume le rétroéclairage
}

void majAffichage(float tMesuree, float tConsigne, int pwm, EtatRegulation etat) {
  int pourcent = (int) round(pwm * 100.0 / 255.0);   // vitesse ventilateur en %

  // On construit chaque ligne dans un tampon de largeur FIXE (16 car.) :
  // "%-16s" justifie à gauche et complète avec des espaces -> efface les résidus
  // de l'affichage précédent, sans jamais dépasser 16 colonnes.
  // NB : sur AVR, printf("%f") est désactivé par défaut -> on convertit les float
  // avec dtostrf() avant de les insérer via %s.
  char valT[8], valC[8], contenu[24], ligne[17];

  dtostrf(tMesuree, 0, 1, valT);   // ex. "25.0"
  dtostrf(tConsigne, 0, 1, valC);

  // Ligne 1 : température mesurée et consigne.
  snprintf(contenu, sizeof(contenu), "T:%s C:%s", valT, valC);
  snprintf(ligne, sizeof(ligne), "%-16s", contenu);
  lcd.setCursor(0, 0);
  lcd.print(ligne);

  // Ligne 2 : vitesse ventilateur (%) et état abrégé.
  snprintf(contenu, sizeof(contenu), "Vt:%d%% %s", pourcent, etatCourt(etat));
  snprintf(ligne, sizeof(ligne), "%-16s", contenu);
  lcd.setCursor(0, 1);
  lcd.print(ligne);
}
