#ifndef CONTACTS_INTERNE_ADAM_H
#define CONTACTS_INTERNE_ADAM_H
#include "contacts.h"

typedef struct contact_cle_noeud_s {
    contact_cle_t infos;
    struct contact_cle_noeud_s *suivant;
} contact_cle_noeud_t;

struct contact_s {
    contact_infos_t infos;
    contact_cle_noeud_t *cles;
    struct contact_s *suivant;
};

int contacts_identifiant_valide(const char *id);
int contacts_chercher_id_ou_nom(annuaire_contacts_t *annuaire, const char *texte,
                               contact_t **contact);

#endif
