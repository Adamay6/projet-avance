#include "contacts_interpreteur.h"
#include "../phase1_adam/chiffrement.h"
#include <string.h>

int contacts_resoudre_entree(const annuaire_contacts_t *annuaire, const char *reference,
                            entree_clef_t *entree)
{
    contact_cle_t cle;
    if (entree == NULL) return RSA_ERREUR_PARAMETRE;

    int resultat = contacts_lire_cle(annuaire, reference, &cle);
    if (resultat == CONTACTS_ERREUR_INTROUVABLE) return CMD_ERREUR_ID_INCONNU;
    if (resultat == CONTACTS_ERREUR_IDENTIFIANT) return CMD_ERREUR_SYNTAXE;
    if (resultat != CONTACTS_SUCCES) return RSA_ERREUR_PARAMETRE;

    entree_clef_t publique = {
        .type = cle.type == CONTACT_SIGN ? CLEF_SIGN : CLEF_CRYPT,
        .pub = cle.publique,
        .a_priv = false
    };
    strcpy(publique.id, cle.identifiant);
    *entree = publique;
    return CMD_SUCCES;
}
