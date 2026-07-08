#!/usr/bin/env python3
"""Mesure le rapport cyclique d'un signal PWM dans un fichier VCD, en régime établi.

Le symbole VCD du signal est résolu automatiquement à partir du NOM de la variable
(ex. "D0"), pour ne pas dépendre de l'attribution interne de Wokwi.

Usage : mesure_duty.py <fichier.vcd> <nom_var> <fenetre_ms> <attendu_%> <tolerance_%>
Sortie : affiche le rapport cyclique ; code retour 0 si dans la tolérance, 1 sinon.
"""
import sys

vcd, varname = sys.argv[1], sys.argv[2]
win_ms, attendu, tol = float(sys.argv[3]), float(sys.argv[4]), float(sys.argv[5])

sym = None
changes = []
t = 0
for line in open(vcd):
    s = line.strip()
    if s.startswith('$var'):
        # format : $var wire 1 <symbole> <nom> $end
        p = s.split()
        if len(p) >= 5 and p[-2] == varname:
            sym = p[3]
    elif s.startswith('#'):
        t = int(s[1:])
    elif sym and len(s) >= 2 and s[0] in '01' and s[1:] == sym:
        changes.append((t, int(s[0])))

if sym is None:
    print(f"ERREUR : variable '{varname}' introuvable dans le VCD")
    sys.exit(2)
if not changes:
    print(f"ERREUR : aucun changement d'état pour '{varname}'")
    sys.exit(2)

tmax = changes[-1][0]
win_start = tmax - win_ms * 1e6   # fenêtre = derniers win_ms (régime établi)
high = total = 0.0
prev_t = None
prev_v = 0
for (t, v) in changes:
    if prev_t is not None:
        a = max(prev_t, win_start)
        b = t
        if b > a:
            total += (b - a)
            if prev_v == 1:
                high += (b - a)
    prev_t, prev_v = t, v

duty = 100.0 * high / total if total else 0.0
ok = abs(duty - attendu) <= tol
print(f"Rapport cyclique D9 (regime etabli {win_ms:.0f} ms) : {duty:.1f} % "
      f"(attendu {attendu:.1f} +/- {tol})  [{'OK' if ok else 'ECHEC'}]")
sys.exit(0 if ok else 1)
