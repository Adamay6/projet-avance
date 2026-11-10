/// \file commandes_nada.h
/// \author Nada ARABA
/// \brief Phase 3 : commandes crypt, uncrypt, show et savepub de l'interpreteur.
///        Elles suivent le prototype commande_t de interpreteur.h et la syntaxe de interprete.pdf.
///
/// <keyid> designe une clef de l'utilisateur, ou (V2) une clef de contact : "contact/clef",
/// ou "contact" seul s'il n'a qu'une clef. Les clefs de l'utilisateur sont cherchees en premier.
/// Les erreurs de contact sont CMD_ERREUR_ID_INCONNU, CMD_ERREUR_SYNTAXE ou CMD_ERREUR_AMBIGU.
#ifndef COMMANDES_NADA_H
#define COMMANDES_NADA_H

#include "interpreteur.h"

/*
 * show <keyid> ["pub"] ["priv"] : affiche le type et les clefs en hexadecimal (format de la phase 1).
 * Sans option : la clef publique, puis la clef privee ou "absente".
 * "priv" sur une clef sans partie privee (clef de contact) : CMD_ERREUR_PAS_DE_PRIVEE.
 * Retourne aussi CMD_ERREUR_SYNTAXE (option inconnue ou repetee), CMD_ERREUR_ID_INCONNU
 * ou RSA_ERREUR_PARAMETRE (table ou argv NULL).
 */
int cmd_show(table_clefs_t *table, int argc, char **argv);

/*
 * crypt <fileIn> <fileOut> <keyid> : chiffre un fichier par blocs de 4 octets avec la clef
 * publique de <keyid> (format DIYPG2 d'Adam, phase 2). Le type de la clef doit etre crypt,
 * et la clef venir de gen_keys_gmp (N > 2^32).
 * Retourne CMD_SUCCES, CMD_ERREUR_SYNTAXE, CMD_ERREUR_ID_INCONNU, CMD_ERREUR_AMBIGU, CMD_ERREUR_TYPE,
 * ou le code RSA_* de chiffrer_fichier_blocs_gmp (RSA_ERREUR_CLE, RSA_ERREUR_FICHIER, ...).
 */
int cmd_crypt(table_clefs_t *table, int argc, char **argv);

/*
 * uncrypt <fileIn> <fileOut> <keyid> : dechiffre avec la clef privee de <keyid>.
 * Memes retours que crypt, plus CMD_ERREUR_PAS_DE_PRIVEE pour une clef publique seule
 * (clef de contact) et RSA_ERREUR_CHIFFRE pour un fichier chiffre invalide ou une mauvaise clef.
 */
int cmd_uncrypt(table_clefs_t *table, int argc, char **argv);

/*
 * savepub <keyid> <file> : exporte la clef publique de <keyid> en base64 (export_pubkey_b64 de Loann,
 * phase 2), pour l'envoyer a un contact. Le type (crypt ou sign) est ecrit dans le fichier.
 * Accepte les deux types : une clef publique sign sert a verifier les signatures.
 * La clef privee n'est jamais ecrite.
 * Retourne CMD_SUCCES, CMD_ERREUR_SYNTAXE, CMD_ERREUR_ID_INCONNU, CMD_ERREUR_AMBIGU,
 * RSA_ERREUR_PARAMETRE ou RSA_ERREUR_FICHIER (ecriture impossible).
 */
int cmd_savepub(table_clefs_t *table, int argc, char **argv);

#endif
