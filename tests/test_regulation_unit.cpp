// ============================================================================
//  Test UNITAIRE NATIF du module de régulation (FP3, EF3).
//  Compile et s'exécute sur PC (g++) : calculerRegulation() étant de la logique
//  pure, on lui fournit des couples (T_mesurée, T_consigne) EXACTS et on vérifie
//  la sortie de façon DÉTERMINISTE — ce que la simulation analogique (quantifiée)
//  ne permet pas.
//
//  Compilation :  g++ -Isrc tests/test_regulation_unit.cpp src/regulation.cpp \
//                     -o build/test_regulation_unit
// ============================================================================
#include "regulation.h"
#include <cstdio>
#include <cmath>

static int echecs = 0;

static void verifie(const char* nom, float tMes, float tCons,
                    float eAttendu, int pwmAttendu, EtatRegulation etatAttendu) {
  SortieRegulation s = calculerRegulation(tMes, tCons);
  bool ok = (fabs(s.erreur - eAttendu) < 0.001f) &&
            (s.commande == pwmAttendu) &&
            (s.etat == etatAttendu);
  printf("%-26s -> e=%5.1f  PWM=%3d  ETAT=%-10s  [%s]\n",
         nom, s.erreur, s.commande, nomEtat(s.etat), ok ? "OK" : "ECHEC");
  if (!ok) {
    printf("     ATTENDU : e=%.1f  PWM=%d  ETAT=%s\n",
           eAttendu, pwmAttendu, nomEtat(etatAttendu));
    echecs++;
  }
}

int main() {
  printf("== Tests unitaires FP3 (loi proportionnelle + machine a etats) ==\n");

  // --- e <= 0 : REPOS, ventilateur arrêté ---
  verifie("froid (e=-5)",        20.0f, 25.0f,  -5.0f,   0, REPOS);
  verifie("pile consigne (e=0)", 25.0f, 25.0f,   0.0f,   0, REPOS);

  // --- Zone proportionnelle : PWM = round(255 * e / 5) ---
  verifie("e=1  -> ~51",         26.0f, 25.0f,   1.0f,  51, REGULATION);
  verifie("e=2.5 -> moitie",     27.5f, 25.0f,   2.5f, 128, REGULATION);

  // --- Saturation dès e >= BP (5 °C) ---
  verifie("e=5  -> sature 255",  30.0f, 25.0f,   5.0f, 255, REGULATION);
  verifie("e=6  -> sature 255",  31.0f, 25.0f,   6.0f, 255, REGULATION);

  // --- Alarme dès e >= 8 °C (PWM déjà saturé) ---
  verifie("e=7.9 -> encore REGUL",32.9f, 25.0f,  7.9f, 255, REGULATION);
  verifie("e=8  -> ALARME",      33.0f, 25.0f,   8.0f, 255, ALARME);
  verifie("e=15 -> ALARME",      40.0f, 25.0f,  15.0f, 255, ALARME);

  printf("\n%s (%d echec(s))\n",
         echecs == 0 ? ">>> TOUS LES TESTS UNITAIRES : OK" : ">>> DES TESTS ONT ECHOUE",
         echecs);
  return echecs == 0 ? 0 : 1;
}
