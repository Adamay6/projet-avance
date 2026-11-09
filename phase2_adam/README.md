# Phase 2 — fichiers par blocs (Adam)

J'ai réalisé le traitement des fichiers par blocs de quatre octets,
de la lecture du fichier clair jusqu'à sa restitution après déchiffrement.

J'utilise le module GMP de Nada pour chiffrer et déchiffrer chaque bloc.
Le Base64 et l'export des clefs font partie du lot de Loann.

## Fonctions à utiliser

Les déclarations sont dans `chiffrement_blocs.h`.

- `chiffrer_fichier_blocs_gmp(entree, sortie, &cle_publique)`
- `dechiffrer_fichier_blocs_gmp(entree, sortie, &cle_privee)`

Les fonctions renvoient `RSA_SUCCES` (0) ou un code d'erreur de
`chiffrement.h`. Les paires adaptées aux blocs de quatre octets sont produites
par `gen_keys_gmp` dans `phase2_nada`. La clef doit vérifier
`N > UINT32_MAX` et `0 < E < N`.

Le raccordement se fait dans `fichiers_gmp.c`. Les fonctions
`chiffrer_fichier_blocs` et `dechiffrer_fichier_blocs` acceptent toujours une
fonction de calcul en paramètre. Les tests s'en servent pour simuler des erreurs.
Pour utiliser les fonctions GMP, la compilation inclut `chiffrement_blocs.c`,
`fichiers_gmp.c`, `phase2_nada/gmp_rsa.c` et `phase2_nada/convert_bytes.c`,
avec `-lgmp` à l'édition des liens.

## Format proposé

| Position | Contenu |
| --- | --- |
| 0 à 7 | Les huit octets DIYPG2\r\n |
| 8 à 15 | Longueur du fichier clair, uint64 little endian |
| À partir de 16 | Un uint64 little endian par bloc clair de quatre octets |

Le dernier bloc clair est complété par des zéros. L'en-tête donne sa longueur
utile, ce qui permet de conserver les vrais zéros en fin de fichier.
Un fichier vide donne seulement les seize octets d'en-tête.

Je propose ce format pour les échanges de fichiers de la phase 2.
La phase 1 utilise un format par octet.
La longueur est visible et le format ne fournit pas d'authentification.
Un fichier modifié peut donc rester lisible si sa structure est valide.

## Erreurs et fichiers

Les fonctions vérifient les chemins, les paramètres de clef, les blocs hors
module, les longueurs, le bourrage non nul et les données supplémentaires.
L'entrée est traitée dans un temporaire avant l'ouverture de la destination.
Une erreur d'entrée ou de déchiffrement conserve donc une destination déjà
présente. Utiliser le même chemin dans les deux sens fonctionne.

Une erreur pendant la copie finale peut laisser une sortie partielle.
Une erreur de fermeture ou de suppression du temporaire est aussi remontée.
Le contenu final peut déjà avoir été écrit à ce moment-là.
La mémoire utilisée reste constante. Le temporaire occupe de l'espace disque
proportionnel au résultat.

## Compiler et tester

Depuis la racine, avec Python 3.9+, GCC ou Clang et GMP installé.

    python3 phase2_adam/tester_phase2.py

Sous Windows, la commande est `python phase2_adam/tester_phase2.py`.
Les étapes d'installation de GMP sont dans le README à la racine.

La compilation est aussi possible avec GNU Make et `pkg-config`.

    make -C phase2_adam test

`GMP_PREFIX` peut indiquer le dossier qui contient `include/gmp.h` et `lib/`,
comme pour les tests à la racine. Pour GMP statique sous Windows, le lanceur
Python accepte `GMP_ROOT`. Les exécutables et les données de test vont dans
`build/adam_phase2`.

## Vérifications

- 815 contrôles sur les fichiers, avec deux paires fixes et une paire générée
  par GMP. Les tests couvrent les fichiers vides, les blocs incomplets, les
  zéros finaux, le traitement sur place et les fichiers chiffrés invalides.
- 15 contrôles d'erreurs d'E/S, dont la lecture, l'écriture et la fermeture.
  La destination est vérifiée en cas d'erreur avant la copie finale.
- Le lanceur Python recalcule chaque bloc chiffré avec `pow` pour le comparer
  au résultat du module GMP.
