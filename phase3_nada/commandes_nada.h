/// \file commandes_nada.h
/// \author Nada ARABA
/// \brief Phase 3 : commandes crypt, uncrypt, show et savepub de l'interpreteur.
///        Elles suivent le prototype commande_t de interpreteur.h.
#ifndef COMMANDES_NADA_H
#define COMMANDES_NADA_H

#include "interpreteur.h"

/*
 * show <id> : affiche l'identificateur, le type et les clefs en hexadecimal (format de la phase 1).
 * La clef privee est affichee si elle existe, sinon "absente".
 * Retourne CMD_SUCCES, CMD_ERREUR_SYNTAXE, CMD_ERREUR_ID_INCONNU ou RSA_ERREUR_PARAMETRE (table ou argv NULL).
 */
int cmd_show(table_clefs_t *table, int argc, char **argv);

/*
 * crypt <id> <fichier_clair> <fichier_chiffre> : chiffre un fichier par blocs de 4 octets
 * avec la clef publique de <id> (format DIYPG2 d'Adam, phase 2).
 * La clef doit etre de type crypt et venir de gen_keys_gmp (N > 2^32).
 * Retourne CMD_SUCCES, CMD_ERREUR_SYNTAXE, CMD_ERREUR_ID_INCONNU, CMD_ERREUR_TYPE,
 * ou le code RSA_* de chiffrer_fichier_blocs_gmp (RSA_ERREUR_CLE, RSA_ERREUR_FICHIER, ...).
 */
int cmd_crypt(table_clefs_t *table, int argc, char **argv);

/*
 * uncrypt <id> <fichier_chiffre> <fichier_clair> : dechiffre avec la clef privee de <id>.
 * Memes retours que crypt, plus CMD_ERREUR_PAS_DE_PRIVEE pour une clef publique importee
 * et RSA_ERREUR_CHIFFRE pour un fichier chiffre invalide ou une mauvaise clef.
 */
int cmd_uncrypt(table_clefs_t *table, int argc, char **argv);

#endif
