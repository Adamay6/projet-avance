/// \file commandes_nada.c
/// \author Nada ARABA
#include "commandes_nada.h"
#include "../phase1_nada/phase1/phase1.h"   /* print_key_hex */
#include "../phase1_adam/chiffrement.h"     /* codes RSA_* */

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
