"""Verifie le lancement apres copie, sans configuration propre a une machine."""

import os
from pathlib import Path
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
    essais = ROOT / "build" / "verification-lancement"
    essais.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="copie-", dir=essais) as temporaire:
        copie = Path(temporaire) / "copie du projet"
        shutil.copytree(ROOT, copie, ignore=shutil.ignore_patterns(
            ".git", "build", "__pycache__", "*.pyc", "*.o", "*.obj", "*.exe",
            ".DS_Store", "outils-locaux.txt", "w64devkit", "projet-avance"))
        commande = [sys.executable, str(copie / "tester.py")]
        # Le repertoire courant est different de celui des sources.
        resultat = subprocess.run(commande, cwd=temporaire, check=True,
                                  stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
        texte = resultat.stdout.decode("utf-8").replace("\r\n", "\n")
        print(texte, end="")
        if "le caractère 'é'" not in texte or "Intégrité du fichier conservée" not in texte:
            raise RuntimeError("Les accents des programmes doivent rester en UTF-8.")

        environnement = dict(os.environ, CC="compilateur_qui_n_existe_pas_phase1")
        resultat = subprocess.run(commande + ["all"], cwd=temporaire,
                                  env=environnement, capture_output=True)
        if resultat.returncode == 0 or b"CC introuvable" not in resultat.stderr:
            raise RuntimeError("Un compilateur absent doit produire une erreur explicite.")

        # Python refuse les options C : on verifie la remontee d'un echec de compilation.
        environnement["CC"] = sys.executable
        resultat = subprocess.run(commande + ["all"], cwd=temporaire,
                                  env=environnement, capture_output=True)
        if resultat.returncode == 0:
            raise RuntimeError("Un echec de compilation ne doit pas etre ignore.")

        if os.name == "nt":
            lanceur = str(copie / "tester.cmd")
            resultat = subprocess.run(["cmd", "/d", "/c", lanceur, "cible_inconnue"],
                                      cwd=temporaire, capture_output=True)
            if resultat.returncode == 0:
                raise RuntimeError("Le lanceur Windows doit transmettre les erreurs.")

    print("Copie avec espaces, lancement hors du dossier et erreurs : OK.")


if __name__ == "__main__":
    main()
