# Modifications de la phase 1

Base reprise : branche phase1, commit `3772eaba` du dépôt du groupe.
Les quatre dossiers ont été gardés. Les corrections ont été faites dans une copie locale.

## Assemblage

- Ajout d'un Makefile à la racine pour compiler les fichiers des quatre parties ensemble.
- Le Makefile de Malek renvoie vers celui de la racine. Il cherchait plusieurs sources au mauvais endroit.
- Dans `phase1_malek/phase1.h`, la garde a été renommée et les bons headers ont été inclus. Les deux `phase1.h` se masquaient.
- Le `logfp` du main a été retiré : il existe déjà dans le `log.c` de Nada.
- Ajout de `tester.cmd` pour lancer les tests sous Windows. `outils-locaux.txt` indique le chemin du compilateur sur cette machine ; ce fichier est ignoré par Git.

## Partie de Nada

- Dans `rsa_tools.c`, les calculs RSA ont été gardés. Les petites bornes et les cas 0 et 1 sont maintenant contrôlés. Le tirage utilise plusieurs appels à `rand()`, car un seul ne permettait pas de tester les grandes bornes sous Windows.
- Les traces ne sont écrites que si le fichier de log est ouvert. Dans `bezout.c`, seuls ce contrôle et les conversions de types dans les assertions ont changé.
- `gen_keypair` renvoie maintenant 0 ou -1. Il refuse les bornes en dehors de 20 à 65536 pour le chiffrement courant. L'étude MAX_PRIME utilise directement le générateur jusqu'à 5000000.
- Dans `keys_io.c`, le format de fichier et l'affichage ont été conservés. Les lectures refusent les nombres trop grands, les signes, les données ajoutées et les paires qui ne permettent pas de retrouver les 256 octets. Les erreurs d'écriture et de fermeture sont vérifiées.
- Les constantes et les déclarations ont été ajoutées dans le `phase1.h` de Nada. Le fichier des types et `log.c` sont restés tels quels.
- Les deux programmes de test vérifient leurs arguments. Une graine peut être donnée pour refaire les mêmes essais.
- Les deux README MAX_PRIME gardent les anciens tableaux, mais la conclusion a été corrigée : au-dessus de la borne, toutes les paires n'échouent pas forcément. Le script de mesures appelle maintenant le Makefile commun et laisse l'ancien relevé intact.

## Partie d'Adam

- Dans `chiffrement.h`, le chemin vers `rsa_common_header.h` a été corrigé.
- `chiffrement.c` et les deux fichiers de tests n'ont pas été changés.

## Partie de Loann

- L'algorithme d'encodage et de décodage de `other_base64.c` a été gardé. La sortie encodée se termine maintenant par `\0`. Le cas vide est traité avant de lire les derniers caractères.
- Le décodeur refuse les caractères inconnus et les `=` mal placés. La table est initialisée et remise à NULL après sa libération. Les indices de boucle sont des `size_t`.
- Dans `base64.c`, les ouvertures, lectures, écritures et fermetures sont contrôlées. Les fichiers vides passent aussi. Le décodeur de fichiers accepte les espaces et retours ligne produits par la commande Unix.
- Les variables uint32 et uint64 du test sont maintenant vraiment testées. La comparaison des fichiers vérifie aussi leur fin. Les commentaires des tests ont été gardés.
- `base64.h` déclare les fonctions de gestion de la table et précise le retour des buffers.
- Le chemin du header dans son Makefile a été corrigé. Une fin de ligne a été ajoutée après la dernière accolade de `main_test_loann.c` pour éviter une erreur avec Clang. Les commentaires sont les mêmes.

## Partie de Malek

- Dans `phase1.c`, les noms des fonctions et les commentaires numérotés ont été gardés. La sauvegarde et le chargement appellent les fonctions de Nada ; le chiffrement appelle celles d'Adam. L'ancien code ne gardait qu'un octet du résultat RSA.
- Les deux fonctions Base64 en double ont été retirées de ce fichier. Celles de Loann sont utilisées directement.
- Le main garde ses quatre étapes. Il compare maintenant les clefs et les fichiers et renvoie une erreur si un contrôle échoue. Il lance aussi les tests d'Adam et de Loann.
- `printKeyHex` vérifie ses pointeurs et convertit les valeurs en `unsigned long long` pour garder ses formats d'affichage.
- Les retours Base64 sont testés avec `== 0`, comme prévu dans les fonctions de Loann.

## Tests ajoutés

- `tests/test_regressions.c` reprend les cas qui posaient problème dans l'audit : Base64 vide ou invalide, clefs incorrectes, fichiers tronqués, destination à conserver et aller-retour complet RSA/Base64.
- `tests/test_io_errors.c` simule des erreurs de lecture, d'écriture et de fermeture pour vérifier qu'elles remontent bien.
- `tests/resultats_maxprime.txt` garde les nouvelles mesures avec la graine et le compilateur utilisés.

