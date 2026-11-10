#include "contacts_interne.h"

int contacts_identifiant_valide(const char *id)
{
    size_t taille;
    if (id == NULL) return 0;
    taille = strlen(id);
    if (taille == 0 || taille > NAME_MAX_SIZE) return 0;
    for (size_t i = 0; i < taille; ++i) {
        unsigned char c = (unsigned char)id[i];
        if (c <= 32 || c == 127 || c == '/') return 0;
    }
    return 1;
}

static int texte_valide(const char *texte, size_t capacite)
{
    return texte != NULL && strlen(texte) < capacite &&
           strchr(texte, '\n') == NULL && strchr(texte, '\r') == NULL;
}

static contact_t *chercher(const annuaire_contacts_t *annuaire, const char *id)
{
    if (annuaire == NULL || id == NULL) return NULL;
    for (contact_t *p = annuaire->premier; p != NULL; p = p->suivant)
        if (strcmp(p->infos.identifiant, id) == 0) return p;
    return NULL;
}

int contacts_chercher_id_ou_nom(annuaire_contacts_t *annuaire, const char *texte,
                               contact_t **contact)
{
    contact_t *trouve = chercher(annuaire, texte);
    if (trouve != NULL) { *contact = trouve; return CONTACTS_SUCCES; }
    for (contact_t *p = annuaire->premier; p != NULL; p = p->suivant) {
        if (strcmp(p->infos.nom, texte) == 0) {
            if (trouve != NULL) return CONTACTS_ERREUR_AMBIGU;
            trouve = p;
        }
    }
    if (trouve == NULL) return CONTACTS_ERREUR_INTROUVABLE;
    *contact = trouve;
    return CONTACTS_SUCCES;
}

static int decouper_reference(const char *reference, char contact[NAME_MAX_SIZE + 1],
                              char cle[NAME_MAX_SIZE + 1])
{
    const char *barre;
    size_t taille;
    if (reference == NULL) return CONTACTS_ERREUR_PARAMETRE;
    barre = strchr(reference, '/');
    if (barre == NULL) return CONTACTS_ERREUR_IDENTIFIANT;
    taille = (size_t)(barre - reference);
    if (taille == 0 || taille > NAME_MAX_SIZE ||
        strlen(barre + 1) > NAME_MAX_SIZE) return CONTACTS_ERREUR_IDENTIFIANT;
    memcpy(contact, reference, taille);
    contact[taille] = '\0';
    strcpy(cle, barre + 1);
    return contacts_identifiant_valide(contact) && contacts_identifiant_valide(cle) ?
           CONTACTS_SUCCES : CONTACTS_ERREUR_IDENTIFIANT;
}

static void liberer_cles(contact_cle_noeud_t *cle)
{
    while (cle != NULL) {
        contact_cle_noeud_t *suivante = cle->suivant;
        free(cle);
        cle = suivante;
    }
}

void contacts_initialiser(annuaire_contacts_t *annuaire)
{
    if (annuaire != NULL) annuaire->premier = NULL;
}

void contacts_liberer(annuaire_contacts_t *annuaire)
{
    if (annuaire == NULL) return;
    while (annuaire->premier != NULL) {
        contact_t *contact = annuaire->premier;
        annuaire->premier = contact->suivant;
        liberer_cles(contact->cles);
        free(contact);
    }
}

size_t contacts_nombre(const annuaire_contacts_t *annuaire)
{
    size_t nombre = 0;
    if (annuaire != NULL)
        for (contact_t *p = annuaire->premier; p != NULL; p = p->suivant) ++nombre;
    return nombre;
}

int contacts_ajouter(annuaire_contacts_t *annuaire, const char *id,
                    const char *nom, const char *prenom, const char *commentaire)
{
    contact_t *contact, **fin;
    if (annuaire == NULL || id == NULL) return CONTACTS_ERREUR_PARAMETRE;
    if (!contacts_identifiant_valide(id)) return CONTACTS_ERREUR_IDENTIFIANT;
    if (!texte_valide(nom, CONTACT_TEXTE_TAILLE) ||
        !texte_valide(prenom, CONTACT_TEXTE_TAILLE) ||
        !texte_valide(commentaire, CONTACT_COMMENTAIRE_TAILLE))
        return CONTACTS_ERREUR_PARAMETRE;
    if (chercher(annuaire, id) != NULL) return CONTACTS_ERREUR_DOUBLON;
    contact = calloc(1, sizeof *contact);
    if (contact == NULL) return CONTACTS_ERREUR_MEMOIRE;
    strcpy(contact->infos.identifiant, id);
    strcpy(contact->infos.nom, nom);
    strcpy(contact->infos.prenom, prenom);
    strcpy(contact->infos.commentaire, commentaire);
    for (fin = &annuaire->premier; *fin != NULL; fin = &(*fin)->suivant) {}
    *fin = contact;
    return CONTACTS_SUCCES;
}

