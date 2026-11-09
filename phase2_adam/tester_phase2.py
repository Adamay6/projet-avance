"""Compile et teste le traitement des fichiers par blocs."""
import os
from pathlib import Path
import shlex
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parent
PROJET = ROOT.parent
BUILD = PROJET / "build" / "adam_phase2"


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    sys.stderr.reconfigure(encoding="utf-8")
    config = PROJET / "outils-locaux.txt"
    dossiers = [PROJET / "w64devkit" / "bin"]
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
    gmp_statique = os.environ.get("GMP_ROOT")
    gmp = gmp_statique or os.environ.get("GMP_PREFIX")
    if gmp:
        flags = ["-I", str(Path(gmp) / "include")]
        libs = ["-L", str(Path(gmp) / "lib"), "-lgmp"]
        if gmp_statique:
            flags.append("-DGMP_STATICLIB")
        if os.name == "nt":
            os.environ["PATH"] += os.pathsep + str(Path(gmp) / "bin")
    elif shutil.which("pkg-config"):
        flags = shlex.split(subprocess.check_output(
            ["pkg-config", "--static", "--cflags", "gmp"], text=True))
        libs = shlex.split(subprocess.check_output(
            ["pkg-config", "--static", "--libs", "gmp"], text=True))
    else:
        flags, libs = [], ["-lgmp"]
    BUILD.mkdir(parents=True, exist_ok=True)
    exe = BUILD / ("test_fichiers_blocs.exe" if os.name == "nt" else "test_fichiers_blocs")
    sources = ["chiffrement_blocs.c", "fichiers_gmp.c", "../phase2_nada/gmp_rsa.c",
               "../phase2_nada/convert_bytes.c", "tests/test_fichiers_blocs.c"]
    subprocess.run(commande_cc + ["-std=c11", "-O2", "-Wall", "-Wextra", "-Wpedantic",
                                  "-Werror", "-finput-charset=UTF-8", "-fexec-charset=UTF-8"] +
                   flags + [str(ROOT / source) for source in sources] + libs +
                   ["-o", str(exe)], check=True)
    essais = BUILD / "tests"
    essais.mkdir(exist_ok=True)
    subprocess.run([str(exe)], cwd=essais, check=True, timeout=180)
    exe_io = BUILD / ("test_blocs_io.exe" if os.name == "nt" else "test_blocs_io")
    subprocess.run(commande_cc + ["-std=c11", "-O2", "-Wall", "-Wextra", "-Wpedantic",
                                  "-Werror", str(ROOT / "tests/test_blocs_io.c"),
                                  "-o", str(exe_io)], check=True)
    subprocess.run([str(exe_io)], cwd=essais, check=True, timeout=30)

    # Recalculer les blocs avec Python pour verifier les resultats de GMP.
    clair = (essais / "source.bin").read_bytes()
    chiffre = (essais / "chiffre.bin").read_bytes()
    if chiffre[:8] != b"DIYPG2\r\n" or int.from_bytes(chiffre[8:16], "little") != len(clair):
        raise RuntimeError("Entete incompatible avec la reference Python.")
    blocs = [int.from_bytes(clair[i:i+4], "little") for i in range(0, len(clair), 4)]
    attendu = b"".join(pow(bloc, 65537, 18446743979220271189).to_bytes(8, "little")
                       for bloc in blocs)
    if chiffre[16:] != attendu or (essais / "retour.bin").read_bytes() != clair:
        raise RuntimeError("Les blocs RSA ou le fichier restitue different de la reference.")
    print("Reference Python : format, blocs RSA et fichier restitue conformes.", flush=True)


if __name__ == "__main__":
    try:
        main()
    except (OSError, RuntimeError, subprocess.SubprocessError, AssertionError) as erreur:
        sys.exit("Echec des tests phase 2 : " + str(erreur))
