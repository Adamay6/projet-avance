#ifndef CONTACTS_INTERPRETEUR_ADAM_H
#define CONTACTS_INTERPRETEUR_ADAM_H

#include "contacts.h"
#include "../phase3_nada/interpreteur.h"

/* Copie la clef publique de contact/clef au format de l'interpréteur.
 * id contient l'identifiant de la clef. La sortie reste intacte en cas d'erreur.
 */
int contacts_resoudre_entree(const annuaire_contacts_t *annuaire, const char *reference,
                            entree_clef_t *entree);

#endif