## Fichiers de la copie

Le README commun décrit les commandes et les retours des fonctions. Le nouveau `.gitignore` écarte les fichiers de compilation et le chemin local des outils. Les deux `.DS_Store` et le lien imbriqué `phase1_malek/projet-avance` ne sont pas utilisés.

## Vérification du 15 octobre

Compilation avec GCC 16.2.0 sous Windows, en C11 avec `-Wall -Wextra -Wpedantic -Werror`.
Les tests communs passent : 123121 vérifications d'Adam, 1570 vérifications ajoutées et 13 contrôles d'E/S (dont 8 erreurs simulées), sans échec. Les 1000 paires de clefs sont aussi passées sur les 256 octets. La comparaison Base64 avec la commande du système réussit.

L'étude MAX_PRIME a été relancée sur 11 bornes, avec 2000 paires par borne. Aucun échec jusqu'à 65536 ; au-dessus, les erreurs attendues apparaissent. Les chiffres sont dans le relevé ajouté.

Une vérification séparée avec Python a comparé 10000 puissances modulaires, 100 paires de clefs, 1012 tableaux Base64 et 5000 entrées aléatoires. Le chiffre exact et le Base64 d'un fichier de 100003 octets correspondent aussi. Les 22 essais d'arguments incorrects sont refusés, et le lanceur Windows renvoie bien un échec quand Make échoue.

## Préparation pour le groupe — 15 octobre

- Ajout de `tester.py`. Il compile avec GCC ou Clang puis lance les mêmes tests sur Windows, Linux et macOS, sans Make ni commandes Unix obligatoires. Le calcul RSA et le Base64 n'ont pas été réécrits.
- `tester.cmd` appelle ce lanceur avec le Python installé sur le PC. Il peut encore passer par Make si Python est absent et que tous les outils Unix sont présents. Le dossier `w64devkit/bin` est détecté automatiquement sous Windows. Aucun chemin personnel n'est enregistré dans les fichiers à partager.
- Dans le Makefile commun, `base64` lit maintenant l'entrée standard pour fonctionner aussi avec la version de macOS.
- Ajout de `tests/test_lancement.py` pour essayer une copie sans fichiers de compilation ni configuration locale, avec des espaces dans son chemin et un lancement depuis un autre dossier. Il vérifie aussi les erreurs de compilateur.
- Ajout de `.gitattributes` pour garder les bons retours à la ligne après un clone sous Windows ou Unix. Les objets, exécutables et dossiers d'outils sont ignorés par Git.
- Ajout des tests GitHub Actions pour Windows, Linux et macOS. Ils ne seront lancés sur GitHub qu'après publication.
- Le README donne les installations nécessaires et les commandes pour chacun des trois systèmes. Aucun push n'a été fait.
- Les deux `.DS_Store` et le lien de dépôt imbriqué ont été retirés du suivi Git avec `git rm --cached`. Les fichiers locaux ont été conservés.

Le lancement depuis la copie avec espaces a été vérifié sous Windows, sans Make ni commande Unix dans le PATH : les tests passent. Le lancement habituel par `tester.cmd` et les 11 bornes de MAX_PRIME passent aussi. Les exécutions Linux et macOS restent à confirmer dans GitHub Actions après le push.

## Affichage des accents

- Ajout de `console_utf8.h` et d'un appel au début des six mains pour utiliser UTF-8 dans la console Windows. Cela couvre aussi les exécutables lancés directement depuis PowerShell.
- La compilation précise maintenant UTF-8 pour les sources et les chaînes de caractères, dans le lanceur et les trois Makefiles concernés. Un changement du Makefile provoque la recompilation des objets.
- `tester.cmd` prépare la console avant de lancer les outils. Le lanceur Python garde aussi ses sorties en UTF-8 quand elles sont redirigées.
- Le test de copie vérifie que les messages avec « é » et « Intégrité » sont bien conservés. Les commentaires et les algorithmes n'ont pas été changés pour cette correction.
- Les sorties récupérées par Python sont normalisées pour éviter les retours à la ligne doublés sous Windows.

Vérifié sous Windows : les six exécutables passent d'une console en page de codes 850 à UTF-8. Les lettres françaises minuscules et majuscules, dont « œ » et « Œ », restent correctes sur la sortie normale et la sortie d'erreur. La suite commune, les Makefiles de Nada et Loann et le lancement depuis une copie passent aussi.

## Préparation de la publication — 3 novembre

- La version commune est préparée sur la branche `phase1-commune`, pour permettre la relecture avant intégration à `phase1`.
- La commande de clonage du README pointe vers cette branche.
- Les actions GitHub sont fixées aux versions vérifiées `checkout` 7.0.1 et `setup-python` 7.1.0.
- Les tests sont relancés depuis une nouvelle copie des sources à publier. Les fichiers de compilation, outils locaux et clés d'essai restent exclus du dépôt.
