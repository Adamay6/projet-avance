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

#endif
