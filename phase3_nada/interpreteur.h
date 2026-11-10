/// \file interpreteur.h
/// \author Nada ARABA
/// \brief Phase 3 : PROPOSITION d'interface commune entre le coeur de l'interpreteur (Loann),
///        les contacts V2 (Adam) et les commandes crypt / uncrypt / show / savepub (Nada).
/// \note A valider par Loann. Quand son module de table existera, ce fichier sera deplace
///       ou remplace par le sien, et table_clefs_provisoire.c sera supprime.
#ifndef INTERPRETEUR_H
#define INTERPRETEUR_H

#include <stdbool.h>
#include <stddef.h>
#include "../phase1_nada/rsa_common_header.h"   /* rsaKey_t, NAME_MAX_SIZE */

/* Le type est verifie par chaque commande (cahier des charges) : crypt refuse une clef sign, etc. */
typedef enum {
    CLEF_CRYPT,
    CLEF_SIGN
} type_clef_t;

/* Une entree de la table des symboles : identificateur, type, clef publique, clef privee. */
typedef struct {
    char id[NAME_MAX_SIZE + 1];
    type_clef_t type;
    rsaKey_t pub;
    rsaKey_t priv;
    bool a_priv;   /* false pour une clef publique importee : pas de clef privee */
} entree_clef_t;

typedef struct {
    entree_clef_t *entrees;
    size_t nb;
    size_t capacite;
} table_clefs_t;

/*
 * Codes propres a l'interpreteur. Une commande peut aussi renvoyer un code RSA_* de
 * phase1_adam/chiffrement.h (de -1 a -5) quand l'erreur vient du chiffrement ou d'un fichier.
 */
typedef enum {
    CMD_SUCCES = 0,
    CMD_ERREUR_SYNTAXE = -10,        /* mauvais nombre d'arguments */
    CMD_ERREUR_ID_INCONNU = -11,     /* identificateur absent de la table */
    CMD_ERREUR_TYPE = -12,           /* clef sign donnee a crypt, etc. */
    CMD_ERREUR_PAS_DE_PRIVEE = -13   /* commande qui a besoin de la clef privee */
} cmd_resultat_t;

/*
 * Prototype commun des commandes. argv[0] est le nom de la commande, comme pour main.
 * Le parseur (Loann) decoupe la ligne, choisit la commande et affiche le message d'erreur.
 */
typedef int (*commande_t)(table_clefs_t *table, int argc, char **argv);

/* Recherche par identificateur exact. NULL si absent ou si un parametre est NULL. Module de table (Loann). */
const entree_clef_t *table_chercher(const table_clefs_t *table, const char *id);

#endif
