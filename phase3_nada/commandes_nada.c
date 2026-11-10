/// \file commandes_nada.c
/// \author Nada ARABA
#include <string.h>
#include "commandes_nada.h"
#include "../phase1_nada/phase1/phase1.h"   /* print_key_hex */
#include "../phase1_adam/chiffrement.h"     /* codes RSA_* */
#include "../phase2_adam/chiffrement_blocs.h"   /* fichiers par blocs (Adam, phase 2) */
#include "../phase2_loann/base64_v2.h"        /* export_pubkey_b64 (Loann, phase 2) */
#include "../phase3_adam/contacts_interpreteur.h"   /* contacts_resoudre_entree (Adam, V2) */

static const char *nom_type(type_clef_t type) {
    return type == CLEF_SIGN ? "sign" : "crypt";
}

/*
 * Cherche <keyid> et copie la clef trouvee dans *copie. D'abord les clefs de l'utilisateur,
 * puis (V2) les contacts : "contact/clef", ou "contact" seul s'il n'a qu'une clef.
 * Une clef de contact est publique seulement (a_priv == false).
 */
static int chercher_clef(const table_clefs_t *table, const char *ref, entree_clef_t *copie) {
    const entree_clef_t *e = strchr(ref, '/') == NULL ? table_chercher(table, ref) : NULL;
    if (e != NULL) {
        *copie = *e;
        return CMD_SUCCES;
    }
    if (table->contacts == NULL) return CMD_ERREUR_ID_INCONNU;
    return contacts_resoudre_entree(table->contacts, ref, copie);
}

int cmd_show(table_clefs_t *table, int argc, char **argv) {
    bool pub = false, priv = false;
    entree_clef_t e;
    if (table == NULL || argv == NULL) return RSA_ERREUR_PARAMETRE;
    if (argc < 2 || argc > 4 || argv[1] == NULL) return CMD_ERREUR_SYNTAXE;
    for (int i = 2; i < argc; i++) {   /* options "pub" et "priv", chacune au plus une fois */
        if (argv[i] != NULL && strcmp(argv[i], "pub") == 0 && !pub) pub = true;
        else if (argv[i] != NULL && strcmp(argv[i], "priv") == 0 && !priv) priv = true;
        else return CMD_ERREUR_SYNTAXE;
    }
    int res = chercher_clef(table, argv[1], &e);
    if (res != CMD_SUCCES) return res;
    if (priv && !e.a_priv) return CMD_ERREUR_PAS_DE_PRIVEE;   /* "priv" demande explicitement */
    bool sans_option = !pub && !priv;

    printf("%s (%s)\n", argv[1], nom_type(e.type));
    if (pub || sans_option) print_key_hex("clef publique", &e.pub);
    if (priv || (sans_option && e.a_priv)) print_key_hex("clef privee  ", &e.priv);
    else if (sans_option) printf("clef privee   : absente\n");
    return CMD_SUCCES;
}

/* Controles communs a crypt et uncrypt : <fichier_entree> <fichier_sortie> <keyid>, clef crypt. */
static int clef_crypt(table_clefs_t *table, int argc, char **argv, entree_clef_t *e) {
    if (table == NULL || argv == NULL) return RSA_ERREUR_PARAMETRE;
    if (argc != 4 || argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) return CMD_ERREUR_SYNTAXE;
    int res = chercher_clef(table, argv[3], e);
    if (res != CMD_SUCCES) return res;
    if (e->type != CLEF_CRYPT) return CMD_ERREUR_TYPE;
    return CMD_SUCCES;
}

int cmd_crypt(table_clefs_t *table, int argc, char **argv) {
    entree_clef_t e;
    int res = clef_crypt(table, argc, argv, &e);
    if (res != CMD_SUCCES) return res;
    return chiffrer_fichier_blocs_gmp(argv[1], argv[2], &e.pub);
}

int cmd_uncrypt(table_clefs_t *table, int argc, char **argv) {
    entree_clef_t e;
    int res = clef_crypt(table, argc, argv, &e);
    if (res != CMD_SUCCES) return res;
    if (!e.a_priv) return CMD_ERREUR_PAS_DE_PRIVEE;
    return dechiffrer_fichier_blocs_gmp(argv[1], argv[2], &e.priv);
}

int cmd_savepub(table_clefs_t *table, int argc, char **argv) {
    entree_clef_t e;
    if (table == NULL || argv == NULL) return RSA_ERREUR_PARAMETRE;
    if (argc != 3 || argv[1] == NULL || argv[2] == NULL) return CMD_ERREUR_SYNTAXE;
    int res = chercher_clef(table, argv[1], &e);
    if (res != CMD_SUCCES) return res;
    /* export_pubkey_b64 renvoie 0 ou -1 : -1 vient d'une ecriture impossible (parametres deja verifies) */
    return export_pubkey_b64(argv[2], &e.pub, nom_type(e.type)) == 0 ? CMD_SUCCES : RSA_ERREUR_FICHIER;
}