int contacts_modifier(annuaire_contacts_t *annuaire, const char *id,
                     const char *nom, const char *prenom, const char *commentaire)
{
    contact_t *contact;
    contact_infos_t nouvelle = {0};
    if (annuaire == NULL || id == NULL) return CONTACTS_ERREUR_PARAMETRE;
    if (!texte_valide(nom, sizeof nouvelle.nom) ||
        !texte_valide(prenom, sizeof nouvelle.prenom) ||
        !texte_valide(commentaire, sizeof nouvelle.commentaire))
        return CONTACTS_ERREUR_PARAMETRE;
    contact = chercher(annuaire, id);
    if (contact == NULL) return CONTACTS_ERREUR_INTROUVABLE;
    strcpy(nouvelle.identifiant, contact->infos.identifiant);
    strcpy(nouvelle.nom, nom);
    strcpy(nouvelle.prenom, prenom);
    strcpy(nouvelle.commentaire, commentaire);
    contact->infos = nouvelle;
    return CONTACTS_SUCCES;
}

int contacts_supprimer(annuaire_contacts_t *annuaire, const char *id)
{
    contact_t **position, *contact;
    if (annuaire == NULL || id == NULL) return CONTACTS_ERREUR_PARAMETRE;
    for (position = &annuaire->premier; *position != NULL; position = &(*position)->suivant) {
        if (strcmp((*position)->infos.identifiant, id) == 0) {
            contact = *position;
            *position = contact->suivant;
            liberer_cles(contact->cles);
            free(contact);
            return CONTACTS_SUCCES;
        }
    }
    return CONTACTS_ERREUR_INTROUVABLE;
}

const contact_infos_t *contacts_trouver(const annuaire_contacts_t *annuaire,
                                       const char *id)
{
    contact_t *contact = chercher(annuaire, id);
    return contact == NULL ? NULL : &contact->infos;
}

int contacts_ajouter_cle(annuaire_contacts_t *annuaire, const char *contact_ou_nom,
                        const char *id_cle, contact_type_cle_t type,
                        const rsaKey_t *publique)
{
    contact_t *contact = NULL;
    contact_cle_noeud_t *cle, **fin;
    int resultat;
    if (annuaire == NULL || contact_ou_nom == NULL || publique == NULL)
        return CONTACTS_ERREUR_PARAMETRE;
    if (!contacts_identifiant_valide(id_cle)) return CONTACTS_ERREUR_IDENTIFIANT;
    if (type != CONTACT_CRYPT && type != CONTACT_SIGN) return CONTACTS_ERREUR_TYPE;
    if (publique->N <= 1 || publique->E == 0 || publique->E >= publique->N)
        return CONTACTS_ERREUR_PARAMETRE;
    resultat = contacts_chercher_id_ou_nom(annuaire, contact_ou_nom, &contact);
    if (resultat != 0) return resultat;
    for (fin = &contact->cles; *fin != NULL; fin = &(*fin)->suivant)
        if (strcmp((*fin)->infos.identifiant, id_cle) == 0) return CONTACTS_ERREUR_DOUBLON;
    cle = calloc(1, sizeof *cle);
    if (cle == NULL) return CONTACTS_ERREUR_MEMOIRE;
    strcpy(cle->infos.identifiant, id_cle);
    cle->infos.type = type;
    cle->infos.publique = *publique;
    *fin = cle;
    return CONTACTS_SUCCES;
}

int contacts_supprimer_cle(annuaire_contacts_t *annuaire, const char *reference)
{
    char id[NAME_MAX_SIZE + 1], id_cle[NAME_MAX_SIZE + 1];
    contact_t *contact;
    contact_cle_noeud_t **position, *cle;
    int resultat;
    if (annuaire == NULL) return CONTACTS_ERREUR_PARAMETRE;
    resultat = decouper_reference(reference, id, id_cle);
    if (resultat != 0) return resultat;
    contact = chercher(annuaire, id);
    if (contact == NULL) return CONTACTS_ERREUR_INTROUVABLE;
    for (position = &contact->cles; *position != NULL; position = &(*position)->suivant) {
        if (strcmp((*position)->infos.identifiant, id_cle) == 0) {
            cle = *position; *position = cle->suivant; free(cle);
            return CONTACTS_SUCCES;
        }
    }
    return CONTACTS_ERREUR_INTROUVABLE;
}

