#!/usr/bin/env python3
"""Vérifie l'état FINAL (0/1) d'un signal digital dans un fichier VCD.

Le symbole est résolu à partir du nom de variable (ex. "D1").
Usage : verifie_pin.py <fichier.vcd> <nom_var> <attendu 0|1>
Sortie : affiche l'état final ; code retour 0 si conforme, 1 sinon.
"""
import sys

vcd, varname, attendu = sys.argv[1], sys.argv[2], int(sys.argv[3])
sym = None
val = None
for line in open(vcd):
    s = line.strip()
    if s.startswith('$var'):
        p = s.split()
        if len(p) >= 5 and p[-2] == varname:
            sym = p[3]
    elif sym and len(s) >= 2 and s[0] in '01' and s[1:] == sym:
        val = int(s[0])

if sym is None:
    print(f"ERREUR : variable '{varname}' introuvable dans le VCD")
    sys.exit(2)
if val is None:
    print(f"ERREUR : aucune valeur pour '{varname}'")
    sys.exit(2)

ok = (val == attendu)
print(f"Etat final {varname} (= broche D8, LED alarme) : {val} "
      f"(attendu {attendu})  [{'OK' if ok else 'ECHEC'}]")
sys.exit(0 if ok else 1)
