"""Compile et teste la gestion des contacts V2."""
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
PROJET = ROOT.parent
BUILD = PROJET / "build" / "adam_contacts"


def reference_python(chemin):
    donnees = chemin.read_bytes()
    position = 8
    if donnees[:8] != b"DYCONT01":
        raise RuntimeError("Signature des contacts invalide.")

    def nombre(taille):
        nonlocal position
        if position + taille > len(donnees):
            raise RuntimeError("Section de contacts tronquee.")
        valeur = int.from_bytes(donnees[position:position+taille], "little")
        position += taille
        return valeur

    def texte(capacite):
        nonlocal position
        taille = nombre(2)
        if taille >= capacite or position + taille > len(donnees):
            raise RuntimeError("Champ de contacts invalide.")
        valeur = donnees[position:position+taille].decode("utf-8")
        position += taille
        return valeur

    contacts = []
    for _ in range(nombre(4)):
        contact = dict(zip(("id", "nom", "prenom", "commentaire"),
                           (texte(33), texte(128), texte(128), texte(512))))
        contact["cles"] = []
        for _ in range(nombre(4)):
            contact["cles"].append((texte(33), nombre(1), nombre(8), nombre(8)))
        contacts.append(contact)
    attendu = [
        {"id": "alice", "nom": "Évrard", "prenom": "Chloé", "commentaire": "modifié",
         "cles": [("pro", 1, 65537, 18446743979220271189), ("signature", 2, 17, 3233)]},
        {"id": "bob", "nom": "Martin", "prenom": "Bob", "commentaire": "",
         "cles": [("pro", 1, 65537, 18446743979220271189)]},
        {"id": "charlie", "nom": "Martin", "prenom": "Charlie", "commentaire": "", "cles": []}
    ]
    if contacts != attendu or position != len(donnees):
        raise RuntimeError("Les contacts sauvegardes different de la reference Python.")
    print("Reference Python : contacts UTF-8, types et clefs uint64 conformes.", flush=True)


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
    dossiers = [PROJET / "w64devkit" / "bin"]
    config = PROJET / "outils-locaux.txt"
    if config.is_file():
        valeur = config.read_text(encoding="utf-8-sig").strip()
        if valeur:
            dossiers.append(PROJET / valeur)
    for dossier in dossiers:
        if dossier.is_dir():
            os.environ["PATH"] = str(dossier) + os.pathsep + os.environ.get("PATH", "")
    cc = os.environ.get("CC", "gcc")
    commande_cc = [cc] if Path(cc).is_file() else shlex.split(cc)
    if not commande_cc or not shutil.which(commande_cc[0]):
        raise RuntimeError("Compilateur introuvable : " + cc)
    flags = ["-std=c11", "-O2", "-Wall", "-Wextra", "-Wpedantic", "-Werror",
             "-finput-charset=UTF-8", "-fexec-charset=UTF-8"]
    commun = ["contacts.c", "contacts_io.c", "contacts_commandes.c"]
    programmes = {
        "test_contacts": commun + ["contacts_interpreteur.c", "tests/test_contacts.c"],
        "test_contacts_memoire": ["tests/test_contacts_memoire.c"],
        "rejouer_contacts": commun + ["tests/rejouer_contacts.c"]
    }
    BUILD.mkdir(parents=True, exist_ok=True)
    essais = BUILD / "tests"
    essais.mkdir(exist_ok=True)
    executables = {}
    for nom, sources in programmes.items():
        exe = BUILD / (nom + (".exe" if os.name == "nt" else ""))
        subprocess.run(commande_cc + flags + [str(ROOT / source) for source in sources] +
                       ["-o", str(exe)], check=True)
        executables[nom] = exe
    for nom in ("test_contacts", "test_contacts_memoire"):
        subprocess.run([str(executables[nom])], cwd=essais, check=True, timeout=60)
    resultat = subprocess.run(
        [str(executables["rejouer_contacts"]), str(ROOT / "tests/scenario_contacts.txt")],
        cwd=essais, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)
    sortie = resultat.stdout.decode("utf-8").replace("\r\n", "\n")
    print(sortie, end="", flush=True)
    resultat.check_returncode()
    for attendu in ("Chloé Le Gall", "Commentaire mis à jour",
                    "N = 0x100040003", "0 contact(s) restant(s)."):
        if attendu not in sortie:
            raise RuntimeError("Scenario incomplet : " + attendu)
    reference_python(essais / "contacts_reference.bin")


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, UnicodeError, subprocess.SubprocessError) as erreur:
        sys.exit("Echec des tests contacts : " + str(erreur))