int contacts_lire_cle(const annuaire_contacts_t *annuaire, const char *reference,
                     contact_cle_t *cle)
{
    char id[NAME_MAX_SIZE + 1], id_cle[NAME_MAX_SIZE + 1];
    contact_t *contact;
    int resultat;
    if (annuaire == NULL || cle == NULL) return CONTACTS_ERREUR_PARAMETRE;
    resultat = decouper_reference(reference, id, id_cle);
    if (resultat != 0) return resultat;
    contact = chercher(annuaire, id);
    if (contact == NULL) return CONTACTS_ERREUR_INTROUVABLE;
    for (contact_cle_noeud_t *p = contact->cles; p != NULL; p = p->suivant) {
        if (strcmp(p->infos.identifiant, id_cle) == 0) {
            *cle = p->infos;
            return CONTACTS_SUCCES;
        }
    }
    return CONTACTS_ERREUR_INTROUVABLE;
}

int contacts_resoudre(const annuaire_contacts_t *annuaire, const char *reference,
                     contact_type_cle_t type_attendu, rsaKey_t *publique)
{
    contact_cle_t cle;
    int resultat;
    if (publique == NULL) return CONTACTS_ERREUR_PARAMETRE;
    if (type_attendu != CONTACT_CRYPT && type_attendu != CONTACT_SIGN)
        return CONTACTS_ERREUR_TYPE;
    resultat = contacts_lire_cle(annuaire, reference, &cle);
    if (resultat != 0) return resultat;
    if (cle.type != type_attendu) return CONTACTS_ERREUR_TYPE;
    *publique = cle.publique;
    return CONTACTS_SUCCES;
}

int contacts_afficher(const annuaire_contacts_t *annuaire, const char *filtre,
                      const char *nom, FILE *sortie)
{
    int trouve = 0;
    int filtre_id;
    if (annuaire == NULL || sortie == NULL) return CONTACTS_ERREUR_PARAMETRE;
    filtre_id = filtre != NULL && chercher(annuaire, filtre) != NULL;
    for (contact_t *p = annuaire->premier; p != NULL; p = p->suivant) {
        if (filtre != NULL &&
            strcmp(filtre_id || nom != NULL ? p->infos.identifiant : p->infos.nom, filtre) != 0)
            continue;
        if (nom != NULL && strcmp(p->infos.nom, nom) != 0) continue;
        trouve = 1;
        if (fprintf(sortie, "%s : %s %s\n  %s\n", p->infos.identifiant,
                    p->infos.prenom, p->infos.nom, p->infos.commentaire) < 0)
            return CONTACTS_ERREUR_FICHIER;
        for (contact_cle_noeud_t *cle = p->cles; cle != NULL; cle = cle->suivant) {
            if (fprintf(sortie, "  %s/%s (%s)\n", p->infos.identifiant,
                        cle->infos.identifiant,
                        cle->infos.type == CONTACT_CRYPT ? "crypt" : "sign") < 0)
                return CONTACTS_ERREUR_FICHIER;
        }
    }
    if (ferror(sortie)) return CONTACTS_ERREUR_FICHIER;
    return !trouve && (filtre != NULL || nom != NULL) ?
           CONTACTS_ERREUR_INTROUVABLE : CONTACTS_SUCCES;
}

const char *contacts_message_erreur(int resultat)
{
    switch (resultat) {
    case CONTACTS_SUCCES: return "Succès.";
    case CONTACTS_COMMANDE_INCONNUE: return "Commande non gérée par le module de contacts.";
    case CONTACTS_ERREUR_PARAMETRE: return "Paramètre invalide.";
    case CONTACTS_ERREUR_IDENTIFIANT: return "Identifiant ou référence contact/clef invalide.";
    case CONTACTS_ERREUR_INTROUVABLE: return "Contact ou clef introuvable.";
    case CONTACTS_ERREUR_DOUBLON: return "Cet identifiant existe déjà.";
    case CONTACTS_ERREUR_MEMOIRE: return "Mémoire insuffisante.";
    case CONTACTS_ERREUR_FICHIER: return "Erreur de lecture ou d'écriture.";
    case CONTACTS_ERREUR_FORMAT: return "Section de contacts invalide.";
    case CONTACTS_ERREUR_TYPE: return "Type de clef incompatible.";
    case CONTACTS_ERREUR_AMBIGU: return "Plusieurs contacts portent ce nom ; utiliser l'identifiant.";
    case CONTACTS_ERREUR_SAISIE: return "Saisie absente, trop longue ou invalide.";
    case CONTACTS_ERREUR_PRIVEE: return "Les contacts contiennent seulement des clefs publiques.";
    default: return "Erreur de contacts inconnue.";
    }
}
