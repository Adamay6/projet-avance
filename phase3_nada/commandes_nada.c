/// \file commandes_nada.c
/// \author Nada ARABA
#include "commandes_nada.h"
#include "../phase1_nada/phase1/phase1.h"   /* print_key_hex */
#include "../phase1_adam/chiffrement.h"     /* codes RSA_* */
#include "../phase2_adam/chiffrement_blocs.h"   /* fichiers par blocs (Adam, phase 2) */

static const char *nom_type(type_clef_t type) {
    return type == CLEF_SIGN ? "sign" : "crypt";
}

int cmd_show(table_clefs_t *table, int argc, char **argv) {
    if (table == NULL || argv == NULL) return RSA_ERREUR_PARAMETRE;
    if (argc != 2 || argv[1] == NULL) return CMD_ERREUR_SYNTAXE;
    const entree_clef_t *e = table_chercher(table, argv[1]);
    if (e == NULL) return CMD_ERREUR_ID_INCONNU;

    printf("%s (%s)\n", e->id, nom_type(e->type));
    print_key_hex("clef publique", &e->pub);
    if (e->a_priv) print_key_hex("clef privee  ", &e->priv);
    else printf("clef privee   : absente\n");
    return CMD_SUCCES;
}

/* Controles communs a crypt et uncrypt : <id> <entree> <sortie>, clef de type crypt. */
static int clef_crypt(table_clefs_t *table, int argc, char **argv, const entree_clef_t **e) {
    if (table == NULL || argv == NULL) return RSA_ERREUR_PARAMETRE;
    if (argc != 4 || argv[1] == NULL || argv[2] == NULL || argv[3] == NULL) return CMD_ERREUR_SYNTAXE;
    *e = table_chercher(table, argv[1]);
    if (*e == NULL) return CMD_ERREUR_ID_INCONNU;
    if ((*e)->type != CLEF_CRYPT) return CMD_ERREUR_TYPE;
    return CMD_SUCCES;
}

int cmd_crypt(table_clefs_t *table, int argc, char **argv) {
    const entree_clef_t *e = NULL;
    int res = clef_crypt(table, argc, argv, &e);
    if (res != CMD_SUCCES) return res;
    return chiffrer_fichier_blocs_gmp(argv[2], argv[3], &e->pub);
}

int cmd_uncrypt(table_clefs_t *table, int argc, char **argv) {
    const entree_clef_t *e = NULL;
    int res = clef_crypt(table, argc, argv, &e);
    if (res != CMD_SUCCES) return res;
    if (!e->a_priv) return CMD_ERREUR_PAS_DE_PRIVEE;
    return dechiffrer_fichier_blocs_gmp(argv[2], argv[3], &e->priv);
}
