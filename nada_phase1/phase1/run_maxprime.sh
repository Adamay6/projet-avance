#!/bin/sh
# Lance l'étude de MAX_PRIME pour plusieurs valeurs et garde les résultats dans build/resultats_maxprime.txt
# Utilisation (dans le dossier phase1/, après "make") :   sh run_maxprime.sh
set -eu
DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
make -C "$DIR/../.." test-maxprime
echo "Résultats écrits dans build/resultats_maxprime.txt"
